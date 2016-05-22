#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libudev.h>

#include "usb.h"

int usb_print(char *usb_path)
{
	printf("PRINT %s\n", usb_path);

	return 0;
}

int usb_print_multiple(char *usb_paths[], int num_usb_paths)
{
	int ret_code = 0;

	for (int i = 0; i < num_usb_paths; i++)
		if ((ret_code = usb_print(usb_paths[i])) != 0)
			break;

	return ret_code;
}

int usb_print_all()
{
	printf("PRINT ALL\n");

	struct usb_device_list *list = usb_device_list_get();

	while (list) {
		printf("%p\n", list->device);

		list = list->next;
	}

	return 0;
}

int usb_mount(char *usb_path)
{
	printf("MOUNT %s\n", usb_path);

	return 0;
}

int usb_mount_multiple(char *usb_paths[], int num_usb_paths)
{
	int ret_code = 0;

	for (int i = 0; i < num_usb_paths; i++)
		if ((ret_code = usb_mount(usb_paths[i])) != 0)
			break;

	return ret_code;
}

int usb_mount_all()
{
	printf("MOUNT ALL\n");

	return 0;
}

int usb_umount(char *usb_path)
{
	printf("UMOUNT %s\n", usb_path);

	return 0;
}

int usb_umount_multiple(char *usb_paths[], int num_usb_paths)
{
	int ret_code = 0;

	for (int i = 0; i < num_usb_paths; i++)
		if ((ret_code = usb_umount(usb_paths[i])) != 0)
			break;

	return ret_code;
}

int usb_umount_all()
{
	printf("UMOUNT ALL\n");

	return 0;
}

void usb_device_list_add(struct usb_device_list *list, struct udev_device *device)
{
	if (!list->device) {
		list->device = device;
	} else {
		while (list->next) {
			list = list->next;
		}

		list->next = usb_device_list_new();
		list->next->device = device;
	}
}

struct usb_device_list *usb_device_list_new()
{
	struct usb_device_list *list = malloc(sizeof(struct usb_device_list));

	memset(list, 0, sizeof(struct usb_device_list));

	return list;
}

struct usb_device_list *usb_device_list_get()
{
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
	struct usb_device_list *list = usb_device_list_new();

	while (device_entry) {
		const char *device_name = udev_list_entry_get_name(device_entry);
		struct udev_device *scsi_device = udev_device_new_from_syspath(udev, device_name);
		struct udev_device *usb_device = udev_device_get_parent_with_subsystem_devtype(scsi_device, "usb", "usb_device");
		struct udev_device *hub_device = udev_device_get_parent_with_subsystem_devtype(usb_device, "usb", "usb_device");
		struct udev_device *block_device = usb_device_get_child(udev, scsi_device, "block");
		struct udev_device *scsi_disk_device = usb_device_get_child(udev, scsi_device, "scsi_disk");

		if (usb_device && block_device && scsi_disk_device) {
			//printf("%s\n", udev_device_get_syspath(usb_device));
			//printf("\t%s\n", udev_device_get_syspath(scsi_device));
			//printf("\t\t%s\n", udev_device_get_syspath(block_device));
			//printf("\t\t%s\n", udev_device_get_syspath(scsi_disk_device));

			/*printf("Device node: %s\n", udev_device_get_devnode(block_device));
			printf("\tManufacturer: %s\n", udev_device_get_sysattr_value(usb_device, "manufacturer"));
			printf("\tProduct: %s\n", udev_device_get_sysattr_value(usb_device, "product"));
			printf("\tSerial: %s\n", udev_device_get_sysattr_value(usb_device, "serial"));
			printf("\tUSB bus number: %s\n", udev_device_get_sysattr_value(usb_device, "busnum"));
			printf("\tUSB device path: %s\n", udev_device_get_sysattr_value(usb_device, "devpath"));
			printf("\tUSB version: %s\n", udev_device_get_sysattr_value(usb_device, "version"));
			printf("\tUSB speed: %sMbps\n", udev_device_get_sysattr_value(usb_device, "speed"));*/
			//printf("\t%s\n", udev_device_get_sysattr_value(usb_device, "maxchild"));

			//struct udev_list_entry *entry = udev_device_get_properties_list_entry(hub_device);

			//while (entry) {
			//	printf("\t%s %s\n", udev_list_entry_get_name(entry), udev_list_entry_get_value(entry));
			//	entry = udev_list_entry_get_next(entry);
			//}

			//get_child_device2(udev, block_device, "block");

			usb_device_list_add(list, usb_device);
		}

		//printf("\t%s\n", udev_device_get_sysattr_value(usb_device, "maxchild"));

		udev_device_unref(block_device);
		udev_device_unref(scsi_disk_device);
		udev_device_unref(scsi_device);

		device_entry = udev_list_entry_get_next(device_entry);
	}

	udev_enumerate_unref(enumerate);

	udev_unref(udev);

	return list;
}

struct udev_device *usb_device_get_child(struct udev *udev, struct udev_device *parent_device, const char *subsystem)
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

/*void usb_device_get_child2(struct udev *udev, struct udev_device *parent_device, const char *subsystem)
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
}*/
