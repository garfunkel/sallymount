#include <argp.h>

error_t argp_err_exit_status;
const char *argp_program_version;
const char *argp_program_bug_address;
static const char cli_doc[];
static char cli_args_doc[];
static struct argp_option cli_options[];

struct cli_args {
	int verbose;
	int all;
	char *command;
};

error_t cli_parse_opt(int key, char *arg, struct argp_state *state);

struct argp cli_argp;
