/*
 * asymm_qsort.c
 *
 *  The final Asymmetric Quicksort with debugging statements.
 *
 *  Created on: 2016/03/07
 *      Author: Takeuchi Leorge <qmisort@gmail.com>
 */
#include <math.h>
#include "sort.h"

#define MIDDLE      63      // Choose the middle element as a pivot when N <= MIDDLE
#define MEDIAN3     127     // Median-of-3
#define MEDIAN5     4095    // Median-of-5

static int      (*comp)(const void *, const void *);
static size_t   length;

static void copy(void *dst, const void *src)
{
#ifdef  DEBUG
    dump_copy(dst, src);
#endif
    memcpy(dst, src, length);
}

static void sort(void *base, size_t nmemb) {
#ifdef  DEBUG
    void *init_base = base;
    size_t init_nmemb = nmemb;
    if (init_nmemb > 1) dump_array("sort() start in " __FILE__, init_base, init_nmemb, 0, 0, length);
#endif
    while (nmemb > 1) {
#ifdef DEBUG
        qsort_called++;
        dump_array("sort() partition start.", base, nmemb, 0, 0, length);
#endif
        char *hole, *first = (char *)base, *last = first + (nmemb - 1) * length;
        // choose a pivot element
        if (nmemb <= MIDDLE) {  // middle element
            hole = first + (nmemb >> 1) * length;
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP)
                fprintf(OUT, "Pivot is at the middle of %s elements : %s at %ld\n",
                        dump_size_t(NULL, nmemb), dump_data(hole), (hole - first) / length);
#endif
        } else if (nmemb <= MEDIAN3) {  // conventional median-of-3
            char *middle = first + (nmemb >> 1) * length;
            hole = (comp(first, last)   < 0 ?
                   (comp(middle, first) < 0 ? first: (comp(middle, last)  < 0 ? middle : last)) :
                   (comp(middle, last)  < 0 ? last:  (comp(middle, first) < 0 ? middle : first)));
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP) fprintf(OUT,
                "Median of 3 in %s from %s %s %s --> %s\n",
                dump_size_t(NULL, nmemb), dump_data(first), dump_data(middle), dump_data(last), dump_data(hole));
#endif
        } else if (nmemb <= MEDIAN5) {  // median-of-5
            char *p1, *p2, *p3, *p4, *p5, *tmp;
            p1 = (char *)base + (((nmemb >> 2) * rand()) / ((size_t)RAND_MAX + 1)) * length;    // [0, N/4)
            size_t distance = ((nmemb >> 3) + (nmemb >> 4)) * length;   // N/8 + N/16 = 3N/16
            p5 = (p4 = (p3 = (p2 = p1 + distance) + distance) + distance) + distance;   // [3N/16, N)
            // You can rewrite below with a plenty of ternary operators ?:.
            if (comp(p2, p4) > 0) {tmp = p2; p2 = p4; p4 = tmp;}    // p2 <--> P4 then *p2 < *p4
            if (comp(p3, p2) < 0) {tmp = p2; p2 = p3; p3 = tmp;}    // p3 <--> p2 then *p2 < *p3 < *p4
            else if (comp(p4, p3) < 0) {tmp = p4; p4 = p3; p3 = tmp;}   // p4 <--> p3 then *p2 < *p3 < *p4
            if (comp(p1, p5) > 0) {tmp = p1; p1 = p5; p5 = tmp;}    // p1 <--> p5 then *p1 < *p5
            hole =  comp(p3, p1) < 0 ? (comp(p1, p4) < 0 ? p1 : p4)
                 : (comp(p5, p3) < 0 ? (comp(p5, p2) < 0 ? p2 : p5) : p3);
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP) fprintf(OUT, "Median of 5 in %s from %s %s %s %s %s --> %s\n",
                dump_size_t(NULL, nmemb), dump_data(p1), dump_data(p2), dump_data(p3),
                dump_data(p4), dump_data(p5), dump_data(hole));
