/*
 * Just include stdint instead.. it's good enough for us.
 */
#ifndef INTTYPES_H
#define INTTYPES_H
#ifndef HAVE_STDINT_h
#include "win_stdint.h"
#else
#include <stdint.h>
#endif

#endif
