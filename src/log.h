#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libconfig.h>
#include <string.h>
#include <time.h>
#include <config.h>

#define STAT_ERROR 1
#define STAT_WARN 2
#define STAT_NOTICE 3
#define STAT_DEBUG 4

#define LOG_SUCCESS 0
#define LOG_FAILED -1

#define LOG_CONFIG_FILE "log.conf"


typedef struct LogConfig {
	FILE *logFile;
	char *logFileName;
	int logLevel;
	char *logPrefix;
	int printLog;
} LogConfig;

static LogConfig *logConf;

static int initLog(const char *, long int, const char *, long int);
static int openLog(void);
static int closeLog(void);
static char * getLevelString(int);
int loadLogConf(void);
int writeLog(int, const char*, ...);
int freeLog(void);

