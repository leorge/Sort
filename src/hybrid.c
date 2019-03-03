/*
 * hybrid.c
 *
 *  Hybrid sort with Asymmetric Quicksort and another.
 *  A pivot is the median of Logarithmic.
 *
 *  Created on: 2013/03/23
 *      Author: leo
 */
#include <math.h>
#include "sort.h"

static int      (*comp)(const void *, const void *);
static size_t   length;

static void copy(void *dst, const void *src)
{
#ifdef  DEBUG
    qsort_moved++;
#endif
    memcpy(dst, src, length);
}

static void sort(void *base, size_t nmemb) {
#ifdef  DEBUG
    void *init_base = base;
    size_t init_nmemb = nmemb;
#endif
    while (nmemb > medium_boundary) {
#ifdef DEBUG
        dump_array("sort() start in " __FILE__, base, nmemb, 0, 0, length);
        qsort_called++;
#endif
        char *first = (char *)base, *last = first + (nmemb - 1) * length;
        char *hole = pivot_array(first, nmemb, length, ((size_t)log2(nmemb) >> 1) | 1, comp);
        char pivot[length]; copy(pivot, hole); copy(hole, last);    // pivot <-- hole <-- last
        char *lo = first,  *hi = (hole = last) - length, *eq = NULL;
        for (int chk; lo < hole; lo += length) {
            if ((chk = comp(lo, pivot)) >= 0) {
                if (chk > 0) eq = NULL; // discontinued
                else if (eq == NULL) eq = hole;
                copy(hole, lo); hole = lo;
                for (; hi > hole; hi -= length) {
                    if ((chk =comp(hi, pivot)) < 0) {
                        copy(hole, hi); hole = hi;
                    }
                    else if (chk > 0) eq = NULL;
                    else if (eq == NULL) eq = hi;   // first equal element
                }
            }
        }
        copy(hole, pivot);  // restore
        if (eq == NULL) eq = hole;
        // sort sub-arrays iterative and recursive.
        size_t  n_lo = (hole - first) / length; // number of element in lower partition
        size_t  n_hi = (last - eq) / length;
        if (n_lo < n_hi) {
            sort(base, n_lo);  // sort shorter sub-array
            base = eq + length; nmemb = n_hi;
        } else {
            sort(eq + length, n_hi);
            nmemb = n_lo;
        }
    }
    if (medium_func) (*medium_func)(base, nmemb, length, comp);
#ifdef DEBUG
    if (init_nmemb > 1) dump_array("sort() done in " __FILE__, init_base, init_nmemb, 0, 0, length);
#endif
}

void hybrid(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *))
{
    if (nmemb <= 1) return;
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT,
            "hybrid(base=%p, nmemb=%s, size=%s, compare) start.\n",
            base, dump_size_t(NULL, nmemb), dump_size_t(NULL, size));
#endif
    length = size; comp = compare;
    sort(base, nmemb);
}
