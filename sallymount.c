#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libudev.h>
#include <libusb-1.0/libusb.h>
#include <argp.h>

#include "cli.h"
#include "usb.h"

static struct udev_device *get_child_device(struct udev *udev, struct udev_device *parent_device, const char *subsystem)
{
	struct udev_enumerate *enumerate = udev_enumerate_new(udev);

	udev_enumerate_add_match_parent(enumerate, parent_device);
	udev_enumerate_add_match_subsystem(enumerate, subsystem);
	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry *device_entry = udev_enumerate_get_list_entry(enumerate);
	const char *device_name = udev_list_entry_get_name(device_entry);
	struct udev_device *child_device = udev_device_new_from_syspath(udev, device_name);

	udev_enumerate_unref(enumerate);

	return child_device;
}

static void get_child_device2(struct udev *udev, struct udev_device *parent_device, const char *subsystem)
{
	struct udev_enumerate *enumerate = udev_enumerate_new(udev);

	udev_enumerate_add_match_parent(enumerate, parent_device);
	udev_enumerate_add_match_subsystem(enumerate, subsystem);
	udev_enumerate_scan_devices(enumerate);

	struct udev_list_entry *device_entry = udev_enumerate_get_list_entry(enumerate);

	while (device_entry) {
		const char *device_name = udev_list_entry_get_name(device_entry);
		struct udev_device *child_device = udev_device_new_from_syspath(udev, device_name);

		printf("%s\n", udev_device_get_sysattr_value(child_device, "size"));

		device_entry = udev_list_entry_get_next(device_entry);
	}

	udev_enumerate_unref(enumerate);
}

int main(int argc, char **argv)
{
	struct cli_args cli_args = {0};

	argp_parse(&cli_argp, argc, argv, ARGP_IN_ORDER, NULL, &cli_args);

	if (!cli_args.command) {
		if (cli_args.all) {
			usb_print_all();
		} else {
			usb_print_multiple(argv + 1, argc - 1);
		}
	}

	return 0;

	struct udev *udev = udev_new();

	if (!udev) {
		perror("udev_new() failed");

		exit(1);
	}

	struct udev_enumerate *enumerate = udev_enumerate_new(udev);

	udev_enumerate_add_match_subsystem(enumerate, "scsi");
	udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");

	udev_enumerate_scan_devices(enumerate);
	struct udev_list_entry *device_entry = udev_enumerate_get_list_entry(enumerate);

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

	while (device_entry) {
		const char *device_name = udev_list_entry_get_name(device_entry);
		struct udev_device *scsi_device = udev_device_new_from_syspath(udev, device_name);
		struct udev_device *usb_device = udev_device_get_parent_with_subsystem_devtype(scsi_device, "usb", "usb_device");
		struct udev_device *hub_device = udev_device_get_parent_with_subsystem_devtype(usb_device, "usb", "usb_device");
		struct udev_device *block_device = get_child_device(udev, scsi_device, "block");
		struct udev_device *scsi_disk_device = get_child_device(udev, scsi_device, "scsi_disk");

		if (usb_device && block_device && scsi_disk_device) {
			//printf("%s\n", udev_device_get_syspath(usb_device));
			//printf("\t%s\n", udev_device_get_syspath(scsi_device));
			//printf("\t\t%s\n", udev_device_get_syspath(block_device));
			//printf("\t\t%s\n", udev_device_get_syspath(scsi_disk_device));

			printf("Device node: %s\n", udev_device_get_devnode(block_device));
			printf("\tManufacturer: %s\n", udev_device_get_sysattr_value(usb_device, "manufacturer"));
			printf("\tProduct: %s\n", udev_device_get_sysattr_value(usb_device, "product"));
			printf("\tSerial: %s\n", udev_device_get_sysattr_value(usb_device, "serial"));
			printf("\tUSB bus number: %s\n", udev_device_get_sysattr_value(usb_device, "busnum"));
			printf("\tUSB device path: %s\n", udev_device_get_sysattr_value(usb_device, "devpath"));
			printf("\tUSB version: %s\n", udev_device_get_sysattr_value(usb_device, "version"));
			printf("\tUSB speed: %sMbps\n", udev_device_get_sysattr_value(usb_device, "speed"));
			//printf("\t%s\n", udev_device_get_sysattr_value(usb_device, "maxchild"));

			//struct udev_list_entry *entry = udev_device_get_properties_list_entry(hub_device);

			//while (entry) {
			//	printf("\t%s %s\n", udev_list_entry_get_name(entry), udev_list_entry_get_value(entry));
			//	entry = udev_list_entry_get_next(entry);
			//}

			//get_child_device2(udev, block_device, "block");
		}

		//printf("\t%s\n", udev_device_get_sysattr_value(usb_device, "maxchild"));

		udev_device_unref(block_device);
		udev_device_unref(scsi_disk_device);
		udev_device_unref(scsi_device);

		device_entry = udev_list_entry_get_next(device_entry);
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);
}

