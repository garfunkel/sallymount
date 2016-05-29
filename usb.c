#define _GNU_SOURCE

#include <unistd.h>
#include <err.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <libudev.h>
#include <libmount/libmount.h>

#include "usb.h"

static const char *MOUNT_DIR_PREFIX = "/media";

static const char *HEADER_NODE = "NODE";
static const char *HEADER_MANUFACTURER = "MANUFACTURER";
static const char *HEADER_PRODUCT = "PRODUCT";
static const char *HEADER_SIZE = "SIZE";
static const char *HEADER_LABEL = "LABEL";
static const char *HEADER_TYPE = "TYPE";
static const char *HEADER_BUS = "BUS";
static const char *HEADER_DEV_PATH = "DEV_PATH";
static const char *HEADER_SYS_PATH = "SYS_PATH";
static const char *HEADER_SERIAL = "SERIAL";
static const char *HEADER_VERSION = "VERSION";
static const char *HEADER_SPEED = "SPEED";
static const char *HEADER_PARTITION = "PARTITION";

static const char *CELL_NONE = "(none)";
static const char *CELL_NA = "(n/a)";
static const char *CELL_UNKNOWN = "(unknown)";

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

static struct usb_device *usb_device_new();
static void usb_device_free(struct usb_device *device);

static struct usb_partition *usb_partition_new();
static void usb_partition_free(struct usb_partition *partition);

static struct usb_device_list *usb_device_list_new();
static struct usb_device_list *usb_device_list_get();
static void usb_device_list_add(struct usb_device_list *list, struct usb_device *device);
static void usb_device_list_free(struct usb_device_list *list);
static char *usb_device_list_detail_str(struct usb_device_list *list, int human_readable);
static char *usb_device_list_table_str(struct usb_device_list *list, int human_readable);
static char *usb_device_list_table_label_formatter(const char *str);
static char *usb_device_list_table_type_formatter(const char *str);

static struct usb_partition_list *usb_partition_list_new();
static void usb_partition_list_add(struct usb_partition_list *list,
                                   struct usb_partition *partition);
static void usb_partition_list_free(struct usb_partition_list *list);

static size_t usb_device_and_partition_list_size(struct usb_device_list *list);
static char *usb_get_partition_mount_directory(struct usb_partition *partition);
static int usb_create_partition_mount_directory(char *mount_path);
static int usb_delete_partition_mount_directory(char *mount_path);
static int usb_mount_device(struct usb_device *device);
static int usb_mount_partition(struct usb_partition *partition);
static int usb_umount_device(struct usb_device *device);
static int usb_umount_partition(struct usb_partition *partition);

static size_t usb_device_list_table_max_width_dev_path(struct usb_device_list *list);
static size_t usb_device_list_table_max_width_label(struct usb_device_list *list);
static size_t usb_device_list_table_max_width_manufacturer(struct usb_device_list *list);
static size_t usb_device_list_table_max_width_node(struct usb_device_list *list);
static size_t usb_device_list_table_max_width_product(struct usb_device_list *list);
static size_t usb_device_list_table_max_width_size(struct usb_device_list *list,
                                                   int human_readable_mode);
static size_t usb_device_list_table_max_width_type(struct usb_device_list *list);

static struct udev_device *usb_udev_device_get_child(struct udev *udev,
                                                     struct udev_device *parent_device,
                                                     const char *subsystem);

static char *human_readable_size(size_t num_bytes, int human_readable_mode);
static char *trim(char *str);

int usb_print(char *usb_path, int verbose, int human_readable)
{
	char *usb_paths[1] = {usb_path};

	return usb_print_multiple(usb_paths, 1, verbose, human_readable);
}

