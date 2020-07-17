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

void to_lowercase(char *str) {
  for (char *p = str; *p != '\0'; p++) {
    *p = tolower(*p);
  }
}

int get_color_code(char *color_str) {
  if (strcmp(color_str, "red") == 0) {
    return 0;
  }
  if (strcmp(color_str, "purple") == 0) {
    return 127;
  }
  if (strcmp(color_str, "blue") == 0) {
    return 255;
  }

  return atoi(color_str); // TODO: use 'strol' instead
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: logitech-g110-color-driver <color>\n");
    return EXIT_FAILURE;
  }

  int color = get_color_code(argv[1]);
  if (color < 0 || color > 255) {
    printf("Color must be between 0 and 255\n");
    return EXIT_FAILURE;
  }

  int result = libusb_init(NULL);
  if (result < 0) {
    printf("Failed to initialize libusb: %s\n", libusb_error_name(result));
    return EXIT_FAILURE;
  }

  printf("libusb initialized!\n");

  libusb_device *keyboard_device = get_keyboard_device();
  if (keyboard_device == NULL) {
    printf("Could not find a Logitech G110 keyboard\n");
    return EXIT_FAILURE;
  }

  printf("Logitech G110 keyboard found!\n");

  libusb_device_handle *keyboard_handle;
  result = libusb_open(keyboard_device, &keyboard_handle);
  if (result != 0) {
    printf("Could not open keyboard device: %s\n", libusb_error_name(result));
    if (result == LIBUSB_ERROR_ACCESS) {
      printf("Permission denied opening device\n");
    }
    return EXIT_FAILURE;
  }

  printf("Logitech G110 keyboard opened!\n");

  result = libusb_kernel_driver_active(keyboard_handle, 0);

  if (result == 1) {
      printf("Logitech G110 kernel driver active\n");
      result = libusb_detach_kernel_driver(keyboard_handle, 0);
      if (result != 0) {
        printf("Failed to detach kernel driver\n");
        return EXIT_FAILURE;
      }
  } else if (result == 0) {
      printf("Logitech G110 no kernel driver active\n");
  } else {
    printf("Could not check if kernel driver is attached: %s\n", libusb_error_name(result));
  }

  result = libusb_set_configuration(keyboard_handle, 1);
  if (result != 0) {
    printf("Could not set configuration: %s\n", libusb_error_name(result));
    return EXIT_FAILURE;
  }

  printf("Logitech G110 configuration set!\n");

  /*
   * Color is a scale from 0 to 255, where 0 is the most blue and 255 is the most red.
   */
  unsigned char buffer[] = {0x07, (char) color, 0x00, 0x00, 0xff};
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
