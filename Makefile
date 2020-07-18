GCC_FLAGS=-std=c11 -Wall -Wextra 

all: main_driver

main_driver:
	gcc $(GCC_FLAGS) -o logitech-g110-color-driver logitech-g110-color-driver.c $(shell pkg-config --libs libusb-1.0)
