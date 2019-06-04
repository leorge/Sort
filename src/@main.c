/*
 ============================================================================
 Name        : @main.c
 Author      : Takeuchi Leorge <qmisort@gmail.com>
 Version     : 1.0.0
 Copyright   : free
 Description : Test sorting algorithms in C language Ansi-style.
 ============================================================================

 Compile option
     Release : -O3 -Wall -c -fmessage-length=0 -std=c99
     Debug   : -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -std=c99

 Link option
     -lm
 */
#include    <ctype.h>
#include    <math.h>
#include    <sys/resource.h>
#include    <sys/time.h>
#include    <time.h>
#include    <unistd.h>
#include    <limits.h>
#include    <errno.h>
#include    "sort.h"

#define     CR      '\r'
#define     LF      '\n'

/*
 * Uncomment or define symbols to examine algorithm.
 */

/****   Public  ****/
Trace   trace_level = TRACE_NONE;   // to debug
size_t  middle1 = 0;                // if N <= this then the middle element is a pivot.
size_t  random1 = 0;                // if N <= this then a random pivot
size_t  median3 = 0;                // if N <= this then a pivot is the median of 3 elements
size_t  median5 = 0;                // if N <= this then a pivot is the median of 5 elements
size_t  medianL = 0;                // if N <= this then a pivot is the median of log2(N)-1|1 elements
                                    //              else a pivot is the median of log2(sqrt(N))|1

size_t  medium_boundary = 0;        //  nmemb to switch to medium_func()
void    (*medium_func)() = NULL;
size_t  small_boundary = 0;         //  nmemb to switch to small_func()
void    (*small_func)() = NULL;

long    qsort_called, qsort_comp_str, qsort_moved, search_pivot;  // counters

bool    chk_sorted = FALSE;
bool    swap_half = FALSE;


/****   Private ****/

static int      limit = 2;      // boundary precent to pass
static int      skip;

static char     *srcbuf;            // Work buffer to store all input data
static long     *usec = NULL;       // elapsed time
static bool     print_out = FALSE;
static bool     carriage_return = FALSE;

// You can edit this function for other data structure.
int     length_compare;
static int cmpstring(const void *p1, const void *p2)    // Function to compare
{
    int rtn = strncmp(p1, p2, length_compare);
#ifdef  DEBUG
    qsort_comp_str++;
    if (trace_level >= TRACE_COMPARE) fprintf(OUT, "cmpstring(%s, %s) --> %d\n", (char *)p1, (char *)p2, rtn);
#endif
    return  rtn;
}

static  void dump_buffer(void *array, size_t nmemb, size_t size) {
    size_t  i;
    char *p, *p1, *p2;
    for (p = array, i = 0; i++ < nmemb; p += size) {
        for (p1 = p + size; p < p1--;) {    // skip trailing CR, LF, and '\0'
            if ((*p1 != CR) && (*p1 != LF) && (*p1 != '\0')) break;
        }
        for (p2 = p; p2 <= p1;) fputc(*p2++, OUT);
        if (carriage_return) fputc(CR, OUT);
        fputc(LF, OUT);
    }
    fflush(OUT);
}

typedef enum {
    DEBUG_SORT,
    DUMMY_SORT,
    SWAP_FIRST,
    SWAP_MIDDLE,
    SWAP_KR,
    SWAP_MED3,
    SWAP_3WAY,
    SWAP_DUAL,
    QSORT_HOLE,
    QSORT_SECURE,
    QSORT_ASYMM,
    QSORT_RANDOM,
    QSORT_PIVOT,
    QUICK_SORT,
    TICKET_SORT,
    MERGE_INDIRECT,
    MERGE_ARRAY,
} SORT_TYPE;

typedef struct {
    int         option;             // command option
    SORT_TYPE   type;               // see above.
    const char *name;               // function name to display
    void        (*sort_function)();
    const char  *description;
} INFO;

static int  cmp_info(const void *t1, const void *t2) {  // to sort test[] in main().
    return ((INFO*)t1)->type - ((INFO*)t2)->type;
}

// examine

static char *sorted_array = NULL;
static size_t memsize = 0;

