#ifndef _SALLYMOUNT_MOUNT_H
#define _SALLYMOUNT_MOUNT_H

struct cli_args_mount
{
	struct cli_args *cli_args;
	int all;
	char **usb_paths;
	size_t num_usb_paths;
};

error_t cli_parse_mount(int key, char *arg, struct argp_state *state);
void cmd_mount(struct argp_state *state);

#endif
