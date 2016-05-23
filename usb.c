#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libudev.h>

#include "usb.h"

char **usb_device_to_fields(struct usb_device *device) {
	char **fields = calloc(sizeof(char *), 8);

	fields[0] = device->node;
	fields[1] = device->manufacturer;
	fields[2] = device->product;
	fields[3] = device->serial;
	//fields[4] = device->bus;
	fields[5] = device->dev_path;
	fields[6] = device->version;
	fields[7] = device->speed;

	return fields;
}

char *usb_device_to_str(struct usb_device *device)
{
	size_t str_len = strlen(device->node) + 1 +
	                 strlen(device->manufacturer) + 1 +
	                 strlen(device->product) + 1 +
	                 strlen(device->serial) + 1 + 2 + 1 +
	                 strlen(device->dev_path) + 1 +
	                 strlen(device->version) + 1 +
	                 strlen(device->speed) + 1;

	char *str = malloc(str_len + 1);

	sprintf(str, "%s\t%s\t%s\t%s\t%d\t%s\t%s\t%s",
		    device->node,
	        device->manufacturer,
	        device->product,
	        device->serial,
	        device->bus,
	        device->dev_path,
	        device->version,
	        device->speed);

	return str;
}

int usb_print(char *usb_path)
{
	int ret_code = 0;
	struct usb_device_list *list = usb_device_list_get();

	while (list && list->device) {
		if (strcmp(list->device->dev_path, usb_path) == 0) {
			printf("%s\n", usb_device_to_str(list->device));
		}

		list = list->next;
	}

	return ret_code;
}

int usb_print_multiple(char *usb_paths[], int num_usb_paths)
{
	int ret_code = 0;
	struct usb_device_list *head = usb_device_list_get();
	struct usb_device_list *list = NULL;

	for (int i = 0; i < num_usb_paths; i++) {
		list = head;

		while (list && list->device) {
			if (strcmp(list->device->dev_path, usb_paths[i]) == 0) {
				printf("%s\n", usb_device_to_str(list->device));

				break;
			}

			list = list->next;
		}
	}

	return ret_code;
}

int usb_print_all()
{
	struct usb_device_list *list = usb_device_list_get();

	printf("%s\n", usb_device_list_table_str(list));

	/*while (list && list->device) {
		printf("%s\n", usb_device_to_str(list->device));

		list = list->next;
	}*/

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

void usb_device_list_add(struct usb_device_list *list, struct usb_device *device)
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

static size_t usb_device_list_table_max_width_node(struct usb_device_list *list)
{
	size_t max = 0;

	while (list && list->device) {
		size_t size = strlen(list->device->node);

		if (size > max)
			max = size;

		list = list->next;
	}

	return max;
}

static size_t usb_device_list_table_max_width_manufacturer(struct usb_device_list *list)
{
	size_t max = 0;

	while (list && list->device) {
		size_t size = strlen(list->device->manufacturer);

		if (size > max)
			max = size;

		list = list->next;
	}

	return max;
}

static size_t usb_device_list_table_max_width_product(struct usb_device_list *list)
{
	size_t max = 0;

	while (list && list->device) {
		size_t size = strlen(list->device->product);

		if (size > max)
			max = size;

		list = list->next;
	}

	return max;
}

char *usb_device_list_table_str(struct usb_device_list *list)
{
	size_t width_node = usb_device_list_table_max_width_node(list);
	size_t width_manufacturer = usb_device_list_table_max_width_manufacturer(list);
	size_t width_product = usb_device_list_table_max_width_product(list);
	char *table_line_str;
	char *table_fmt_str;

	asprintf(&table_fmt_str,
	         "%%-%lus\t%%-%lus\t%%-%lus",
	         width_node,
	         width_manufacturer,
	         width_product);
	asprintf(&table_line_str, table_fmt_str, "", "", "");

	size_t list_size = usb_device_list_size(list);
	size_t table_str_size = strlen(table_line_str) * list_size + list_size;

	char *table_str = malloc(table_str_size);
	table_str[0] = '\0';

	while (list && list->device) {
		sprintf(table_str + strlen(table_str),
		        table_fmt_str,
		        list->device->node,
		        list->device->manufacturer,
		        list->device->product);

		if (list->next)
			sprintf(table_str + strlen(table_str), "\n");

		list = list->next;
	}

	return table_str;
}

size_t usb_device_list_size(struct usb_device_list *list)
{
	size_t size = 0;

	while (list && list->device) {
		size++;
		list = list->next;
	}

	return size;
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
		struct udev_device *block_device = usb_udev_device_get_child(udev, scsi_device, "block");
		struct udev_device *scsi_disk_device = usb_udev_device_get_child(udev, scsi_device, "scsi_disk");

		if (usb_device && block_device && scsi_disk_device) {
			struct usb_device *device = malloc(sizeof(struct usb_device));

			device->node = strdup((char *)udev_device_get_devnode(block_device));
			device->manufacturer = strdup((char *)udev_device_get_sysattr_value(usb_device, "manufacturer"));
			device->product = strdup((char *)udev_device_get_sysattr_value(usb_device, "product"));
			device->serial = strdup((char *)udev_device_get_sysattr_value(usb_device, "serial"));
			device->dev_path = strdup((char *)udev_device_get_sysattr_value(usb_device, "devpath"));
			device->sys_path = strdup((char *)udev_device_get_syspath(usb_device));
			device->speed = strdup((char *)udev_device_get_sysattr_value(usb_device, "speed"));
			device->version = strdup((char *)udev_device_get_sysattr_value(usb_device, "version"));
			device->max_children = atoi(udev_device_get_sysattr_value(usb_device, "maxchild"));
			device->bus = atoi(udev_device_get_sysattr_value(usb_device, "busnum"));

			//printf("%s\n", udev_device_get_syspath(usb_device));
			//printf("\t%s\n", udev_device_get_syspath(scsi_device));
			//printf("\t\t%s\n", udev_device_get_syspath(block_device));
			//printf("\t\t%s\n", udev_device_get_syspath(scsi_disk_device));

			//struct udev_list_entry *entry = udev_device_get_properties_list_entry(hub_device);

			//while (entry) {
			//	printf("\t%s %s\n", udev_list_entry_get_name(entry), udev_list_entry_get_value(entry));
			//	entry = udev_list_entry_get_next(entry);
			//}

			//get_child_device2(udev, block_device, "block");

			usb_device_list_add(list, device);
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

struct udev_device *usb_udev_device_get_child(struct udev *udev, struct udev_device *parent_device, const char *subsystem)
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
