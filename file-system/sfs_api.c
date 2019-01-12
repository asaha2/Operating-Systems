#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "disk_emu.h"
#include "sfs_api.h"   

/*
 NAME - ADITYA SAHA
 ID - 260453165
 */

int dirLoc = 0;
int numFiles;
directoryEntry *rootDir;
inodeEntry *inodeTable;
fileDescriptorEntry **descriptorTable;

/**********************************************************************************************/
void init_free()
{
    unsigned int *buff = malloc(BLOCKSIZE);
    int i;
    for(i = 0; i < (BLOCKSIZE) / sizeof(unsigned int); i++)
    {
        buff[i] = ~0;
    }
    write_blocks(FREELIST, FREELIST_SIZE, buff);
    free(buff);
}
/**********************************************************************************************/
void init_root()
{

    directoryEntry *buff = malloc(ALIGN(MAX_FILES*sizeof(directoryEntry)));

    int i;
    for(i = 0; i < MAX_FILES; i++)
    {
        buff[i] = (directoryEntry) {.filename = "\0", .inode = 5000};
    }

    write_blocks(DIRECTORY_LOCATION, DIRECTORY_SIZE, buff);
    free(buff);
}
/**********************************************************************************************/
void init_itable()
{

    inodeEntry *buff = malloc(ALIGN((MAX_FILES+1)*sizeof(inodeEntry)));
    int i;
    for(i = 0; i < MAX_FILES + 1; i++)
    {
        buff[i].mode = 0;
        buff[i].linkCount = 0;
        buff[i].size = 0;
        buff[i].singleIndirectPtr = 5000;
        int j;
        for(j = 0; j < 12; j++)
        {
            buff[i].directptr[j] = 5000;
        }
    }

    write_blocks(INODE_TABLE, INODE_TABLE_SIZE, buff);
    free(buff);
}
/**********************************************************************************************/
void do_free(unsigned int index)
{
    if(index > NUM_BLOCKS)
    {
        fprintf(stderr, "@error: overflow detected -- will not allocate\n");
        return;
    }
    int byte = index / (8 * sizeof(unsigned int));
    int bit = index % (8 * sizeof(unsigned int));
    unsigned int *buff = malloc(BLOCKSIZE);

    read_blocks(FREELIST, FREELIST_SIZE, buff);
    buff[byte] |= 1 << bit; //sets bit to 1(free)
    write_blocks(FREELIST, FREELIST_SIZE, buff);
    free(buff);
}
/**********************************************************************************************/
int find_available()
{
    unsigned int *buff = malloc(BLOCKSIZE);

    read_blocks(FREELIST, FREELIST_SIZE, buff);
    int i;
    for(i = 0; i < (BLOCKSIZE) / sizeof(unsigned int); i++)
    {
        int find = ffs(buff[i]);
        if(find)
        {
            if(find + i * 8 * sizeof(unsigned int) - 1 < BLOCKSIZE)
                return find + i * 8 * sizeof(unsigned int) - 1;
        }
    }
    return -1;
}
/**********************************************************************************************/
void do_alloc(unsigned int index)
{

    int byte = index / (8 * sizeof(unsigned int));
    int bit = index % (8 * sizeof(unsigned int));
    unsigned int *buff = malloc(BLOCKSIZE);
    if(index >= BLOCKSIZE)
    {
        fprintf(stderr, "@error: overflow detected -- will not allocate\n");
        return;
    }

    read_blocks(FREELIST, FREELIST_SIZE, buff);
    buff[byte] &= ~(1 << bit);
    write_blocks(FREELIST, FREELIST_SIZE, buff);
    free(buff);
}
/**********************************************************************************************/
void init_superblock()
{

    unsigned int *buff = malloc(BLOCKSIZE);

    buff[0] = MAGIC_NUMBER;
    buff[1] = BLOCKSIZE;
    buff[2] = NUM_BLOCKS;
    buff[3] = FREELIST;
    buff[4] = DIRECTORY_LOCATION;
    buff[5] = DIRECTORY_SIZE;
    buff[6] = INODE_TABLE;
    buff[7] = INODE_TABLE_SIZE;
    buff[8] = START;

    write_blocks(SUPERBLOCK, SUPERBLOCK_SIZE, buff);
    free(buff);
}
/**********************************************************************************************/
int sfs_remove(char *file)
{
    int i;
    for(i = 0; i < MAX_FILES; i++)
    {
        /*printf("iteration no. %d\n", i);
         printf("rootDir[%d].filename = %s\n", i, rootDir[i].filename);
         printf("rootDir[%d].inode = %d\n", i, rootDir[i].inode);
         printf("file = %s\n", file);*/
        //if(strncmp(rootDir[i].filename, file, MAXFILENAME + 1) == 0 && rootDir[i].inode != 5000)  //if we find the file remove data and set space to free
        if(strcmp(rootDir[i].filename, file) == 0 && rootDir[i].inode != 5000)
        {
            directoryEntry *removeEntry = &(rootDir[i]);  //update root directory
            int inode = removeEntry->inode;
            strcpy(removeEntry->filename, "\0");
            inodeEntry *inodeRemove = &(inodeTable[inode]);
            removeEntry->inode = 5000;
            int k;

            if(inodeRemove->linkCount > 12)
            {
                unsigned int *buff = malloc(BLOCKSIZE);
                read_blocks(START + inodeRemove->singleIndirectPtr, 1, buff);

                for(k = 0; k < inodeRemove->linkCount - 12; k++)
                {
                    do_free(buff[k]);
                }

                free(buff);
                inodeRemove->linkCount = inodeRemove->linkCount - 12;
                do_free(inodeRemove->singleIndirectPtr);
                inodeRemove->singleIndirectPtr = 5000;
            }
            for(k = 0; k < 12; k++)
            {
                do_free(inodeRemove->directptr[k]);
                inodeRemove->directptr[k] = 5000;
            }

            inodeRemove->mode = 0;
            inodeRemove->linkCount = 0;
            inodeRemove->size = 0;
            write_blocks(INODE_TABLE, INODE_TABLE_SIZE, inodeTable);
            return 0;
        }
    }
    fprintf(stderr,
        "@error: no filename match found -- check offset or display directory contents\n");
    return -1;
}
/**********************************************************************************************/
int sfs_getnextfilename(char* filename)
{
    //printf("current index value is %d\n", dirLoc);
    if(dirLoc == MAX_FILES)
    {
        printf("@error: no. of files overflow -- remove some files or scrap filesystem");
        return 0;
    }
    strncpy(filename, rootDir[dirLoc].filename, MAXFILENAME + 1);

    //printf("Name of next file is %s\n", filename);
    if((int) strlen(filename) == 0)
    {
        return 0;
    }

    dirLoc++;
    return 1;
}
/**********************************************************************************************/
int sfs_getfilesize(const char* path)
{
    int i;
    // printf("sfs_getfilesize(path) = %s\n", path);
    for(i = 0; i < MAX_FILES; i++)
    {
        // printf("rootDir[%d].filename = %s\n", i, rootDir[i].filename);
        //if(strncmp(rootDir[i].filename, path, MAXFILENAME + 1) == 0) 
        if(strcmp(rootDir[i].filename, path) == 0)
        {
            unsigned int inode = rootDir[i].inode;
            unsigned int size = inodeTable[inode].size;
            return size;
        }
    }
    return -1;  //if not return -1;
}
/**********************************************************************************************/
int sfs_fread(int fileID, char *buf, int length)
{ //returns -1 for failure
    if(descriptorTable[fileID] == NULL || length < 0 || fileID >= numFiles || buf == NULL)
    {
        //fprintf(stderr, "fileID %d\n numfiles %d\n length %d\n", fileID,numFiles, length);
        fprintf(stderr, "@error: read error, check conditions\n");
        return -1;
        //exit(EXIT_FAILURE);
    }

    fileDescriptorEntry *readFile = descriptorTable[fileID];
    inodeEntry *inode = &(inodeTable[readFile->inode]);

    if(readFile->inode == 5000)
    {
        fprintf(stderr, "@error: restricted inode reached -- shouldn't get here\n");
        return -1;
    }

    if(readFile->rwPointer + length > inode->size)
    {
        length = inode->size - readFile->rwPointer;
    }

    int readLength = length;
    char *diskBuffer = malloc(BLOCKSIZE);

    int block = (readFile->rwPointer) / BLOCKSIZE;
    int bytes = (readFile->rwPointer) % BLOCKSIZE;
    int eofBlock = (inode->size) / BLOCKSIZE;
    unsigned int readLoc;
    int offset = 0;
    if(block > 139)
    {
        fprintf(stderr, "@error: maximum filesize reached -- overflow detected\n");
        return -1;
    }

    else if(block > 11)
    {
        unsigned int *indirectBuff = malloc(BLOCKSIZE);
        read_blocks(START + inode->singleIndirectPtr, 1, indirectBuff);
        readLoc = indirectBuff[block - 12];
        free(indirectBuff);
    }

    else
        readLoc = inode->directptr[block];

    while(length > 0)
    {
        read_blocks(START + readLoc, 1, diskBuffer);
        int bytesRead;

        if(BLOCKSIZE - bytes < length)
        {
            bytesRead = BLOCKSIZE - bytes;
        }
        else
            bytesRead = length;

        memcpy(&buf[offset], &diskBuffer[bytes], bytesRead);

        length -= (bytesRead);
        offset += (bytesRead);
        bytes = 0;

        if(length > 0)
        {
            block++;
            if(eofBlock < block)
                return -1;

            if(block > 139)
            {
                fprintf(stderr, "@error: cannot read more than maximum filesize -- strict check\n");
                return -1;
            }

            else if(block > 11)
            {
                unsigned int *nextBuff = malloc(BLOCKSIZE);
                read_blocks(START + inode->singleIndirectPtr, 1, nextBuff);
                readLoc = nextBuff[block - 12];
                free(nextBuff);
            }
            else
                readLoc = inode->directptr[block];
        }
    }

    free(diskBuffer);
    readFile->rwPointer += readLength;
    return readLength;
}
/**********************************************************************************************/
int sfs_fwrite(int fileID, const char *buf, int length)
{
    if(fileID >= numFiles || descriptorTable[fileID] == NULL || buf == NULL || length < 0)
    {
        fprintf(stderr, "@error: write failed 1 -- check condition\n");
        return -1;
    }

    int writeLength = length;
    fileDescriptorEntry *writeFile = descriptorTable[fileID];
    inodeEntry *inode = &(inodeTable[writeFile->inode]);

    if(writeFile->inode == 5000)
    {
        fprintf(stderr, "@error: restricted inode reached\n");
        return -1;
    }

    char *diskBuffer = malloc(BLOCKSIZE);
    int block = (writeFile->rwPointer) / BLOCKSIZE;
    int bytes = (writeFile->rwPointer) % BLOCKSIZE;
    int eofBlock = (inode->size) / BLOCKSIZE;

    unsigned int writeLoc;
    int offset = 0;
    if(block > 139)
    {
        fprintf(stderr, "@error: max file size reached -- overflow detected\n");
        return -1;
    }

    else if(block > 11)
    {
        unsigned int *indirectBuff = malloc(BLOCKSIZE);
        read_blocks(START + inode->singleIndirectPtr, 1, indirectBuff);
        writeLoc = indirectBuff[block - 12];
        free(indirectBuff);
    }

    else
        writeLoc = inode->directptr[block];

    if(writeLoc == -1)
    {
        fprintf(stderr, "@error: write failure\n");
        return -1;
    }

    while(length > 0)
    {

        read_blocks(START + writeLoc, 1, diskBuffer);
        int byteWrite;
        if(BLOCKSIZE - bytes < length)
        {
            byteWrite = BLOCKSIZE - bytes;
        }
        else
            byteWrite = length;

        memcpy(&diskBuffer[bytes], &buf[offset], byteWrite);
        write_blocks(START + writeLoc, 1, diskBuffer);

        length -= (byteWrite);
        offset += (byteWrite);
        bytes = 0;
        block++;
        if(length > 0)
        {
            if(block > 139)
            {
                fprintf(stderr, "@error: maximum filesize reached -- overflow detected\n");
                return -1;
            }

            else if(eofBlock < block)
            {
                if(block == 12 && inode->singleIndirectPtr == 5000)
                {
                    int indirPtr = find_available();
                    do_alloc(indirPtr);
                    inode->singleIndirectPtr = indirPtr;
                }

                int next = find_available();
                do_alloc(next);
                if(next == -1)
                    return -1;

                writeLoc = next;
                if(block > 11)
                {
                    unsigned int *nextBuff = malloc(BLOCKSIZE);
                    read_blocks(START + inode->singleIndirectPtr, 1, nextBuff);
                    nextBuff[block - 12] = writeLoc;
                    write_blocks(START + inode->singleIndirectPtr, 1, nextBuff);
                    free(nextBuff);
                }
                else
                    inode->directptr[block] = writeLoc;

                inode->linkCount++;
            }

            else
            {
                if(block > 11)
                {
                    unsigned int *nextBuff = malloc(BLOCKSIZE);
                    read_blocks(START + inode->singleIndirectPtr, 1, nextBuff);
                    writeLoc = nextBuff[block - 12];
                    free(nextBuff);
                }
                else
                    writeLoc = inode->directptr[block];
            }
        }
    }

    if(writeFile->rwPointer + writeLength > inode->size)
    {
        inode->size = writeFile->rwPointer + writeLength;
    }

    writeFile->rwPointer += writeLength;

    write_blocks(INODE_TABLE, INODE_TABLE_SIZE, inodeTable);
    free(diskBuffer);
    return writeLength;
}
/**********************************************************************************************/
int sfs_fseek(int fileID, int offset)
{
    if(fileID >= numFiles || descriptorTable[fileID] == NULL
        || inodeTable[descriptorTable[fileID]->inode].size < offset)
    {
        fprintf(stderr,
            "@error: no file descriptor entry found -- check offset or plug in debugger\n");
        return -1;
    }

    descriptorTable[fileID]->rwPointer = offset;
    return 0;
}
/**********************************************************************************************/
int sfs_fopen(char *name)
{
    if(strlen(name) > MAX_FNAME_LENGTH)
    {
        fprintf(stderr, "@error: filename length exceeded -- increase in header\n");
        return -1;
    }

    if(rootDir == NULL)
    {
        fprintf(stderr, "@error: filesystem not properly initialized -- check mksfs status\n");
        return -1;
    }

    int i;
    for(i = 0; i < MAX_FILES; i++)
    {
        if(strncmp(rootDir[i].filename, name, MAXFILENAME + 1) == 0)
        {
            if(rootDir[i].inode == 5000)
            {
                fprintf(stderr,
                    "@error: restricted inode reached -- check leak or scrap filesystem and re-initialize\n");
                return -1;
            }

            int j, entry = -1;
            for(j = 0; j < numFiles; j++)
            {
                if(descriptorTable[j] && rootDir[i].inode == descriptorTable[j]->inode
                    && rootDir[i].inode != 5000)
                {
                    return j;
                }
            }

            for(j = 0; j < numFiles; j++)
            {
                if(descriptorTable[j] == NULL)
                {
                    descriptorTable[j] = malloc(sizeof(fileDescriptorEntry));
                    entry = j;
                    break;
                }
            }

            if(entry == -1)
            {
                if(descriptorTable == NULL)
                    descriptorTable = malloc(sizeof(fileDescriptorEntry*));
                else
                    descriptorTable = realloc(descriptorTable,
                        (1 + numFiles) * (sizeof(fileDescriptorEntry*)));
                descriptorTable[numFiles] = (fileDescriptorEntry *) malloc(
                    sizeof(fileDescriptorEntry));
                entry = numFiles;
                numFiles++;
            }

            fileDescriptorEntry *update = descriptorTable[entry];
            if(update == NULL)
            {
                fprintf(stderr, "@error: file opening failed -- no file descriptor entry found\n");
                return -1;
            }

            update->rwPointer = inodeTable[rootDir[i].inode].size;
            update->inode = rootDir[i].inode;
            return entry;
        }
    }

    for(i = 0; i < MAX_FILES; i++)
    {
        if(strncmp(rootDir[i].filename, "\0", 1) == 0 && rootDir[i].inode == 5000)
        {
            int entry = -1;
            int j;
            for(j = 0; j < numFiles; j++)
            {
                if(descriptorTable[j] == NULL)
                {
                    descriptorTable[j] = malloc(sizeof(fileDescriptorEntry));
                    entry = j;
                    break;
                }
            }

            if(entry == -1)
            {
                if(descriptorTable == NULL)
                    descriptorTable = malloc(sizeof(fileDescriptorEntry*));
                else
                    descriptorTable = realloc(descriptorTable,
                        (1 + numFiles) * (sizeof(fileDescriptorEntry*)));

                descriptorTable[numFiles] = (fileDescriptorEntry *) malloc(
                    sizeof(fileDescriptorEntry));
                entry = numFiles;
                numFiles++;
            }

            fileDescriptorEntry *newEntry = descriptorTable[entry];

            if(newEntry == NULL)
            {
                fprintf(stderr,
                    "@error: new file creation failed -- no file descriptor entry found\n");
                return -1;
            }

            int inode = -1;
            int k;
            for(k = 1; k < MAX_FILES + 1; k++)
            {
                if(inodeTable[k].mode == 0)
                {
                    inodeTable[k].mode = 1;
                    inode = k;
                    break;
                }
            }

            if(inode == -1)
            {
                fprintf(stderr,
                    "@error: maximum inode no. reached -- scrap filesystem and re-initialize\n");
                return -1;
            }

            int writeLoc = find_available();
            if(writeLoc == -1)
                return -1;

            do_alloc(writeLoc);
            newEntry->rwPointer = 0;
            newEntry->inode = inode;

            strncpy(rootDir[i].filename, name, MAXFILENAME + 1);
            rootDir[i].inode = inode;
            write_blocks(DIRECTORY_LOCATION, DIRECTORY_SIZE, rootDir);

            inodeTable[inode].size = 0;
            inodeTable[inode].linkCount = 1;
            inodeTable[inode].mode = 1;
            inodeTable[inode].directptr[0] = writeLoc;
            write_blocks(INODE_TABLE, INODE_TABLE_SIZE, inodeTable);
            return entry;
        }
    }
    return -1;
}
/**********************************************************************************************/
int sfs_fclose(int fileID)
{
    if(fileID >= numFiles || descriptorTable[fileID] == NULL)
    {
        fprintf(stderr, "@error: file has already been closed -- cannot re-close\n");
        return -1;
    }

    free(descriptorTable[fileID]);
    descriptorTable[fileID] = NULL;
    return 0;
}
/**********************************************************************************************/
void mksfs(int fresh)
{
    if(fresh == 1)
    {
        if(access(FILENAME, F_OK) != -1)
            unlink(FILENAME);

        if(init_fresh_disk(FILENAME, BLOCKSIZE, NUM_BLOCKS) != 0)
        {
            fprintf(stderr, "@error: fresh file system creation failed -- check method\n");
            exit(EXIT_FAILURE);
        }

        init_superblock();
        init_free();
        init_root();
        init_itable();

        inodeEntry *inode = malloc(ALIGN((MAX_FILES+1)*sizeof(inodeEntry)));
        read_blocks(INODE_TABLE, INODE_TABLE_SIZE, inode);

        if(inode == NULL)
        {
            exit(EXIT_FAILURE);
        }

        inode[0].size = DIRECTORY_SIZE * BLOCKSIZE;
        inode[0].linkCount = DIRECTORY_SIZE;
        inode[0].mode = 1;

        if(DIRECTORY_SIZE > 12)
        {
            inode[0].singleIndirectPtr = find_available();
            do_alloc(inode[0].singleIndirectPtr);
            unsigned int *buff = malloc(BLOCKSIZE);
            write_blocks(START + inode[0].singleIndirectPtr, 1, buff);
            free(buff);
        }

        int k;
        for(k = 0; k < DIRECTORY_SIZE; k++)
        {
            if(k > 11)
            {
                unsigned int *buff = malloc(BLOCKSIZE);
                read_blocks(START + inode[0].singleIndirectPtr, 1, buff);
                buff[k - 12] = DIRECTORY_LOCATION + k;
                write_blocks(START + inode[0].singleIndirectPtr, 1, buff);
                free(buff);
            }

            else
            {
                inode[0].directptr[k] = DIRECTORY_LOCATION + k;
            }
        }

        write_blocks(INODE_TABLE, INODE_TABLE_SIZE, inode);
        free(inode);
    }

    else if(fresh == 0)
    {
        if(init_disk(FILENAME, BLOCKSIZE, NUM_BLOCKS) != 0)
        {
            fprintf(stderr, "@error: failed to pull up previous disk data -- check method\n");
            exit(EXIT_FAILURE);
        }
    }

    int *superblock = malloc(BLOCKSIZE * SUPERBLOCK_SIZE);
    read_blocks(SUPERBLOCK, SUPERBLOCK_SIZE, superblock);
    rootDir = malloc(ALIGN(sizeof(directoryEntry)*MAX_FILES));
    read_blocks(DIRECTORY_LOCATION, DIRECTORY_SIZE, rootDir);
    inodeTable = malloc(ALIGN(sizeof(inodeEntry)*(MAX_FILES+1)));
    read_blocks(INODE_TABLE, INODE_TABLE_SIZE, inodeTable);
    numFiles = 0;
    descriptorTable = NULL;
}
