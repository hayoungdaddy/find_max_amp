/*
**	read_sac
**
**	written 1/89 - 2/89 by Scott MacHaffie
**	added error checking 9/5/90 - Matthew Hendrickson
**
**	this file converts files from sac ascii format to sac binary
**	format.  it writes the binary file to a temporary file.
**
**	convert [[-f options] f1 f2 ... fn]
**
**	05/21/92
**	Allow final line of data to be partial line.
**	04/09/92
**	converted to subroutine by Doug Neuhauser, doug@perry.berkeley.edu
*/

#ifndef lint
static char sccsid[] = "@(#)read_sac.c	1.1 7/6/93 19:22:22";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>


#include    "sachead.h"
#include    "procs.h"
#include    "externals.h"

/* used to identify program in stderr output */
static char prog[] = "read_sac";

#define	MAX_OPT		50

/************************************************************************/
/*  read_sac:								*/
/*	Read a SAC header, and return # points and ptr to the data.	*/
/************************************************************************/
int
read_sac	(fp, p_sachead, p_input_data, p_npts, bin_flag)
    FILE    *fp;
    struct SAChead **p_sachead;
    float   **p_input_data;
    int	    *p_npts;
    int	    bin_flag;
{
	int	tot_read, nread;
	int  i;
	long l;			/* return codes from convert functions */
	long read_hdr();	/* convert SAC header */
	int	status = 0;	/* exit status */

	*p_sachead = (struct SAChead *)(malloc(sizeof(struct SAChead)));

	if (bin_flag) {
	    /*	Read binary SAC file.	*/
	    if ((i = fread(*p_sachead, sizeof(struct SAChead), 1, fp)) != 1) {
		fprintf(stderr, "%s: error reading header\n", prog);
		status |= 4;
		goto ABORT;
	    }

	    *p_npts = (*p_sachead)->npts;
	    *p_input_data = (float *)malloc(*p_npts * sizeof(float));

	    tot_read = 0;
	    while (tot_read < *p_npts ) {
		if ((nread = fread(*p_input_data+tot_read, sizeof(float), *p_npts-tot_read, fp)) <= 0) {
		    fprintf(stderr, "%s: error reading body\n", prog);
		    status |= 8;
		    goto ABORT;
		}
		tot_read += nread;
	    }
	}
	else {
	    /*	Read ASCII SAC file.	*/
	    if ((*p_npts = read_hdr(fp, (struct SAChead2 *)*p_sachead)) < 0L) {
		fprintf(stderr, "%s: error converting header\n", prog);
		status |= 4;
		goto ABORT;
	    }

	    *p_input_data = (float *)malloc(*p_npts * sizeof(float));

	    if ((l = read_body(fp, *p_sachead, *p_input_data, *p_npts)) < 0L) {
		    fprintf(stderr, "%s: error converting body\n", prog);
		    status |= 8;
		    goto ABORT;
	    }
	}
ABORT:
	(void) fclose(fp);
	return(status);
}

/*
**	read_hdr() -- read header from input file 
*/
long	read_hdr(inp, header)
	FILE	*inp;	/* input file handle */
	struct SAChead2 *header;
{
	int i, j;
	long l1, l2, l3, l4, l5;
	unsigned long u1, u2, u3, u4, u5;
	float f1, f2, f3, f4, f5;
	int	ch;
	char	*strncpy();

	/* read floating point data - 5 numbers on a line, 14 lines */
	for (i = 0; i < NUM_FLOAT / 5; i++) {
		if (fscanf(inp, "%f %f %f %f %f\n",
				&f1, &f2, &f3, &f4, &f5) < 5) goto READ_EOF;
		header->SACfloat[i * 5] = f1;
		header->SACfloat[i * 5 + 1] = f2;
		header->SACfloat[i * 5 + 2] = f3;
		header->SACfloat[i * 5 + 3] = f4;
		header->SACfloat[i * 5 + 4] = f5;
	}

