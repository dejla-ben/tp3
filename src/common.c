#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"


void
error (bool b, const char *fmt, ...)
{
  va_list valist;

  va_start (valist, fmt);
  if (b)
    {
      vfprintf (stderr, fmt, valist);
      exit (EXIT_FAILURE);
    }

  va_end (valist);
}
