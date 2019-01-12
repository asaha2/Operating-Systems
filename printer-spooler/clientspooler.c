#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <semaphore.h>
#include <string.h>

#define BUFF_SIZE   10			/* total number of slots */
#define INIT_SIZE   0			/* initial number of full slots */

struct sbuf_t
{
    int buf[BUFF_SIZE]; /* shared circular buffer */
    int id[BUFF_SIZE]; /* shared circular buffer for storing client ID */
    int duration[BUFF_SIZE]; /* shared circular buffer for storing time duration of jobs */

    int in; /* buf[in%BUFF_SIZE] is the first empty slot */
    int out; /* buf[out%BUFF_SIZE] is the first full slot */

    sem_t full; /* keep track of the number of full spots */
    sem_t empty; /* keep track of the number of empty spots */
    sem_t mutex; /* enforce mutual exclusion to shared data */

};

/* global variable declarations */
typedef struct sbuf_t shared;
int client_id, pages, time_duration;
const char *name = "/shm-spooler";
shared *shared_mem_ptr;
int shm_fd;

int main(int argc, char *argv[])
{

    /*
     NAME- ADITYA SAHA
     ID- 260453165
     */

    /* open the shared memory segment as if it was a file */
    shm_fd = shm_open(name, O_RDWR, 0666);

    /* if fails exit process */
    if(shm_fd == -1)
    {
        printf("@error_info: failed to allocate shared memory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* map the shared memory segment to the address space of the process */
    shared_mem_ptr = (shared *) mmap(0, sizeof(shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,
        0);

    /* exit if memory mapping fails */
    if(shared_mem_ptr == MAP_FAILED)
    {
        printf("@error_info: failed to map shared memory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* type/error checking for input parameters passed */
    if(argc != 4)
    {
        printf("@error_info: not enough input arguments passed\n");
        printf("@usage_info: ./client <client_id> <# of pages> <sleep duration>\n");
        exit(EXIT_FAILURE);
    }

    if(sscanf(argv[1], "%i", &client_id) != 1)
    {
        printf("@error_info: input argument not an integer\n");
        printf("@usage_info: ./client <client_id> <# of pages> <sleep duration>\n");
        exit(EXIT_FAILURE);
    }

    else if(sscanf(argv[2], "%i", &pages) != 1)
    {
        printf("@error_info: input argument not an integer\n");
        printf("@usage_info: ./client <client_id> <# of pages> <sleep duration>\n");
        exit(EXIT_FAILURE);
    }

    else if(sscanf(argv[3], "%i", &time_duration) != 1)
    {
        printf("@error_info: input argument not an integer\n");
        printf("@usage_info: ./client <client_id> <# of pages> <sleep duration>\n");
        exit(EXIT_FAILURE);
    }

    else if(client_id < 1 || pages < 1 || time_duration < 1)
    {
        printf("@error_info: input argument(s) not a positive integer\n");
        printf("@usage_info: ./client <client-id> <# of pages> <sleep duration>\n");
        exit(EXIT_FAILURE);
    }

    int value, currentpos;
    sem_getvalue(&(shared_mem_ptr->empty), &value);
    if(value <= 0)
    {
        printf("No space left in queue -- waiting!\n");
    }

    sem_wait(&(shared_mem_ptr->empty));
    sem_wait(&(shared_mem_ptr->mutex));

    currentpos = shared_mem_ptr->in;
    shared_mem_ptr->buf[currentpos] = pages;
    shared_mem_ptr->id[currentpos] = client_id;
    shared_mem_ptr->duration[currentpos] = time_duration;
    shared_mem_ptr->in = (shared_mem_ptr->in + 1) % BUFF_SIZE;

    sem_post(&(shared_mem_ptr->mutex));
    sem_post(&(shared_mem_ptr->full));

    return 1;
}
