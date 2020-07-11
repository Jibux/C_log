#include "log.h"


static int close_log_file(void)
{
	if (log_config.file == NULL)
		return LOG_SUCCESS;

	if (fclose(log_config.file) != 0)
		return LOG_FAILED;

	log_config.file = NULL;
	return LOG_SUCCESS;
}

static FILE *get_print_fd(int level)
{
	return (level >= LOG_ERROR) ? stderr : stdout;
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

static void init_time_prfx(char *time_prfx, size_t length)
{
	struct tm *time;
	long usec;

	init_time(&time, &usec);
	strftime(time_prfx, length, "%Y-%m-%d %H:%M:%S", time);
	sprintf(&time_prfx[19], ".%06ld", usec);
}

static struct s_log_data init_log_data(const char *time_prfx, int level, const char *file, int line, const char *fmt)
{
	struct s_log_data log_data;
	log_data.time_prfx = time_prfx;
	log_data.level = level;
	log_data.file = file;
	log_data.line = line;
	log_data.fmt = fmt;

	return log_data;
}

static void write_log(FILE *fd, struct s_log_data log_data, va_list args)
{
	fprintf(fd, "%s %-5s %s:%d: ", log_data.time_prfx,
		get_log_level_string(log_data.level), log_data.file,
		log_data.line);
	if (log_config.prefix != NULL)
		fprintf(fd, "= %s = ", log_config.prefix);
	vfprintf(fd, log_data.fmt, args);
	fprintf(fd, "\n");
	fflush(fd);
}

static void init_mutex(void)
{
	pthread_mutex_init(&s_mutex, NULL);
}

static void lock(void)
{
	pthread_mutex_lock(&s_mutex);
}

static void unlock(void)
{
	pthread_mutex_unlock(&s_mutex);
}

static int open_log_file()
{
	int ret = LOG_SUCCESS;

	if (log_config.filename == NULL) {
		fprintf(stderr, "Log filename should not be NULL!\n");
		return LOG_FAILED;
	}

	lock();
	close_log_file();

	log_config.file = fopen(log_config.filename, "a+");

	if (log_config.file == NULL) {
		fprintf(stderr, "Can't open log file '%s'!\n", log_config.filename);
		ret = LOG_FAILED;
	}

	unlock();
	return ret;
}

static char *get_bool_string(bool b)
{
	return (b) ? "true" : "false";
}

int init_log(void)
{
	init_mutex();
	if (log_config.filename != NULL)
		return open_log_file();
	else
		return LOG_SUCCESS;

}

int load_log_config_from_file(const char *log_config_file)
{
	if (log_config_file == NULL)
		log_config_file = LOG_CONFIG_FILE_DEFAULT;

	config_init(&cf);

	if (!config_read_file(&cf, log_config_file)) {
		//fprintf(stderr, "%s:%d - %s\n", config_error_file(cf), config_error_line(cf), config_error_text(cf));
		fprintf(stderr, "%s: %d - %s\n", log_config_file,
			config_error_line(&cf), config_error_text(&cf));
		config_destroy(&cf);
		return LOG_FAILED;
	}

	if (!config_lookup_int(&cf, "log_level", &(log_config.level)))
		log_config.level = log_config_default.level;

	if (!config_lookup_string(&cf, "log_filename", &log_config.filename))
		log_config.filename = log_config_default.filename;

	if (!config_lookup_string(&cf, "log_prefix", &(log_config.prefix)))
		log_config.prefix = log_config_default.prefix;

	if (!config_lookup_bool(&cf, "print_log", (int*)&(log_config.print)))
		log_config.print = log_config_default.print;

	return LOG_SUCCESS;
}

void set_log_level(int level)
{
	log_config.level = level;
}

void set_print_log(bool print)
{
	log_config.print = print;
}

void set_log_filename(const char *filename)
{
	if (log_config.file == NULL)
		log_config.filename = filename;
	else
		fprintf(stderr, "Please do free_log() before setting another filename\n");
}

void set_log_prefix(const char *prefix)
{
	log_config.prefix = prefix;
}

void display_log_config(void)
{
	printf("======================\n");
	printf("C logger configuration\n");
	printf("======================\n");
	printf("Parameter | Default\t| Configured\n");
	printf("------------------------------------\n");
	printf("level     | %s\t| %s\n",
		get_log_level_string(log_config_default.level),
		get_log_level_string(log_config.level));
	printf("filename  | %s\t| %s\n", log_config_default.filename,
		log_config.filename);
	printf("prefix    | %s\t| %s\n", log_config_default.prefix,
		log_config.prefix);
	printf("print     | %s\t| %s\n",
		get_bool_string(log_config_default.print),
		get_bool_string(log_config.print));
}

int do_log(int level, const char *file, int line, const char *fmt, ...)
{
	char time_prfx[64];
	struct s_log_data log_data;
	va_list args_1, args_2;

	if (level > log_config.level)
		return LOG_SUCCESS;

	init_time_prfx(time_prfx, sizeof(time_prfx));
	log_data = init_log_data(time_prfx, level, file, line, fmt);

	lock();
	va_start(args_1, fmt);
	if (log_config.file != NULL)
		write_log(log_config.file, log_data, args_1);
	if (log_config.print) {
		va_start(args_2, fmt);
		write_log(get_print_fd(level), log_data, args_2);
		va_end(args_2);
	}
	va_end(args_1);
	unlock();

	return LOG_SUCCESS;
}

int free_log(void)
{
	config_destroy(&cf);

	return close_log_file();
}