int usb_print_multiple(char *usb_paths[], int num_usb_paths, int verbose, int human_readable)
{
	int ret_code = 0;
	struct usb_device_list *head = usb_device_list_get();
	struct usb_device_list *list_to_print = usb_device_list_new();
	struct usb_device_list *list = NULL;
	char *print_str = NULL;

	for (int i = 0; i < num_usb_paths; i++) {
		list = head;

		while (list && list->device) {
			if (strcmp(list->device->dev_path, usb_paths[i]) == 0) {
				usb_device_list_add(list_to_print, list->device);

				break;
			}

			struct usb_partition_list *partition_list = list->device->partition_list;

			while (partition_list && partition_list->partition) {
				if (strcmp(partition_list->partition->dev_path, usb_paths[i]) == 0) {
					usb_device_list_add(list_to_print, list->device);

					break;
				}

				partition_list = partition_list->next;
			}

			if (partition_list)
				break;

			list = list->next;
		}
	}

	if (verbose)
		print_str = usb_device_list_detail_str(list_to_print, human_readable);

	else
		print_str = usb_device_list_table_str(list_to_print, human_readable);

	usb_device_list_free(head);

	printf("%s\n", print_str);

	free(print_str);

	return ret_code;
}

int usb_print_all(int verbose, int human_readable)
{
	struct usb_device_list *list = usb_device_list_get();
	char *print_str = NULL;

	if (verbose)
		print_str = usb_device_list_detail_str(list, human_readable);

	else
		print_str = usb_device_list_table_str(list, human_readable);

	usb_device_list_free(list);

	printf("%s\n", print_str);

	free(print_str);

	return 0;
}

static char *usb_get_partition_mount_directory(struct usb_partition *partition)
{
	char *mount_fmt_str = "%s/usb%s/partition%d";
	char *mount_path = NULL;

	asprintf(&mount_path,
	         mount_fmt_str,
	         MOUNT_DIR_PREFIX,
	         partition->device->dev_path,
	         partition->num);

	return mount_path;
}

static int usb_create_partition_mount_directory(char *mount_path)
{
	size_t size = strlen(mount_path);
	int retcode = 0;

	for (int i = 1; i < size; i++) {
		if (mount_path[i] == '/' || i == (size - 1)) {
			if (mount_path[i] == '/')
				mount_path[i] = '\0';

			mode_t mode = umask(0);
			umask(mode);

			if ((retcode = mkdir(mount_path, 0777 - mode))) {
				if (errno == EEXIST) {
					errno = 0;
					retcode = 0;
				} else {
					return retcode;
				}
			}

			if (mount_path[i] == '\0')
				mount_path[i] = '/';
		}
	}

	return retcode;
}

static int usb_delete_partition_mount_directory(char *mount_path)
{
	int size = strlen(mount_path);
	int retcode = 0;

	for (int i = size - 1; i >= 0; i--) {
		if (mount_path[i] == '/' || i == (size - 1)) {
			if (mount_path[i] == '/') {
				mount_path[i] = '\0';
			}

			if (strcmp(mount_path, MOUNT_DIR_PREFIX) == 0) {
				mount_path[i] = '/';

				break;
			}

			if ((retcode = rmdir(mount_path))) {
				if (errno == ENOENT || errno == ENOTEMPTY) {
					errno = 0;
					retcode = 0;
				} else {
					return retcode;
				}
			}

			if (mount_path[i] == '\0')
				mount_path[i] = '/';
		}
	}

	return retcode;
}

static int usb_mount_partition(struct usb_partition *partition)
{
	struct libmnt_context *context = mnt_new_context();

	if (!context) {
		perror("mnt_new_context()");

		exit(EXIT_FAILURE);
	}

	char *mount_path = usb_get_partition_mount_directory(partition);
	int retcode = 0;

	if ((retcode = usb_create_partition_mount_directory(mount_path))) {
		free(mount_path);

		mnt_free_context(context);

		return retcode;
	}

	if ((retcode = mnt_context_set_source(context, partition->node))) {
		free(mount_path);

		mnt_free_context(context);

		return retcode;
	}

	if ((retcode = mnt_context_set_target(context, mount_path))) {
		free(mount_path);

		mnt_free_context(context);

		return retcode;
	}

	struct libmnt_fs *fs = mnt_context_get_fs(context);
	int mounted = 0;

	if ((retcode = mnt_context_is_fs_mounted(context, fs, &mounted))) {
		free(mount_path);

		mnt_free_context(context);

		return retcode;
	}

	if (mounted){
		free(mount_path);

		mnt_free_context(context);

		errno = EBUSY;

		return EBUSY;
	}

	free(mount_path);

	retcode = mnt_context_mount(context);

	mnt_free_context(context);

	return retcode;
}