	/* read the integers - 5 per line, 7 lines */
	for (i = 0; i < MAXINT / 5; i++) {
		if (fscanf(inp, "%ld %ld %ld %ld %ld\n",
				&l1, &l2, &l3, &l4, &l5) < 5) goto READ_EOF;
		header->SACint[i * 5] = l1;
		header->SACint[i * 5 + 1] = l2;
		header->SACint[i * 5 + 2] = l3;
		header->SACint[i * 5 + 3] = l4;
		header->SACint[i * 5 + 4] = l5;
	}

	/* read the unsigned integers - 5 per line, 1 line */
	if (fscanf(inp, "%lu %lu %lu %lu %lu", &u1, &u2, &u3, &u4, &u5) < 5)
							goto READ_EOF;
	header->SACun[0] = u1;
	header->SACun[1] = u2;
	header->SACun[2] = u3;
	header->SACun[3] = u4;
	header->SACun[4] = u5;

	/* eat newline */
	if (getc(inp) == EOF) goto READ_EOF;

	/* this string is a different size than the others */
	for (i = 0; i < 8; i++) {
		if ((ch = getc(inp)) == EOF) goto READ_EOF;
		header->SACstring[0][i] = (char) ch;
	}

	/* read the rest of the strings */
	for (i = 1; i < MAXSTRING; i++) {
		for (j = 0; j < K_LEN; j++) {
			if ((ch = getc(inp)) == EOF) goto READ_EOF;
			header->SACstring[i][j] = (char) ch;
		}
		/* shift "  -12345" to "-12345  " */
		if (!strncmp(header->SACstring[i], "  -12345", K_LEN)) {
			(void) strncpy(header->SACstring[i], "-12345  ",
								K_LEN);
		}
		/* every third string is followed by a newline */
		if ((i + 1) % 3 == 0)
			/* kill '\n' */
			if (getc(inp) == EOF) goto READ_EOF;
	}

	/* return the number of data points in the file */
	return(header->SACint[9]);
READ_EOF:
	fprintf(stderr, "%s: Premature EOF in header\n", prog);
	return(-1L);
}

/*
**	read_body() -- convert the floating point data from ascii
**	to binary
*/
int	read_body(inp, header, body, num)
FILE	*inp;	/* input file pointer */
struct SAChead *header;
float	*body;	/* output array for body */
long	num;	/* number of data points */
{
	long	cnt;		/* number of data points read */
	long	w_cnt;		/* number of data points to write */
	int	i;
	float	f[5];
	float	depmax = -1e30;	/* maximum data value */
	float	depmin = 1e30;	/* minimum data value */
	float	depmen = 0;	/* mean data value */
	long	lseek();
	int	nread;		/* number of points read.   */

	/* read lines of 5 data points */
	for (cnt = 0L; cnt < num; cnt += 5) {
		/* read the data */
		if ((nread = fscanf(inp, "%f %f %f %f %f\n",
			&f[0], &f[1], &f[2], &f[3], &f[4])) < 5) {
		    /* final line may validly contain less points.  */
		    if (nread + cnt < num) goto DATA_READ;
		}

		/* how many point do we need to write ? */
		if ((w_cnt = num - cnt) > nread) w_cnt = nread;

		/* write the data */
		for (i=0; i<w_cnt; i++)
		    *body++ = f[i];

		/* keep track of max, min, and sum */
		for (i = 0; i < w_cnt; ++i) {
			if (f[i] > depmax) depmax = f[i];
			if (f[i] < depmin) depmin = f[i];
			depmen += f[i];	/* depmen is the sum for now */
		}
	}

	/* compute the mean */
	depmen /= num;

	/* now update the header fields */
	header->depmax = depmax;
	header->depmin = depmin;
	header->depmen = depmen;
	return(0L);
DATA_READ:
	fprintf(stderr, "%s: Premature EOF in data, expected %d points, found %d\n", 
		prog, num, cnt+nread);
	
	return(-1L);
DATA_WRITE:
	fprintf(stderr, "%s: Could not write data to output file\n", prog);
	return(-1L);
}
