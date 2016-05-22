#include <argp.h>

static const char cli_doc_umount[];
static char cli_args_doc_umount[];
static struct argp_option cli_options_umount[];

struct cli_args_umount
{
	struct cli_args* cli_args;
	int all;
};

error_t cli_parse_umount(int key, char* arg, struct argp_state* state);
void cmd_umount(struct argp_state* state);
