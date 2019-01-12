#include "disk_emu.h"

/*
 NAME- ADITYA SAHA
 ID- 260453165
 */

#define DISK_FILE "adi_sfs_disk.disk"
#define MAX_FNAME_LENGTH 20
#define MAX_EXTENSION_LENGTH 3
#define MAX_INODE_SIZE 4
#define BLOCK_SIZE 512
#define MAX_BLOCKS 2048
#define MAX_FILES 140
#define MAX_INODES 143
#define MAXFILENAME 60
#define MAGIC_NUMBER 256            
#define BLOCKSIZE 512               
#define NUM_BLOCKS 6144  
#define SUPERBLOCK 0                
#define SUPERBLOCK_SIZE 1           
#define FREELIST 1          
#define FREELIST_SIZE 1             
#define DIRECTORY_LOCATION 2        
#define DIRECTORY_SIZE 4            
#define INODE_TABLE DIRECTORY_LOCATION + DIRECTORY_SIZE
#define INODE_TABLE_SIZE 13
#define START INODE_TABLE + INODE_TABLE_SIZE
#define FILENAME "aditya.sfs"
#define ALIGN(x)((x/BLOCKSIZE + 1) * BLOCKSIZE)

typedef struct directoryEntry
{
    char filename[MAXFILENAME + 1];
    unsigned int inode;
} directoryEntry;

typedef struct inodeEntry
{
    unsigned int mode;
    unsigned int linkCount;
    unsigned int size;
    unsigned int directptr[12];
    unsigned int singleIndirectPtr;
} inodeEntry;

typedef struct fileDescriptorEntry
{
    unsigned int inode;
    unsigned int rwPointer;
} fileDescriptorEntry;

void mksfs(int fresh);
int sfs_getnextfilename(char *fname);
int sfs_getfilesize(const char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char *file);

void init_root();
void init_itable();
void do_free(unsigned int index);
int find_available();
void init_free();
void do_alloc(unsigned int index);
void init_superblock();

