/* SPDX-License-Identifier: GPL-2.0 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libconfig.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>

#define LOG_SUCCESS 0
#define LOG_FAILED -1

enum { LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_TRACE };
static const char *log_level_strings[] = {
	"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
};

#define get_log_level_string(level) (log_level_strings[level])

struct prefix_element {
	char *fmt;
	void (*fn)();
};

struct log_config {
	FILE *file;
	int level;
	bool print;
	const char *filename;
	const char *pfx_format;
	struct prefix_element *pfx_elem;
	size_t pfx_length;
};

struct log_data {
	int level;
	const char *file;
	int line;
	const char *fmt;
	const char *time_pfx;
};

static const struct log_config log_cfg_default = {
	NULL,
	LOG_ERROR,
	false,
	NULL,
	"%d %l %f:%n: ",
	NULL,
	0,
};

static struct log_config log_cfg = log_cfg_default;

static config_t cf;

static pthread_mutex_t mutex;

#define log_trace(...) do_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) do_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  do_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  do_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) do_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) do_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

static FILE *get_print_fd(int level);
static void init_mutex(void);
static bool str_elem_empty(const char c);
static bool str_ended(const char *str);
static int process_pattern(const char *format, struct prefix_element *pfx_elem, int i);
static int init_pfx_elem(struct prefix_element *pfx_elem, size_t length);
static void re_index_pfx(struct prefix_element **pfx_elem, int *i);
static int add_pfx_elem(struct prefix_element **pfx_elem, size_t length);
static int process_fmt(const char *fmt, struct prefix_element **pfx_elem, size_t length);
static int init_prefix(void);
static void init_time(struct tm **time, long *usec);
static void init_time_prfx(char *time_prfx, size_t length);
static struct log_data init_log_data(const char *time_prfx, int level, const char *file, int line, const char *fmt);
static void write_ld_time_pfx(FILE *fd, struct log_data log_data, char *fmt);
static void write_ld_level_string(FILE *fd, struct log_data log_data, char *fmt);
static void write_ld_file(FILE *fd, struct log_data log_data, char *fmt);
static void write_ld_line(FILE *fd, struct log_data log_data, char *fmt);
static void write_prefix(FILE *fd, struct log_data log_data);
static void write_log(FILE *fd, struct log_data log_data, va_list args);
static void lock(void);
static void unlock(void);
static int open_log_file(void);
static int close_log_file(void);
int init_log(void);
int load_log_config_from_file(const char *log_config_file);
void set_log_level(int level);
void set_print_log(bool print);
void set_log_filename(const char *filename);
void set_log_pfx_format(const char *format);
void display_log_config(void);
int do_log(int level, const char *file, int line, const char *fmt, ...);
static void free_log_prefix(void);
int free_log(void);

#endif /* LOG_H */

