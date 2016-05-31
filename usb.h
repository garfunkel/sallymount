#ifndef _SALLYMOUNT_USB_H
#define _SALLYMOUNT_USB_H

int usb_print(char *usb_path, int verbose, int human_readable);
int usb_print_multiple(char *usb_paths[], int num_usb_paths, int verbose, int human_readable);
int usb_print_all(int verbose, int human_readable);
int usb_mount(char *usb_path, char *options);
int usb_mount_multiple(char *usb_paths[], int num_usb_paths, char *options);
int usb_mount_all(char *options);
int usb_umount(char *usb_path);
int usb_umount_multiple(char *usb_paths[], int num_usb_paths);
int usb_umount_all();

#endif
