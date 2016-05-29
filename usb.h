struct udev;

struct usb_partition {
	struct usb_device *device;
	char *node;
	int num;
	char *dev_path;
	char *label;
	char *type;
	char *sys_path;
	size_t size;
};

struct usb_partition_list {
	struct usb_partition *partition;
	struct usb_partition_list *next;
};

struct usb_device {
	char *node;
	char *manufacturer;
	char *product;
	char *serial;
	char *dev_path;
	char *label;
	char *type;
	char *sys_path;
	char *version;
	char *speed;
	int bus;
	size_t size;
	size_t max_children;
	struct usb_partition_list *partition_list;
};

struct usb_device_list {
	struct usb_device *device;
	struct usb_device_list *next;
};

int usb_print(char *usb_path, int verbose, int human_readable);
int usb_print_multiple(char *usb_paths[], int num_usb_paths, int verbose, int human_readable);
int usb_print_all(int verbose, int human_readable);
int usb_mount(char *usb_path);
int usb_mount_multiple(char *usb_paths[], int num_usb_paths);
int usb_mount_all();
int usb_umount(char *usb_path);
int usb_umount_multiple(char *usb_paths[], int num_usb_paths);
int usb_umount_all();

void usb_device_list_add(struct usb_device_list *list, struct usb_device *device);
char *usb_device_list_detail_str(struct usb_device_list *list, int human_readable);
char *usb_device_list_table_str(struct usb_device_list *list, int human_readable);
size_t usb_device_list_size(struct usb_device_list *list);
size_t usb_device_and_partition_list_size(struct usb_device_list *list);
struct usb_device_list *usb_device_list_new();
struct usb_device_list *usb_device_list_get();

struct usb_partition_list *usb_partition_list_new();
void usb_partition_list_add(struct usb_partition_list *list, struct usb_partition *partition);

struct udev_device *usb_udev_device_get_child(struct udev *udev, struct udev_device *parent_device, const char *subsystem);
//struct udev_device *usb_device_get_child2(struct udev *udev, struct udev_device *parent_device, const char *subsystem);

void usb_device_free(struct usb_device *device);
void usb_device_list_free(struct usb_device_list *list);
void usb_partition_list_free(struct usb_partition_list *list);
void usb_partition_free(struct usb_partition *partition);
