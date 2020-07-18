# Logitech G110 Color Driver

This is a Linux userspace USB driver for changing the key backlight color of Logitech G110 keyboards.

## Building from Source

### Required Build Tools
- C compiler that supports the C11 standard (gcc or clang)
- make
- pkg-config

### Required Libraries
- `libusb-1.0`
  - On Debian, you need the `libusb-1.0-0` and `libusb-1.0-0-dev` packages.

### Compiling

Clone this repository and run this command in the directory:

```
make
```

This will create an executable called `logitech-g110-color-driver` that you can now use to change the backlight color on your keyboard.

## Usage

### Changing Backlight Colors
```
logitech-g110-color-driver <color>
```

Where `color` is one of the following:

- `red` (equivalent to passing `0`)
- `blue` (equivalent to passing `255`)
- `purple` (equivalent to passing `127`)
- A number between `0` to `255` inclusive

## Errors
If you get an error message saying `"Permission denied"`, run the program as root.