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

 
/* UPDATE_APPROXIMATIONS
   29mar95 wmt: convert caluations to double precision

   Updates the classification and individual class likelihood
   (the clsf-DS field log-A<X/H.pi.theta> and the class_DS field
   log-A<w.S/H.pi.theta>) and marginal likelihood (the clsf-DS
   field log-A<X/H> and the class_DS field log-A<w.S/H_j>) approximations,
   with respect to the weights and parameters.
   Note:  When initializing a classification from assigned weights,
   call update-ln-p<X/pi.theta> before calling this. 
*/
void update_approximations( clsf_DS clsf)
{
   int i, n_classes, n_data;
   double inv_n_classes, log_a, log_a_pi_theta;
   class_DS cl, *classes;
   database_DS data_base;

   n_classes = clsf->n_classes;
   inv_n_classes = -1.0 / (double) n_classes;
   classes = clsf->classes;
   data_base = clsf->database;
   n_data = data_base->n_data;
   log_a = 0.0;                            /* default value for single class. */
   log_a_pi_theta = 0.0;

   /* Add the discrete class log probabilities, modeled as a single-multinomial
      attribute. */
   if (n_classes == 1)                      /* Special to avoid (log-gamma 0) */
      log_a += -1.0 * log_gamma( (double) n_data, FALSE);
   else log_a += log_gamma( (double) (n_classes - 1), FALSE) -
                 log_gamma( (double) (n_data + n_classes - 1), FALSE) -
                 ((double) n_classes * log_gamma( (double) (1.0 + inv_n_classes),
                                                     FALSE));
   /* Collect the individual class contributations to the approximate
      log probabilities. */
   for (i=0; i<n_classes; i++) {
      cl = classes[i];
      log_a_pi_theta += update_l_approx_fn(cl);
      log_a += update_m_approx_fn(cl); 
      log_a += log_gamma( (double) (cl->w_j + 1.0 + inv_n_classes), FALSE);
   }
 
   /*  Add the log permutation factor (log J!) to the marginal: */
   log_a += log_gamma( (double) (1 + n_classes), FALSE);

   /*commented printf(" calc clsf->log_a_x_h= log_a + clsf->log_p_x_h_pi_theta -
     log_a_pi_theta\n %f %f %f\n",
     log_a , clsf->log_p_x_h_pi_theta , log_a_pi_theta); dbg*********/

   /* Rescale the marginal posterior using the ratio of observed and
      approximate likelihoods: */
   log_a += clsf->log_p_x_h_pi_theta - log_a_pi_theta;
   clsf->log_a_x_h = log_a;
   /*commented printf(" at end of update_approximations, clsf->log_a_x_h=%f\n",
     clsf->log_a_x_h); dbg*/
}


/* Updates the class parameters and the clsf_DS field min-class-wt.
   The update-params-fn updates each class-DS field updatable-p class
   with respect to the updated weights. */

void update_parameters( clsf_DS clsf)
{
   int n_classes, n_data, n_cl, collect = TRUE;
   class_DS *classes, cl;
   database_DS database;

   n_classes = clsf->n_classes;
   classes = clsf->classes;
   database = clsf->database;
   n_data = database->n_data;

   clsf->min_class_wt =
      max(ABSOLUTE_MIN_CLASS_WT,              /* SN model requires w_j > 1.0 */
	   ((MIN_CLASS_WT_FACTOR * (float) n_data) / n_classes));
   for (n_cl=0; n_cl<n_classes; n_cl++) {
      cl = classes[n_cl];
      update_params_fn(cl, n_classes, database, collect);
   }
}


/* DELETE_NULL_CLASSES
   09jan95 wmt: pass clsf_DS_max_n_classes to store_class_DS

   This deletes from clsf any classes where known-params-p is nil and
   the class-wt is less than min-wt, storing the deleted classes in the
   model's class-store.
   */
