#include "mem_alloc.h"
#define BSIZE 16384

/* variables to keep track of user defined healthcheck parameters */
int allocationType = 0;
int blockAlloc = 0;
int freeSpace = 0;
int bigChunk = 0;
int malloc_call_tracker = 0;

/*======================================
 helper functions for bookkeeping
 =======================================*/

/* returns largest current contiguous space */
int largeSpaceTracker()
{

    return (int) bigChunk;
}

/* returns total byte allocations */
int byteAllocTracker()
{

    return (int) blockAlloc;
}

/* returns total free space */
int freeSpaceTracker()
{

    return (int) freeSpace;
}

/* tracks status of the topmost block on the heap and checks if it is free, update if required */
void topBlockTracker()
{

    int* touchHead = (int*) (top_block);
    touchHead = (int*) DECREMENT(touchHead, 4);
    int top_tag = FREE_TAG(touchHead[0]);
    if(top_tag == 0)
    {
        unsigned top_len = GET_TAG(touchHead[0]);
        /* from specification: if top block is a free block is larger than 128KB decrease program break and update bookkeeping */
        if(top_len >= TOP_FREE)
        {
            touchHead = (int*) DECREMENT(touchHead, (top_len - 4));
            sbrk(-20000);
            top_len -= 20000;

            *touchHead = SET_TAG(top_len, 1);
            touchHead = (int*) INCREMENT(touchHead, (top_len - 4));

            *touchHead = SET_TAG(top_len, 1);
            top_block = (int*) DECREMENT(top_block, 20000);

            freeSpace -= 20000;
        }
    }

    /* updates largest contiguous block if required  */
    contBlockTracker();
}

/* tracks status of the largest available contiguous block in the free list, update if required */
void contBlockTracker()
{

    MapFreeStruct *iter = head;
    int biggest = 0;
    while(iter != NULL)
    {
        int* update = (int*) iter;
        update = (int*) DECREMENT(update, 4);
        int update_len = GET_TAG(update[0]);
        if(update_len > biggest)
        {
            biggest = update_len;
        }
        iter = iter->next;
    }
    bigChunk = biggest;
}

/*========================================================
 helper functions for node creation/deletion/addition
 =========================================================*/

/* adds node to the free list */
void nodeAdder(MapFreeStruct *new)
{

    if(head != NULL)
    {
        head->prev = new;
        new->next = head;
        head = new;
        new->prev = NULL;
    }

    else
    {
        head = new;
        tail = new;
    }
}

/* creates a new block to the if avaliable len doesn't suffice */
void* nodeCreator(int len, int best_fit)
{
    int whole_fit = ALIGN_4(len);

    if(best_fit == -1 || best_fit == 131073)
    {

        /* spawn a free block if the requested len has enough excess */
        if(whole_fit > (len + sizeof(MapFreeStruct) + 8))
        {
            whole_fit = ALIGN_4(len);
        }

        /* if not, create a 2nd block for free */
        else
        {
            whole_fit += BSIZE;
        }

        /* program break increment */
        int* start = (int*) sbrk(whole_fit + 8);
        int* bitmap_exit = (int*) sbrk(0);
        blockAlloc += len;
        top_block = bitmap_exit;

        *start = SET_TAG(len, 0);
        start = (int*) INCREMENT(start, 4);
        int* end = (int*) INCREMENT(start, len);
        *end = SET_TAG(len, 0);

        int* bitmap_init = (int*) INCREMENT(end, 4);
        *bitmap_init = SET_TAG((whole_fit - len), 1);
        bitmap_init = (int*) INCREMENT(bitmap_init, 4);

        /* use helper function to add the block to list */
        MapFreeStruct *new = (MapFreeStruct*) bitmap_init;
        nodeAdder(new);

        freeSpace += (whole_fit - len);
        bitmap_exit = (int*) INCREMENT(bitmap_init, (whole_fit - (len + 8)));
        *bitmap_exit = SET_TAG((whole_fit - len), 1);

        contBlockTracker();
        return (void*) start;
    }

    /* search for specified block */
    else
    {
        MapFreeStruct *iter = head;
        while(iter != NULL)
        {
            int* find = (int*) (iter);
            find = (int*) DECREMENT(find, 4);
            unsigned get_block = GET_TAG(find[0]);
            if(get_block == best_fit)
                break;
            else
                iter = iter->next;
        }

        /* strict checking for the block found */
        int* check = (int*) (iter);
        check = (int*) DECREMENT(check, 4);
        unsigned get_block = GET_TAG(check[0]);

        /* take the entire free space, or split up if excess */
        if(get_block - 8 >= len && get_block > (len + sizeof(MapFreeStruct) + 8))
        {
            int new_len = (get_block - (len + 8));
            int* start = (int*) check;
            int* bitmap_exit = (int*) INCREMENT(check, (get_block));

            *start = SET_TAG(len, 0);
            start = (int*) INCREMENT(start, 4);
            int* end = (int*) INCREMENT(start, len);
            *end = SET_TAG(len, 0);

            int* bitmap_init = (int*) INCREMENT(end, 4);
            *bitmap_init = SET_TAG(new_len, 1);
            bitmap_init = (int*) INCREMENT(bitmap_init, 4);

            /* update node */
            removeNode(iter);

            MapFreeStruct *new = (MapFreeStruct*) bitmap_init;
            nodeAdder(new);

            bitmap_exit = (int*) DECREMENT(bitmap_exit, 4);
            *bitmap_exit = SET_TAG(new_len, 1);
            blockAlloc += len;
            freeSpace -= (len + 8);
            contBlockTracker();
            return (void*) start;
        }

        /* take entire free space without splitting */
        else if(get_block - 8 >= len)
        {
            int* start = (int*) check;

            *start = SET_TAG((get_block - 8), 0);
            start = (int*) INCREMENT(start, 4);
            int* end = (int*) INCREMENT(start, (get_block - 8));
            *end = SET_TAG((get_block - 8), 0);

            /* update node */
            removeNode(iter);

            blockAlloc += (get_block - 8);
            freeSpace -= get_block;

            fprintf(stdout,
                "runtime_info: %d extra bytes has been padded to the requested malloc()\n",
                get_block - (len + 8));
            contBlockTracker();
            return (void*) start;
        }
        else
        {
            return NULL;
        }
    }
}

