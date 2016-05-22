struct udev;
struct udev_device;

int usb_print(char *usb_path);
int usb_print_multiple(char *usb_paths[], int num_usb_paths);
int usb_print_all();
int usb_mount(char *usb_path);
int usb_mount_multiple(char *usb_paths[], int num_usb_paths);
int usb_mount_all();
int usb_umount(char *usb_path);
int usb_umount_multiple(char *usb_paths[], int num_usb_paths);
int usb_umount_all();

struct usb_device_list {
	struct udev_device *device;
	struct usb_device_list *next;
};

void usb_device_list_add(struct usb_device_list *list, struct udev_device *device);
struct usb_device_list *usb_device_list_new();
struct usb_device_list *usb_device_list_get();

struct udev_device *usb_device_get_child(struct udev *udev, struct udev_device *parent_device, const char *subsystem);
//struct udev_device *usb_device_get_child2(struct udev *udev, struct udev_device *parent_device, const char *subsystem);