static void *run(INFO *info, void *data, int repeat_count, size_t nmemb, size_t size) {
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT, "Start %s : %s\n", info->name, info->description);
#else
RETRY:   ;
#endif
    int i;  // temporary variables
    void    *rtn = NULL, *workbuff;
    for (i = 0; i < repeat_count; i++) {
#ifdef DEBUG
        if (trace_level >= TRACE_DUMP) fprintf(OUT, "%s start %d / %dth in " __FILE__ "\n", info->name, i + 1, repeat_count);
        assert(memsize);
#endif
        qsort_comp_str = qsort_called = qsort_moved = search_pivot = 0;    // reset all of counters
        workbuff = get_buffer();
        memcpy(workbuff, data, memsize);     // memory copy : workbuff <-- srcbuf
        start_timer();
        (*info->sort_function)(workbuff, nmemb, size, cmpstring);
        long sorting_time = stop_timer();
#ifdef  DEBUG
        dump_array("sorted.", workbuff, nmemb, 0, 0, size);
        fprintf(OUT, "%s\tusec = %ld", info->name, sorting_time);
        if (trace_level >= TRACE_COUNT) {
            fprintf(OUT, "\tcall = %ld\tcompare = %ld\tcopy = %ld",
                    qsort_called, qsort_comp_str, qsort_moved);
        }
        fprintf(OUT, "\n"); // Carriage Return
#else
        usec[i] = sorting_time;
#endif
        assert(memsize != 0);
        assert(sorted_array != NULL);
        if (memcmp(workbuff, sorted_array, memsize)) {
            fprintf(ERR, "Not sorted in %s\n", info->name);
#ifdef  DEBUG
            char *p1 = workbuff, *p2 = sorted_array;
            for (size_t z = 0; z < nmemb; z++, p1 += size, p2 += size)
                if (cmpstring(p1, p2)) {
                    fprintf(OUT, "%s should be %s at %ld\n", p1, p2, z);
                    break;
                }
#endif
            break;
        }
        else    rtn = workbuff;
    }
#ifndef  DEBUG
    int     peak = 0;
    double  sum = 0.;
    long    y, max_y = 0, n = 0;
    for (i = skip; i < repeat_count; n++, i++) {
        sum += (y = usec[i]);
        if (y > max_y) {
            max_y = y;  // value
            peak = i;   // position
        }
    }
    double  esd = 0.0, delta = 0.0, mean = sum;
    if (n > 1) {
        mean = (sum -= max_y) / --n;    // Omit the largest value
        for (i = skip; i < repeat_count; i++) {
            if (i != peak) {    // Skip the largest value
                delta = (double)usec[i] - mean;
                esd += delta * delta;
            }
        }
        esd = --n ? sqrt(esd / n) : 0;
    }

    fprintf(OUT, "%s\tusec = %.0f", info->name, mean);
    int percent = 0;
    if (repeat_count > 1) {
        percent = 100. * esd / mean + .5;
        fprintf(OUT, "\tspread = %.0f\t%3d %%", esd, percent);
        int     i;
        long    *tlen = usec;
        fprintf(OUT, " [%ld]", *tlen++);
        for (i = 1; i < skip; i++) fprintf(OUT, " [%ld]", *tlen++);
        for (i = skip; i < repeat_count; i++, tlen++) {
            if (i == peak) fprintf(OUT, " (%ld)", *tlen);
            else fprintf(OUT, " %ld", *tlen);
        }
    }
    fprintf(OUT, "\n");
    if (percent > limit) goto RETRY;
#endif
    return  rtn;
}

void dummy_sort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *)) {
}

