# C logger

## Description
Simple C logger library.

## Platform
Linux only (C99).

## Features
* Can be used as is
* Can be used as shared lib
* Thread safe
* Customizable log format

**Warning: A lot of functions are not signal safe!**

## Usage
### Case 1: Use lib sources directly
**[liblog.c](src/liblog.c?raw=1)** and **[liblog.h](src/liblog.h?raw=1)**
should be used into an existing project and compiled with it.

### Case 2: Use compiled shared lib
Build and install:
```bash
make && make install
```
Then just add this include in your project:
```c
#include <liblog.h>
```

### Build and run example
Source: [example.c](src/example.c?raw=1)
```c
cd src
make
./logtest
```

### Logging functions
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

## Log initialization

### Configuration file
We can use a configuration file
for the log initialization (see **[liblog.conf.sample](src/liblog.conf.sample?raw=1)**).
### Functions
```c
int load_log_config_from_file(const char *log_config_file);
```
Load log configuration from file. **Should be called before init_log().**
```c
int init_log(void);
```
Initialize the log by:
* Init mutex
* Init the prefix format
* Open log file

**Should be called before any logging function.**
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
Sets the filename to be opened by init_log(). **Should be called before init_log().**

```c
void set_log_pfx_format(const char *pfx_format);
```
Sets the prefix format that will be printed before each log.
For ex. line number, source filename, date, etc.
More explanations in the file [liblog.conf.sample](src/liblog.conf.sample?raw=1).
**Should be called before init_log().**

```c
void display_log_config(void);
```
Display the log config to stdout.

```c
int free_log(void);
```
Close log file then free configuration. **Should be called at the end of the program.**

### Return codes
```c
#define LOG_SUCCESS 0
#define LOG_FAILED -1
```

### Examples

#### With configuration file
```c
load_log_config_from_file("/opt/my_app/log.conf");
init_log();

// Do stuff...

free_log();
```

#### Without configuration file
```c
set_log_filename("log.txt");
set_print_log(true);
set_log_level(LOG_INFO);
init_log();

// Do stuff...

free_log();
```

