#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#include "autoclass.h"
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


/* LOG_GAMMA
 03nov94 wmt: move into separate file
 19dec94 wmt: return double rather than float
        */
double log_gamma( double x, int low_precision)
{
   if (x > 3.0) {
      if (low_precision == TRUE)
         return(((x - 0.5) * (safe_log(x))) +
		   (-1.0 * x) +
	           0.9189385332046727 +                    /* log(sqrt(2*pi)) */
		   (0.08333333333333333 / x) +                  /* (1/12) / x */
		                                            /* -(1/360 / x^3) */
                   (-1.0 * (0.002777777777777778 / (x*x*x))));
      else
	 return(((x - 0.5) * (safe_log(x))) +
		   (-1.0 * x) +
		   0.9189385332046727 +                    /* log(sqrt(2*pi)) */
		   (0.08333333333333333 / x) +                  /* (1/12) / x */
		  				            /* -(1/360 / x^3) */
		   (-1.0 * (0.002777777777777778 / (x*x*x))) +
		   (0.00007936507936507937 / (x*x*x*x*x)) + /* (1/1260 / x^5) */
					                   /* -(1/1680 / x^7) */
                   (0.00005952380952380953 / pow( x, 7)));
   }
   if ((x == 1.0) || (x == 2.0))
      return(0.0);
   if (x > 0.0)
      return(log_gamma( 3.0 + x, low_precision ) -
	     safe_log((double) (x * (1.0 + x) * (2.0 + x))));
   fprintf( stderr, "Attempted to take log_gamma %20.15f\n", x);
   return 0.0; /*this is not any good but must return something*/
}


/* ATOI_P
   01dec94 wmt: new

   convert string to integer - set integer_p FALSE is not an integer
   since:
   > atoi("1@3");  => (int) 1
   and
   > atoi("103");  => (int) 103
   all characters in the token must be validated
   */
int atoi_p (char *string_num, int *integer_p_ptr)
{
  int num = 0, i, string_length, a_char;

  string_length = strlen(string_num);
  *integer_p_ptr = TRUE;
  for (i=0; i<string_length; i++) {
    a_char = (int) string_num[i];
    if ((a_char >= '0') && (a_char <= '9'))
      continue;
    if ((a_char == '+') || (a_char == '-'))
      continue;
    *integer_p_ptr = FALSE;
    break;
  }
  if (string_length == 0) 
    *integer_p_ptr = FALSE;

  if (*integer_p_ptr == TRUE)
    num = atoi(string_num);
  return(num);
}


/* ATOF_P
   01dec94 wmt: new

   convert string to float - set float_p FALSE if not an float
   */
double atof_p (char *string_num, int *float_p_ptr)
{
  double num = 0.0;
  int a_char, string_length, i;

  string_length = strlen(string_num);
  *float_p_ptr = TRUE;
  for (i=0; i<string_length; i++) {
    a_char = (int) string_num[i];
    if ((a_char >= '0') && (a_char <= '9'))
      continue;
    if ((a_char == '+') || (a_char == '-') || (a_char == '.') ||
        (a_char == 'e') || (a_char == 'E'))
      continue;
    *float_p_ptr = FALSE;
    break;
  }
  if (string_length == 0) 
    *float_p_ptr = FALSE;

  if (*float_p_ptr == TRUE)
    num = atof(string_num);
  
  return(num);
}


/*  SAFE_EXP
    14feb95 wmt: new

    Satisfies attempts to take an out-of-bounds exponent, by providing limiting values.
    */
double safe_exp( double x)
{
  if (x >= MOST_POSITIVE_LONG_LOG)
    return (MOST_POSITIVE_LONG_FLOAT);
  else if (x <= LEAST_POSITIVE_LONG_LOG)
    return (0.0);
  else
    return (exp( x));
}


/* MEAN_AND_VARIANCE
   27mar95 wmt: new

   compute mean and variance of a vector of doubles, using
   direct variance calculation to prevent numerical problems
   */
void mean_and_variance( double *vector, int cnt, double *mean_ptr, double *variance_ptr)
{
  double sum = 0.0, sum_sq = 0.0, double_mean;
  int i;

  for (i=0; i<cnt; i++)
    sum += vector[i];

  double_mean = sum / ((double) cnt);
  *mean_ptr = double_mean;

  for (i=0; i<cnt; i++)
    sum_sq += square( (double) vector[i] - double_mean);

  *variance_ptr = sum_sq / ((double) cnt);
}


/*  SAFE_LOG
    28jul95 wmt: new
    24apr01 jcw: now returns LEAST_POSITIVE_SINGLE_LOG for x near 0

    Satisfies attempts to take an out-of-bounds log, by providing limiting values.
    */
double safe_log( double x)
{
  if (x >= MOST_POSITIVE_SINGLE_FLOAT)
    return (MOST_POSITIVE_SINGLE_LOG);
  else if (x <= LEAST_POSITIVE_SINGLE_FLOAT)
    return( LEAST_POSITIVE_SINGLE_LOG );
  else
    return (log( x));
}