static int usb_mount_device(struct usb_device *device)
{
	int retcode = 0;
	int mount_retcode = 0;
	struct usb_partition_list *list = device->partition_list;

	while (list && list->partition) {
		if ((mount_retcode = usb_mount_partition(list->partition))) {
			warn("Mounting partition %s failed", list->partition->node);

			retcode = mount_retcode;
		}

		list = list->next;
	}

	return retcode;
}

static int usb_umount_partition(struct usb_partition *partition)
{
	struct libmnt_context *context = mnt_new_context();

	if (!context) {
		perror("mnt_new_context()");

		exit(EXIT_FAILURE);
	}

	char *mount_path = usb_get_partition_mount_directory(partition);
	int retcode = 0;

	if ((retcode = mnt_context_set_source(context, partition->node))) {
		free(mount_path);

		mnt_free_context(context);

		return retcode;
	}

	if ((retcode = mnt_context_set_target(context, mount_path))) {
		free(mount_path);

		mnt_free_context(context);

		return retcode;
	}

	struct libmnt_fs *fs = mnt_context_get_fs(context);
	int mounted = 0;

	if ((retcode = mnt_context_is_fs_mounted(context, fs, &mounted))) {
		free(mount_path);

		mnt_free_context(context);

		return retcode;
	}

	if (!mounted){
		warnx("Unmounting partition %s failed: Not mounted", partition->node);
	} else {
		retcode = mnt_context_umount(context);
	}

	mnt_free_context(context);

	if (retcode) {
		free(mount_path);

		return retcode;
	}

	retcode = usb_delete_partition_mount_directory(mount_path);

	free(mount_path);

	return retcode;
}

static int usb_umount_device(struct usb_device *device)
{
	int retcode = 0;
	int umount_retcode = 0;
	struct usb_partition_list *list = device->partition_list;

	while (list && list->partition) {
		if ((umount_retcode = usb_umount_partition(list->partition))) {
			warn("Unmounting partition %s failed", list->partition->node);

			retcode = umount_retcode;
		}

		list = list->next;
	}

	return retcode;
}

int usb_mount(char *usb_path)
{
	char *usb_paths[1] = {usb_path};

	return usb_mount_multiple(usb_paths, 1);
}

int usb_mount_multiple(char *usb_paths[], int num_usb_paths)
{
	struct usb_device_list *list = usb_device_list_get();
	struct usb_device_list *head = list;
	int retcode = 0;
	int mount_retcode = 0;

	while (list && list->device) {
		for (int i = 0; i < num_usb_paths; i++) {
			if (strcmp(list->device->dev_path, usb_paths[i]) == 0) {
				if ((mount_retcode = usb_mount_device(list->device)))
					retcode = mount_retcode;

				break;
			} else {
				struct usb_partition_list *partition_list = list->device->partition_list;

				while (partition_list && partition_list->partition) {
					if (strcmp(partition_list->partition->dev_path, usb_paths[i]) == 0) {
						if ((mount_retcode = usb_mount_partition(partition_list->partition))) {
							warn("Mounting partition %s failed", partition_list->partition->node);

							retcode = mount_retcode;
						}
					}

					partition_list = partition_list->next;
				}
			}
		}

		list = list->next;
	}

	usb_device_list_free(head);

	return retcode;
}

int usb_mount_all()
{
	int retcode = 0;
	int mount_retcode = 0;
	struct usb_device_list *list = usb_device_list_get();
	struct usb_device_list *head = list;

	while (list && list->device) {
		if ((mount_retcode = usb_mount_device(list->device)))
			retcode = mount_retcode;

		list = list->next;
	}

	usb_device_list_free(head);

	return retcode;
}

int usb_umount(char *usb_path)
{
	char *usb_paths[1] = {usb_path};

	return usb_umount_multiple(usb_paths, 1);
}

