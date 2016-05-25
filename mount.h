#include <argp.h>

static const char cli_doc_mount[];
static char cli_args_doc_mount[];
static struct argp_option cli_options_mount[];

struct cli_args_mount
{
	struct cli_args *cli_args;
	int all;
	char **usb_paths;
	size_t num_usb_paths;
};

error_t cli_parse_mount(int key, char *arg, struct argp_state *state);
void cmd_mount(struct argp_state *state);