int delete_null_classes(clsf_DS clsf) /* min wt is option lisp arg, min_wt) */
{
  int i;
  class_DS cl,
  *classes=clsf->classes;
  int n_classes = clsf->n_classes,
  j = 0,
  n_stored = 0;
  float min_wt = max(ABSOLUTE_MIN_CLASS_WT, clsf->min_class_wt);

  /* in the lisp opt arg min wt is handled as follows:
     (setf min-wt (max *absolute-min-class-wt*
     (or min-wt
     (clsf-DS-min-class-wt clsf))))
     ******/

  while( j<n_classes) {
    cl = classes[j];
    if ( (cl->w_j <= min_wt) && (cl->known_parms_p != TRUE)) {
      n_stored++;
      n_classes--;
      store_class_DS(cl, clsf_DS_max_n_classes(clsf));
      for (i=j; i<n_classes; i++)
        classes[i] = classes[i+1];
    }
    else
      j++;                      /* not bumped when scoot since need to do j again*/
  }
  clsf->n_classes = n_classes;
  return(n_stored);
}


/* UPDATE_WTS
   29mar95 wmt: calculation in double
   30may95 wmt: pass in two clsfs to facilitate autoclass_predict, otherwise
                pass in the same clsf for both args.

   Updates the class-DS->wts of the active classes to be the normalized class
   probabilities with respect to the current parameterizations.  Sums and
   updates the class-DS->w_j. Updates and returns probability of the database
   relative to the model parameterizations: clsf->log-p_pi_theta.

   Note that the notation used here is closely modeled upon that used in Matt's
   math papers: read ln-p<X_i.C_j/pi.theta>_j as  "j'th element of the
   J-array of Log of Probability of (datum-X-sub-i and Class-sub-j GIVEN
   class-prob-sub-j and class-theta-sub-j)" */
   
