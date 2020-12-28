#include <stdio.h>
#include <math.h>

#define PEAK    2               /* Search mode looking for peak */
#define TROUGH  1               /* Search mode looking for trough */
#define NEITHER 0               /* Search mode looking for either one */

#define AMP_PEAK_TROUGH         "peak_trough"
#define AMP_ZERO_PEAK           "zero_peak"
#define AMP_FM                  "fm"
#define AMP_FM_SIGN             "fm_sign"
#define AMP_FM_A                "fm_a"
#define AMP_FM_B                "fm_b"
#define AMP_RMS                 "rms"
#define AMP_MEAN_SQR            "mean_sqr"
#define AMP_ABS_MAX             "abs_max"
#define AMP_AVG_MAX             "avg_max" /* For Sonseca */
#define AMP_STAV                "stav"

#define STREQ(a,b)		(strcmp ((a), (b)) == 0)

static void set_peak_trough(float *beam, int npts, int *pts, int i);

int
find_peak_trough (beam, npts, pts)
float   *beam;
int     npts;
int     *pts;
{
        int     maxpt = 0;      /* Index of highest point in beam */
        int     minpt = 0;      /* Index of lowest point in beam */
        int     i;


        /* Check for error conditions */
        if (beam == NULL || pts == NULL || npts <= 0 )
        {
		printf("error find peak_trough\n");
                return -1;
        }


        pts[0] = NEITHER;
        set_peak_trough (beam, npts, pts, 1);
        for (i = 2; i < npts; i++)
        {
                set_peak_trough (beam, npts, pts, i);


                /* Save min,max points in beam for threshold determination */
                if (beam[i] > beam[maxpt])
                {
                        maxpt = i;
                }
                else if (beam[i] < beam[minpt])
                {
                        minpt = i;
                }

        }
        pts[0] = NEITHER;
        pts[npts - 1] = NEITHER;

        return 0;
}

static
void
set_peak_trough (beam, npts, pts, i)
float   *beam;
int     npts;
int     *pts;
int     i;
{
        int     im1;

        im1 = i - 1;

        if (beam[i] > beam[im1])
        {
                pts[i] = PEAK;

                if (pts[im1] == PEAK)
                {
                        pts[im1] = NEITHER;
                }
                else if (pts[im1] == NEITHER)
                {
                        pts[im1] = TROUGH;
                }
        }
        else if (beam[i] < beam[im1])
        {
                pts[i] = TROUGH;

                if (pts[im1] == TROUGH)
                {
                        pts[im1] = NEITHER;
                }
                else if (pts[im1] == NEITHER)
                {
                        pts[im1] = PEAK;
                }
        }
        else
        {
                pts[i] = NEITHER;
        }

        return;
}


int
peak_trough (beam, npts, pts, btime, samprate, amp, per, time)
float   *beam;
int     npts;
int     *pts;
double  btime;
double  samprate;
double  *amp;
double  *per;
double  *time;
{
        double  maxamp;
        int     max0;
        int     max1;
        double  local_amp;
        double  local_per;
        double  local_time;
        int     ret;

        amp[0] = 0.0;
        per[0]= -1.0;
        time[0] = btime;

        /* Check for error conditions */
        if (beam == NULL || pts == NULL || npts <= 0 )
        {
		printf("error peak_trough\n");
                return -1;
        }


        /* Find maximum abs peak to trough amplitude */
        ret = max_peak_trough (beam, npts, pts, &maxamp, &max0, &max1);
        if (ret < 0)
        {
                goto RETURN;
        }

        local_amp = maxamp;
//      if (STREQ (type, AMP_ZERO_PEAK))
//      {
                local_amp /= 2.0;
//      }
        local_per = 2.0 * (double) (max1 - max0) / samprate;
        local_time = btime + (double) max0 / samprate;

        amp[0] = local_amp;
        per[0] = local_per;
        time[0] = local_time;

        ret = 0;

 RETURN:

        return (ret);
}

int
max_peak_trough (beam, npts, pts, max_amp, max_0, max_1)
float   *beam;
int     npts;
int     *pts;
double  *max_amp;       /* Largest peak to trough amp */
int     *max_0;         /* Max peak-trough pair */
int     *max_1;
{
        double  amp;            /* Current amplitude */
        double  maxamp;         /* Largest peak to trough amp so far */
        int     max0;           /* Max peak-trough pair */
        int     max1;
        int     right_end_pt;   /* Rightmost end-point in buffer */
        int     left_end_pt;    /* Leftmost end-point in buffer */
        int     first_i;
        int     last_i;
        int     ret = -1;
        int     i;

        /* Initialize */
        max_amp[0] = 0.0;
        max_0[0] = 0;
        max_1[0] = 0;
        maxamp = 0.0;
        max0 = 0;
        max1 = 0;

        /* Error checks */
        if (!beam || npts < 1 || !pts)
        {
		printf("error get max peak amp.\n");
                goto RETURN;
        }


        /* Find first and last peak/trough points */
        for (i = 0; pts[i] == NEITHER && i < npts; i++);
        first_i = i;
        if (first_i == npts - 1)
        {
                goto RETURN;
        }
        for (i = npts - 1; pts[i] == NEITHER && i > first_i; i--);
        last_i = i;
        if (last_i == first_i)
        {
                goto RETURN;
        }


        /* Initialize first extrema as left_end_pt */
        left_end_pt = first_i;


        /* Loop thru remaining points and find max amplitude and indices */
        for (i = first_i + 1; i < last_i; i++)
        {
                /* Find next peak or trough, skip if neither */
                if (pts[i] != NEITHER)
                {
                        /* Assign right_end_pt */
                        right_end_pt = i;


                        /* Compute abs amplitude */
                        amp = fabs (beam[left_end_pt] - beam[right_end_pt]);


                        /* Find max amp and indices */
                        if (amp > maxamp)
                        {
                                max0 = left_end_pt;
                                max1 = right_end_pt;
                                maxamp = amp;
                        }

                        /* Re-assign left end point */
                        left_end_pt = right_end_pt;
                }
        }

        max_amp[0] = maxamp;
        max_0[0] = max0;
        max_1[0] = max1;

        ret = 0;

 RETURN:

        return (ret);
}

