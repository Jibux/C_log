#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libconfig.h>
#include <config.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>

#define LOG_SUCCESS 0
#define LOG_FAILED -1

#define LOG_CONFIG_FILE_DEFAULT "log.conf"


enum { LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_TRACE };
static const char *log_level_strings[] = {
	"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
};

#define get_log_level_string(level) (log_level_strings[level])

struct s_log_config {
	FILE *file;
	int level;
	bool print;
	const char *filename;
	const char *prefix;
};

struct s_log_data {
	int level;
	const char *file;
	int line;
	const char *fmt;
	const char *time_prfx;
};

static const struct s_log_config log_config_default = {
	NULL,
	LOG_ERROR,
	false,
	NULL,
	NULL,
};

static struct s_log_config log_config = log_config_default;

static config_t cf;

static pthread_mutex_t s_mutex;

#define log_trace(...) do_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) do_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  do_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  do_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) do_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) do_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

static int close_log_file(void);
static FILE *get_print_fd(int level);
static void init_time(struct tm **time, long *usec);
static void init_time_prfx(char *time_prfx, size_t length);
static struct s_log_data init_log_data(const char *time_prfx, int level, const char *file, int line, const char *fmt);
static void write_log(FILE *fd, struct s_log_data log_data, va_list args);
static void init_mutex(void);
static void lock(void);
static void unlock(void);
static int open_log_file(void);
int init_log(void);
int load_log_config_from_file(const char *log_config_file);
void set_log_level(int level);
void set_print_log(bool print);
void set_log_filename(const char *filename);
void set_log_prefix(const char *prefix);
void display_log_config(void);
int do_log(int level, const char *file, int line, const char *fmt, ...);
int free_log(void);