void update_wts( clsf_DS training_clsf, clsf_DS test_clsf)
{
   class_DS class, *classes = training_clsf->classes; /* Vector of classes with parameters */
   class_DS *test_classes = test_clsf->classes; /* vector of classes with weights */
   database_DS data_base = test_clsf->database;

   int i, j, significant_terms, last_j_max, j_max;

   int 	n_j = training_clsf->n_classes,            /* No. of classes J */
	n_i = data_base->n_data;          /* No. of data i */
   float **X = data_base->data,
        *X_i;		 /* Vector of data-attr values */
   double *wt_j, 		/* Sums class probs over data */
   w_j,
   *ln_p, 		/* ln-p<X_i.C_j/pi.theta> log-likelihood of X_i in C_j */
   ln_p_X_i_C_j_pi_theta_j, ln_p_X_i_C_j_pi_theta_max,


   *p_pi_theta, 	/* p<C_j/X_i.pi.theta> Prob of C_j given X_i & params */
   p_X_i_pi_theta_div_p_X_i_C_j_pi_theta_max,
   p_X_i_C_j_pi_theta_div_p_X_i_C_j_pi_theta_max,

   log_cutoff = MOST_NEGATIVE_SINGLE_FLOAT_DIV_2 + safe_log( DOUBLE_FLOAT_EPSILON),

                                     /* Log of minimum relative probability */
   log_tolerance =  safe_log((double) max( DOUBLE_FLOAT_EPSILON,
                                             G_likelihood_tolerance_ratio)),

   ln_p_X_i_pi_theta=0.0, /* ln-p<X_i/pi.theta> log-prob of X_i given all classes */
   ln_p_X_pi_theta=0.0;   /* ln-p<X/pi.theta>  log-prob of data given all classes */

   if (training_clsf->n_classes == 0) {
      fprintf(stderr, "ERROR: update_weights called without any classes\n");
      abort();
   }
   wt_j = (double *) malloc(n_j * sizeof(double)); /*freed @end*/
   ln_p = (double *) malloc(n_j * sizeof(double)); /*freed @end*/
   p_pi_theta = (double *) malloc(n_j * sizeof(double)); /*freed @end*/

   for (j=0; j<n_j; j++)
      wt_j[j] = 0.0;


   /* Cycle over the data, computing the the class likelihood vectors
      ln-p_X_i_pi_theta and normalized probabilities for each datum.
      Accumulate the likelihood vector p_pi_theta & ln_p_X_i_pi_theta */

   for (i=0; i<n_i; i++) {
      X_i = X[i];                   /* For each datum x_i: */
				    /* datum_i */
      p_X_i_pi_theta_div_p_X_i_C_j_pi_theta_max = 1.0;
      ln_p_X_i_C_j_pi_theta_max = MOST_NEGATIVE_SINGLE_FLOAT_DIV_2;
      log_cutoff = MOST_NEGATIVE_SINGLE_FLOAT_DIV_2 + safe_log( DOUBLE_FLOAT_EPSILON);

      /* Compute the class likelihood vector for this datum
	 ln_p_X_i_C_j_p_i_theta.  Record the maximum class likelihood
	 ln_p_X_i_C_j_pi_theta_max, and its index j_max, for use in
	 normalization. */
      /* Dotimes starts with most probable class to initialize
	 ln_p_X_i_C_j_pi_theta_max.  Note that we abort the computation
	 of ln_p_X_i_C_j_pi_theta_j for values that are smaller than
	 (ln_p_X_i_C_j_pi_theta_max + log_tolerance), since they will
	 produce zeros in the normalized probabilities. */
      
      last_j_max = most_probable_class_for_datum_i(i, classes, n_j);
      j_max = -1;
      significant_terms = 1;

      for (j=0; j<n_j; j++) {                          /* For each Class C_j: */
	 /* do last_j_max first by switching with 0*/
	 if (j == last_j_max) 
	    j=0;
	 else if(j ==0)
	    j=last_j_max;

         class = classes[j];   
						       /* Compute raw ln-prob */
         ln_p_X_i_C_j_pi_theta_j = log_likelihood_fn( X_i, class, log_cutoff);
						  /* Remember max raw ln-prob */
         ln_p[j] = ln_p_X_i_C_j_pi_theta_j;
         if (ln_p_X_i_C_j_pi_theta_j > ln_p_X_i_C_j_pi_theta_max) {
            ln_p_X_i_C_j_pi_theta_max = ln_p_X_i_C_j_pi_theta_j;
            log_cutoff = ln_p_X_i_C_j_pi_theta_max + log_tolerance;
            j_max = j;
	 }
	 if (j == last_j_max) 
	    j=0;
	 else if(j ==0)
	    j=last_j_max;
	 
      }
      /* Note: the most negative values are NOT fully computed. */
      /*commented printf("\nlnp[j] for i=%d",i); for(j=0;j<n_j;j++)
        printf( " %f", ln_p[j]); dbg JTP */ 
      /* Normalize the raw ln_p_X_i_C_j_pi.theta to get p_C_j_X_i_pi_theta.
	 We avoid math problems in the division by rescaling the
	 ln_p_X_i_C_j_pi_theta_j w.r.t. ln_p_X_i_C_j_pi.theta_max and then
	 filtering out the too small values with log_tolerance. */
                       /* Collect significant p_X_i_C_j_pi_theta_j, rescaling
			  them w.r.t. ln_p_X_i_C_j_pi.theta_max               */
      for (j=0; j<n_j; j++) {
	 if (j == j_max)
	    p_pi_theta[j] = 1.0;  /* Set most significant value to 1. */
	 else {
					  /* Process the other values: */
	    ln_p_X_i_C_j_pi_theta_j = ln_p[j];
	    if (ln_p_X_i_C_j_pi_theta_j > log_cutoff) {  /* Significance test */
              /* Rescale significant probs relative to p_C_j_X_i_pi_theta_j-max = 1: */
               p_X_i_C_j_pi_theta_div_p_X_i_C_j_pi_theta_max =
                 safe_exp( ln_p_X_i_C_j_pi_theta_j - ln_p_X_i_C_j_pi_theta_max);
	       significant_terms++;
	       p_X_i_pi_theta_div_p_X_i_C_j_pi_theta_max +=
		  p_X_i_C_j_pi_theta_div_p_X_i_C_j_pi_theta_max;
	       p_pi_theta[j] =
		  p_X_i_C_j_pi_theta_div_p_X_i_C_j_pi_theta_max;
	    }
            /* Set insignificant probs to 0.0 */
	    else p_pi_theta[j] = 0.0;
	 }
      }
      /* Compute ln-p<X_i/pi.theta>  and  normalize  DSp<C_j/X_i.pi.theta>: */
      ln_p_X_i_pi_theta = ln_p_X_i_C_j_pi_theta_max;
      if (significant_terms != 1){
	 ln_p_X_i_pi_theta += safe_log(p_X_i_pi_theta_div_p_X_i_C_j_pi_theta_max);
         j=-1;
	 while( significant_terms > 0 )
	    if (p_pi_theta[++j] != 0.0) {
	       p_pi_theta[j] /= p_X_i_pi_theta_div_p_X_i_C_j_pi_theta_max;
	       significant_terms--;
	    }
      }
      /* Sum the datum log-likelihood into the data-base log-likelihood: */
      ln_p_X_pi_theta += ln_p_X_i_pi_theta;

   /* Save this datum's class probabilities as class weights & sum into wt_j: */
      for (j=0; j<n_j; j++) {
	 test_classes[j]->wts[i] = p_pi_theta[j];    /* Updating class wts */
/*commented printf(" cl %d wt %d  %f",j,i,test_classes[j]->wts[i]);  dbg JTP*/
         wt_j[j] += test_classes[j]->wts[i];
      }
/*       fprintf(stderr, "ln_p_X_i_pi_theta = %f\n", ln_p_X_i_pi_theta); */

   }/*end i*/

   /* Save the classification log-likelihood and update the class w_j: */
   training_clsf->log_p_x_h_pi_theta = ln_p_X_pi_theta;

/*commented printf("\nclsf->log_p_x_h_pi_theta = %f",clsf->log_p_x_h_pi_theta); dbg JTP*/

   for (j=0; j<n_j; j++) {
      class = classes[j];
      w_j = wt_j[j];
      wt_j[j] = w_j - class->w_j;            /* Saving the delta-wts as DSwt_j */
      class->w_j = w_j;                               /* Update the class wts */
      if (w_j == 0.0)
         class->log_w_j = LEAST_POSITIVE_SINGLE_LOG;
      else class->log_w_j = (float) safe_log((double) w_j);
/*commented printf("\n class %d w_j,log_w_j=%f %f",j,class->w_j,class->log_w_j);  dbg*/

   }
   free(p_pi_theta);
   free(ln_p);
   free(wt_j);
/* lisp version returns ln-p<x/pi.theta> and wt_j but no call seems to use*/
}


