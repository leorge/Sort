/*
 * ticket_qsort.c
 *
 *  Ticket sort using asymmetric quicksort
 *  to make the time complexity to O(n log(n)) when the element size is large.
 *
 *  Sorting key is the first member of an array element up to 16 bytes.
 *
 *  Created on: 2018/06/14
 *      Author: leo
 */

#include "sort.h"

typedef struct {
    void    *key1, *key2, *body;
} TICKET;

static size_t   length;
static void copy(void *dst, const void *src)
{
    assert(dst != src);
    memcpy(dst, src, length); /* restore an element  */
}

static int (*comp)(const void *, const void *);
static int my_comp(const void *p1, const void *p2) {
    int rtn = comp(p1, p2);
#ifdef DEBUG
    const TICKET    *t1 = p1, *t2 = p2;
    if (trace_level >= TRACE_DUMP) fprintf(OUT, "my_comp(\"%s\", \"%s\")"
        , (char *)&t1->key1, (char *)&t2->key1);
#endif
    if ((rtn == 0)) {
#ifdef DEBUG
        if (trace_level >= TRACE_DUMP) fprintf(OUT, "at %p and %p", t1->body, t2->body);
#endif
        rtn = (((TICKET *)p1)->body> ((TICKET *)p2)->body) ? 1 : -1;
    }
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT, " --> %d\n", rtn);
#endif
    return  rtn;
}

void ticket_sort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *)) {
    if (nmemb <= 1) return;
#ifdef DEBUG
    char    dmp1[16], dmp2[16];
    if (trace_level >= TRACE_DUMP) fprintf(OUT,
            "tag_sort(base=%p, nmemb=%s, compare) start.\n", base, dump_size_t(dmp1, nmemb));
#endif
    assert(base != NULL);
    assert(size > 0);
    assert(compare != NULL);
    TICKET *tickets = calloc(sizeof(TICKET), nmemb), *tic = tickets;
    if ( !tic)   // failed to allocate memory
        asymm_qsort(base, nmemb, size, compare);
    else {
#ifdef DEBUG
        if (trace_level >= TRACE_DUMP)
            fprintf(OUT, "Create and sort USER_INDEX tags[%s]\n", dump_size_t(NULL, nmemb));
#endif
        comp = compare;
        char    save[length = size], *body = base;
        for (size_t i = 0; i < nmemb; i++) {    // Make an index.
            tic->body = body;                   // Point an array element.
            tic->key1 = ((TICKET *)body)->key1; // Copy the first 8 bytes.
            tic->key2 = ((TICKET *)body)->key2; // Copy the next 8 bytes.
            tic++; body += size;
        }
        asymm_qsort(tickets, nmemb, sizeof(TICKET), my_comp); // Sort the index
#ifdef DEBUG
        if (trace_level >= TRACE_DUMP) {
            for (size_t i = 0; i < nmemb; i++) {   // dump result
                fprintf(OUT, "tags[%s] points array[%s] \"%s\"\n", dump_size_t(dmp1, i)
                    , dump_size_t(dmp2, (tickets[i].body - base) / size), (char *)&tickets[i].key1);
            }
        }
#endif
        // reorder array elements
        TICKET  *t;
        void    *src = base, *dst;
        tic = tickets; body = base;
        for (size_t i = 0; i < nmemb; i++) {
            if ((t = tic)->body != body) {  // an element is not placed at the correct position
#ifdef DEBUG
                if (trace_level >= TRACE_DUMP) fprintf(OUT, "tags[%s] points array[%s].\n"
                            , dump_size_t(dmp1, i), dump_size_t(dmp2, (tic->body - base) / size));
#endif
                copy(save, dst = body);         // save an element
                do {
                    copy(dst, src = t->body);   // move an element
#ifdef DEBUG
                    if (trace_level >= TRACE_MOVE) fprintf(OUT, "array[%s] <-- array[%s] \"%s\"\n"
                            , dump_size_t(dmp1, (dst - base) / size)
                            , dump_size_t(dmp2, (src - base) / size), (char *)src);
#endif
                    t->body = dst;              // reset the address
                    t = &tickets[((dst = src) - base) / size];   // points the new hole
                } while (t->body != body);      // cyclic permutation
                copy((t->body = src), save); // restore saved element
#ifdef DEBUG
                if (trace_level >= TRACE_MOVE) fprintf(OUT, "restore \"%s\" into array[%s]\n"
                    , (char *)src, dump_size_t(dmp1, ((src - base) / size)));
#endif
            }
            body += size; tic++;
        }
        free(tickets);
    }
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) dump_array("tag_sort() done.", base, nmemb, 0, 0, size);
#endif
}
