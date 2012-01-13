#ifndef TESTS_MACROS_H
#define TESTS_MACROS_H

#define assert(expr)                                        \
    do {                                                    \
        if (!(expr)) {                                      \
            fprintf(stderr, "%s:%d: assertion failed\n",    \
                    __FILE__, __LINE__);                    \
            fflush(stderr);                                 \
            abort();                                        \
        }                                                   \
    }  while (0)

#endif
