#file-system

Implementation of simple file system that can be mounted by a user under any directory in Linux. The API implemented interfaces with the FUSE wrapper functions to test the functionality and integrity of the file system.

##API implemented

void mksfs(int fresh);                              // creates the file system 
int sfs_getnextfilename(char *fname);               // get the name of the next file in directory
int sfs_getfilesize(const char* path);              // get the size of the given file
int sfs_fopen(char *name);                          // opens the given file
void sfs_fclose(int fileID);                        // closes the given file
void sfs_fwrite(int fileID, char *buf, int length); // write buf characters into disk
void sfs_fread(int fileID, char *buf, int length);  // read characters from disk into buf
void sfs_fseek(int fileID, int loc);                // seek to the location from beginning
int sfs_remove(char *file);                         // removes a file from the filesystem

##Implementation detail

The mksfs() formats the virtual disk implemented by the disk emulator and creates an instance of the
simple file system on top of it. The mksfs() has a fresh flag to signal that the file system should be created
from scratch. If flag is false, the file system is opened from the disk (i.e., we assume that a valid file system
is already there in the file system. The support for persistence is important so you can reuse existing data
or create a new file system.

The sfs_getnextfilename(char *fname) copies the name of the next file in the directory into fname
and returns non zero if there is a new file. Once all the files have been returned, this function returns 0. So,
you should be able to use this function to loop through the directory. In implementing this function, you
need to ensure that the function remembers the current position in the directory at each call. Remember in
SFS we have a single-level directory. The sfs_getfilesize(char *path) returns the size of a given file.

The sfs_fopen() opens a file and returns the index that corresponds to the newly opened file in the file
descriptor table. If the file does not exist, it creates a new file and sets its size to 0. If the file exists, the file
is opened in append mode (i.e., set the file pointer to the end of the file). The sfs_fclose() closes a file,
i.e., removes the entry from the open file descriptor table. The sfs_fwrite() writes the given number of
bytes of buffered data in buf into the open file, starting from the current file pointer. This in effect could
increase the size of the file by the given number of bytes (it may not increase the file size by the number of
bytes written if the write pointer is located at a location other than the end of the file). The sfs_fseek()
moves the read/write pointer (a single pointer in SFS) to the given location. The sfs_remove() removes the
file from the directory entry, releases the file allocation table entries and releases the data blocks used by
the file, so that they can be used by new files in the future.

A file system is somewhat different from other components because it maintains data structures in
memory as well as disk! The disk data structures are important to manage the space in disk and allocate and
de-allocate the disk space in an intelligent manner. Also, the disk data structures indicate where a file is
allocated. This information is necessary to access the file.
