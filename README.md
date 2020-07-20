# C logger

## Description
Simple C logger library.

## Platform
Linux only (C99).

## Usage
**[log.c](src/log.c?raw=1)** and **[log.h](src/log.h?raw=1)** should be used
into an existing project and compiled with it. We can use a configuration file
for the log initialization (see **[log.conf.sample](src/log.conf.sample?raw=1)**).
The library provides 6 function-like macros for logging:

```c
log_trace(const char *fmt, ...);
log_debug(const char *fmt, ...);
log_info(const char *fmt, ...);
log_warn(const char *fmt, ...);
log_error(const char *fmt, ...);
log_fatal(const char *fmt, ...);
```

Each function takes a printf format string followed by additional arguments:

```c
log_trace("Hello %s", "world");
```

### Initialization examples

#### With configuration file
```c
load_log_config_from_file("/opt/my_app/log.conf");
init_log();
```

#### Without configuration file
```c
set_log_filename("log.txt");
set_print_log(true);
set_log_level(LOG_INFO);
init_log();
```

### All initialization functions

```c
int init_log(void);
```
Initialize the log by:
* Init mutex
* Init the prefix format
* Open log file

```c
void set_log_level(int level);
```
Should be "LOG_FATAL", "LOG_ERROR", "LOG_WARN", "LOG_INFO", "LOG_DEBUG", "LOG_TRACE".

```c
void set_print_log(bool print);
```
If 'true', the log will be printed to stdout or stderr (for error and fatal levels).

```c
void set_log_filename(const char *filename);
```
Sets the filename to be opened by init_log().

```c
void set_log_pfx_format(const char *pfx_format);
```
Sets the prefix format that will be printed before each log.
For ex. line number, source filename, date, etc.
More explanations in the file [log.conf.sample](src/log.conf.sample?raw=1).

```c
void display_log_config(void);
```
Display the log config to stdout.

### Close log / free
```c
int free_log(void);
```

