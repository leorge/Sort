/*
 * index.c
 *
 *  Use a pair of make_index() and unindex().
 *
 *  Created on: 2013/01/09
 *      Author: leo
 */

#include "sort.h"

static void copy(void *dst, const void *src, size_t n)
{
#ifdef DEBUG
    //  qsort_moved += n;
    qsort_moved++;
    if (trace_level >= TRACE_MOVE) fprintf(OUT, "copy(dst = %p : %s, src = %p : %s , size = %s)\n"
        , dst, dump_data(dst), src, dump_data(src), dump_size_t(NULL, n));
#endif
    if (dst != src) memcpy(dst, src, n); /* restore an elements  */
#ifdef  DEBUG
    else {
        if (trace_level >= TRACE_MOVE) fprintf(OUT,
            "Copy(dst == src : %p, size = %s)\n", dst, dump_size_t(NULL, n));
        exit(-1);
    }
#endif
}

/***** Make an idex table from array 1d *****/
void **edit_index(void **rtn, char *array1d, size_t nmemb, size_t size) {
    void    **ptr, *adrs;
    size_t  i;
    for (adrs = array1d, ptr = rtn, i = 0; i++ < nmemb; adrs += size) {
        *ptr++ = adrs;  // make an index table
    }
#ifdef DEBUG
    char    msg[300];
    if (trace_level >= TRACE_DUMP) {
        sprintf(msg, "edit_index(%p) at %p", array1d, rtn);
        dump_pointer(msg, rtn, nmemb);
    }
#endif
    return  rtn;
}
void **make_index(char *array1d, size_t nmemb, size_t size)
{
    void    **rtn;
    if (! (rtn = calloc(nmemb, sizeof(void *)))) {
        char    buf[100];
        sprintf(buf, "calloc() error in make_index(array1d, nmemb=%ld, size=%ld)", nmemb, size);
        perror(buf);
    } else
        rtn = edit_index(rtn, array1d, nmemb, size);
    return  rtn;
}

/***** Restore array 1d from index table *****/
void unindex(void *array1d, void *idxtbl[], size_t nmemb, size_t size)
{
#ifdef DEBUG
    static char *name = "unindex()";
    if (trace_level >= TRACE_DUMP) {
        dump_pointer(name, idxtbl, nmemb);
        if (trace_level >= TRACE_DEBUG)
            for (size_t i = 0; i < nmemb; i++)
                fprintf(OUT, "idxtbl[%s] = %p\t%s\n",
                	dump_size_t(NULL, i), idxtbl[i], (char *)idxtbl[i]);
    }
#endif
    void    **ptr, *dest;   // Pointer and destination
    char    tbuf[size];     // Temporary buffer
    dest = array1d; ptr = idxtbl;
    for (size_t i = 0; i < nmemb; i++) {   // cyclic permutation
#ifdef DEBUG
        if (trace_level >= TRACE_DEBUG) fprintf(OUT, "array1d[%s] = %p\t%s\n"
            , dump_size_t(NULL, i), dest, dump_data(dest));
#endif
        void    **p, *dst, *src;
        if (*(p = ptr) != dest) {
            copy(tbuf, dst = dest, size);   // save an element
            do {
                copy(dst, src = *p, size);  // copy an element
                *p = dst;                   // restore in idxtbl
                p = &idxtbl[((dst = src) - array1d) / size];
            } while (*p != dest);
            copy(*p = src, tbuf, size);     // restore saved element
        }
        dest += size; ptr++;
    }
}

/***** Indirect sort of qsort(3) *****/
static int      (*comp)(const void *, const void *);

static int  comp_idx(const void *p1, const void *p2) {
#ifdef DEBUG
    if (trace_level >= TRACE_COMPARE) fprintf(OUT, "comp_idx(%s, %s)\n",
        dump_data(*(char * const *)p1), dump_data(*(char * const *)p2));
#endif
    return  comp(*(char * const *)p1, *(char * const *)p2);
}

void qsort3_indr(void **base, size_t nmemb, int (*compare)(const void *, const void *)) {
    comp = compare;
    qsort((void *)base, nmemb, sizeof(void *), comp_idx);
}
