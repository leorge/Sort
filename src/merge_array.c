/*
 * merge_array.c
 *
 *  Entire merge sort.
 *
 *  Created on: 2013/01/11
 *      Author: leo
 */

#include "sort.h"

static int  (*comp)(const void *, const void *);
static size_t   length;

/* array_sort */
static void copy(void *dst, const void *src, size_t size)
{
#ifdef  DEBUG
    qsort_moved++;
#endif
    memcpy(dst, src, size * length); /* restore an elements  */
}

static void sort(void *dst, void *src, size_t nmemb) {
    if (nmemb <= 1) return;
#ifdef DEBUG
    qsort_called++;
    if (trace_level >= TRACE_DUMP) dump_array("sort() start in " __FILE__, src, nmemb, 0, 0, length);
#endif
    size_t n_lo = nmemb >> 1;   // = nmemb / 2
    size_t n_hi = nmemb - n_lo;
    size_t lo_length = n_lo * length;
    sort(src, dst, n_lo);
    sort((char *)src + lo_length, (char *)dst + lo_length, n_hi);
    char *left = src;
    char *right = &left[lo_length];
    char *store = dst;
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) dump_array("merge sub-arrays", left, n_lo, 0, n_hi, length);
#endif
    while (TRUE) {
        if (comp(left, right) <= 0) {
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP) dump_array("merge left", left, 1, 0, 0, length);
#endif
            copy(store, left, 1); store += length; left += length;      // add one
            if (--n_lo <= 0) {  // empty?
#ifdef DEBUG
                if (trace_level >= TRACE_DUMP) dump_array("append right", right, n_hi, 0, 0, length);
#endif
                copy(store, right, n_hi);   // append remained data
                break;
            }
        }
        else {
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP) dump_array("merge right", right, 1, 0, 0, length);
#endif
            copy(store, right, 1); store += length; right += length;
            if (--n_hi <= 0) {
#ifdef DEBUG
                if (trace_level >= TRACE_DUMP) dump_array("append left", left, n_lo, 0, 0, length);
#endif
                copy(store, left, n_lo);
                break;
            }
        }
    }
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) dump_array("sort() done.", dst, nmemb, 0, 0, length);
#endif
}

void merge_array(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *)) {
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT, "merge_array() nmemb = %s\n", dump_size_t(NULL, nmemb));
#endif
    if (nmemb <= 1) return;
    void *dup = calloc(nmemb, size);
    if (dup != NULL) {
        length = size; comp = compare;
        memcpy(dup, base, nmemb * size);
        sort(base, dup, nmemb);
        free(dup);
    }
}
