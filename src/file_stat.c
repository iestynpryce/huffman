#include "file_stat.h"

size_t fwrite_stat(const void *ptr, size_t size, size_t count, f_stat *stream)
{
	size_t write_count;
	
	write_count = fwrite(ptr,size,count,stream->file);
	stream->byte_count += write_count*size;
	
	return write_count;
}

int fputc_stat (int character, f_stat *stream)
{
	int ret_char;
	ret_char = fputc(character,stream->file);
	if (ret_char != EOF)
	{
		stream->byte_count++;
	}
	return ret_char;
}

int fgetc_stat(f_stat *stream)
{
	int char_val;
	
	char_val = fgetc(stream->file);
	if (char_val != EOF)
	{
		stream->byte_count++;
	}

	return char_val;
}

void rewind_stat(f_stat *stream)
{
	rewind(stream->file);
}

int fflush_stat(f_stat *stream)
{
	return fflush(stream->file);
}

int fclose_stat(f_stat *stream)
{
	return fclose(stream->file);
}
