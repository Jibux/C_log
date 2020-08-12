/* SPDX-License-Identifier: GPL-2.0 */

#include "liblog.h"

static FILE *get_print_fd(int level)
{
	return (level >= LOG_ERROR) ? stderr : stdout;
}

static void init_mutex(void)
{
	pthread_mutex_init(&mutex, NULL);
}

static bool str_elem_empty(const char c)
{
	return (c == '\0') ? true : false;
}

static bool str_ended(const char *str)
{
	return (str == NULL || str_elem_empty(*str)) ? true : false;
}

static int process_pattern(const char *format, struct prefix_element *pfx_elem, int i)
{
	if (str_ended(format)) {
		return LOG_FAILED;
	}
	switch (*format) {
	case 'd':
		pfx_elem->fmt[i] = 's';
		pfx_elem->fn = write_ld_time_pfx;
		break;
	case 'l':
		pfx_elem->fmt[i++] = '-';
		pfx_elem->fmt[i++] = '5';
		pfx_elem->fmt[i] = 's';
		pfx_elem->fn = write_ld_level_string;
		break;
	case 'f':
		pfx_elem->fmt[i] = 's';
		pfx_elem->fn = write_ld_file;
		break;
	case 'n':
		pfx_elem->fmt[i] = 'd';
		pfx_elem->fn = write_ld_line;
		break;
	default:
		pfx_elem->fmt[i] = *format;
		break;
	}

	return LOG_SUCCESS;
}

static int init_pfx_elem(struct prefix_element *pfx_elem, size_t length)
{
	pfx_elem->fn = NULL;
	if ((pfx_elem->fmt = (char *)malloc(sizeof(char) * (length + 1))) == NULL)
		return LOG_FAILED;
	memset(pfx_elem->fmt, '\0', (sizeof(char) * (length + 1)));

	return LOG_SUCCESS;
}

static void re_index_pfx(struct prefix_element **pfx_elem, int *i)
{
	*i = 0;
	(*pfx_elem)++;
}

static int add_pfx_elem(struct prefix_element **pfx_elem, size_t length)
{
	int ret;

	if ((ret = init_pfx_elem((*pfx_elem) + 1, length)) != LOG_SUCCESS)
		return ret;

	log_cfg.pfx_length++;

	return ret;
}

static int process_fmt(const char *fmt, struct prefix_element **pfx_elem, size_t length)
{
	static bool do_process_pattern = false;
	static int i = 0;
	int ret = LOG_SUCCESS;

	if (do_process_pattern) {
		ret = process_pattern(fmt, *pfx_elem, i);
		if (ret != LOG_SUCCESS)
			return ret;
		ret = add_pfx_elem(pfx_elem, length);
		if (ret != LOG_SUCCESS)
			return ret;
		re_index_pfx(pfx_elem, &i);
	} else {
		(*pfx_elem)->fmt[i++] = *fmt;
	}
	do_process_pattern = (*fmt == '%') ? true : false;

	return ret;
}

static int init_prefix(void)
{
	const char *fmt = log_cfg.pfx_format;
	size_t length = strlen(log_cfg.pfx_format);
	struct prefix_element *pfx_elem;
	int ret = LOG_SUCCESS;

	if (str_ended(fmt) || log_cfg.pfx_elem != NULL)
		return LOG_SUCCESS;

	if ((log_cfg.pfx_elem = malloc(sizeof(struct prefix_element) * length)) == NULL)
		return LOG_FAILED;
	pfx_elem = log_cfg.pfx_elem;
	init_pfx_elem(pfx_elem, length);

	log_cfg.pfx_length++;

	while (!str_ended(fmt)) {
		ret = process_fmt(fmt, &pfx_elem, length);
		if (ret != LOG_SUCCESS)
			return ret;
		fmt++;
	}

	return ret;
}

static void init_time(struct tm **time, long *usec)
{
	struct timeval tv;
	time_t t;

	gettimeofday(&tv, NULL);
	t = tv.tv_sec;
	*time = localtime(&t);
	*usec = tv.tv_usec;
}

static void init_time_prfx(char *time_pfx, size_t length)
{
	struct tm *time;
	long usec;

	init_time(&time, &usec);
	strftime(time_pfx, length, "%Y-%m-%d %H:%M:%S", time);
	sprintf(&time_pfx[19], ".%06ld", usec);
}

static struct log_data init_log_data(const char *time_pfx, int level, const char *file, int line, const char *fmt)
{
	struct log_data log_data;
	log_data.time_pfx = time_pfx;
	log_data.level = level;
	log_data.file = file;
	log_data.line = line;
	log_data.fmt = fmt;

	return log_data;
}

static void write_ld_time_pfx(FILE *fd, struct log_data log_data, char *fmt)
{
	fprintf(fd, fmt, log_data.time_pfx);
}

static void write_ld_level_string(FILE *fd, struct log_data log_data, char *fmt)
{
	fprintf(fd, fmt, get_log_level_string(log_data.level));
}

static void write_ld_file(FILE *fd, struct log_data log_data, char *fmt)
{
	fprintf(fd, fmt, log_data.file);
}

static void write_ld_line(FILE *fd, struct log_data log_data, char *fmt)
{
	fprintf(fd, fmt, log_data.line);
}

static void write_prefix(FILE *fd, struct log_data log_data)
{
	int i = 0;
	struct prefix_element pfx_elem;

	while (i < log_cfg.pfx_length) {
		pfx_elem = log_cfg.pfx_elem[i];
		if (pfx_elem.fn == NULL) {
			fprintf(fd, "%s", pfx_elem.fmt);
		} else {
			pfx_elem.fn(fd, log_data, pfx_elem.fmt);
		}
		i++;
	}
}

