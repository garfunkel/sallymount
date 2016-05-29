#ifndef _SALLYMOUNT_UMOUNT_H
#define _SALLYMOUNT_UMOUNT_H

struct cli_args_umount
{
	struct cli_args* cli_args;
	int all;
	char **usb_paths;
	size_t num_usb_paths;
};

error_t cli_parse_umount(int key, char* arg, struct argp_state* state);
void cmd_umount(struct argp_state* state);

#endif
