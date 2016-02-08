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
