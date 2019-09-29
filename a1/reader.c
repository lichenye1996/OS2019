/* Author: Robbert van Renesse 2018
 *
 * The interface is as follows:
 *	reader_t reader_create(int fd);
 *		Create a reader that reads characters from the given file descriptor.
 *
 *	char reader_next(reader_t reader):
 *		Return the next character or -1 upon EOF (or error...)
 *
 *	void reader_free(reader_t reader):
 *		Release any memory allocated.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "shall.h"

struct reader {
	int fd;
};

reader_t reader_create(int fd){
	reader_t reader = (reader_t) calloc(1, sizeof(*reader));
	reader->fd = fd;
	return reader;
}

char reader_next(reader_t reader){
	//Originsl Code
	//char c;
	//int n = read(reader->fd, &c, 1);
	//return n == 1 ? c : EOF;

	//Read Up to 512 Characters at a Time
	static char c[512];
	static int readerLocation = 0;
	static int n = 0;
	char result;
	if (n > 0 && readerLocation <= n - 1) {
		result = c[readerLocation];
		readerLocation++;
		return result;
	}
	else {
		n = read(reader->fd, &c, sizeof(c));
		readerLocation = 0;
		if (n <= 0) {
			return EOF;
		}
		else {
			result = c[readerLocation];
                	readerLocation++;
                	return result;
		}
	}
}

void reader_free(reader_t reader){
	free(reader);
}
