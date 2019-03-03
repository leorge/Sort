/*
 * timer.c
 *
 *  Estimate time in microseconds
 *
 *  Created on: Feb 18, 2018
 *      Author: leo
 */
#include    <sys/resource.h>
#include    <sys/time.h>
#include    <time.h>
#include    "sort.h"

#define CLOCK_TYPE  CLOCK_PROCESS_CPUTIME_ID

static struct timeval   when;
static struct rusage    usage;
static struct timespec  spec;

void start_timer() {
    clock_getres(CLOCK_TYPE, &spec);    // _POSIX_C_SOURCE >= 199309L
    clock_gettime(CLOCK_TYPE, &spec);
    getrusage(RUSAGE_SELF, &usage);
    gettimeofday(&when, NULL);
}

long stop_timer() {
    // get data
    struct timeval thistime; gettimeofday(&thistime, NULL);
    struct rusage used; getrusage(RUSAGE_SELF, &used);
    struct timespec cpu_to;
    clock_getres(CLOCK_TYPE, &cpu_to);
    clock_gettime(CLOCK_TYPE, &cpu_to);
    // calculate elapsed time in micro seconds
    long elapsedtime = (thistime.tv_sec - when.tv_sec) * 1000000 +  (thistime.tv_usec - when.tv_usec);
    long utime = (used.ru_utime.tv_sec - usage.ru_utime.tv_sec) * 1000000 + (used.ru_utime.tv_usec - usage.ru_utime.tv_usec);
    long process_time = (cpu_to.tv_sec - spec.tv_sec) * 1000000 + (cpu_to.tv_nsec - spec.tv_nsec) / 1000;
    if (getenv("DumpElapsedTime") != NULL) {
        fprintf(ERR, "gettimeofday()  --> %ld\n", elapsedtime);
        fprintf(ERR, "getrusage()     --> %ld\n", utime);
        fprintf(ERR, "clock_gettime() --> %ld\n", process_time);
        fprintf(ERR, "\n");
    }
    return  process_time;
}
