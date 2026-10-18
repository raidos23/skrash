#ifndef SKRASH_TIME_UTIL_H
#define SKRASH_TIME_UTIL_H
#include <stddef.h>
#include <sys/time.h>

static inline double sb3_now_seconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

#endif /* SKRASH_TIME_UTIL_H */