/*****************************************************************************/
int main(int argc, char *argv[])
{
    extern int getopt(int argc, char * const argv[], const char *optstring);
    extern  int optind;
    extern  char *optarg;

    char    c, *p;  // temporary variables
    size_t  i;

#define SENTINEL    1000
   INFO *info, large_functions[] = {  // reordered by SORT_TYPE
            {'3', SWAP_MED3, "qsort_med3()", qsort_med3, "quicksort : pivot is the median of 3 elements using sWaps."},
            {'a', QSORT_ASYMM, "quick_asymm()", quick_asymm, "quicksort : simple Asymmetric quicksort."},
            {'d', SWAP_MIDDLE, "qsort_middle()", qsort_middle, "quicksort : pivot is the miDDle element using swaps."},
            {'f', SWAP_FIRST, "qsort_first()", qsort_first, "quicksort : pivot is the First element using swaps."},
            {'h', QSORT_HOLE, "quick_hole()", quick_hole, "quicksort : prototype using a pivot Hole."},
            {'j', SWAP_DUAL, "dual_pivot()", dual_pivot, "quicksort : implemented dualpivot quicksort in Java."},
            {'K', SWAP_KR, "qsort_kr()", qsort_kr, "quicksort : pivot is the middle element in K&R style."},
            {'k', TICKET_SORT, "ticket_sort()", ticket_sort, "quicksort : ticKet sort using quicksort."},
            {'m', MERGE_ARRAY, "merge_array()", merge_array, "mergesort : conventional top-down Mergeosrt."},
            {'q', QUICK_SORT, "asymm_qsort()", asymm_qsort, "quicksort : final asymmetric QuickSort."},
            {'r', QSORT_RANDOM, "quick_random()", quick_random, "quicksort : choose a Random element as a pivot."},
            {'S', QSORT_SECURE, "quick_secure()", quick_secure, "quicksort : Secured quick_hole()."},
            {'t', QSORT_PIVOT, "quick_pivot()", quick_pivot, "quicksort : choose the median of several elements as a pivoT."},
            {'U', DUMMY_SORT, "dummy_sort()", dummy_sort, "dUmmy sort : do nothing to cause error."},
            {'w', SWAP_3WAY, "qsort_3way()", qsort_3way, "quicksort : 3-way partitioning."},
            { 0 , 0, NULL, NULL}    // sentinel
    };

    INFO    medium_functions[] = { // reorder manually
            {'q', 1, "iqsort(&)", iqsort, "indirect Quicksort."},
            {'m', 0, "imsort(&)", imsort, "indirect Mergesort."},
            { 0 , 0, NULL, NULL}    // sentinel
    };

    INFO    small_functions[] = { // reorder manually
            {'i', 1, "insert_linear(*)", insert_linear, "linear insertion sort."},
            {'b', 0, "insert_binary(*)", insert_binary, "Binary insertion sort."},
            {'a', 0, "ai_sort(*)", ai_sort, "Accelerated linear insertion sort."},
            {'L', 0, "shell_sort(*)", shell_sort, "shell sort."},
            {'h', 0, "heap_top(*)", heap_top, "Heap sort. build a heap by top-down."},
            {'H', 0, "heap_bottom(*)", heap_bottom, "Heap sort. build a heap by bottom-up."},
            {'B', 0, "bubble_sort(*)", bubble_sort, "Bubble sort."},
            {'T', 0, "cocktail_sort(*)", cocktail_sort, "cockTail sort."},
            {'C', 0, "comb_sort(*)", comb_sort, "Comb sort."},
            {'r', 0, "rabbit_sort(*)", rabbit_sort, "rabbit sort."},
            { 0 , 0, NULL, NULL}    // sentinel
    };
    // prepare to analyze command arguments with INFO
    const char *options = "?A:B:I:N:pP:R:T:V:Y:Z:";
    qsort(large_functions, sizeof(large_functions) / sizeof(INFO) - 1, sizeof(INFO), cmp_info);   // sort a table according to the SORT_TYPE.
    char    optstring[sizeof(large_functions) / sizeof(INFO) + strlen(options)];
    for (info = large_functions, p = optstring; (c = info->option); info++) *p++ = c;
    strcat(optstring, options);
    char    med_algo[sizeof(medium_functions) / sizeof(INFO)];
    char    small_algo[sizeof(small_functions) / sizeof(INFO)];
    *med_algo = *small_algo = '\0';
    /**** Analyze command arguments ****/
    char    *fin = NULL;                // file name to input
    typedef long INDEX; INDEX large_index = 0, medium_index = 0, small_index = 0, idx;
    int     repeat_count, buffer_length = 1;
    size_t  nmemb = 31, size = 0;


#ifdef  DEBUG   // Default
    repeat_count = 1;
#else
    repeat_count = 12;
#endif
    for (int opt;(opt = getopt(argc, argv, optstring)) != -1;) {
        switch (opt) {
        case '?':
            p = strrchr(argv[0], '/');   // Program name without path
            printf("Usage : %s [options] [filename]\n", p ? p + 1 : argv[0]);
            for (info = large_functions; info->option; info++)
                printf("\t-%c : %s\n", info->option, info->description);
            puts("\t-I : Indirect sorting algorithms for medium N.");
            for (info = medium_functions; info->option; info++)
                printf("\t       %c %s %s\n", info->option, info->description, info->type ? "(default)" : "");
            puts("\t-A : sorting Algorithms for small N.");
            for (info = small_functions; info->option; info++)
                printf("\t       %c %s %s\n", info->option, info->description, info->type ? "(default)" : "");
            puts(
                "\t-B m,s : Boundary number of elements to change algorithm.\n"
                "\t       N <= m -- sorting algorithm for medium N.\n"
                "\t       N <= s -- sorting algorithm for Small N.\n"
                "\t-P l,m,n,s,t : threshold to change the choice of Pivot in quicksort.\n"
                "\t     0 : use a default value   1 : ignore this value\n"
                "\t       N <= l -- middle element.\n"
                "\t       N <= m -- random element.\n"
                "\t       N <= n -- median of 3 elements.\n"
                "\t       N <= s -- median of 5 elements.\n"
                "\t       N <= t -- median of (log2(N)-1)|1 elements.\n"
                "\t       else   -- median of (log2(N)/2)|1 elements.\n"
                "\t-N : Number of members (default is 31).\n"
                "\t-Z : siZe of an array element.\n"
                "\n"
                "\t-p : print Out the last sorted data.\n"
                "\t-R : Repeat count "
#ifndef DEBUG
                "of sampling to calculate (default is 12).\n"
#else
                "to test (default is 1).\n"
                "\t-T : uncerTainTy percenT to pass a test (default is 2%).\n"
#endif
                "\t-Y : The number of cYclic work buffer."
#ifdef DEBUG
                "\n"
                "\t-V : trace level for Debugging.\n"
                "\t       1 - Counts.\n"
                "\t       2 - and actions.\n"
                "\t       3 - and copies.\n"
                "\t       4 - and comparisons.\n"
                "\t       5 - and Others."
#endif
            );
            return EXIT_SUCCESS;
            break;
        case 'A':   // Sorting algorithm for small N
            for (p = optarg; *p; p++)
                for (info = small_functions, idx = 1; (c = info->option); idx <<= 1,  info++)
                    if (c == *p) {
                        small_index |= idx;
                        break;
                    }
            break;
        case 'B':   // Boundary to change algorithms
            {
                char popt[strlen(optarg) + 1]; strcpy(popt, optarg);
                if ((p = strtok(popt, ",")) != NULL) {
                    medium_boundary = strtoul(p, NULL, 0);
                    if ((p = strtok(NULL, ",")) != NULL) {
                        small_boundary = strtoul(p, NULL, 0);
                    }
                }
            }
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP) {    // !!!!
                char    tmp1[32], tmp2[32];
                fprintf(OUT, "Boundaries to change algorithm -B %s : small <= %s , medium <= %s\n"
                    , optarg, dump_size_t(tmp1, small_boundary), dump_size_t(tmp2, medium_boundary));
            }
#endif
            break;
        case 'I':   // Indirect sorting algorithms for medium N
            for (p = optarg; *p; p++)
                for (info = medium_functions, idx = 1; (c = info->option); idx <<= 1, info++)
                    if (c == *p) {
                        medium_index |= idx;
                        break;
                    }
            break;
        case 'N':
            nmemb = strtoul(optarg, NULL, 0);
            break;
        case 'p':
            print_out = TRUE;
            break;
        case 'P':
            {
                char popt[strlen(optarg) + 1]; strcpy(popt, optarg);
                if ((p = strtok(popt, ",")) != NULL) {
                    middle1 = strtoul(p, NULL, 0);
                    if ((p = strtok(NULL, ",")) != NULL) {
                        random1 = strtoul(p, NULL, 0);
                        if ((p = strtok(NULL, ",")) != NULL) {
                            median3 = strtoul(p, NULL, 0);
                            if ((p = strtok(NULL, ",")) != NULL) {
                                median5 = strtoul(p, NULL, 0);
                                if ((p = strtok(NULL, ",")) != NULL) {
                                    medianL = strtoul(p, NULL, 0);
                                }
                            }
                        }
                    }
                }
            }
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP)
                fprintf(OUT, "Choice of Pivot -P %s : middle <= %ld , random <= %ld , median-of-3 <= %ld "
                    ", median-of-5 <= %ld, median-of-(log2(N)-1)|1 <= %ld < median-of-log2(sqrt(N))|1\n",
                    optarg, middle1, random1, median3, median5, medianL);
#endif
            break;
        case 'R':
            repeat_count = strtoul(optarg, NULL, 0);
            break;
        case 'T':
            limit = atoi(optarg);
            break;
        case 'V':   // ignored in Release
            trace_level = strtoul(optarg, NULL, 0);
            break;
        case 'Y':
            buffer_length = (int)strtoul(optarg, NULL, 0);
            break;
        case 'Z':
            size = (size_t)strtoul(optarg, NULL, 0);
            break;
        default:    // select sorting algorithm
            for (info = large_functions, idx = 1; info->option; idx <<= 1, info++)
                if (opt == info->option) {
                    large_index |= idx;
                    break;
                }
            break;
        }
    }
    if (argc > optind) fin = argv[optind];
