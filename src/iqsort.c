/*
 * indirect_qsort.c
 *
 *  Indirect asymmetric quicksort
 *
 *  Created on: 2015/09/08
 *      Author: leo
 */

#include "sort.h"
#include <math.h>

static int  (*comp)(const void *, const void *) = NULL;
static size_t boundary = 1;
static size_t mid1 = 63, med3 = 255, med5 = 4095; // Threshold to change a choosing pivot

static void sort(void **base, size_t nmemb) {
    assert(comp != NULL);
    assert(boundary);
#ifdef DEBUG
    void *init_base = base;
    size_t init_nmemb = nmemb;
    int CHK;
#define SIDE(a) CHK = comp((*a), *hole); if (CHK < 0) LEFT++; else if (CHK > 0) RIGHT++;
    if (init_nmemb > 1) dump_pointer("sort() start in " __FILE__, init_base, init_nmemb);
#endif
    while (nmemb > boundary) {
#ifdef DEBUG
        qsort_called++;
        dump_pointer("sort() partition start", base, nmemb);
#endif
        void **hole = NULL, **first = base, **last = first + (nmemb - 1), **middle;
        if (nmemb <= mid1) {  // middle element
            hole = first + (nmemb >> 1);
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP)
                fprintf(OUT, "Pivot is the middle of %s elements --> %s\n",
                        dump_size_t(NULL, nmemb), dump_data(*hole));
#endif
        } else if (nmemb <= med3) {  // median-of-3
            middle = first + (nmemb >> 1);
            hole = comp(*first , *last ) < 0 ?
                  (comp(*middle, *first) < 0 ? first : (comp(*middle, *last ) < 0 ? middle : last )):
                  (comp(*middle, *last)  < 0 ? last  : (comp(*middle, *first) < 0 ? middle : first));
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP) fprintf(OUT,
                "Pivot is the median-of-3 in %s elements : %s %s %s --> %s\n",
                dump_size_t(NULL, nmemb), dump_data(*first), dump_data(*middle), dump_data(*last), dump_data(*hole));
            int LEFT = 0, RIGHT=0;
            SIDE(first); SIDE(middle); SIDE(last);
            assert(LEFT < 2 && RIGHT < 2);
#endif
        } else if (nmemb <= med5) {  // median-of-5
            void **p1, **p2, **p3, **p4, **p5, **tmp;
            p1 = base + (((nmemb >> 2) * rand()) / ((size_t)RAND_MAX + 1)); // [0, N/4)
            size_t distance = ((nmemb >> 3) + (nmemb >> 4));    // N/8 + N/16 = 3N/16
            p5 = (p4 = (p3 = (p2 = p1 + distance) + distance) + distance) + distance;   // [3N/16, N)
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP) fprintf(OUT,
                "Pivot is the median-of-5 in %s elements : %s %s %s %s %s", dump_size_t(NULL, nmemb)
                , dump_data(*p1), dump_data(*p2), dump_data(*p3), dump_data(*p4), dump_data(*p5));
#endif
            // You can rewrite below with a plenty of ternary operators ?:.
            if (comp(*p2, *p4) > 0) {tmp = p2; p2 = p4; p4 = tmp;}  // p2 <--> P4 then *p2 < *p4
            if (comp(*p3, *p2) < 0) {tmp = p2; p2 = p3; p3 = tmp;}  // p3 <--> p2 then *p2 < *p3 < *p4
            else if (comp(*p4, *p3) < 0) {tmp = p4; p4 = p3; p3 = tmp;} // p4 <--> p3 then *p2 < *p3 < *p4
            if (comp(*p1, *p5) > 0) {tmp = p1; p1 = p5; p5 = tmp;}  // p1 <--> p5 then *p1 < *p5
            hole =  comp(*p3, *p1) < 0 ? (comp(*p1, *p4) < 0 ? p1 : p4)
                 : (comp(*p5, *p3) < 0 ? (comp(*p5, *p2) < 0 ? p2 : p5) : p3);
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP) fprintf(OUT, " --> %s\n", dump_data(*hole));
            int LEFT = 0, RIGHT=0;
            SIDE(p1); SIDE(p2); SIDE(p3); SIDE(p4); SIDE(p5);
            assert(LEFT < 3 && RIGHT < 3);
