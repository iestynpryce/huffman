#include "file_stat.h"
#include "file_stat_error.h"
#include <stdlib.h>
#include <errno.h>

#define INIT_BUF_SIZE 24;

size_t fwrite_stat(const void *ptr, size_t size, size_t count, f_stat *stream)
{
	size_t write_count;

	/* Validate input */
	if (ptr == NULL || stream == NULL)
	{
		return E_UNEXPECTED_NULL_POINTER;
	}
	
	write_count = fwrite(ptr,size,count,stream->file);
	if (ferror(stream->file))
	{
		return E_FAILED_FILE_WRITE;
	}		
	stream->byte_count += write_count*size;
	
	return write_count;
}

int fputc_stat (int character, f_stat *stream)
{
	int ret_char;

	/* Validate input */
	if (stream == NULL)
	{
		return E_UNEXPECTED_NULL_POINTER;
	}

	ret_char = fputc(character,stream->file);
	if (ret_char == EOF)
	{
		return E_FAILED_FILE_WRITE;
	}
	stream->byte_count++;

	return ret_char;
}

int fgetc_stat(f_stat *stream)
{
	int char_val;

	if (stream == NULL)
	{
		return E_UNEXPECTED_NULL_POINTER;
	}

	if (stream->fully_buffered)
	{
		if (stream->buffer_ptr >= stream->buffer_usage)
		{
			char_val = EOF;
		}
		else
		{
			char_val = ((int*)stream->buffer)[stream->buffer_ptr];
			stream->buffer_ptr++;
		}
	}
	else
	{
		char_val = fgetc(stream->file);
		if (ferror(stream->file))
		{
			return errno;
		}
	}

	if (char_val != EOF && stream->fully_buffered == false)
	{
		stream->byte_count++;
		if (stream->buffer == NULL)
		{
			stream->buffer_size = INIT_BUF_SIZE;
			stream->buffer = calloc(stream->buffer_size,sizeof(int));
			if (stream->buffer == NULL)
			{
				/* Out of memory */
				perror("Unable to allocate memory for stream buffer");
				return E_OUT_OF_MEMORY;
			}
		}
		if (stream->buffer != NULL)
		{
			if (stream->buffer_size <= stream->buffer_usage)
			{
				int new_buffer_size = stream->buffer_size *2;
				void *tmp = realloc(stream->buffer,new_buffer_size*sizeof(int));
				if (tmp)
				{
					stream->buffer = tmp;
					stream->buffer_size = new_buffer_size;
				}
				else
				{
					/* Out of memory */
					perror("Unable to allocate memory for stream buffer");
					return E_OUT_OF_MEMORY;
				}
			}
			((int*)stream->buffer)[stream->buffer_ptr] = char_val;

			stream->buffer_usage += 1;
			stream->buffer_ptr = stream->buffer_usage;
		}
	}
	else
	{
		/* Mark that we've buffered the entire file */
		stream->fully_buffered = true;
	}

	return char_val;
}

int rewind_stat(f_stat *stream)
{
	if (stream == NULL)
	{
		return E_UNEXPECTED_NULL_POINTER;
	}

	stream->buffer_ptr = 0;
	rewind(stream->file);
	return stream->buffer_ptr;
}

int fflush_stat(f_stat *stream)
{
	if (stream == NULL)
	{
		return E_UNEXPECTED_NULL_POINTER;
	}
	return fflush(stream->file);
}

int fclose_stat(f_stat *stream)
{
	if (stream == NULL)
	{
		return E_UNEXPECTED_NULL_POINTER;
	}

	free(stream->buffer);
	return fclose(stream->file);
}
