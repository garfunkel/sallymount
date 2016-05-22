#include <stdlib.h>
#include <string.h>

#include "umount.h"
#include "usb.h"

static const char cli_doc_umount[] =
	"\n"
	"Unmount USB mass storage devices.";

static char cli_args_doc_umount[] = "[USB-PATH...]";

struct argp cli_argp_umount = {
	cli_options_umount,
	cli_parse_umount,
	cli_args_doc_umount,
	cli_doc_umount
};

static struct argp_option cli_options_umount[] = {
	{
		"all",
		'a',
		0,
		0,
		"Unmount all USB devices"
	},
	{
		0
	}
};

error_t cli_parse_umount(int key, char* arg, struct argp_state* state)
{
	struct cli_args_umount *cli_args_umount = state->input;

	switch(key)
	{
		case 'a':
			cli_args_umount->all = 1;

			break;
	}

	return 0;
}

void cmd_umount(struct argp_state* state)
{
	struct cli_args_umount cli_args_umount = {0};
	int    argc = state->argc - state->next + 1;
	char** argv = &state->argv[state->next - 1];
	char*  argv0 =  argv[0];

	cli_args_umount.cli_args = state->input;

	argv[0] = malloc(strlen(state->name) + strlen("umount") + 2);

	if(!argv[0])
		argp_failure(state, 1, ENOMEM, 0);

	sprintf(argv[0], "%s umount", state->name);

	argp_parse(&cli_argp_umount, argc, argv, ARGP_IN_ORDER, &argc, &cli_args_umount);

	free(argv[0]);

	argv[0] = argv0;

	state->next += argc - 1;

	if (cli_args_umount.all) {
		usb_umount_all();
	} else {
		usb_umount_multiple(argv + 1, argc - 1);
	}

	return;
}