/*	removes node from the free list */
void removeNode(MapFreeStruct *iter)
{

    if(iter->prev != NULL)
    {
        iter->prev->next = iter->next;
    }
    if(iter->next != NULL)
    {
        iter->next->prev = iter->prev;
    }
    if(iter == head)
    {
        head = iter->next;
    }
    if(iter == tail)
    {
        tail = iter->prev;
    }

    iter->next = NULL;
    iter->prev = NULL;
    iter = NULL;
}

/*=========================
 main required APIs
 =========================*/

/* cause specified memory allocation policy to take effect */
void my_mallopt(int policy)
{

    /* refer to the header files for macro definitions */
    if(policy == FIRST_FIT)
    {
        allocationType = FIRST_FIT;
    }
    else if(policy == BEST_FIT)
    {
        allocationType = BEST_FIT;
    }
    else
    {
        fprintf(stderr, "@error_alert: chosen policy is not a valid!\n");
        return;
    }
}

/* deallocates memory blocks previously addressed */
void my_free(void *ptr)
{

    /* initialize pointer and register the len */
    int* new_free = (int*) (ptr);
    new_free = (int*) DECREMENT(new_free, 4);
    int free_len = GET_TAG(new_free[0]);
    blockAlloc -= free_len;
    freeSpace += (free_len + 8);
    int* quick_checker = (int*) DECREMENT(new_free, 4);
    int* top_check = (int*) INCREMENT(new_free, (free_len + 8));
    int dummy_free = FREE_TAG(quick_checker[0]);
    int high_avail_free = FREE_TAG(top_check[0]);

    /* strict checking for start of malloc at blocks below bottom block */
    int dummy_len_check = GET_TAG(quick_checker[0]);
    if(dummy_len_check == 0)
    {
        dummy_free = -1;
    }

    /* remove two old free list nodes, update metada and add corresponding node to list when both bottom and top blocks are free */
    if(!dummy_free && !high_avail_free)
    {

        int dummy_len = GET_TAG(quick_checker[0]);
        quick_checker = (int*) DECREMENT(quick_checker, (dummy_len - 8));

        int top_len = GET_TAG(top_check[0]);
        top_check = (int*) INCREMENT(top_check, 4);

        MapFreeStruct *rem_bot = (MapFreeStruct*) (quick_checker);
        MapFreeStruct *rem_top = (MapFreeStruct*) (top_check);
        removeNode(rem_bot);
        removeNode(rem_top);

        free_len += (dummy_len + top_len + 8);
        quick_checker = (int*) DECREMENT(quick_checker, 4);
        top_check = (int*) INCREMENT(top_check, (top_len - 8));
        *quick_checker = SET_TAG(free_len, 1);
        *top_check = SET_TAG(free_len, 1);

        int* start = (int*) INCREMENT(quick_checker, 4);
        MapFreeStruct *new = (MapFreeStruct*) start;
        nodeAdder(new);
    }

    /* remove old free list node, update metada and add corresponding node to list when only bottom block is free */
    else if(!dummy_free)
    {

        int dummy_len = GET_TAG(quick_checker[0]);
        quick_checker = (int*) DECREMENT(quick_checker, (dummy_len - 8));

        MapFreeStruct *rem_bot = (MapFreeStruct*) (quick_checker);
        removeNode(rem_bot);

        quick_checker = (int*) DECREMENT(quick_checker, 4);
        new_free = (int*) INCREMENT(new_free, (free_len + 4));

        free_len += (dummy_len + 8);
        *quick_checker = SET_TAG(free_len, 1);
        *new_free = SET_TAG(free_len, 1);
        int* start = (int*) INCREMENT(quick_checker, 4);
        MapFreeStruct *new = (MapFreeStruct*) start;
        nodeAdder(new);
    }

    /* remove old free list node, update metadata and add corresponding node to list when only top block is free */
    else if(!high_avail_free)
    {

        int top_len = GET_TAG(top_check[0]);
        top_check = (int*) INCREMENT(top_check, (4));
        MapFreeStruct *rem_top = (MapFreeStruct*) (top_check);
        removeNode(rem_top);
        top_check = (int*) INCREMENT(top_check, (top_len - 8));
        free_len += (top_len + 8);
        *new_free = SET_TAG(free_len, 1);
        *top_check = SET_TAG(free_len, 1);
        int* start = (int*) INCREMENT(new_free, 4);
        MapFreeStruct *new = (MapFreeStruct*) start;
        nodeAdder(new);
    }

    /* free block and add corresponding node to list if no adjacent free blocks found */
    else
    {
        *new_free = SET_TAG((free_len + 8), 1);
        new_free = (int*) INCREMENT(new_free, (free_len + 4));
        *new_free = SET_TAG((free_len + 8), 1);
        int* start = (int*) DECREMENT(new_free, free_len);
        MapFreeStruct *new = (MapFreeStruct*) start;
        nodeAdder(new);
    }

    /* update free block and contiguous space */
    topBlockTracker();
}

