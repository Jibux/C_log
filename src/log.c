#include "log.h"

static int initLog(const char *filename, long int level, const char *prefix, long int print) {
	logConf = (LogConfig*)malloc(sizeof(LogConfig));

	logConf->logFile = NULL;
	logConf->logFileName = (char*)malloc(sizeof(char)*strlen(filename)+1);
	strcpy(logConf->logFileName, filename);
	logConf->logLevel = (int)level;
	logConf->logPrefix = (char*)malloc(sizeof(char)*strlen(prefix)+1);
	strcpy(logConf->logPrefix, prefix);
	logConf->printLog = (int)print;

	return LOG_SUCCESS;
}

static int openLog(void) {
	logConf->logFile = fopen(logConf->logFileName, "a+");

	if(logConf->logFile == NULL) {
		fprintf(stderr, "Can't open log file\n");
		return LOG_FAILED;
	}

	return LOG_SUCCESS;
}

static int closeLog(void) {
	if(fclose(logConf->logFile)) {
		return LOG_FAILED;
	}
	logConf->logFile = NULL;
	return LOG_SUCCESS;
}

static char * getLevelString(int level) {
	switch(level) {
		case STAT_ERROR: return "ERROR";
		break;
		case STAT_WARN: return "WARN";
		break;
		case STAT_NOTICE: return "NOTICE";
		break;
		case STAT_DEBUG: return "DEBUG";
		break;
		default: return "";
		break;
	}

	return "";
}

int loadLogConf(void) {
	config_t cfg, *cf;
	int level, print;
	const char *filename;
	const char *prefix;

	logConf = NULL;

	cf = &cfg;
	config_init(cf);

	if (!config_read_file(cf, LOG_CONFIG_FILE)) {
		//fprintf(stderr, "%s:%d - %s\n", config_error_file(cf), config_error_line(cf), config_error_text(cf));
		fprintf(stderr, "%s: %d - %s\n", LOG_CONFIG_FILE, config_error_line(cf), config_error_text(cf));
		config_destroy(cf);
		return LOG_FAILED;
	}

	if (config_lookup_int(cf, "logLevel", &level))
		printf("logLevel: %d\n", level);
	else {
		fprintf(stderr, "logLevel not defined\n");
		config_destroy(cf);
		return LOG_FAILED;
	}
	
	if (config_lookup_string(cf, "logFileName", &filename))
		printf("logFileName: %s\n", filename);
	else {
		fprintf(stderr, "logFileName not defined\n");
		config_destroy(cf);
		return LOG_FAILED;
	}

	if (config_lookup_string(cf, "logPrefix", &prefix))
		printf("logPrefix: %s\n", prefix);
	else {
		fprintf(stderr, "logPrefix not defined\n");
		config_destroy(cf);
		return LOG_FAILED;
	}

	if (config_lookup_int(cf, "printLog", &print))
		printf("print: %d\n", print);
	else {
		print = 0;
	}

	initLog(filename, level, prefix, print);
	
	config_destroy(cf);

	return LOG_SUCCESS;
}

int writeLog(int level, const char *format,...) {
	time_t intps;
	struct tm * p_datetime;
	char buf[20];
	char *levelString;
	va_list args;
	FILE *fd = stdout;

	if(openLog() != LOG_SUCCESS) {
		return LOG_FAILED;
	}

	if(level == STAT_ERROR)
		fd = stderr;

	if(level <= logConf->logLevel) {
		levelString = getLevelString(level);
		intps = time(NULL);
		p_datetime = localtime(&intps);
		sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d",p_datetime->tm_year+1900, p_datetime->tm_mon, p_datetime->tm_mday, p_datetime->tm_hour, p_datetime->tm_min, p_datetime->tm_sec);

		fprintf(logConf->logFile, "=== %s === %s = %s: ", logConf->logPrefix, buf, levelString);
		va_start (args, format);
		vfprintf(logConf->logFile, format, args);
		va_end (args);
		fprintf(logConf->logFile, "\n");

		if(logConf->printLog) {
			fprintf(fd, "=== %s === %s = %s: ", logConf->logPrefix, buf, levelString);
			va_start (args, format);
			vfprintf(fd, format, args);
			va_end (args);
			fprintf(fd, "\n");
		}
	}

	closeLog();
}

int freeLog(void) {
	if(logConf != NULL) {
		if(logConf->logFileName != NULL)
			free(logConf->logFileName);
		if(logConf->logPrefix != NULL)
			free(logConf->logPrefix);
	
		logConf->logFileName = NULL;
		logConf->logPrefix = NULL;
		free(logConf);
		logConf = NULL;
	}
	
	return LOG_SUCCESS;
}
