#ifndef FS_H
#define FS_H

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <direct.h>
#include <errno.h>
 
#ifdef _WIN32
#pragma warning(disable:4996)
#endif

#define FO_RDWR  "wb"
#define FO_CRDWR "wb+"
#define FO_RD    "rb"

typedef FILE* FILEHANDLE;

#define IsValidFile(f)  (f!=NULL)
#define fileopen  fopen
#define fileclose fclose
#define filelseek fseek


inline int fileread(FILEHANDLE fd, void *buf, unsigned int nbyte)
{
	return (int )fread(buf, nbyte, 1, fd); 
}

inline int filewrite(FILEHANDLE fd, const void *buf, unsigned int nbyte)
{
	return (int )fwrite(buf, nbyte, 1, fd);
}



#endif  //FS_H


