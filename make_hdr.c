/************************************************************************/
/*  Routine to construct a DATA_HDR structure from the SAC header.	*/
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

/*
  1998/12/14	DSN	Allow unspecified msec and B field in sac header.
			Prevent usec overflow for large B value.
*/

#ifndef lint
static char sccsid[] = "$Id: make_hdr.c,v 1.7 2012/04/17 20:00:05 doug Exp $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "qlib2.h"

#include "sachead.h"
#include "procs.h"
#include "externals.h"

int herrno;

#define	SAC_INT		0
#define	SAC_FLOAT	1
#define	SAC_BOOL	2
#define	SAC_STRING	3
#define	STREAM(sps) \
    ( sps == 80 ? "VSP" : \
     ( sps == 20 ? "VBB" : \
      ( sps == 1 ? "LP" : \
       ( sps >= .099 && sps <= .101 ? "VLP" : \
	( sps >= .0099 && sps <= .0101 ? "ULP" : "UNK" )))))

char	*trim();

/************************************************************************/
/*  make_hdr:								*/
/*	Read SAC file, create and return ptr to DATA_HDR.		*/
/************************************************************************/
DATA_HDR *make_hdr
   (struct SAChead *sachead)	/* ptr to SAC header structure.		*/
{
    char component[3], stream[4];
    static char sac_station[K_LEN+1];
    char comp_stream[KEVNMLEN+1];
    char *station_id;
    char *channel_id;
    int ib;
    double rate;
    double ddelta;
    char *seed;
    DATA_HDR *hdr;
    EXT_TIME ext;
    BLOCKETTE_1000 b1000;

    if ((hdr = new_data_hdr()) == NULL) FATAL ("unable to alloc DATA_HDR")

    /*  Do a little work to avoid roundoff error.   */
    /*	1.  Assume that either delta or 1/delta is an integer.	*/
    
    rate = 1./sachead->delta;
    hdr->sample_rate = (rate >= 1.) ? roundoff(rate) : roundoff(-1./rate);
    hdr->sample_rate_mult = 1;

    /* Check for unspecified values */
    if (sachead->nzyear == -12345) FATAL("Unspecified year in sac header.")
    if (sachead->nzjday == -12345) FATAL("Unspecified jday in sac header.")
    if (sachead->nzhour == -12345) FATAL("Unspecified hour in sac header.")
    if (sachead->nzmin  == -12345) FATAL("Unspecified minute in sac header.")
    if (sachead->nzsec  == -12345) FATAL("Unspecified sec in sac header.")
    if (sachead->nzmsec == -12345) sachead->nzmsec = 0;
    if (sachead->b == -12345.) sachead->b = 0;

    ext.year = (sachead->nzyear>100) ? sachead->nzyear : sachead->nzyear+1900;
    ext.doy = sachead->nzjday;
    dy_to_mdy (ext.doy, ext.year, &ext.month, &ext.day);
    ext.hour = sachead->nzhour;
    ext.minute = sachead->nzmin;
    ext.second = sachead->nzsec;
    /* Reduce b to less than 1 second */
    ib = sachead->b;
    sachead->b = sachead->b - ib;
    ext.second += ib;
    ext.usec = (sachead->nzmsec * USECS_PER_MSEC) + 
	roundoff(sachead->b * USECS_PER_SEC);
    
    hdr->hdrtime = hdr->begtime = ext_to_int (ext);

    /*	Get the station name.  It is not necessarily left-justified.	*/
    if (station[0] == '\0') {
	strncpy(sac_station, sachead->kstnm, K_LEN);
	*(sac_station+K_LEN) = '\0';
	station_id = trim(sac_station);
	while (*station_id == ' ') ++station_id;
	strncpy (hdr->station_id, station_id, 5);
    }
    else {
	strncpy (hdr->station_id, station, 5);
    }

    /*	Quanterra convention -- stream and component are in KEVNM	*/
    /*	However, some stations (such as spa) don't have the stream	*/
    /*	name, and put the component name in kcmpnm field.		*/
    /*	They also use V instead of Z.					*/
    /*  Some stations now put the SEED channel name in kcmpnm field.	*/
    strncpy(comp_stream, sachead->kevnm+10, KEVNMLEN);
    comp_stream[KEVNMLEN-10] = '\0';

    if (channel[0] == '\0') {
	trim (comp_stream);
	/* Try Quanterra convention of stream+comp in event name.	*/
	if (unpack_comp_stream (comp_stream, component, stream, &seed) != 0) {
	    /*  Try ad-hod mapping convention.			*/
	    strncpy (comp_stream, sachead->kcmpnm, K_LEN);
	    *(comp_stream+K_LEN) = '\0';
	    trim(comp_stream);
	    if (strcmp(comp_stream, "V") == 0) strcpy (comp_stream, "Z");
	    strcat (comp_stream, STREAM(sps_rate(hdr->sample_rate,hdr->sample_rate_mult)));
	    if (unpack_comp_stream (comp_stream, component, stream, &seed) != 0) {
		/* Try taking just the kcmpnm field as the seed name.	*/
		strncpy (comp_stream, sachead->kcmpnm, K_LEN);
		*(comp_stream+K_LEN) = '\0';
		trim(comp_stream);
		if (strlen(comp_stream) <= 3) {
		    seed = &comp_stream[0];
		}
		else {
		    FATAL ("unknown component/stream");
		}
	    }
	}
	strncpy (hdr->channel_id, seed, 3);
    }
    else {
	strncpy (hdr->channel_id, channel, 3);
    }
	
    /* Get network code from knetwk and location code from khole. */
    strncpy(hdr->network_id, sachead->knetwk, 2);
    strncpy(hdr->location_id, sachead->khole, 2);

    /* Fill in the data header.						*/
    /* Note that any non-specified field will be zero.			*/
    hdr->seq_no = 1;
    hdr->num_samples = sachead->npts;

    return(hdr);
}

/************************************************************************/
/*  unpack_comp_stream:							*/
/*	Unpack component and stream.					*/
/************************************************************************/
int unpack_comp_stream 
   (char	*comp_stream,	/* ptr to SAC component and stream.	*/
    char	*component,	/* ptr to SAC component.		*/
    char	*stream,	/* ptr to SAC stream.			*/
    char	**p_seed)	/* ptr to ptr to SEED channel (returned)*/
{
    int	    clen;
    static char seed[4];

    /*	Unpack component and stream.  This is a bit tricky, since	*/
    /*	newer stations can have multi-character components.		*/
    /*	eg. ZVLP    ->	Z, VLP
	    ZLP	    ->	Z, LP
	    ZLLP    ->	ZL, LP
	    ZVLP    ->	Z, VLP
	    ZLVLP   ->	ZL, VLP
    */
    clen = 1;
    *p_seed = NULL;
    if (strlen(comp_stream) < 3) return (-1);
    for (clen = 1; clen <= 2; clen++) {
	strncpy (component, comp_stream, clen);
	*(component+clen) = '\0';
	strcpy (stream, comp_stream+clen);
	comp_to_seed(stream, component, p_seed);
	if (*p_seed != NULL && strcmp(*p_seed,UNKNOWN_STREAM)!= 0) return(0);
    }
    /* Return the comp_stream as the seed channel name.			*/
    if (strlen(comp_stream) <= 3) {
	strcpy (seed,comp_stream);
	*p_seed = seed;
	return (0);
    }
    return(-1);
}
