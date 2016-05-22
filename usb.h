int usb_print(char *usb_path);
int usb_print_multiple(char *usb_paths[], int num_usb_paths);
int usb_print_all();
int usb_mount(char *usb_path);
int usb_mount_multiple(char *usb_paths[], int num_usb_paths);
int usb_mount_all();
int usb_umount(char *usb_path);
int usb_umount_multiple(char *usb_paths[], int num_usb_paths);
int usb_umount_all();