#endif
        } else {    // median-of log2(sqrt(N))|1 random elements
            size_t idx, pickup = ((size_t)log2(nmemb) >> 1) | 1;    // number of elements
            void **index[pickup];
            void **p = first + (nmemb * rand() / ((size_t)RAND_MAX + 1) / pickup);
            size_t  distance = (size_t)(nmemb / pickup);    // distance of elements
            for (idx = 0; idx < pickup; p += distance) index[idx++] = p;    // build an index
            void ***left = index, ***right = &index[pickup - 1], ***center = &index[pickup >> 1];
#ifdef  DEBUG
            if (trace_level >= TRACE_DUMP) {
                fprintf(OUT, "Picked up array :");
                for (idx = 0; idx < pickup;) fprintf(OUT, " %s", dump_data(*index[idx++]));
                fprintf(OUT, "\n");
            }
#endif
            while (left < right) {  // search a pointer to the middle element
                void ***phole = &left[(right - left) >> 1];  // hole in the index
                void **pivot = *phole;  // save the middle pointer
                *phole = *right;    // move the last pointer to the middle of index
                phole = right;      // dig a hole at the last of index
                void ***plo = left, ***phi = right - 1;
                for (int chk; plo < phole; plo++) {
                    if ((chk = comp(**plo, *pivot)) >= 0) {
                        *phole = *plo; phole = plo;
                        for (; phi > phole; phi--) {
                            if ((chk = comp(**phi, *pivot)) < 0) {
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
            hole = *center; // hole is in the middle of index[]
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP) fprintf(OUT,
                "Pivot is the median-of-logarithmic in %s elements --> %s\n", dump_size_t(NULL, nmemb), dump_data(**center));
            int LEFT = 0, RIGHT=0;
            for (idx = 0; idx < pickup; idx++) {
                CHK = comp(*index[idx], *hole);
                if (CHK < 0) LEFT++; else if (CHK > 0) RIGHT++;
            }
            idx = pickup >> 1;
            assert(LEFT <= idx && RIGHT <= idx);
#endif
        }
        assert(hole != NULL);
        void *pivot = *hole;        // save a pivot
#ifdef  DEBUG
        if (trace_level >= TRACE_DUMP) fprintf(OUT, "pivot = %s [%s]\n"
            , dump_data(pivot), dump_size_t(NULL, hole - first));
#endif
        *hole = *last; hole = last; // move the last element to the middle position.
        void **lo = first, **hi = last - 1, **eq = NULL;
        for (int chk; lo < hole; lo++) {
            if ((chk = comp(*lo, pivot)) >= 0) {
#ifdef  DEBUG
                if (trace_level >= TRACE_DUMP) fprintf(OUT, "move %s --> %s\n", dump_data(*lo), dump_data(*hole));
#endif
                if (chk > 0) eq = NULL;
                else if (eq == NULL) eq = hole;
                *hole = *lo; hole = lo;
                for (; hi > hole; hi--) {
                    if ((chk = comp(*hi, pivot)) < 0) {  // symmetric comparison
#ifdef  DEBUG
                        if (trace_level >= TRACE_DUMP) fprintf(OUT, "move %s <-- %s\n", dump_data(*hole), dump_data(*hi));
#endif
                        *hole = *hi; hole = hi; eq = NULL;
                    }
                    else if (chk > 0) eq = NULL;
                    else if (eq == NULL) eq = hi;   // first equal element
                }
            }
        }
#ifdef  DEBUG
        if (trace_level >= TRACE_DUMP) fprintf(OUT, "restore pivot %s to %s [%s]\n",
                dump_data(pivot), dump_data(*hole), dump_size_t(NULL, hole - first));
#endif
        *hole = pivot;  // restore
#ifdef DEBUG
        dump_pointer("sort() partitioned", base, nmemb);
#endif
        if (eq == NULL) eq = hole;
#ifdef DEBUG
        else if (trace_level >= TRACE_DUMP)
            fprintf(OUT, "skip higher %s elements\n", dump_size_t(NULL, eq - hole));
#endif
        size_t  n_lo = hole - first; // number of element in lower partition
        size_t  n_hi = last - eq;
#ifdef DEBUG
        dump_rate(n_lo, n_hi);
#endif
        if (n_lo < n_hi) {
            sort(first, n_lo);
            base = eq + 1; nmemb = n_hi;
        }
        else {
            sort(eq + 1, n_hi);
            nmemb = n_lo;
        }
    }
    if (small_func) (*small_func)(base, nmemb, comp);
#ifdef DEBUG
    dump_pointer("sort() done in " __FILE__ ".", init_base, init_nmemb);
#endif
}

// indirect sorting

void iqsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *)) {
    if (nmemb <= 1) return;
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT,
            "iqsort(base=%p, nmemb=%s, compare) start.\n", base, dump_size_t(NULL, nmemb));
#endif
    void **work;
    if ((work = make_index(base, nmemb, size))) {
        comp = compare;
        if (small_boundary) boundary = small_boundary;
        if (middle1) mid1 = middle1;
        if (median3) med3 = median3;
        if (median5) med5 = median5;
        sort(work, nmemb);
        unindex(base, work, nmemb, size);
        free(work);
    }
    else asymm_qsort(base, nmemb, size, compare);
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) dump_array("iqsort() done.", base, nmemb, 0, 0, size);
#endif
}
