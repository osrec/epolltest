#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include "fserveTypes.h"

#define DEBUG 1

#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stdout, "%s:%d:%s(): " fmt "\n", __FILE__, \
                                __LINE__, __func__, ##__VA_ARGS__); } while (0)


void exitWithError(char *errMsg);

int createSocketAndBind (char *port);

int makeSocketNonBlocking (int sfd);

int writeAll(int fd, char *buf, int len);

int readAll(int fd);


void free_FSConnection(FSConnection * conn);

void free_FSEvent(FSEvent * event);

void free_FSTask(FSTask * task);

/*
 
#include <math.h>
#define toString(var, buffer) _Generic((var), \
							int: print_int, \
							long: print_long, \
							unsigned int: print_unsigned_int, \
							unsigned long: print_unsigned_long, \
                            float: print_float)(var, buffer)

#define GENERATE_TO_STRING_FUNCTION(NAME, TYPE, SPECIFIER_STR) \
		void print_##NAME(TYPE x, char * buffer) { \
			int size = log10(x) + 1; \
			snprintf(buffer, size+3, SPECIFIER_STR, x); }

GENERATE_TO_STRING_FUNCTION(int, int, "FD%d");
GENERATE_TO_STRING_FUNCTION(long, long, "FD%ld");
GENERATE_TO_STRING_FUNCTION(unsigned_int, unsigned int, "FD%u");
GENERATE_TO_STRING_FUNCTION(unsigned_long, unsigned long, "FD%lu");
GENERATE_TO_STRING_FUNCTION(float, float, "FD%f");

*/

#endif //__UTILS__
