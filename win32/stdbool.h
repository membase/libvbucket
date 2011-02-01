/*
 * Small little wrapper script due to the fact that we don't have a stddef.h
 * by default when using msdev
 */
#ifndef STDBOOL_H
#define STDBOOL_H

typedef char bool;
#define false 0
#define true 1

#endif
