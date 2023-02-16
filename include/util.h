#ifndef BRIDGE_UTIL_H
#define BRIDGE_UTIL_H

#include <stdio.h>

#define OK  (0)
#define ERR (-1)

/* To enable debug logs, compile with the -DDEBUG flag to define it */

#ifdef DEBUG
    #define debug(fmt, ...) printf("%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
    #define debug(fmt, ...)
#endif

#endif
