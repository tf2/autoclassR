#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "autoclass.h"
#include "minmax.h"
#include "globals.h"

/* SUPRESS CODECENTER WARNING MESSSAGES */

/* empty body for 'while' statement */
/*SUPPRESS 570*/
/* formal parameter '<---->' was not used */
/*SUPPRESS 761*/
/* automatic variable '<---->' was not used */
/*SUPPRESS 762*/
/* automatic variable '<---->' was set but not used */
/*SUPPRESS 765*/


/* CENTRAL_MEASURES_X
   27nov94 wmt: use percent_equal for float tests
   27dec94 wmt: remove INT_UNKNOWN test
   
   A variation of CENTRAL-MEASURES modified for use with Single-Normal-xxx
   models. Treats nil values as unknown. Returns the values: unknown-wt
   known-wt mean variance skewness kurtosis.  This version calculates higher
   moments with respect to est-mean in order to obtain reasonable
   accuracy when the variance is very small relative to the true mean.
   Kurtosis has range (-1,+^N), is ~0 for Gaussian distributions, and
   ~-.6 for uniform ones. 
*/
void central_measures_x( float **data, int n_data,int  n_att, float *wts, double est_mean,
			float *unknown, float *known, float *mean, float *variance, 
  			float *skewness, float *kurtosis)
{
  int i;
  float sum1, sum2, nu1, nu2, mu2, wt, value, temp, delta;

  *unknown = 0.0;   *known = 0.0;   sum1 = 0.0;   sum2 = 0.0;
  for (i=0; i<n_data; i++) {
    wt = wts[i];
    if (wt > 0.0) {
      value = data[i][n_att];
      if (!(percent_equal( (double) value, FLOAT_UNKNOWN, REL_ERROR))) {
        *known += wt;
        delta = value - (float) est_mean;
        temp = wt * delta;
        sum1 += temp;
        sum2 += temp * delta;
      }
      else *unknown += wt;
    }
  }
  if (*known == 0.0) {
    *mean = FLOAT_UNKNOWN;
    *variance = FLOAT_UNKNOWN;
    *skewness = FLOAT_UNKNOWN;
    *kurtosis = FLOAT_UNKNOWN;
  }
  else {
    nu1 = sum1 / *known;
    nu2 = sum2 / *known;
    mu2 = max(0.0, nu2 - (nu1 * nu1));
    *mean = nu1 + (float) est_mean;
    *variance = mu2;
    *skewness = 0.0;
    *kurtosis = 0.0;
  }
}