#endif
        } else {    // median-of log2(sqrt(N))|1 random elements
            size_t  pickup = ((size_t)log2(nmemb) >> 1) | 1;        // number of elements
            void *index[pickup];
            char *p = first + (nmemb * rand() / ((size_t)RAND_MAX + 1) / pickup) * length;
            size_t  distance = (size_t)(nmemb / pickup) * length;   // distance of elements
            for (size_t idx = 0; idx < pickup; p += distance) index[idx++] = p;    // build an index
            void **left = index, **right = &index[pickup -1], **center = &index[pickup >> 1];
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP) dump_pointer("Picked up array", left, right - left + 1);
#endif
            while (left < right) {  // search a pointer to the middle element
                void **phole = &left[(right - left) >> 1];  // hole in the index
                char *pivot = *phole;       // save the middle pointer
                *phole = *right;    // move the last pointer to the middle of index
                phole = right;      // dig a hole at the last of index
                void **plo = left, **phi = right - 1;
                for (int chk; plo < phole; plo++) {
                    if ((chk = comp(*plo, pivot)) >= 0) {
                        *phole = *plo; phole = plo;
                        for (; phi > phole; phi--) {
                            if ((chk = comp(*phi, pivot)) < 0) {
                                *phole = *phi; phole = phi;
                            }
                        }
                    }
                }
                *phole = pivot;     //restore
                if (center < phole) right = phole - 1;
                else if (phole < center) left = phole + 1;
                else break;         // phole == center
            }
            hole = *center;     // hole is in the middle of index[]
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP) fprintf(OUT,"Median of logarithmic --> %s\n", dump_data(*center));
#endif
        }
        // partition
        char save[length];
#ifdef  DEBUG
        if (trace_level >= TRACE_DUMP) fprintf(OUT, "pivot <-- hole = %s [%s] <-- last = %s\n"
                , dump_data(hole), dump_size_t(NULL, (hole - first) / length), dump_data(last));
#endif
        copy(save, hole); copy(hole, last);    // save <-- hole <-- last
        char *lo = first,  *hi = (hole = last) - length, *eq = NULL;
        for (int chk; lo < hole; lo += length) {
            if ((chk = comp(lo, save)) >= 0) {
                if (chk > 0) eq = NULL; // discontinued
                else if (eq == NULL) eq = hole;
#ifdef  DEBUG
                if (trace_level >= TRACE_DUMP) fprintf(OUT, "move %s --> %s\n", dump_data(lo), dump_data(hole));
#endif
                copy(hole, lo); hole = lo;
                for (; hi > hole; hi -= length) {
                    int chk;
                    if ((chk = comp(hi, save)) < 0) {
#ifdef  DEBUG
                        if (trace_level >= TRACE_DUMP) fprintf(OUT, "move %s <-- %s\n", dump_data(hole), dump_data(hi));
#endif
                        copy(hole, hi); hole = hi;
                    }
                    else if (chk > 0) eq = NULL;
                    else if (eq == NULL) eq = hi;   // first equal element
                }
            }
        }
        if (eq == NULL) eq = hole;
#ifdef  DEBUG
        if (trace_level >= TRACE_DUMP) fprintf(OUT, "restore pivot %s to %s [%s]\n",
                dump_data(save), dump_data(hole), dump_size_t(NULL, (hole - first) / length));
#endif
        copy(hole, save);  // restore

        // sort sub-arrays recursively and iteratively.
        size_t  n_lo = (hole - first) / length; // number of element in lower partition
        size_t  n_hi = (last - eq) / length;
#ifdef DEBUG
        dump_array("sort() partitioned.", base, n_lo, (eq - hole) / length + 1, n_hi, length);
        dump_rate(n_lo, n_hi);
#endif
        if (n_lo < n_hi) {
            sort(base, n_lo);  // sort shorter sub-array
            nmemb = n_hi; base = eq + length;
        } else {
            sort(eq + length, n_hi);
            nmemb = n_lo;
        }
    }
#ifdef DEBUG
    if (init_nmemb > 1) dump_array("sort() done.", init_base, init_nmemb, 0, 0, length);
#endif
}

void asymm_qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *))
{
    assert(base != NULL);
    assert(compare != NULL);
    length = size; comp = compare;
    sort(base, nmemb);
}