int usb_umount_multiple(char *usb_paths[], int num_usb_paths)
{
	int retcode = 0;
	int umount_retcode = 0;
	struct usb_device_list *list = usb_device_list_get();
	struct usb_device_list *head = list;

	while (list && list->device) {
		for (int i = 0; i < num_usb_paths; i++) {
			if (strcmp(list->device->dev_path, usb_paths[i]) == 0) {
				if ((umount_retcode = usb_umount_device(list->device)))
					retcode = umount_retcode;

				break;
			} else {
				struct usb_partition_list *partition_list = list->device->partition_list;

				while (partition_list && partition_list->partition) {
					if (strcmp(partition_list->partition->dev_path, usb_paths[i]) == 0) {
						if ((umount_retcode = usb_umount_partition(partition_list->partition)) != 0)
							warn("Unmounting partition %s failed", partition_list->partition->node);

							retcode = umount_retcode;
					}

					partition_list = partition_list->next;
				}
			}
		}

		list = list->next;
	}

	usb_device_list_free(head);

	return retcode;
}

int usb_umount_all()
{
	int retcode = 0;
	int umount_retcode = 0;
	struct usb_device_list *list = usb_device_list_get();
	struct usb_device_list *head = list;

	while (list && list->device) {
		if ((umount_retcode = usb_umount_device(list->device)))
			retcode = umount_retcode;

		list = list->next;
	}

	usb_device_list_free(head);

	return retcode;
}

static char *human_readable_size(size_t num_bytes, int human_readable_mode)
{
	char *human_readable_size;

	if (human_readable_mode == 0) {
		asprintf(&human_readable_size, "%lu", num_bytes);

		return human_readable_size;
	}

	int power;
	char *suffixes[] = {
		"K",
		"M",
		"G",
		"T",
		"P",
		"E",
		"Z",
		"Y",
		NULL
	};

	if (human_readable_mode == 1)
		power = 1024;

	else
		power = 1000;

	if (num_bytes < power) {
		asprintf(&human_readable_size, "%lu", num_bytes);

		return human_readable_size;
	}

	int i = 0;
	long double size = num_bytes;

	while (suffixes[i] != NULL) {
		size /= power;

		if (size < power || suffixes[i + 1] == NULL)
			break;

		i++;
	}

	asprintf(&human_readable_size, "%lu%s", (size_t)size, suffixes[i]);

	return human_readable_size;
}

static char *trim(char *str)
{
	char *end;

	while (isspace(*str))
		str++;

	if (*str == 0)
		return str;

	end = str + strlen(str) - 1;

	while (end > str && isspace(*end))
		end--;

	*(end + 1) = 0;

	return str;
}

static void usb_device_list_add(struct usb_device_list *list, struct usb_device *device)
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

static struct usb_device_list *usb_device_list_new()
{
	struct usb_device_list *list = malloc(sizeof(struct usb_device_list));

	memset(list, 0, sizeof(struct usb_device_list));

	return list;
}

static size_t usb_device_list_table_max_width_node(struct usb_device_list *list)
{
	size_t max = strlen(HEADER_NODE);

	while (list && list->device) {
		size_t size = strlen(list->device->node);

		if (size > max)
			max = size;

		struct usb_partition_list *partition_list = list->device->partition_list;

		while (partition_list && partition_list->partition) {
			size = strlen(partition_list->partition->node) + 4;

			if (size > max)
				max = size;

			partition_list = partition_list->next;
		}

		list = list->next;
	}

	return max;
}

static size_t usb_device_list_table_max_width_manufacturer(struct usb_device_list *list)
{
	size_t max = strlen(HEADER_MANUFACTURER);
	size_t size = strlen(CELL_NA);

	if (size > max)
		max = size;

	while (list && list->device) {
		size = strlen(list->device->manufacturer);

		if (size > max)
			max = size;

		list = list->next;
	}

	return max;
}

static size_t usb_device_list_table_max_width_product(struct usb_device_list *list)
{
	size_t max = strlen(HEADER_PRODUCT);
	size_t size = strlen(CELL_NA);

	if (size > max)
		max = size;

	while (list && list->device) {
		size = strlen(list->device->product);

		if (size > max)
			max = size;

		list = list->next;
	}

	return max;
}

