#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FIRST_FIT 1
#define BEST_FIT 2

#define TOP_FREE 131072
#define ALIGN_4(x) ((x/BSIZE + 1) * BSIZE)

#define SET_TAG(len,free) ((len << 1) + (free == 1 ? 0b0 : 0b1))
#define GET_TAG(len) (len >> 1)
#define FREE_TAG(ptr) (ptr & 0b1)

#define INCREMENT(ptr, len) (((char*)ptr) + len)
#define DECREMENT(ptr, len) (((char*)ptr) - len)

typedef struct MapFreeStruct
{
    struct MapFreeStruct *next;
    struct MapFreeStruct *prev;
} MapFreeStruct;

MapFreeStruct *head;
MapFreeStruct *tail;

char *my_malloc_error;
int* top_block;

/*=========================
 main required APIs
 =========================*/
extern char *my_malloc_error;
void *my_malloc(int len);
void my_free(void *ptr);
void my_mallopt(int policy);
void my_mallinfo();

/*================================================
 helper functions for node creation/deletion
 =================================================*/
void* nodeCreator(int len, int best_fit);
void removeNode(MapFreeStruct *iter);
void nodeAdder(MapFreeStruct *new);

/*======================================
 helper functions for bookkeeping
 =======================================*/
int freeSpaceTracker();
int byteAllocTracker();
int largeSpaceTracker();
void contBlockTracker();
void topBlockTracker();
