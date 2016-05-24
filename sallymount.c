#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <argp.h>

#include "cli.h"
#include "usb.h"

int main(int argc, char **argv)
{
	struct cli_args cli_args = {0};
	cli_args.usb_paths = calloc(sizeof(char *), argc - 1);

	argp_parse(&cli_argp, argc, argv, ARGP_IN_ORDER, NULL, &cli_args);

	if (!cli_args.command) {
		if (cli_args.all || cli_args.num_usb_paths == 0) {
			usb_print_all(cli_args.verbose, cli_args.human_readable);
		} else {
			usb_print_multiple(cli_args.usb_paths, cli_args.num_usb_paths, cli_args.verbose, cli_args.human_readable);
		}
	}

	/*libusb_init(NULL);
	libusb_context *context = NULL;
	libusb_device **list = NULL;
	ssize_t count = libusb_get_device_list(context, &list);

	for (size_t i = 0; i < count; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor descriptor;

		libusb_get_device_descriptor(device, &descriptor);
		int bus = libusb_get_bus_number(device);

		unsigned char buf[256];
		libusb_device_handle *handle = NULL;

		int r = libusb_open(device, &handle);
		//libusb_get_string_descriptor_ascii(handle, descriptor.bDeviceClass, buf, 256);

		printf("%i\n", r);

		//printf("%i\t%i\t%i\t%i\t", descriptor.bDeviceClass, descriptor.bDeviceSubClass, descriptor.bDescriptorType, bus);

		//uint8_t port_numbers[8];

		//int x = libusb_get_port_numbers(device, port_numbers, 7);

		//for (int j = 0; j < x; j++) {
		//	printf("%i ", port_numbers[j]);
		//}



		//printf("\t%i\n", libusb_get_device_address(device));
	}*/

	return 0;
}

