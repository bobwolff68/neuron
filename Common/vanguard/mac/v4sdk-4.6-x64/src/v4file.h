
/**
 * @file: v4file.h
 * file i/o operations
 * verbose output support
 *
 * Project: VSS H.264 Codec
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4FILE_H__
#define __V4FILE_H__

#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>

#if defined(WIN32) || defined(WIN64)
// setmode function for stdin "binarization"
#include <io.h>
#endif

#ifndef O_BINARY
	#define O_BINARY 0
#endif

FILE *file_in  = NULL;	// global input file handle
FILE *file_out = NULL;	// global output file handle
int verbose    = 0;		// global verbose level

static void verbose_print(int level, char *fmt, ...)
{
	if (level <= verbose)
	{
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}
}

static int open_std_files(char *input_file, char *output_file);
static void close_std_files(void);

/**
* @return number of bytes read
*/
static int read_input_file(void *buf, int buf_size);

/**
* @return number of bytes written
*/
static int write_output_file(const void *buf, int buf_size);

static int rewind_input_file(void);

/////////////////////////////////////////////////////////////////////////////////
static int open_std_files(char *input_file, char *output_file)
{
	// input file
	if (!input_file)
	{
		verbose_print(0, "Error: input file not specified\n");
		return -1;
	}
	if (0 == strcmp(input_file, "stdin"))
	{
		file_in = stdin;
#if defined(WIN32) || defined(WIN64)
		_setmode(_fileno(stdin), O_BINARY);
#endif
	}
	else
	{
		file_in = fopen(input_file, "rb");
	}
	if (file_in == NULL)
	{
		verbose_print(0, "Error: can't open input file [%s]\n", input_file);
		return -1;
	}
	// output file
	if (output_file)
	{
		if (0 == strcmp(output_file, input_file))
		{
			verbose_print(0, "Error: attempt to write to input file [%s]\n", input_file);
			return -1;
		}
		if (0 == strcmp(output_file, "stdout"))
		{
			file_out = stdout;
#if defined(WIN32) || defined(WIN64)
			_setmode(_fileno(stdout), O_BINARY);
#endif
		}
		else
		{
			file_out = fopen(output_file, "wb");
		}
		if (file_out == NULL)
		{
			verbose_print(0, "Error: can't open output file [%s]\n", output_file);
			return -1;
		}
	}
	return 0;
}

static void close_std_files(void)
{
	if ((file_in != NULL)  && (file_in != stdin))   fclose(file_in);
	if ((file_out != NULL) && (file_out != stdout)) fclose(file_out);
}

static int read_input_file(void *buf, int buf_size)
{
	int bytes = 0;
	if (file_in)
	{
		bytes = (int)fread(buf, 1, buf_size, file_in);
	}
	return bytes;
}

static int write_output_file(const void *buf, int buf_size)
{
	int bytes = 0;
	if (file_out)
	{
		bytes = (int)fwrite(buf, 1, buf_size, file_out);
		if(bytes != buf_size)
		{
			printf("write file operation failed!!!\n");
			exit(-1);
		}
	}
	return bytes;
}

static int rewind_input_file(void)
{
	if (file_in)
	{
		return fseek(file_in, 0, 0);
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////

#endif	// __V4FILE_H__
