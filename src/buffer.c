/*
 * buffer.c
 *
 *  Manage a cyclic buffer for data to be sorted.
 *
 *  Created on: Feb. 17, 2018
 *      Author: Takeuchi Leorge <qmisort@gmail.com>
 */
#include    <errno.h>
#include    "sort.h"

static  int size = 0, current = 0;
static  void **workarea;

void free_buffer(){
    if (size) {
        for (void **p = workarea; size; size--) free(*p++);
        free(workarea); workarea = NULL;
    }
}

int alloc_buffer(size_t nmemb, size_t length, int count) {
    int rtn = 0;
    free_buffer();  // initialize anyway
    current = 0;
    workarea = (void **)calloc(sizeof(void *), count);
    for (int i = 0; i < count; size++) {
        void    *workbuff;
        if ((workbuff = calloc(nmemb, length)) == NULL) {
            rtn = errno;
            fprintf(ERR, "%s : nmemb = %ld  length = %ld  count = %d\n",
                    strerror(rtn), nmemb, length, count);
            break;
        }
        workarea[i++] = workbuff;
    }
    return rtn;
}

void *get_buffer() {
    void *rtn = NULL;
    if (size) {
        rtn = workarea[current];
        assert(rtn);
        current = (current + 1) % size;
    }
    return  rtn;
}

