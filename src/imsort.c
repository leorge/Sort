/*
 * imsort.c
 *
 *  Indirect mergesort
 *
 *  Created on: 2015/02/04
 *      Author: leo
 */

#include "sort.h"

static int  (*comp)(const void *, const void *);
static size_t boundary;

static void sort(void **dst, void **src, size_t nmemb) {
    assert(boundary);
    if (nmemb > boundary) {     // all of elements in src equal to that in dst.
#ifdef DEBUG
        qsort_called++;
        dump_pointer("sort() start in " __FILE__, src, nmemb);
#endif
        size_t n_lo = nmemb >> 1;   // = nmemb / 2
        size_t n_hi = nmemb - n_lo;
        sort(src, dst, n_lo);
        sort(&src[n_lo], &dst[n_lo], n_hi);
        // elements in src are sorted, and elements is dst is not sorted.
        void **left = src;
        void **right = &left[n_lo];
        void **store = dst;
#ifdef DEBUG
        if (trace_level >= TRACE_DUMP) {
            dump_pointer("left", left, n_lo);
            dump_pointer("right", right, nmemb - n_lo);
        }
#endif
        while (TRUE) {
            if (! n_lo) {
#ifdef DEBUG
                if (trace_level >= TRACE_DUMP) fprintf(OUT, "append %s\n", dump_data(*right));
#endif
                *store++ = *right++;
                if (! --n_hi) break;
            } else if (! n_hi) {
#ifdef DEBUG
                if (trace_level >= TRACE_DUMP) fprintf(OUT, "append %s\n", dump_data(*left));
#endif
                *store++ = *left++;
                if (! --n_lo) break;
            } else if (comp(*left, *right) <= 0) {
#ifdef DEBUG
                if (trace_level >= TRACE_DUMP) fprintf(OUT, "merge %s\n", dump_data(*left));
#endif
                *store++ = *left++;     // add one
                n_lo--;
            }
            else {
#ifdef DEBUG
                if (trace_level >= TRACE_DUMP) fprintf(OUT, "merge %s\n", dump_data(*right));
#endif
                *store++ = *right++;
                n_hi--;
            }
        }   // elements in dst are sorted.
    }
    if (small_func) small_func(dst, nmemb, comp);
#ifdef DEBUG
    dump_pointer("sort() done in " __FILE__ ".", dst, nmemb);
#endif
}

// indirect sorting

void imsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *)) {
    if (nmemb <= 1) return;
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT,
            "imsort(base=%p, nmemb=%s, size=%s, compare) start.\n",
            base, dump_size_t(NULL, nmemb), dump_size_t(NULL, size));
#endif
    void **src, **dst;
    if ((dst = calloc(nmemb, sizeof(void *)))) {
        if ((src = make_index(base, nmemb, size))) {
            memcpy(dst, src, sizeof(void *) * nmemb);
            comp = compare;
            boundary = small_boundary ? small_boundary : 1;
            sort(dst, src, nmemb);
            unindex(base, dst, nmemb, size);
            free(src); free(dst);
        }
        else {
            free(dst);
            char    buf[100];
            sprintf(buf, "calloc() error in indirect_msort(base, nmemb=%ld, size=%ld, compare)", nmemb, size);
            perror(buf);
            iqsort(base, nmemb, size, compare); // indirect asymmetric quicksort
        }
    } else asymm_qsort(base, nmemb, size, compare);
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) dump_array("imsort() done.", base, nmemb, 0, 0, size);
#endif
}
