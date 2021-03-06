/*
 * heap_sort.c
 *
 *  Heap sort
 *
 *  Created on: 2013/06/06
 *      Author: leo
 */

#include "sort.h"

static int  (*comp)(const void *, const void *);
static void **root;

#ifdef DEBUG
static void dump_heap(void **base, size_t nmemb) {
    if (trace_level >= TRACE_DUMP) {
        char    buf[100];
        sprintf(buf, "Heap is built.  compare = %ld", qsort_comp_str);
        dump_pointer(buf, base, nmemb);
    }
}
#endif

static void heap(size_t nmemb, size_t node)
{
#ifdef DEBUG
    dump_pointer("heap() start", root, nmemb);
    qsort_called++;
#endif
    void    **child, **start = root + node, **parent = start;
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT, "node = %s\tstart = %s\n", dump_size_t(NULL, node), dump_data(*start));
#endif
    void    *key = *start;  // save
    size_t  leaf;
    while(TRUE) {
        leaf = (node << 1) + 1;         // 2n + 1
        if (leaf >= nmemb) break;       // no child
        child = root + leaf;
        if (leaf + 1 < nmemb && comp(*child, *(child + 1)) < 0 ) {
            child++;
            leaf++;
        }   // right leaf is larger than left leaf
        if (comp(key, *child) >= 0) break;
        *parent = *child; parent = child;    //slide
        node = leaf;
    }
    if (parent > start) *parent = key;      // restore
#ifdef DEBUG
    dump_pointer("heap() done ", root, nmemb);
#endif
}

// sort
static void sort(size_t nmemb) {
#ifdef DEBUG
    dump_pointer("sort()", root, nmemb);
#endif
    size_t  n = nmemb;
    void **last = root + n - 1;     // next element of the last element
    do {    // sort
        void *tmp = *last; *last = *root; *root = tmp;  // swap first <--> last
        last--; heap(--n, 0);     // shrink and re-build the heap
    } while (n);
}

// build a heap by top down (sift up)
void heap_top(void **base, size_t nmemb, int (*compare)(const void *, const void *)) {
    if (nmemb <= 1) return;
#ifdef DEBUG
    dump_pointer("sort() start in " __FILE__, base, nmemb);
#endif
    for (size_t node = 1; node < nmemb; node++) {
#ifdef DEBUG
        qsort_called++;
#endif
        size_t  child = node;
        size_t  parent = (child - 1) >> 1;
        while (compare(base[parent], base[child]) < 0) {
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP) {
                char tmp1[32], tmp2[32];
                fprintf(OUT, "swap : %s [%s] <--> %s [%s]\n"
                , dump_data(base[parent]), dump_size_t(tmp1, parent)
                , dump_data(base[child]),  dump_size_t(tmp2, child));
            }
#endif
            void *tmp = base[parent]; base[parent] = base[child]; base[child] = tmp;
            if (! parent) break;    // root
            child = parent; parent = (child - 1) >> 1;
        }
    }
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) dump_heap(base, nmemb);
#endif
    comp = compare;
    root = base;
    sort(nmemb);
#ifdef DEBUG
    dump_pointer("heap_top done", root, nmemb);
#endif
}

// build a heap by bottom up (sift down)
void heap_bottom(void **base, size_t nmemb, int (*compare)(const void *, const void *)) {
    if (nmemb <= 1) return;
#ifdef DEBUG
    dump_pointer("heap_bottom() start in " __FILE__, base, nmemb);
#endif
    comp = compare;
    root = base;
    size_t node = (nmemb - 1) >> 1;     // lowest parent node
    do {
        heap(nmemb, node);
    } while (node--);
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) dump_heap(base, nmemb);
#endif
    sort(nmemb);
#ifdef DEBUG
    dump_pointer("heap_bottom done", root, nmemb);
#endif
}
