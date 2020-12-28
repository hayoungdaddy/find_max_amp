#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "sachead.h"
#include "procs.h"
#include "externals.h"

#define	argc	(*pargc)
#define	argv	(*pargv)

/************************************************************************/
/*  parse_cmdline:							*/
/*	Parse command line.						*/
/************************************************************************/
int
parse_cmdline( pargc, pargv)
    int	    *pargc;
    char    ***pargv;
{
    int i;
    int	nfiles;
    char *p;

    /* Variables needed for getopt. */
    extern char	*optarg;
    extern int	optind, opterr;
    int		c;

    scale_factor = 1.0;

    /*	Parse command line options.					*/

    /*	Skip over all options and their arguments.			*/
    argv = &(argv[optind]);
    argc -= optind;

    /*	Verify that options are OK.					*/
    if (blksize <= 0) FATAL("Error - invalid blksize.");
    if (strchr("AaBb", sac_format[0]) == NULL)
	FATAL("Error - invalid sac format specified.")

    /* The remaining arguments are [ input [and output] ] files.	*/

    infile = "<stdin>";
    input = stdin;
    outfile = "<stdout>";
    output = stdout;
    info = stdout;
    switch (argc) {
      case 0:
	info = stderr;
	break;
      case 1:
	infile = argv[0];
	if ((input = fopen (infile, "r")) == NULL)
	    FATAL ("unable to open input file")
	info = stderr;
	break;
      case 2: 
	infile = argv[0];
	if ((input = fopen (infile, "r")) == NULL)
	    FATAL ("unable to open input file")
	break;
      case 3: 
	infile = argv[2];
	if ((input = fopen (infile, "r")) == NULL)
	    FATAL ("unable to open input file")
	info = stderr;
	break;
      default:
	printf("----\n");
    }
    return(0);
}