#ifdef  DEBUG
    if (trace_level > TRACE_NONE) {
        fprintf(OUT, "arguments:");
        char **p = argv;
        for (int i = 1; i++ < argc;) fprintf(OUT, " %s", *++p);
        fprintf(OUT, "\n");
    }
#endif

#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) {    // !!!!
        char    tmp1[32], tmp2[32], tmp3[32], tmp4[32];
        fprintf(OUT, "Threshold to choice of a pivot : %s --> %s %s %s %s\n", optarg,
            dump_size_t(tmp1, middle1), dump_size_t(tmp2, median3), dump_size_t(tmp3, median5), dump_size_t(tmp4, medianL));
    }
#endif

    /**** Read data ****/

    // Open file
    FILE *fp = fin ? fopen(fin, "r") : stdin;   // open file or use /dev/stdin
    if (fp == NULL) {
        if (fin) fprintf(ERR, "Filename = %s\n", fin);
        perror(NULL);       // Output system error message to OUT.
        return EXIT_FAILURE;
    }

    // Read the first line to get a record size
    char read_buff[1024];
    if (! fgets(read_buff, sizeof(read_buff) - 1, fp)) return EXIT_SUCCESS; // EOF
    if (! isprint((int)read_buff[0])) {     // not a printable character
        fprintf(ERR, "Use printable characters.\n");
        return EXIT_FAILURE;
    }
    p = memchr(read_buff, LF, sizeof(read_buff));   // fgets(3) stores EOL.
    assert(p != NULL);  // Don't replace the LF to '\0'.
    size_t data_len;    // length of data before LF
    if ((data_len = p - read_buff) && (*--p == CR)) {   // CR + LF at the EOL.
        carriage_return = TRUE; // for Microsoft OS.
        data_len--;
    }
    if (data_len == 0) {
        fprintf(ERR, "The first line is empty.\n");
        return EXIT_FAILURE;
    } else if (size == 0) { // -Z is undefined.
        size = data_len;
    } else if (data_len > size) {
        fprintf(ERR, "The first line is too long! Use the -Z option.\n");
        return EXIT_FAILURE;
    }
    i = 0;
    length_compare = strlen(read_buff); // to omit after '\0'.
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT, "Size of an element = %s\n", dump_size_t(NULL, size));
#endif

    // allocate data area
    if ((srcbuf = calloc(nmemb, size)) == NULL) // The memory is set to zero.
    {   /* Can't get work area  */
        perror(NULL);
        return EXIT_FAILURE;
    }

    // read data
    char *src = srcbuf;
    i = 1;
    do {    // read_buff is terminated with null character '\0'.
        size_t l;
        p = memchr(read_buff, LF, sizeof(read_buff));
        if ((l = read_buff - p) > data_len) l = data_len;
        memcpy(src, read_buff, l);      // don't use strncpy(3).
        src += size;
        if (! fgets(read_buff, sizeof(read_buff), fp)) break;       // EOF
    } while (i++ < nmemb);
    if (fin) fclose(fp);
    if (i < 1) return EXIT_SUCCESS;     // Output no line if the input is Only one line.
    else if (i < nmemb) nmemb = i + 1;
    memsize = nmemb * size;
    assert(memsize != 0);
    srcbuf = realloc(srcbuf, memsize);  // reduce the size