static void write_log(FILE *fd, struct log_data log_data, va_list args)
{
	write_prefix(fd, log_data);
	vfprintf(fd, log_data.fmt, args);
	fprintf(fd, "\n");
	fflush(fd);
}

static void lock(void)
{
	pthread_mutex_lock(&mutex);
}

static void unlock(void)
{
	pthread_mutex_unlock(&mutex);
}

static int open_log_file()
{
	int ret = LOG_SUCCESS;

	if (log_cfg.filename == NULL) {
		fprintf(stderr, "Log filename should not be NULL!\n");
		return LOG_FAILED;
	}

	lock();
	close_log_file();

	log_cfg.file = fopen(log_cfg.filename, "a+");

	if (log_cfg.file == NULL) {
		fprintf(stderr, "Can't open log file '%s'!\n", log_cfg.filename);
		ret = LOG_FAILED;
	}

	unlock();
	return ret;
}

static int close_log_file(void)
{
	if (log_cfg.file == NULL)
		return LOG_SUCCESS;

	if (fclose(log_cfg.file) != 0)
		return LOG_FAILED;

	log_cfg.file = NULL;
	return LOG_SUCCESS;
}

static char *get_bool_string(bool b)
{
	return (b) ? "true" : "false";
}

int init_log(void)
{
	init_mutex();
	if (init_prefix() != LOG_SUCCESS)
		return LOG_FAILED;

	if (log_cfg.filename != NULL)
		return open_log_file();
	else
		return LOG_SUCCESS;

}

int load_log_config_from_file(const char *log_config_file)
{
	if (log_config_file == NULL) {
		fprintf(stderr, "log_config_file cannot be NULL\n");
		return LOG_FAILED;
	}

	config_init(&cf);

	if (!config_read_file(&cf, log_config_file)) {
		//fprintf(stderr, "%s:%d - %s\n", config_error_file(cf), config_error_line(cf), config_error_text(cf));
		fprintf(stderr, "%s: %d - %s\n", log_config_file,
			config_error_line(&cf), config_error_text(&cf));
		config_destroy(&cf);
		return LOG_FAILED;
	}

	if (!config_lookup_int(&cf, "log_level", &(log_cfg.level)))
		log_cfg.level = log_cfg_default.level;

	if (!config_lookup_string(&cf, "log_filename", &log_cfg.filename))
		log_cfg.filename = log_cfg_default.filename;

	if (!config_lookup_string(&cf, "log_prefix_format", &(log_cfg.pfx_format)))
		log_cfg.pfx_format = log_cfg_default.pfx_format;

	if (!config_lookup_bool(&cf, "print_log", (int*)&(log_cfg.print)))
		log_cfg.print = log_cfg_default.print;

	return LOG_SUCCESS;
}

void set_log_level(int level)
{
	log_cfg.level = level;
}

void set_print_log(bool print)
{
	log_cfg.print = print;
}

void set_log_filename(const char *filename)
{
	if (log_cfg.file == NULL)
		log_cfg.filename = filename;
	else
		fprintf(stderr, "Please do free_log() before setting another filename\n");
}

void set_log_pfx_format(const char *format)
{
	if (log_cfg.pfx_elem == NULL)
		log_cfg.pfx_format = format;
	else
		fprintf(stderr, "Please do free_log() before setting another pfx_format\n");
}

void display_log_config(void)
{
	printf("======================\n");
	printf("C logger configuration\n");
	printf("======================\n");
	printf("Parameter | Default\t\t| Configured\n");
	printf("--------------------------------------------\n");
	printf("level     | %s\t\t| %s\n",
		get_log_level_string(log_cfg_default.level),
		get_log_level_string(log_cfg.level));
	printf("filename  | %s\t\t| %s\n", log_cfg_default.filename,
		log_cfg.filename);
	printf("format    | \"%s\"\t| \"%s\"\n", log_cfg_default.pfx_format,
		log_cfg.pfx_format);
	printf("print     | %s\t\t| %s\n",
		get_bool_string(log_cfg_default.print),
		get_bool_string(log_cfg.print));
	printf("pfx_length: %d\n", log_cfg.pfx_length);
}

int do_log(int level, const char *file, int line, const char *fmt, ...)
{
	char time_prfx[64];
	struct log_data log_data;
	va_list args_1, args_2;

	if (level > log_cfg.level)
		return LOG_SUCCESS;

	init_time_prfx(time_prfx, sizeof(time_prfx));
	log_data = init_log_data(time_prfx, level, file, line, fmt);

	lock();
	va_start(args_1, fmt);
	if (log_cfg.file != NULL)
		write_log(log_cfg.file, log_data, args_1);
	if (log_cfg.print) {
		va_start(args_2, fmt);
		write_log(get_print_fd(level), log_data, args_2);
		va_end(args_2);
	}
	va_end(args_1);
	unlock();

	return LOG_SUCCESS;
}

static void free_log_prefix(void)
{
	int i;

	if (log_cfg.pfx_elem == NULL)
		return;

	for (i = 0; i < log_cfg.pfx_length; i++)
		free(log_cfg.pfx_elem[i].fmt);

	free(log_cfg.pfx_elem);
	log_cfg.pfx_elem = NULL;
}

int free_log(void)
{
	free_log_prefix();
	config_destroy(&cf);
	return close_log_file();
}

