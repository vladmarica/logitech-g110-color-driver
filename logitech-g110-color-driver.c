/*
 * Linux userspace USB driver for changing the key backlight color on Logitech G110 keyboards
 * 
 * Forked from https://github.com/tomvanbraeckel/logitech-keyboard-change-color
 * 
 * Copyleft 2020, Vlad Marica
 * Credits: Vlad Marica, Tom Van Braeckel, Rich Budman
 * Licensed under GPLv2
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libusb-1.0/libusb.h>

#define LOGITECH_VENDOR_ID 0x046D
#define G110_PRODUCT_ID 0xC22B

libusb_device *get_keyboard_device() {
  libusb_device **devices;
  int count = libusb_get_device_list(NULL, &devices);
  if (count <= 0) {
    return NULL;
  }

  for (int i = 0; i < count; i++) {
    libusb_device *dev = devices[i];
    struct libusb_device_descriptor descriptor;
    libusb_get_device_descriptor(dev, &descriptor);
    if (descriptor.idVendor == LOGITECH_VENDOR_ID && descriptor.idProduct == G110_PRODUCT_ID) {
      return dev;
    }
  }

  return NULL;
}

/*
 * Attempts to parse the given string as a color value. If the string
 * cannot be converted to a color code, -1 is returned.
 */
short get_color_code(char *color_str) {
  if (strcmp(color_str, "red") == 0) {
    return 0;
  }
  if (strcmp(color_str, "purple") == 0) {
    return 127;
  }
  if (strcmp(color_str, "blue") == 0) {
    return 255;
  }

  return -1;
}

void print_usage() {
  printf("Usage: logitech-g110-color-driver <color>\n");
  printf("\t<color> is one of the following: red, blue, purple, or a number between 0 and 255\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    print_usage();
    return EXIT_FAILURE;
  }

  long color = get_color_code(argv[1]);
  if (color == -1) {
    char *end_ptr;
    color = strtol(argv[1], &end_ptr, 10);
    if (*end_ptr != '\0') {
      printf("Invalid color code: %s\n\n", argv[1]);
      print_usage();
      return EXIT_FAILURE;
    }
  }
  
  if (color < 0 || color > 255) {
    printf("Color must be between 0 and 255\n");
    return EXIT_FAILURE;
  }

  // Initialize the libusb library
  int result = libusb_init(NULL);
  if (result < 0) {
    printf("Failed to initialize libusb: %s\n", libusb_error_name(result));
    return EXIT_FAILURE;
  }

  // Search a libusb_device that is a G110 keyboard
  libusb_device *keyboard_device = get_keyboard_device();
  if (keyboard_device == NULL) {
    printf("Could not find a Logitech G110 keyboard connected\n");
    return EXIT_FAILURE;
  }

  // Attempt to open a handle to the keyboard
  libusb_device_handle *keyboard_handle;
  result = libusb_open(keyboard_device, &keyboard_handle);
  if (result != 0) {
    printf("Could not open keyboard device: %s\n", libusb_error_name(result));
    if (result == LIBUSB_ERROR_ACCESS) {
      printf("Permission denied\n");
    }
    return EXIT_FAILURE;
  }

  // If a kernel driver is attached to the keyboard already, we need to detach it
  result = libusb_kernel_driver_active(keyboard_handle, 0);
  if (result == 1) {
      printf("Logitech G110 kernel driver active\n");
      result = libusb_detach_kernel_driver(keyboard_handle, 0);
      if (result != 0) {
        printf("Failed to detach kernel driver\n");
        return EXIT_FAILURE;
      }
  } else if (result != 0) {
    printf("Could not check if kernel driver is attached: %s\n", libusb_error_name(result));
  }

  result = libusb_set_configuration(keyboard_handle, 1);
  if (result != 0) {
    printf("Could not set configuration: %s\n", libusb_error_name(result));
    return EXIT_FAILURE;
  }

  //  Color is a scale from 0 to 255, where 0 is the most blue and 255 is the most red.
  unsigned char buffer[] = {0x07, (unsigned char) color, 0x00, 0x00, 0xff};
  result = libusb_control_transfer(keyboard_handle, 0x21, 0x00000009, 0x00000307, 0x00000000, buffer, 0x00000005, 5000);
  if (result != 5) {
    printf("Failed to set color: %s\n", libusb_error_name(result));
    return EXIT_FAILURE;
  }

  // TODO: should we reattach to the kernel drive if we disconnected from one?

  libusb_close(keyboard_handle);
  libusb_exit(NULL);

  return EXIT_SUCCESS;
}