//    dump_buffer(srcbuf, nmemb, size);

    /***** Prepare *****/

    // Environment variables by a command "export Xx=123"

    chk_sorted = (getenv("checkSorted") != NULL);   // $ export MergesortCheckDup=1
    swap_half = (getenv("swapHalf") != NULL);


#ifdef DEBUG
    usec = malloc(sizeof(long));
#else
    usec = calloc(sizeof(long),repeat_count);
#endif

    skip = repeat_count > 1 ? 1: 0;
    if ((sorted_array = calloc(nmemb, size)) == NULL)
    {   /* Can't get work area  */
        perror(NULL);
        return EXIT_FAILURE;
    }
    else {
    }

    unsigned seed = (unsigned)time(NULL) + (unsigned)clock();
#ifdef DEBUG
    if (trace_level >= TRACE_DUMP) fprintf(OUT, "random seed = %d\n", seed);
#endif
    srand(seed);

    // Get buffers

    if ((i = alloc_buffer(nmemb, size, buffer_length))) return i;

    assert(memsize != 0);
    if (! (sorted_array = malloc(memsize))) {
        i = errno;
        fprintf(ERR, "%s : getting a buffer for sorted_array.\n", strerror(i));
        return i;
    }
    memcpy(sorted_array, srcbuf, memsize);
    qsort(sorted_array, nmemb, size, cmpstring);

    /***** Evaluate algorithms  *****/

    INFO    qsort3 = {0, 0, "qsort(3)", qsort, "Standard C library"};
    void *result = run(&qsort3, srcbuf, repeat_count, nmemb, size);
    if (! result) goto DONE;

    // Test algorithms to sort an array.
    for (info = large_functions, idx = 1; info->option; idx <<= 1, info++) {
        if (large_index & idx) {
            result = run(info, srcbuf, repeat_count, nmemb, size);
            if (! result) goto DONE;
        }
    }

    // Test hybrid sorting algorithms

    if (medium_boundary) {  // Hybrid
        if (small_boundary && ! small_index) {  // There is no functions for small N
            set_gaps(small_boundary);   // fibonacci numbers
            for (small_index = 1, info = small_functions; info->option && ! info->type; info++) small_index <<= 1;    // search default
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP)
                fprintf(OUT, "set a default function %s for small N : %s\n", info->name, info->description);
#endif
        }
        if (! medium_index) {   // There is no functions for medium N.
            for (medium_index = 1, info = medium_functions; info->option && ! info->type; info++) medium_index <<= 1;
#ifdef DEBUG
            if (trace_level >= TRACE_DUMP)
                fprintf(OUT, "set a default function %s for medium N : %s\n", info->name, info->description);
#endif
        }
        INDEX   med = 1;
        for (INFO *medium_info = medium_functions; medium_info->option; med <<= 1, medium_info++) {
            if (medium_index & med) {
#ifdef DEBUG
                if (trace_level >= TRACE_DUMP) fprintf(OUT, "%s when N <= %ld: %s\n",
                        medium_info->name, medium_boundary, medium_info->description);
#endif
                medium_func = medium_info->sort_function;
                INFO    run_info; memcpy(&run_info, medium_info, sizeof(INFO));
                run_info.sort_function = hybrid;
                if (small_boundary || small_index) {  // call sorting functions for small N.
                    idx = 1;
                    for (INFO *small_info = small_functions; small_info->option; idx <<= 1, small_info++) {
                        if (small_index & idx) {
#ifdef DEBUG
                            if (trace_level >= TRACE_DUMP) fprintf(OUT, "%s when N <= %ld: %s\n",
                                    small_info->name, small_boundary, small_info->description);
#endif
                            small_func = small_info->sort_function;
                            memcpy(&run_info, small_info, sizeof(INFO));    // overwrite
                            run_info.sort_function = hybrid;                // reset
                            result = run(&run_info, srcbuf, repeat_count, nmemb, size);
                            if (! result) goto DONE;    // break 2
                        }
                    }
                }
                else {
                    result = run(&run_info, srcbuf, repeat_count, nmemb, size);
                    if (! result) goto DONE;
                }
            }
        }
    }
    // output result
DONE:   ;
    if (print_out && result) dump_buffer(result, nmemb, size);  // -p option
    free_buffer();
    return EXIT_SUCCESS;
}
