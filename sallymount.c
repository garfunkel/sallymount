#include <stdlib.h>
#include <argp.h>

#include "cli.h"
#include "usb.h"

int main(int argc, char **argv)
{
	struct cli_args cli_args = {0};
	cli_args.usb_paths = calloc(sizeof(char *), argc - 1);

	argp_parse(&cli_argp, argc, argv, ARGP_IN_ORDER, NULL, &cli_args);

	if (!cli_args.command) {
		if (cli_args.all || cli_args.num_usb_paths == 0) {
			usb_print_all(cli_args.verbose, cli_args.human_readable);
		} else {
			usb_print_multiple(cli_args.usb_paths, cli_args.num_usb_paths, cli_args.verbose, cli_args.human_readable);
		}
	}

	free(cli_args.usb_paths);

	return EXIT_SUCCESS;
}

