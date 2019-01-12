#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mem_alloc.h"

#define BSIZE 16384
#define TEST_SIZE_1 2048
#define TEST_SIZE_2 4096
#define TEST_SIZE_3 15000
#define BLOCK_SIZE 16384

/* 
 NOTE:
 This suite is designed with moderate rigorosity to test the main APIs coded.
 3 sample data sizes are systematically used to allocate and deallocate
 memory bytes in an order that will stress the flexibility and functionality of the APIs.
 */

int main(int argc, char* argv[])
{

    printf("===================DATA INTEGRITY TEST===================\n");
    printf("@test_info: initial shakedown = no initialization/allocations made yet\n");
    my_mallinfo();

    printf("@test_info: memory initialized and allocation policy set to first fit\n");
    my_mallopt(1);

    printf("@test_info: : allocate negative memory bytes. . .\n");
    int* fail = my_malloc(-10);
    fail++;
    printf("%s\n", my_malloc_error);
    printf("=========================================================\n");

    /* FIRST TEST CASE -- ALLOCATE 2048 BYTES */

    printf("@test_case_1: allocation of %d bytes. . .\n", TEST_SIZE_1);
    int* buffer = my_malloc(TEST_SIZE_1);

    printf("@test_info: filling allocated blocks with integers. . .\n");
    int i = 0;
    int k = 0;
    for(i = 0; i < TEST_SIZE_1 / sizeof(int); i++)
    {
        buffer[i] = i + k;
        k++;
    }
    my_mallinfo();

    printf("@test_info: printing current buffer statistics and doing healthcheck. . .\n\n");
    printf("buffer: %p\n", buffer);

    if(byteAllocTracker() == TEST_SIZE_1)
    {
        printf("@test_success: allocated bytes match cumulative requested size!\n");
    }
    else
    {
        printf("@test_failure: allocated bytes does not match cumulative requested size!\n");
    }

    if(freeSpaceTracker() == (BLOCK_SIZE - TEST_SIZE_1))
    {
        printf("@test_success: free space cumulative size matches!\n");
    }
    else
    {
        printf("@test_failure: free space size cumulative does not match!\n");
    }

    if(largeSpaceTracker() == (BLOCK_SIZE - TEST_SIZE_1))
    {
        printf("@test_success: largest contiguous free space matches!\n");
    }
    else
    {
        printf("@test_failure: largest contiguous free space does not match!\n");
    }
    printf("=========================================================\n");

    /* SECOND TEST CASE -- ALLOCATE 4096 BYTES */

    printf("@test_case_2: allocation of %d bytes. . .\n", TEST_SIZE_2);
    int* buffer2 = my_malloc(TEST_SIZE_2);

    printf("@test_info: filling allocated blocks with integers. . .\n");
    k = 0;
    for(i = 0; i < TEST_SIZE_2 / sizeof(int); i++)
    {
        buffer2[i] = i + k;
        k++;
    }
    my_mallinfo();

    printf("@test_info: printing current buffer statistics and doing healthcheck. . .\n\n");
    printf("buffer2: %p\n", buffer2);

    if(byteAllocTracker() == TEST_SIZE_1 + TEST_SIZE_2)
    {
        printf("@test_success: allocated bytes match cumulative requested size!\n");
    }
    else
    {
        printf("@test_failure: allocated bytes does not match cumulative requested size!\n");
    }

    if(freeSpaceTracker() == (BLOCK_SIZE - (TEST_SIZE_1 + TEST_SIZE_2 + 8)))
    {
        printf("@test_success: free space cumulative size matches!\n");
    }
    else
    {
        printf("@test_failure: free space size cumulative does not match!\n");
    }

    if(largeSpaceTracker() == (BLOCK_SIZE - (TEST_SIZE_1 + TEST_SIZE_2 + 8)))
    {
        printf("@test_success: largest contiguous free space matches!\n");
    }
    else
    {
        printf("@test_failure: largest contiguous free space does not match!\n");
    }
    printf("=========================================================\n");

    /* THIRD TEST CASE -- FREE 2048 BYTES ALLOCATION */

    printf("@test_case_3: deallocating bytes %d bytes. . .\n", TEST_SIZE_1);
    my_free(buffer);

    my_mallinfo();

    if(byteAllocTracker() == TEST_SIZE_2)
    {
        printf(
            "@test_success: remaining allocated bytes following deallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_failure: remaining allocated bytes following deallocation does not match as expected!\n");
    }

    if(freeSpaceTracker() == (BLOCK_SIZE - TEST_SIZE_2))
    {
        printf(
            "@test_success: free space cumulative size following deallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_failure: free space cumulative size following deallocation does not match as expected!\n");
    }

    if(largeSpaceTracker() == (BLOCK_SIZE - (TEST_SIZE_1 + TEST_SIZE_2 + 8)))
    {
        printf(
            "@test_success: largest contiguous free space following deallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_failure: largest contiguous free space following deallocation does not match as expected!\n");
    }
    printf("=========================================================\n");

    /* FOURTH TEST CASE -- REALLOCATE 2048 BYTES (TEST FOR CONTIGUOUS SPACING SELECTION) */

    printf("@test_case_4: reallocating 2048 bytes -- test for free space selection\n");
    int* buffer3 = my_malloc(TEST_SIZE_1);

    printf("@test_info: filling allocated blocks with integers. . .\n");
    k = 0;
    for(i = 0; i < TEST_SIZE_1 / sizeof(int); i++)
    {
        buffer3[i] = i + k;
        k++;
    }
    my_mallinfo();

    printf("@test_info: printing current buffer statistics and doing healthcheck. . .\n\n");
    printf("buffer3: %p\n", buffer3);
    if(byteAllocTracker() == TEST_SIZE_1 + TEST_SIZE_2)
    {
        printf(
            "@test_success: total allocated bytes following reallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_failure: total allocated bytes following reallocation does not match as expected!\n");
    }

    if(freeSpaceTracker() == (BLOCK_SIZE - (TEST_SIZE_1 + TEST_SIZE_2 + 8)))
    {
        printf(
            "@test_success: total free space cumulative size following reallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_success: total free space cumulative size following reallocation does not match as expected!\n");
    }

    if(largeSpaceTracker() == (BLOCK_SIZE - (TEST_SIZE_1 + TEST_SIZE_2 + 8)))
    {
        printf(
            "@test_success: largest contiguous free space following reallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_failure: largest contiguous free space following reallocation does not match as expected!\n");
    }
    printf("=========================================================\n");

    /* FIFTH TEST CASE -- FREE 2048 BYTES ALLOCATION */

    printf("@test_case_5: deallocating bytes %d bytes. . .\n", TEST_SIZE_1);
    my_free(buffer3);
    my_mallinfo();

    if(byteAllocTracker() == TEST_SIZE_2)
    {
        printf(
            "@test_success: total allocated bytes following 2nd deallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_failure: total allocated bytes following 2nd deallocation does not match as expected!\n");
    }

    if(freeSpaceTracker() == (BLOCK_SIZE - TEST_SIZE_2))
    {
        printf(
            "@test_success: total free space cumulative size following 2nd deallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_failure: total free space cumulative size following 2nd deallocation does not match as expected!\n");
    }

    if(largeSpaceTracker() == (BLOCK_SIZE - (TEST_SIZE_1 + TEST_SIZE_2 + 8)))
    {
        printf(
            "@test_success: largest contiguous free space following deallocation matches as expected!\n");
    }
    else
    {
        printf(
            "@test_failure: largest contiguous free space following deallocation does not match as expected!\n");
    }
    printf("=========================================================\n");

    /* SIXTH TEST CASE -- ALLOCATION OF 15000 BYTES */

    printf("@test_case_6: allocation of %d bytes. . .\n", TEST_SIZE_3);
    int* buffer4 = my_malloc(TEST_SIZE_3);
    k = 0;

    printf("@test_info: filling allocated blocks with integers. . .\n");
    for(i = 0; i < TEST_SIZE_3 / sizeof(int); i++)
    {
        buffer4[i] = i + k;
        k++;
    }
    my_mallinfo();

    printf("@test_info: printing current buffer statistics and doing healthcheck. . .\n\n");
    printf("buffer4: %p\n", buffer4);
    if(byteAllocTracker() == TEST_SIZE_2 + TEST_SIZE_3)
    {
        printf("@test_success: allocated bytes match cumulative requested size!\n");
    }
    else
    {
        printf("@test_failure: allocated bytes does not match cumulative requested size!\n");
    }

    if(freeSpaceTracker() == (2 * (BLOCK_SIZE) - (TEST_SIZE_2 + TEST_SIZE_3)))
    {
        printf("@test_success: free space cumulative size matches!\n");
    }
    else
    {
        printf("@test_failure: free space cumulative size does not match!\n");
    }

    if(largeSpaceTracker() == (BLOCK_SIZE - (TEST_SIZE_1 + TEST_SIZE_2 + 8)))
    {
        printf("@test_success: largest contiguous free space matches!\n");
    }
    else
    {
        printf("@test_failure: largest contiguous free space does not match!\n");
    }
    printf("=========================================================\n");

    /* SEVENTH TEST CASE -- FREE 4096 BYTES ALLOCATION */

    printf("@test_case_7: deallocating bytes %d bytes. . .\n", TEST_SIZE_2);
    my_free(buffer2);
    my_mallinfo();

    printf("@test_info: printing current buffer statistics and doing healthcheck. . .\n");
    if(byteAllocTracker() == TEST_SIZE_3)
    {
        printf("@test_success: total allocated bytes following deallocation matches!\n");
    }
    else
    {
        printf("@test_failure: total allocated bytes following deallocation does not match!\n");
    }

    if(freeSpaceTracker() == (2 * (BLOCK_SIZE) + 8 - TEST_SIZE_3))
    {
        printf("@test_success: total free space cumulative size following deallocation matches!\n");
    }
    else
    {
        printf(
            "@test_failure: total free space cumulative size following deallocation does not match!\n");
    }

    if(largeSpaceTracker() == (BLOCK_SIZE + 8))
    {
        printf("@test_success: largest contiguous free space following deallocation matches!\n");
    }
    else
    {
        printf(
            "@test_failure: largest contiguous free space following deallocation does not match!\n");
    }
    printf("=========================================================\n");

    /* EIGHTH TEST CASE -- FREE 15000 BYTES ALLOCATION */

    printf("@test_case_8: deallocating bytes %d bytes. . .\n", TEST_SIZE_3);
    my_free(buffer4);
    my_mallinfo();

    printf("@test_info: printing current buffer statistics and doing healthcheck. . .\n");
    if(byteAllocTracker() == 0)
    {
        printf("@test_success: total allocated bytes following deallocation matches!\n");
    }
    else
    {
        printf("@test_failure: total allocated bytes following deallocation does not match!\n");
    }

    if(freeSpaceTracker() == (2 * (BLOCK_SIZE) + 16))
    {
        printf("@test_success: total free space cumulative size following deallocation matches!\n");
    }
    else
    {
        printf(
            "@test_failure: total free space cumulative size following deallocation does not match!\n");
    }
    if(largeSpaceTracker() == (2 * (BLOCK_SIZE) + 16))
    {
        printf("@test_success: largest contiguous free space following deallocation matches!\n");
    }
    else
    {
        printf(
            "@test_failure: largest contiguous free space following deallocation does not match!\n");
    }
    printf("=========================================================\n");

    return 0;
}
