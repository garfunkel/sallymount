struct udev;
struct udev_device;

struct usb_device {
	char *node;
	char *manufacturer;
	char *product;
	char *serial;
	int bus;
	char *dev_path;
	char *sys_path;
	char *version;
	char *speed;
	int max_children;
};

struct usb_device_list {
	struct usb_device *device;
	struct usb_device_list *next;
};

char *usb_device_to_str(struct usb_device *device);
int usb_print(char *usb_path);
int usb_print_multiple(char *usb_paths[], int num_usb_paths);
int usb_print_all();
int usb_mount(char *usb_path);
int usb_mount_multiple(char *usb_paths[], int num_usb_paths);
int usb_mount_all();
int usb_umount(char *usb_path);
int usb_umount_multiple(char *usb_paths[], int num_usb_paths);
int usb_umount_all();

void usb_device_list_add(struct usb_device_list *list, struct usb_device *device);
char *usb_device_list_table_str(struct usb_device_list *list);
size_t usb_device_list_size(struct usb_device_list *list);
struct usb_device_list *usb_device_list_new();
struct usb_device_list *usb_device_list_get();

struct udev_device *usb_udev_device_get_child(struct udev *udev, struct udev_device *parent_device, const char *subsystem);
//struct udev_device *usb_device_get_child2(struct udev *udev, struct udev_device *parent_device, const char *subsystem);
