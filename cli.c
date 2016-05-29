#include <string.h>
#include <argp.h>

#include "cli.h"
#include "mount.h"
#include "umount.h"

const char *argp_program_version = "1.0 - \"Sleep deprecation\"";
const char *argp_program_bug_address = "simon@simonallen.org";

static const char cli_doc[] =
	"\n"
	"Mount manager for USB mass storage devices."
	"\v"
	"Supported commands are:\n"
	"  mount    Mount USB mass storage devices\n"
	"  umount   Unmount USB mass storage devices";

static const char cli_args_doc[] = "[COMMAND [OPTION...]...]";

static struct argp_option cli_options[] = {
	{
		"verbose",
		'v',
		0,
		0,
		"Produce verbose output"
	},
	{
		"all",
		'a',
		0,
		0,
		"Print all USB devices"
	},
	{
		"human-readable",
		'h',
		0,
		0,
		"Print sizes in powers of 1024 (e.g., 1023M)"
	},
	{
		"si",
		'H',
		0,
		0,
		"Print sizes in powers of 1000 (e.g., 1.1G)"
	},
	{NULL}
};

struct argp cli_argp = {
	cli_options,
	cli_parse_opt,
	cli_args_doc,
	cli_doc
};

error_t argp_err_exit_status = 1;

error_t cli_parse_opt(int key, char *arg, struct argp_state *state) {
	struct cli_args *cli_args = state->input;

	switch (key) {
		case 'v':
			cli_args->verbose = 1;

			break;

		case 'a':
			cli_args->all = 1;

			break;

		case 'h':
			cli_args->human_readable = 1;

			break;

		case 'H':
			cli_args->human_readable = 2;

			break;

		case ARGP_KEY_ARG:
			if (strcmp(arg, "mount") == 0) {
				cli_args->command = arg;

				cmd_mount(state);
			} else if (strcmp(arg, "umount") == 0) {
				cli_args->command = arg;

				cmd_umount(state);
			} else {
				for (int i = 0; i < state->argc; i++) {
					if (!cli_args->usb_paths[i]) {
						cli_args->usb_paths[i] = arg;
						cli_args->num_usb_paths++;

						break;
					}
				}

				//TODO: Check for valid USB paths?
				//argp_error(state, "%s is not a valid command", arg);
			}

			break;

		//TODO: Check for valid USB paths?
		//default:
		//	return ARGP_ERR_UNKNOWN;
	}

	return 0;
}
