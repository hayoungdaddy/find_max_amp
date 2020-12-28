#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifndef	DEFAULT_BLKSIZE
#define	DEFAULT_BLKSIZE		512
#endif

#ifndef	DEFAULT_SAC_FORMAT
#define	DEFAULT_SAC_FORMAT	"BINARY"
#endif

#define	EXTERN
#define	DEFINE

#include "sachead.h"
#include "procs.h"
#include "externals.h"

struct tm *gmtime_ew( const time_t *epochsec, struct tm *res )
{
    gmtime_r( epochsec, res );
    return( res );
}

int main (int argc, char **argv)
{
	if(argc != 4)
	{
		printf("Usage : find_max_amp <start seconds(s)> <duration(s)> <sacfile>\n");
		printf("If you want use full data, then you shoud be set duration=0\n");
		exit(0);		
	}

	int i, npts, k, realnpts;
	struct SAChead *sachead = NULL;
	float *input_data = NULL;	/* sac reader allocates buffer.		*/
	int *data = NULL;
	int status;
	float *inpData;
	int sps;
	long startS, duration;

	int ret; 
	double *amp, *per, *time;
	int *pts; 

	double epoch, startE, endE;
	char msec[5];
	struct tm tm;
	time_t epoch2 ;
	char buf[255];

	FILE *fp;
	char output[128];
	char htime[512];

	amp = (double *)malloc(sizeof(double)*2);
	per = (double *)malloc(sizeof(double)*2);
	time = (double *)malloc(sizeof(double)*2);

	parse_cmdline(&argc,&argv);
	status = read_sac(input, &sachead, &input_data, &npts, strchr("Aa", sac_format[0]) ? 0 : 1);

	if (status != 0) 
		printf("error reading input file");

	data = (int *)input_data;
	//inpData = (float *)malloc(sizeof(float)*npts);

	sps = 1 / sachead->delta;
	startS = strtol(argv[0], NULL, 10);
	duration = strtol(argv[1], NULL, 10);

	if(duration == 0)
		realnpts = npts;
	else
		realnpts = duration * sps;

	if(npts < realnpts)
	{
	    FATAL ("Too short data length")
	}

	npts = realnpts;

	inpData = (float *)malloc(sizeof(float)*npts);
	
	//printf("%d %d %d %d\n", sps, startS, duration, npts);

	for(i=0;i<npts;i++)
	{
		data[i] = (int)(input_data[(i + (startS * sps))] * scale_factor); 
		inpData[i] = (float) data[i];
	}

	pts = (int *)malloc(sizeof(int)*npts);
	ret = find_peak_trough(inpData, npts, pts);

	//printf("%d %d %d %d %d %d\n", sachead->nzyear, sachead->nzjday, sachead->nzhour, sachead->nzmin, sachead->nzsec, sachead->nzmsec);

	sprintf(msec, "0.%d", sachead->nzmsec);
	sprintf(buf, "%d %d %d %d %d",sachead->nzyear, sachead->nzjday, sachead->nzhour, sachead->nzmin, sachead->nzsec);
	strptime(buf, "%Y %j %H %M %S", &tm);
	epoch2 = timegm(&tm) ;
	epoch = epoch2;
	epoch = epoch + atof(msec);
	//printf("%f\n", sachead->b);
	epoch = epoch + (double)sachead->b;
	//printf("epochtime : %f\n", epoch);

	if(ret == 0)
		ret = peak_trough(inpData, npts, pts, epoch, (int)1 / sachead->delta, amp, per, time);

	//printf("Before Time Changing %.2f %.2f %f\n", amp[0], per[0], time[0]);

	/* change time */
	for(i=0;i<npts;i++)
	{
		double x1, x2;
		x1 = epoch;
		x2 = time[0];
		time_t t1, t2;
		char xxx[15], yyy[15];
		t1 = x1;
		t2 = x2;
		snprintf(xxx, 15, "%.3f", x1);
		snprintf(yyy, 15, "%.3f", x2);
		if(t1 == t2 && xxx[11] == yyy[11] && xxx[12] == yyy[12] && xxx[13] == yyy[13])
		{
			//printf("=============== time %.3f %d \n", x1, pts[i]);

			if(pts[i] == 1)
			{
				for(k=i;k<npts;k++)
				{
					if(pts[k] == 2) 
					{
						endE = myRound(time[0] + myRound((sachead->delta * (k-i)),3),3);
						break;
					}
				}
				break;
			}
			else if(pts[i] == 2)
			{
				for(k=i;k<npts;k++)
				{
					if(pts[k] == 1)
					{
						endE = myRound(time[0] + myRound((sachead->delta * (k-i)),3),3);
						break;
					}
				}
				break;
			}
		}
		epoch = epoch + sachead->delta;
	}

	startE = time[0];

	datestr24(startE, htime, 256, 1);
	printf("Max_Amp:%.2f\n", amp[0]);
	printf("Period:%.2f\n", per[0]);
	printf("StartEpochTime:%.3f\n", startE);
	printf("StartHumanTime:%s\n", htime);
	datestr24(endE, htime, 256, 1);
	printf("EndEpochTime:%.3f\n", endE);
	printf("EndHumanTime:%s\n", htime);

	strcpy(output,"output.txt");
	
	if((fp = fopen(output, "w+")) == NULL)
	{
		printf("Could not open file %s\n", output);
		exit(0);
	}

	datestr24(startE, htime, 256, 1);
	fprintf(fp, "Max_Amp:%.2f\n", amp[0]);
	fprintf(fp, "Period:%.2f\n", per[0]);
	fprintf(fp, "StartEpochTime:%.3f\n", startE);
	fprintf(fp, "StartHumanTime:%s\n", htime);
	datestr24(endE, htime, 256, 1);
	fprintf(fp, "EndEpochTime:%.3f\n", endE);
	fprintf(fp, "EndHumanTime:%s\n", htime);

	fclose(fp);

/*
	free (sachead);
	free (input_data);
	free (inpData);
	free (amp);
	free (per);
	free (time);
*/

	return(0);
}

double myRound(double n, unsigned int c)
{
	double marge = pow (10, c);
	double up    = n * marge;
	double ret   = round (up) / marge;

	return ret;
}

char *datestr24( double t, char *pbuf, int len, int type )
{
	time_t    tt;       /* time as time_t                  */
	struct tm stm;      /* time as struct tm               */
	char arr[20];
	char msec[4];

	if( len < 23 ) return( (char *)NULL );

	tt     = (time_t) t;
	sprintf(arr,"%10.3f",t);
	msec[0] = arr[11]; msec[1] = arr[12]; msec[2] = arr[13]; msec[3] = '\0';
	gmtime_ew( &tt, &stm );

	if(type)
	{
		sprintf( pbuf, "%04d%02d%02d%02d%02d%02d.%s",
			stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday, stm.tm_hour,
			stm.tm_min, stm.tm_sec, msec );
	}
	else
	{
		sprintf( pbuf, "%04d-%02d-%02d %02d:%02d:%02d.%s",
			stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday, stm.tm_hour,
			stm.tm_min, stm.tm_sec, msec );
	}

	return( pbuf );
}
