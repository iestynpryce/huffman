/* Define enum of error codes for the file_stat.h abstraction   *
 * of common file access function 				*/
#ifndef FILE_STAT_ERROR_H
#define FILE_STAT_ERROR_H

enum file_stat_error {
	E_SUCCESS = 0,
	E_OUT_OF_MEMORY = -1,
	E_UNEXPECTED_NULL_POINTER = -2,
	E_FAILED_FILE_WRITE = -3,
};

#endif /* FILE_STAT_ERROR_H */