int most_probable_class_for_datum_i(int i, class_DS *classes,  int n_classes)
{
   int j,max_j;
   float maxval, wt_i_j;

   maxval = LEAST_NEGATIVE_SINGLE_FLOAT;
   max_j = 0;
   for (j=0; j<n_classes; j++) {
      wt_i_j = classes[j]->wts[i];
      if (maxval < wt_i_j) {
	 maxval = wt_i_j;
	 max_j = j;
      }
   }
   return(max_j);
}


/* UPDATE_LN_P_X_PI_THETA
   29mar95 wmt: calculation in double

   This is a simplified version of Update-Wts which calculates
   ln-p_X_H_pi_theta in terms of the current class weights and parameters.
   Note that the notation used here is closely modeled upon that used in
   Matt's math papers: read DSln-p<X_i.C_j/pi.theta>_j as  'j'th element
   of the J-array of Log of Probability of ( datum-X-sub-i and
   Class-sub-j GIVEN class-prob-sub-j and  class-theta-sub-j) */

void update_ln_p_x_pi_theta( clsf_DS clsf, int no_change)
{
   int i, j, n_i, n_j, j_max, last_j_max;
   float **x, *x_i;
   double *ln_p_x_i_c_j_pi_theta, ln_p_x_pi_theta, log_tolerance;
   double p_div_max, ln_p_div_max, ln_p_j;
   class_DS class, *classes;
   database_DS data_base;

   if (clsf->n_classes == 0){
      fprintf(stderr, "ERROR: update_ln_p_x_pi_theta called without any classes\n");
      abort();
   }
   data_base = clsf->database;
   x = data_base->data;		/* Vector of data-attr values */
   n_i = data_base->n_data;	/* No. of data i */
   classes = clsf->classes;	/* Vector of classes with parameters */
   n_j = clsf->n_classes;	/* No. of classes j */
				/* log-likelihood of X_i in C_j */
   ln_p_x_i_c_j_pi_theta = (double *) malloc( n_j * sizeof( double));/*freed @end*/
   ln_p_x_pi_theta = 0.0;	/* log-prob of data given all classes */
				/*  Log of minimum relative probability */
   log_tolerance = safe_log((double) max( DOUBLE_FLOAT_EPSILON,
                                            G_likelihood_tolerance_ratio));
 /* Compute the likelihood vector DSp<C_j/X_i.pi.theta>  &  ln-p<X_i/pi.theta> */
   for (i=0; i<n_i; i++) {	/* For each datum x_i: */
      x_i = x[i];               /* datum_i */
      p_div_max = 1.0;
      ln_p_div_max = MOST_NEGATIVE_SINGLE_FLOAT_DIV_2;            
      last_j_max = most_probable_class_for_datum_i(i, classes, n_j);
      j_max = 0;

      /* Compute the class likelihood array for this datum:
	 DSln-p<X_i.C_j/pi.theta>.  Record the maximum class likelihood
	 ln-p<X_i.C_j/pi.theta>_max, and its index j_max, for use in
	 normalization.  The macros Dotimes-Except-One-First and
	 Sum-Function-Calls-List-Till-Below-Limit in the LOG-LIKELIHOOD-FN
	 ensure that no more calculation is done than necessary to achieve
	 log-tolerance. */
      
      for (j=0; j<n_j; j++){
	 /* do last_j_max first by switching with 0*/
	 if (j == last_j_max) 
	    j=0;
	 else if(j ==0)
	    j=last_j_max;
         class = classes[j];
					               /* Compute raw ln-prob */
         ln_p_j = log_likelihood_fn( x_i, class, (ln_p_div_max + log_tolerance));
         ln_p_x_i_c_j_pi_theta[j] = ln_p_j;            /* Save raw ln-prob */
         if (ln_p_j > ln_p_div_max) {          /* Remember max raw ln-prob */
	    ln_p_div_max = ln_p_j;
	    j_max = j;
         }
	 if (j == last_j_max) 
	    j=0;
	 else if(j ==0)
	    j=last_j_max;
      }

      /* Convert the DSln-p<X_i.C_j/pi.theta> to probabilities and sum.
	 We avoid math problems in the division by rescaling the
	 DSln-p<X_i.C_j/pi.theta>_j w.r.t. ln-p<X_i.C_j/pi.theta>_max
	 and then filtering out the too small values with log-tolerance. */
      for (j=0; j<n_j; j++)
	 if (j != j_max) {  /* Process values with respect to that at the max */
	    ln_p_j = ln_p_x_i_c_j_pi_theta[j];
	    if ((ln_p_j - ln_p_div_max) > log_tolerance)

	       /* Rescale significant probs relative to
		  DSp<C_j/X_i.pi.theta>_j-max = 1 and sum together as
		  p<X_i.C_j/pi.theta>//p<X_i.C_j/pi.theta>_max: */
	       p_div_max += safe_exp( (ln_p_j - ln_p_div_max));
	 }
      /*  Calculate and sum the datum log-likelihood into the
	  data-base log-likelihood: */
      ln_p_x_pi_theta += ln_p_div_max + safe_log( p_div_max);
   }
   if (no_change == FALSE){
/*commented  printf(" at bottom of  update_ln_p_x_pi_theta, ln_p_x_pi_theta=%f\n",ln_p_x_pi_theta); dbg*/
      clsf->log_p_x_h_pi_theta = ln_p_x_pi_theta;
      }
   free( ln_p_x_i_c_j_pi_theta);
}
