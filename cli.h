#ifndef _SALLYMOUNT_CLI_H
#define _SALLYMOUNT_CLI_H

error_t argp_err_exit_status;
const char *argp_program_version;
const char *argp_program_bug_address;

struct cli_args {
	int verbose;
	int all;
	int human_readable;
	char *command;
	char **usb_paths;
	size_t num_usb_paths;
};

error_t cli_parse_opt(int key, char *arg, struct argp_state *state);

struct argp cli_argp;

#endif