static size_t usb_device_list_table_max_width_size(struct usb_device_list *list,
                                                   int human_readable_mode)
{
	size_t max = strlen(HEADER_SIZE);
	char *size_str = NULL;

	while (list && list->device) {
		size_str = human_readable_size(list->device->size, human_readable_mode);
		size_t size = strlen(size_str);
		free(size_str);

		if (size > max)
			max = size;

		struct usb_partition_list *partition_list = list->device->partition_list;

		while (partition_list && partition_list->partition) {
			size_str = human_readable_size(partition_list->partition->size, human_readable_mode);
			size = strlen(size_str) + 4;
			free(size_str);

			if (size > max)
				max = size;

			partition_list = partition_list->next;
		}

		list = list->next;
	}

	return max;
}

static size_t usb_device_list_table_max_width_label(struct usb_device_list *list)
{
	size_t max = strlen(HEADER_LABEL);
	size_t size = strlen(CELL_NONE);

	if (size > max)
		max = size;

	while (list && list->device) {
		size = strlen(list->device->label);

		if (size > max)
			max = size;

		struct usb_partition_list *partition_list = list->device->partition_list;

		while (partition_list && partition_list->partition) {
			size = strlen(partition_list->partition->label) + 4;

			if (size > max)
				max = size;

			partition_list = partition_list->next;
		}

		list = list->next;
	}

	return max;
}

static size_t usb_device_list_table_max_width_type(struct usb_device_list *list)
{
	size_t max = strlen(HEADER_TYPE);
	size_t size = strlen(CELL_UNKNOWN);

	if (size > max)
		max = size;

	while (list && list->device) {
		size = strlen(list->device->type);

		if (size > max)
			max = size;

		struct usb_partition_list *partition_list = list->device->partition_list;

		while (partition_list && partition_list->partition) {
			size = strlen(partition_list->partition->type) + 4;

			if (size > max)
				max = size;

			partition_list = partition_list->next;
		}

		list = list->next;
	}

	return max;
}

static size_t usb_device_list_table_max_width_dev_path(struct usb_device_list *list)
{
	size_t max = strlen(HEADER_DEV_PATH);

	while (list && list->device) {
		size_t size = strlen(list->device->dev_path);

		if (size > max)
			max = size;

		struct usb_partition_list *partition_list = list->device->partition_list;

		while (partition_list && partition_list->partition) {
			size = strlen(partition_list->partition->dev_path) + 4;

			if (size > max)
				max = size;

			partition_list = partition_list->next;
		}

		list = list->next;
	}

	return max;
}

static char *usb_device_list_detail_str(struct usb_device_list *list, int human_readable_mode)
{
	char *detail_str = NULL;
	char *old_detail_str = NULL;
	char *size = NULL;

	while (list && list->device) {
		size = human_readable_size(list->device->size, human_readable_mode);
		old_detail_str = detail_str;

		asprintf(&detail_str,
		         "%s"
		         "%s:        \t%s\n"
		         "%s:         \t%d\n"
		         "%s:    \t%s\n"
		         "%s:        \t%s\n"
		         "%s:       \t%s\n"
		         "%s:        \t%s\n"
		         "%s:\t%s\n"
		         "%s:     \t%s\n"
		         "%s:      \t%s\n"
		         "%s:    \t%s\n"
		         "%s:     \t%s\n"
		         "%s:       \t%s",
		         old_detail_str,
		         HEADER_NODE,
		         list->device->node,
		         HEADER_BUS,
		         list->device->bus,
		         HEADER_DEV_PATH,
		         list->device->dev_path,
		         HEADER_SIZE,
		         size,
		         HEADER_LABEL,
		         list->device->label,
		         HEADER_TYPE,
		         list->device->type,
		         HEADER_MANUFACTURER,
		         list->device->manufacturer,
		         HEADER_PRODUCT,
		         list->device->product,
		         HEADER_SERIAL,
		         list->device->serial,
		         HEADER_SYS_PATH,
		         list->device->sys_path,
		         HEADER_VERSION,
		         trim(list->device->version),
		         HEADER_SPEED,
		         list->device->speed);

		free(old_detail_str);
		free(size);

		struct usb_partition_list *partition_list = list->device->partition_list;

		while (partition_list && partition_list->partition) {
			size = human_readable_size(partition_list->partition->size, human_readable_mode);
			old_detail_str = detail_str;

			asprintf(&detail_str,
			        "%s\n"
			        "%s:   \t%d\n"
			        "    %s:    \t    %s\n"
			        "    %s:    \t    %s\n"
			        "    %s:   \t    %s\n"
			        "    %s:    \t    %s\n"
			        "    %s:\t    %s",
			        old_detail_str,
			        HEADER_PARTITION,
			        partition_list->partition->num,
			        HEADER_NODE,
			        partition_list->partition->node,
			        HEADER_SIZE,
			        size,
			        HEADER_LABEL,
			        partition_list->partition->label,
			        HEADER_TYPE,
			        partition_list->partition->type,
			        HEADER_SYS_PATH,
			        partition_list->partition->sys_path);

			free(old_detail_str);
			free(size);

			partition_list = partition_list->next;
		}

		if (list->next) {
			old_detail_str = detail_str;

			asprintf(&detail_str,
			         "%s\n\n",
			         old_detail_str);

			free(old_detail_str);
		}

		list = list->next;
	}

	return detail_str;
}

