/************************************************************************/
/*  External variable for sac2ms.					*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: externals.h,v 1.5 2000/10/26 13:40:00 doug Exp $ 	*/

#define		FATAL(str)	{ fprintf(stderr,"%s\n",str); exit(1); }
#define		MAX_BLKSIZE	8192
	
#ifndef	EXTERN
#define	EXTERN	extern
#endif	/* EXTERN */


#define DH_STATION_LEN  5
#define DH_CHANNEL_LEN  3
#define DH_LOCATION_LEN 2
#define DH_NETWORK_LEN  2


/************************************************************************/
/*  External variables.							*/
/************************************************************************/
EXTERN char *cmdname;
EXTERN char *infile;
EXTERN char *outfile;
EXTERN FILE *input;
EXTERN FILE *output;
EXTERN FILE *info;
EXTERN char station[DH_STATION_LEN+1];	/* explicit station name.	*/
EXTERN char channel[DH_CHANNEL_LEN+1];	/* explicit channel name.	*/
EXTERN char network[DH_NETWORK_LEN+1];	/* explicit network name.	*/
EXTERN char location[DH_LOCATION_LEN+1];/* explicit location name.	*/
EXTERN int bin_flag;
EXTERN double scale_factor;
extern char *syntax[];

#ifdef	DEFINE
int blksize = DEFAULT_BLKSIZE;			/* MiniSEED blksize.	*/
//int output_format = DEFAULT_OUTPUT_FORMAT;	/* MiniSEED data format.*/
char *sac_format = DEFAULT_SAC_FORMAT;		/* SAC format.		*/

#else
EXTERN int	blksize;	/* MiniSEED blksize.			*/
EXTERN int	output_format;	/* MiniSEED data format.		*/
EXTERN char	*sac_format;	/* SAC format.				*/
#endif

double myRound(double n, unsigned int c);
char *datestr24( double t, char *pbuf, int len, int type );
