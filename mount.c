#include <stdlib.h>
#include <string.h>
#include <argp.h>

#include "mount.h"
#include "usb.h"

static const char cli_doc_mount[] =
	"\n"
	"Mount USB mass storage devices.";

static const char cli_args_doc_mount[] = "[USB-PATH...]";

static struct argp_option cli_options_mount[] = {
	{
		"all",
		'a',
		0,
		0,
		"Mount all USB devices"
	},
	{NULL}
};

static struct argp cli_argp_mount = {
	cli_options_mount,
	cli_parse_mount,
	cli_args_doc_mount,
	cli_doc_mount
};

error_t cli_parse_mount(int key, char* arg, struct argp_state* state)
{
	struct cli_args_mount *cli_args_mount = state->input;

	switch(key)
	{
		case 'a':
			cli_args_mount->all = 1;

			break;

		case ARGP_KEY_ARG:
			for (int i = 0; i < state->argc; i++) {
				if (!cli_args_mount->usb_paths[i]) {
					cli_args_mount->usb_paths[i] = arg;
					cli_args_mount->num_usb_paths++;

					break;
				}
			}

			break;
	}

	return 0;
}

void cmd_mount(struct argp_state* state)
{
	struct cli_args_mount cli_args_mount = {0};
	int    argc = state->argc - state->next + 1;
	char** argv = &state->argv[state->next - 1];
	char*  argv0 =  argv[0];

	cli_args_mount.cli_args = state->input;
	cli_args_mount.usb_paths = calloc(sizeof(char *), argc);

	argv[0] = malloc(strlen(state->name) + strlen("mount") + 2);

	if(!argv[0])
		argp_failure(state, 1, ENOMEM, 0);

	sprintf(argv[0], "%s mount", state->name);

	argp_parse(&cli_argp_mount, argc, argv, ARGP_IN_ORDER, &argc, &cli_args_mount);

	free(argv[0]);

	argv[0] = argv0;

	state->next += argc - 1;

	if (cli_args_mount.all) {
		usb_mount_all();
	} else {
		usb_mount_multiple(cli_args_mount.usb_paths, cli_args_mount.num_usb_paths);
	}

	free(cli_args_mount.usb_paths);

	return;
}