static char *usb_device_list_table_label_formatter(const char *str)
{
	if (strlen(str) == 0)
		return (char *)CELL_NONE;

	else
		return (char *)str;
}

static char *usb_device_list_table_type_formatter(const char *str)
{
	if (strlen(str) == 0)
		return (char *)CELL_UNKNOWN;

	else
		return (char *)str;
}

static char *usb_device_list_table_str(struct usb_device_list *list, int human_readable_mode)
{
	size_t width_node = usb_device_list_table_max_width_node(list);
	size_t width_size = usb_device_list_table_max_width_size(list, human_readable_mode);
	size_t width_manufacturer = usb_device_list_table_max_width_manufacturer(list);
	size_t width_product = usb_device_list_table_max_width_product(list);
	size_t width_label = usb_device_list_table_max_width_label(list);
	size_t width_type = usb_device_list_table_max_width_type(list);
	size_t width_dev_path = usb_device_list_table_max_width_dev_path(list);
	size_t list_size = usb_device_and_partition_list_size(list);
	size_t table_line_str_size = width_node + 1 +
	                             width_dev_path + 1 +
	                             width_size + 1 +
	                             width_label + 1 +
	                             width_type + 1 +
	                             width_manufacturer + 1 +
	                             width_product + 1;
	char *table_fmt_str = NULL;
	char *table_partition_fmt_str = NULL;

	asprintf(&table_fmt_str,
	        "%%-%lus\t%%-%lus\t%%-%lus\t%%-%lus\t%%-%lus\t%%-%lus\t%%-%lus",
	        width_node,
	        width_dev_path,
	        width_size,
	        width_label,
	        width_type,
	        width_manufacturer,
	        width_product);

	char *child_indicator = " ├─ ";
	char *child_indicator_final = " ╰─ ";

	asprintf(&table_partition_fmt_str,
	        "%%s%%-%lus\t%%s%%-%lus\t%%s%%-%lus\t%%s%%-%lus\t%%s%%-%lus\t%%s%%-%lus\t%%s%%-%lus",
	        width_node - 4,
	        width_dev_path - 4,
	        width_size - 4,
	        width_label - 4,
	        width_type - 4,
	        width_manufacturer - 4,
	        width_product - 4);

	size_t table_str_size = table_line_str_size * (list_size + 1) + (list_size * 28) + 1;
	char *table_str = malloc(table_str_size);
	table_str[0] = '\0';

	sprintf(table_str,
	        table_fmt_str,
	        HEADER_NODE,
	        HEADER_DEV_PATH,
	        HEADER_SIZE,
	        HEADER_LABEL,
	        HEADER_TYPE,
	        HEADER_MANUFACTURER,
	        HEADER_PRODUCT);

	while (list && list->device) {
		char *size = human_readable_size(list->device->size, human_readable_mode);
		sprintf(table_str + strlen(table_str), "\n");

		sprintf(table_str + strlen(table_str),
		        table_fmt_str,
		        list->device->node,
		        list->device->dev_path,
		        size,
		        usb_device_list_table_label_formatter(list->device->label),
		        usb_device_list_table_type_formatter(list->device->type),
		        list->device->manufacturer,
		        list->device->product);

		free(size);

		struct usb_partition_list *partition_list = list->device->partition_list;
		char *indicator = NULL;

		while (partition_list && partition_list->partition) {
			size = human_readable_size(partition_list->partition->size, human_readable_mode);
			//sprintf(size, "%lu", partition_list->partition->size);
			sprintf(table_str + strlen(table_str), "\n");

			if (partition_list->next)
				indicator = child_indicator;

			else
				indicator = child_indicator_final;

			sprintf(table_str + strlen(table_str),
			        table_partition_fmt_str,
			        indicator,
			        partition_list->partition->node,
			        indicator,
			        partition_list->partition->dev_path,
			        indicator,
			        size,
			        indicator,
			        usb_device_list_table_label_formatter(partition_list->partition->label),
			        indicator,
			        usb_device_list_table_type_formatter(partition_list->partition->type),
			        indicator,
			        "(n/a)",
			        indicator,
			        "(n/a)");

			free(size);

			partition_list = partition_list->next;
		}

		list = list->next;
	}

	free(table_partition_fmt_str);
	free(table_fmt_str);

	return table_str;
}

