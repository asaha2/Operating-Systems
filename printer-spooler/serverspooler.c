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
#include <signal.h>

#define BUFF_SIZE   10          /* total number of slots */
#define INIT_SIZE   0           /* initial number of full slots */

struct sbuf_t
{
    int buf[BUFF_SIZE]; /* shared circular buffer for storing # of pages */
    int id[BUFF_SIZE]; /* shared circular buffer for storing client ID */
    int duration[BUFF_SIZE]; /* shared circular buffer for storing time duraion of jobs */

    int in; /* buf[in%BUFF_SIZE] is the first empty slot */
    int out; /* buf[out%BUFF_SIZE] is the first full slot */

    sem_t full; /* keep track of the number of full spots */
    sem_t empty; /* keep track of the number of empty spots */
    sem_t mutex; /* enforce mutual exclusion to shared data */

};

/* global variable declarations  */
typedef struct sbuf_t shared;
int client_id, pages, time_duration;
void sig_handler(int signo);          // function prototype for signal handling
const char *name = "/shm-spooler";
shared *shared_mem_ptr;
int shm_fd;

int main(void)
{
    /*
     NAME- ADITYA SAHA
     ID- 260453165
     */

    /* open or create the shared memory segment as if it was a file */
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

    /* if fails exit process */
    if(shm_fd == -1)
    {
        printf("@error_info: failed to allocate shared memory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* configure the size of the shared memory segment */
    ftruncate(shm_fd, sizeof(shared));

    /* map the shared memory segment to the address space of the process */
    shared_mem_ptr = (shared *) mmap(0, sizeof(shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,
        0);

    /* exit if memory mapping fails */
    if(shared_mem_ptr == MAP_FAILED)
    {
        printf("@error_info: failed to map shared memory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* initialize semaphores */
    if(sem_init(&(shared_mem_ptr->full), 1, INIT_SIZE) == -1)
    {
        printf("@error_info: semaphore \"full\" initialization failed!\n");
        exit(EXIT_FAILURE);
    }

    if(sem_init(&(shared_mem_ptr->empty), 1, BUFF_SIZE) == -1)
    {
        printf("@error_info: emaphore \"empty\" initialization failed!\n");
        exit(EXIT_FAILURE);
    }

    if(sem_init(&(shared_mem_ptr->mutex), 1, 1) == -1)
    {
        printf("@error_info: emaphore \"mutex\" initialization failed!\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {

        /* if fails to catch SIGINT signal */
        if(signal(SIGINT, sig_handler) == SIG_ERR)
        {
            printf("\n@error_info: can't catch SIGINT\n");
        }

        int value, currentpos, currentbuf, currentid, currenttime;
        sem_getvalue(&(shared_mem_ptr->full), &value);
        if(value <= 0)
        {
            printf("No print jobs in the queue -- the printer is going to sleep!\n");
        }

        sem_wait(&(shared_mem_ptr->full));
        sem_wait(&(shared_mem_ptr->mutex));

        currentpos = shared_mem_ptr->out;
        currentbuf = shared_mem_ptr->buf[currentpos];
        currentid = shared_mem_ptr->id[currentpos];
        currenttime = shared_mem_ptr->duration[currentpos];
        shared_mem_ptr->out = (shared_mem_ptr->out + 1) % BUFF_SIZE;

        printf("Printing %d pages from Client %d in Buffer[%d]\n", currentbuf, currentid,
            currentpos);

        sem_post(&(shared_mem_ptr->mutex));
        sem_post(&(shared_mem_ptr->empty));

        sleep(currenttime);
        printf("Printer has completed priting!\n");

    }

    return 1;
}

/*  helper function to deallocate memory and exit gracefully
 once Ctl + c is pressed */
void sig_handler(int signo)
{
    if(signo == SIGINT)
        printf("\nReceived SIGINT\n");
    printf("Deallocating memory and exiting. . .\n");
    close(shm_fd);
    munmap(0, sizeof(shared));
    shm_unlink(name);
    exit(EXIT_SUCCESS);
}
