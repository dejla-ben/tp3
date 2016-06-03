#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define new(x) malloc(sizeof (x))

void error(bool b, const char* fmt, ...);

#endif