static size_t usb_device_and_partition_list_size(struct usb_device_list *list)
{
	size_t size = 0;

	while (list && list->device) {
		size++;
		struct usb_partition_list *partition_list = list->device->partition_list;

		while (partition_list && partition_list->partition) {
			size++;

			partition_list = partition_list->next;
		}

		list = list->next;
	}

	return size;
}

static struct udev_device *usb_udev_device_get_child(struct udev *udev,
                                                     struct udev_device *parent_device,
                                                     const char *subsystem)
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

static struct usb_device_list *usb_device_list_get()
{
	struct udev *udev = udev_new();

	if (!udev) {
		perror("udev_new() failed");

		exit(EXIT_FAILURE);
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
		struct udev_device *usb_device = udev_device_get_parent_with_subsystem_devtype(scsi_device,
		                                                                               "usb",
		                                                                               "usb_device");
		struct udev_device *block_device = usb_udev_device_get_child(udev, scsi_device, "block");
		struct udev_device *scsi_disk_device = usb_udev_device_get_child(udev, scsi_device,
		                                                                 "scsi_disk");

		errno = 0;

		if (usb_device && block_device && scsi_disk_device) {
			struct usb_device *device = usb_device_new();

			device->node = strdup((char *)udev_device_get_devnode(block_device));
			device->manufacturer = strdup((char *)udev_device_get_sysattr_value(usb_device,
			                                                                    "manufacturer"));
			device->product = strdup((char *)udev_device_get_sysattr_value(usb_device, "product"));
			device->serial = strdup((char *)udev_device_get_sysattr_value(usb_device, "serial"));
			device->dev_path = strdup((char *)udev_device_get_sysattr_value(usb_device, "devpath"));

			char *label = (char *)udev_device_get_property_value(block_device, "ID_FS_LABEL");

			if (label)
				device->label = strdup(label);

			else
				device->label = strdup("");

			char *type = (char *)udev_device_get_property_value(block_device, "ID_FS_TYPE");

			if (type)
				device->type = strdup(type);

			else
				device->type = strdup("");

			device->sys_path = strdup((char *)udev_device_get_syspath(usb_device));
			device->speed = strdup((char *)udev_device_get_sysattr_value(usb_device, "speed"));
			device->version = strdup((char *)udev_device_get_sysattr_value(usb_device, "version"));
			device->max_children = atoi(udev_device_get_sysattr_value(usb_device, "maxchild"));
			device->bus = atoi(udev_device_get_sysattr_value(usb_device, "busnum"));
			device->size = atol(udev_device_get_sysattr_value(block_device, "size")) * (size_t)512;

			device->partition_list = usb_partition_list_new();
			struct udev_enumerate *enumerate_partitions = udev_enumerate_new(udev);

			udev_enumerate_add_match_parent(enumerate_partitions, block_device);
			udev_enumerate_add_match_subsystem(enumerate_partitions, "block");
			udev_enumerate_add_match_sysattr(enumerate_partitions, "partition", "*");
			udev_enumerate_scan_devices(enumerate_partitions);

			struct udev_list_entry *partition_entry = udev_enumerate_get_list_entry(enumerate_partitions);

			while (partition_entry) {
				const char *partition_name = udev_list_entry_get_name(partition_entry);
				struct udev_device *partition_device = udev_device_new_from_syspath(udev, partition_name);
				struct usb_partition *partition = usb_partition_new();
				char *partition_num = (char *)udev_device_get_sysattr_value(partition_device,
				                                                            "partition");
				partition->device = device;
				partition->node = strdup((char *)udev_device_get_devnode(partition_device));
				partition->sys_path = strdup((char *)udev_device_get_syspath(partition_device));
				partition->num = atoi(partition_num);
				partition->dev_path = malloc(strlen(device->dev_path) + strlen(partition_num) + 2);

				sprintf(partition->dev_path, "%s-%s", device->dev_path, partition_num);

				partition->size = atol(udev_device_get_sysattr_value(partition_device, "size"))
				                * (size_t)512;

				label = (char *)udev_device_get_property_value(partition_device, "ID_FS_LABEL");

				if (label)
					partition->label = strdup(label);

				else
					partition->label = strdup("");

				type = (char *)udev_device_get_property_value(partition_device, "ID_FS_TYPE");

				if (type)
					partition->type = strdup(type);

				else
					partition->type = strdup("");

				udev_device_unref(partition_device);

				usb_partition_list_add(device->partition_list, partition);

				partition_entry = udev_list_entry_get_next(partition_entry);
			}

			udev_enumerate_unref(enumerate_partitions);

			usb_device_list_add(list, device);
		}

		udev_device_unref(block_device);
		udev_device_unref(scsi_disk_device);
		udev_device_unref(scsi_device);

		device_entry = udev_list_entry_get_next(device_entry);
	}

