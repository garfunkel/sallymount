#include <stdio.h>

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
