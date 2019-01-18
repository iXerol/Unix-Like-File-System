#ifndef commands_h
#define commands_h

void ls();

void chmod(char* filename, char* username, int privilege);

void chown(char* filename, char* username);

void chgrp(char* filename, char* groupname);

void pwd();

void cd(char* path);
//a: No such file or directory
//a: Not a directory

void mkdir(char* directoryName);

void rmdir(char* directoryName);

void mv(char* pathBefore, char* pathAfter);

void cp(char* pathOriginal, char* pathDuplicate);

void rm(char* filename);

void ln();

void cat(char* filename);

void passwd();

void umask();

#endif /* commands_h */