	udev_enumerate_unref(enumerate);

	udev_unref(udev);

	return list;
}

static struct usb_device *usb_device_new()
{
	return malloc(sizeof(struct usb_device));
}

static struct usb_partition *usb_partition_new()
{
	return malloc(sizeof(struct usb_partition));
}

static struct usb_partition_list *usb_partition_list_new()
{
	struct usb_partition_list *list = malloc(sizeof(struct usb_partition_list));

	memset(list, 0, sizeof(struct usb_partition_list));

	return list;
}

static void usb_device_free(struct usb_device *device)
{
	if (!device)
		return;

	usb_partition_list_free(device->partition_list);

	free(device->node);
	free(device->manufacturer);
	free(device->product);
	free(device->serial);
	free(device->dev_path);
	free(device->label);
	free(device->type);
	free(device->sys_path);
	free(device->version);
	free(device->speed);
	free(device);
}

static void usb_device_list_free(struct usb_device_list *list)
{
	if (!list)
		return;

	if (list->next)
		usb_device_list_free(list->next);

	usb_device_free(list->device);

	free(list);
}

static void usb_partition_list_free(struct usb_partition_list *list)
{
	if (!list)
		return;

	if (list->next) {
		usb_partition_list_free(list->next);
	}

	usb_partition_free(list->partition);

	free(list);
}

static void usb_partition_free(struct usb_partition *partition)
{
	if (!partition)
		return;

	free(partition->node);
	free(partition->dev_path);
	free(partition->label);
	free(partition->type);
	free(partition->sys_path);
	free(partition);
}

static void usb_partition_list_add(struct usb_partition_list *list,
                                   struct usb_partition *partition)
{
	if (!list->partition) {
		list->partition = partition;
	} else {
		while (list->next) {
			list = list->next;
		}

		list->next = usb_partition_list_new();
		list->next->partition = partition;
	}
}
