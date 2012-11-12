/* Abstraction of file read and write functions to collect data *
 * about the reads and writes.                                  */
#ifndef FILE_STAT_H
#define FILE_STAT_H

#include <stdio.h>

/* Structure for file stream and its statistics */
typedef struct file_stat
{
	FILE    *file;
	size_t   byte_count;
} f_stat;

/* Equivalent of fwrite */
size_t fwrite_stat(const void *ptr, size_t size, size_t count, f_stat *stream);

/* Equivalent of fputc */
int fputc_stat (int character, f_stat *stream);

/* Eqivalent of fgetc */
int fgetc_stat(f_stat *stream);

/* Equivalent of rewind */
void rewind_stat(f_stat *stream);

/* Equivalent of fflush */
int fflush_stat(f_stat *stream);

/* Equivalent of fclose */
int fclose_stat(f_stat *stream);

#endif
