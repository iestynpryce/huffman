/* A command line interface for the huffman coding library */
/* Iestyn Pryce 2012 */

#include "huffman.h"
#include "file_stat.h"

#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

/* Structure to store commandline options */
struct opts
{
	bool statistics;
	bool unhuffman;
	FILE *infile;
	FILE *outfile;
};

/* Usage... */
void usage(char *argv[]) {
	printf("%s [-sc",argv[0]);
#ifndef UNHUFFMAN
	printf("u");
#endif
	printf("] [file] [outfile]\n");
	printf("\n");
	printf("Options:\n");
	printf("-s: print compression statistics to STDOUT\n");
#ifndef UNHUFFMAN
	printf("-u: decompress the input file\n");
#endif
	printf("-c: output to STDOUT\n");
	printf("-h: this message\n");
	printf("\nIf no outfile is specifed STDOUT will be used\n");
}

/* Pasrse the command line arguments */
struct opts optparse(int argc, char *argv[])
{
	char c;
	bool error = false;
	bool standard_output = false;
	struct opts options = { .unhuffman  = false, .statistics = false,
		   		.infile = NULL, .outfile = NULL };

	while ((c = getopt (argc, argv, "csuh")) != -1)
	{
		switch (c)
		{
		case 'c':
			standard_output = true;
			break;
		case 's':
			options.statistics = true;
			break;
#ifndef UNHUFFMAN
		case 'u':
			options.unhuffman = true;
			break;
#endif			
		case 'h':
			usage(argv);
			exit(EXIT_SUCCESS);
		default:
			fprintf(stderr,"Illegal option: %c\n", c);
			error = true;
			break;
		}
		if (error)
		{
			usage(argv);
			exit(2);
		}
	}
	
	int index = optind;
	if (index < argc)
	{
		if (*argv[index] == '-')
		{
			options.infile = stdin;
		}
		else
		{
			options.infile = fopen(argv[index],"rb");
			if (options.infile == NULL)
			{
				fprintf(stderr,"Failed to open file: %s\n",argv[index]);
				exit(2);
			}
		}
		index++;
		if (index < argc && standard_output == false)
		{
			options.outfile = fopen(argv[index],"wb");
			if (options.outfile == NULL)
			{
				fprintf(stderr,"Failed to open file: %s\n",
						argv[index]);
				exit(2);
			}
		}
		else
		{
			options.outfile = stdout;
		}
	}
	else
	{
		fprintf(stderr,"No input file defined\n");
		usage(argv);
		exit(2);
	}
	return options;
}

int main(int argc, char *argv[]) {
	f_stat in;
	f_stat out;
	int rc;

	/* Process the input arguments */
	struct opts options = optparse(argc,argv);

	in.byte_count  = 0;
	in.file        = options.infile;
	out.byte_count = 0;
	out.file       = options.outfile;

#ifdef UNHUFFMAN
	options.unhuffman = true;
#endif

	if (options.unhuffman)
	{
		rc = unhuffman(&in,&out);
	}
	else
	{
		rc = huffman(&in,&out);
	}

	/* Finally we close the input and output file */
	fclose_stat(&in);
	fclose_stat(&out);

	if (options.statistics == true)
	{
		/* We multiply the ratio by 2 because we've read the input *
                 * stream twice.					   */
		printf("Input bytes: %ld\n",in.byte_count/2);
		printf("Output bytes: %ld\n",out.byte_count);
		double compression_ratio = (double)2*out.byte_count/in.byte_count;
		printf("Compression ratio: %.4f\n",compression_ratio);
	}

	return rc;
}
