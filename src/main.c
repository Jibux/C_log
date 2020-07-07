#include "log.h"


int main (int argc, char *argv[]) {
	int ret;

	puts ("This is " PACKAGE_STRING ".");

	if(loadLogConf() != LOG_SUCCESS) {
		fprintf(stderr, "Unabled to load log configuration\n");
		return LOG_FAILED;
	}
	
	writeLog(STAT_NOTICE, "Prog has started");

	freeLog();
	
	return 0;
}

