#include "../src/log.h"


int main (int argc, char *argv[])
{
	int ret;
	char *name = "toto";

	if (load_log_config_from_file("log.conf") != LOG_SUCCESS) {
		fprintf(stderr, "Unabled to load log configuration\n");
		free_log();
		return LOG_FAILED;
	}

	/*set_log_filename("log.log");
	set_print_log(true);*/

	if (init_log() != LOG_SUCCESS) {
		fprintf(stderr, "Unabled to initialize log\n");
		free_log();
		return LOG_FAILED;
	}

	display_log_config();

	log_info("Prog has started");
	log_debug("hello %s", name);
	log_trace("world");
	log_error("Oups!");

	set_log_level(LOG_TRACE);
	log_trace("Miaou!");

	free_log();

	return 0;
}