/* prints commutative memory allocation statistics */
void my_mallinfo()
{

    fprintf(stdout, "\n");
    fprintf(stdout, "MEMORY REPORT:\n");
    fprintf(stdout, "@mem_stat: total no. of my_malloc() calls since beginning = %d\n",
        malloc_call_tracker);
    fprintf(stdout, "@mem_stat: amount of available free space = %d\n", freeSpace);
    fprintf(stdout, "@mem_stat: amount of largest contiguous free space = %d\n", bigChunk);
    fprintf(stdout, "@mem_stat: no. of bytes allocated = %d\n", blockAlloc);
    fprintf(stdout, "\n");
}

/* allocates memory block */
void *my_malloc(int len)
{

    /* if this is the first call made set program break and initialize space for error reporting */
    if(malloc_call_tracker == 0)
    {
        my_malloc_error = (char*) sbrk(1028);
    }
    malloc_call_tracker++;

    /* error handling for negative integers */
    if(len < 0)
    {
        my_malloc_error = "@error_alert: cannot allocate negative memory!";
        return NULL;
    }

    /* returns first free available block */
    if(allocationType == FIRST_FIT)
    {

        /* iterate through the free block list */
        MapFreeStruct *iter = head;
        while(iter != NULL)
        {
            int* check = (int*) (iter);
            check = (int*) DECREMENT(check, 4);
            unsigned get_block = GET_TAG(check[0]);
            if(get_block - 8 >= len)
            {
                return nodeCreator(len, get_block);
            }
            iter = iter->next;
        }
        /* create new if no block found */
        return nodeCreator(len, -1);
    }

    /* returns first free block that fits best */
    if(allocationType == BEST_FIT)
    {
        MapFreeStruct *iter = head;
        unsigned best = 131073;

        while(iter != NULL)
        {
            int* check = (int*) (iter);
            check = (int*) DECREMENT(check, 4);
            unsigned get_block = GET_TAG(check[0]);
            /* strict len check for best fit */
            if(get_block - 8 == len)
            {
                return nodeCreator(len, get_block);
            }
            else if(get_block - 8 >= len && get_block <= best)
            {
                best = get_block;
            }
            iter = iter->next;
        }
        return nodeCreator(len, best);
    }

    /* redundant @error_handling, shouldn't get here */
    my_malloc_error = "@error_alert: couldn't allocate space, check fitting configurations!";
    return NULL;
}

