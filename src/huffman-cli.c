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
	printf("\nIf no outfile is specifed STDOUT will be used\n");
}

/* Pasrse the command line arguments */
struct opts optparse(int argc, char *argv[])
{
	char c;
	bool standard_output = false;
	struct opts options = { .unhuffman  = false, .statistics = false,
		   		.infile = NULL, .outfile = NULL };

	while ((c = getopt (argc, argv, "csu")) != -1)
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
		default:
			fprintf(stderr,"Illegal option: %c\n", c);
			usage(argv);
			break;
		}
	}
	
	int index = optind;
	if (index < argc)
	{
		if (*argv[optind] == '-')
		{
			options.infile = stdin;
		}
		else
		{
			options.infile = fopen(argv[optind],"rb");
			if (options.infile == NULL)
			{
				fprintf(stderr,"Failed to open file: %s\n",argv[0]);
				exit(2);
			}
		}
		index++;
		if (optind < argc && standard_output == false)
		{
			options.outfile = fopen(argv[optind+1],"wb");
			if (options.outfile == NULL)
			{
				fprintf(stderr,"Failed to open file: %s\n",
						argv[1]);
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
		double compression_ratio = (double)2*out.byte_count/in.byte_count;
		printf("Compression ratio: %.2f\n",compression_ratio);
	}

	return rc;
}
