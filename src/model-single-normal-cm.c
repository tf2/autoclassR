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


void sn_cm_params_influence_fn( model_DS model, tparm_DS tparm, int term_index,int  n_att,
	float *v, float *class_mean, float *class_sigma, float *class_known_prob,
	float *global_mean, float *global_sigma, float *global_known_prob)
{
   struct sn_cm_param *param;
   tparm_DS *p;
   float class_variance, global_variance, v1, v2, v3;

   param = &(tparm->ptype.sn_cm);
   *class_mean = param->mean;
   *class_sigma = param->sigma;
   *class_known_prob = param->known_prob;
   class_variance = param->variance;
   p = model_global_tparms(model);
   *global_mean = p[term_index]->ptype.sn_cm.mean;
   *global_sigma = p[term_index]->ptype.sn_cm.sigma;
   *global_known_prob = p[term_index]->ptype.sn_cm.known_prob;
   global_variance = p[term_index]->ptype.sn_cm.variance;
   v1 = *class_known_prob * 
     (param->known_log_prob - p[term_index]->ptype.sn_cm.known_log_prob); 
   v2 = (1.0 - *class_known_prob) * 
     (param->unknown_log_prob - p[term_index]->ptype.sn_cm.unknown_log_prob); 
   v3 = *class_known_prob *  
        ((float) log ((double) (*global_sigma / *class_sigma)) + 
        (((square(*class_mean - *global_mean) +
        (class_variance - global_variance)) / 2.0) / global_variance));
   *v = v1 + v2 + v3;
}


/* BUILD_SN_CM_PRIORS
   30jul95 wmt: change log calls to safe_log to prevent "log: SING error"
   		error messages.

   Builds an SN-CM prior from the information in a fully instantiated att
   structure of the real type.
   */

static priors_DS build_sn_cm_priors( database_DS data_base, att_DS att)
{
   int n;
   float range, sigma_min, sigma_max;
   priors_DS priors;
   real_stats_DS statistics = att->r_statistics;/* Attribute range information */

   range = statistics->mx - statistics->mn;
   sigma_min = SN_CM_SIGMA_SAFETY_FACTOR * att->error;
                                        /* The max sigma for values in range. */
   sigma_max = max(sigma_min, range / 2.0);
   if (sigma_min == sigma_max) {
      if ( (n = att->warnings_and_errors->num_expander_errors ++) == 0)
	 att->warnings_and_errors->model_expander_errors =
	    (fxlstr *) malloc(sizeof(fxlstr));
      else 
	 att->warnings_and_errors->model_expander_errors =
	    (fxlstr *) realloc(att->warnings_and_errors->model_expander_errors,
			    (n+1) * sizeof(fxlstr));
      strcpy(att->warnings_and_errors->model_expander_errors[n],
	     "          single_normal_cm is faulty due to large error-to-range\n"
             "          ratio on sigma priors.\n");
   }
   priors = (priors_DS) malloc(sizeof(struct priors));
   priors->known_prior = (0.5 + statistics->count) / (1.0 + data_base->n_data);
   priors->sigma_min = sigma_min;
   priors->sigma_max = sigma_max;
   priors->mean_mean = statistics->mean;
   priors->mean_sigma =
      max((range / 2.0),
           (SN_CM_SIGMA_SAFETY_FACTOR /
                (float) sqrt( ABSOLUTE_MIN_CLASS_WT)) * sigma_min);
   priors->mean_var = square(priors->mean_sigma);
   priors->minus_log_log_sigmas_ratio =
     - (float) safe_log( max( safe_log( 1.0 / (1.0 - SINGLE_FLOAT_EPSILON)),
                        max( safe_log( (double) (priors->sigma_max / priors->sigma_min)),
                            LEAST_POSITIVE_SHORT_FLOAT)));
   priors->minus_log_mean_sigma = - (float) safe_log( (double) priors->mean_sigma);

   return(priors);
}


/* SINGLE_NORMAL_CM_MODEL_TERM_BUILDER

   21nov94 wmt: initialize all slots in tparm
   30jul95 wmt: change log calls to safe_log to prevent "log: SING error"
   		error messages.

   Funcalled from Expand-Model-Terms.  This constructs parameter, prior, and
   intermediate results structures appropriate to a single-normal likelihood
   term, and places them in the model.  Constructs corresponding log-likelihood
   and parameter update function elements and saves them on the model for later
   compilation. 
*/
void single_normal_cm_model_term_builder( model_DS model, term_DS term, int n_term)
{
   void ***att_trans_data;
   int n, n_att, n_att_trans_data;
   float log_att_delta, log_delta_div_root_2pi, error;
   att_DS att;
   database_DS data_base;
   priors_DS prior_set;
   tparm_DS tparm;
   struct sn_cm_param *sn;

   n_att = term->att_list[0];
   data_base = model->database;
   att = data_base->att_info[n_att];
   error = att->error;
   att_trans_data = (void ***) get("single_normal_cm", "att_trans_data");
   n_att_trans_data = ((int *) get("single_normal_cm", "n_att_trans_data"))[0];
   if (getf(att_trans_data, att->type, n_att_trans_data) == NULL)
     fprintf( stderr,
      "Attribute %d: \"%s\" not one of those allowed for single_normal_cm terms.\n",
      n_att, att->dscrp);
   if ( att->missing == FALSE) {
      if( (n = att->warnings_and_errors->num_expander_warnings ++) == 0)
	 att->warnings_and_errors->model_expander_warnings =
	    (fxlstr *) malloc(sizeof(fxlstr));
      else 
	 att->warnings_and_errors->model_expander_warnings =
            (fxlstr *) realloc(att->warnings_and_errors->model_expander_warnings,
			    (n+1) * sizeof(fxlstr));
      strcpy(att->warnings_and_errors->model_expander_warnings[n],
             "            using single_normal_cm model on att which has NO missing values\n");
   }
   if (term->n_atts != 1)
     fprintf( stderr,
      "Attribute %d: \"%s\":  attempting to use single_normal_cm model in a \n"
              "          non-singleton attribute set\n",
      n_att, att->dscrp);
   if (error <= 0.0)
      fprintf( stderr, "Attribute %d: \"%s\", attempting to use single_normal_cm model "
              "with non-positive error value %f.\n",
      n_att, att->dscrp, error);
   log_att_delta = (float) safe_log((double) error);
   log_delta_div_root_2pi = LN_1_DIV_ROOT_2PI + log_att_delta;

		                              /*  Allocate parameters struct. */
   term->tparm = tparm = (tparm_DS) malloc(sizeof(struct new_term_params));
   tparm->tppt=SN_CM;

   tparm->n_atts = term->n_atts;
   tparm->n_term = n_term;
   tparm->n_att = n_att;
   tparm->n_att_indices = tparm->n_datum = tparm->n_data = 0;
   tparm->wts = tparm->datum = tparm->att_indices = NULL;
   tparm->data = NULL;
   tparm->w_j = tparm->ranges = tparm->class_wt = 0.0;
   tparm->disc_scale = 0.0;
   tparm->wt_m = tparm->log_marginal = 0.0;

   tparm->log_delta = log_delta_div_root_2pi;
   tparm->log_att_delta = log_att_delta;
   tparm->log_pi = (float) (-1.0 * safe_log( M_PI));

   sn = &(tparm->ptype.sn_cm);

   sn->known_wt = sn->known_prob = sn->known_log_prob = sn->unknown_log_prob = 0.0;
   sn->weighted_mean = sn->weighted_var = sn->mean = sn->sigma = 0.0;
   sn->log_sigma = sn->variance = sn->log_variance = sn->inv_variance = 0.0;
   sn->ll_min_diff = sn->skewness = sn->kurtosis = 0.0;
						    /* Allocate & SET priors. */
   prior_set = model->priors[n_term] = build_sn_cm_priors(data_base, att);

   sn->prior_mean_mean = prior_set->mean_mean;
   sn->prior_mean_var = prior_set->mean_var; 
   sn->prior_mean_sigma = prior_set->mean_sigma;
   sn->prior_sigmas_term = (-1.5 * (float) safe_log( 2.0)) +
      prior_set->minus_log_log_sigmas_ratio +
      (-1.0 * (float) safe_log((double) prior_set->mean_sigma));

   sn->prior_sigma_min_2 = square(prior_set->sigma_min);
   sn->prior_sigma_max_2 = square(prior_set->sigma_max);
   sn->prior_known_prior = prior_set->known_prior;

   /*done above temp->parms->prior_sigma_min_2 = square(prior_set->sigma_min);*/
}


/* SINGLE_NORMAL_CM_LOG_LIKELIHOOD
   27nov94 wmt: use percent_equal for float tests
   20dec94 wmt: return type to double
   23dec94 wmt: check unknown values with FLOAT_UNKNOWN, rather than INT_UNKNOWN
   */
double single_normal_cm_log_likelihood( tparm_DS tparm)
{
  int n_att = tparm->n_att;
  struct sn_cm_param *sn = &(tparm->ptype.sn_cm);
  float log_delta = tparm->log_delta, *datum = tparm->datum, value, diff, temp;

  value = datum[n_att];
  if( percent_equal( (double) value, FLOAT_UNKNOWN, REL_ERROR))
    return(sn->unknown_log_prob);

  diff = sn->mean - value;
  if ((float) fabs((double) diff) <= sn->ll_min_diff)
    temp = 0.0;
  else 
    temp = square(diff) * sn->inv_variance;
  return (sn->known_log_prob + log_delta + (-0.5 * (temp + sn->log_variance)));
}


/* SINGLE_NORMAL_CM_UPDATE_L_APPROX
   20dec94 wmt: return type to double
   
   When called within the environment of Update-L-Approx-fn, this calculates
   the approximate log likelihood log-a<w_j.S_j/H_j.theta_j>_k of observing
   the weighted statistics given the class hypothesis and current parameters.
   */
double single_normal_cm_update_l_approx( tparm_DS tparm)
{
  struct sn_cm_param *sn=&(tparm->ptype.sn_cm);
  float w_j_known, log_delta = tparm->log_delta, diff, t1, t2;
  float w_j = tparm->w_j;

  w_j_known = sn->known_wt;
  diff = sn->weighted_mean - sn->mean;
  if (fabs((double) diff) <= sqrt( LEAST_POSITIVE_SINGLE_FLOAT))
    t1 = 0.0;
  else
    t1 = square( diff);
  t1 = -0.5 * w_j_known * ((sn->weighted_var + t1) / sn->variance);
  t2 = ((w_j - w_j_known) * sn->unknown_log_prob) +
    (w_j_known * sn->known_log_prob) +
      (w_j_known * (log_delta - (float) safe_log((double) sn->sigma))) + t1;
  return (t2);
}


/* SINGLE_NORMAL_CM_UPDATE_M_APPROX
   20dec94 wmt: return type to double 
   29mar95 wmt: calculation to double

   When called within the environment of Update-M-Approx-fn, this calculates the
   approximate log marginal likelihood log-a<w_j.S_j/H_j>_k of observing the
   weighted statistics given the class hypothesis alone.  See
   Single-Normal-cn-Update-M-approx-term-caller.
   A LOG-LINEAR APPROXIMATION IS USED IN THE REGION WHERE
   0 <= w_j-known <= (* .75 *absolute-min-class-wt*)
   */
double single_normal_cm_update_m_approx( tparm_DS tparm)
{
  struct sn_cm_param *sn=&(tparm->ptype.sn_cm);
  float log_att_delta = tparm->log_att_delta;
  float prior_mean_mean = sn->prior_mean_mean;
  float prior_sigmas_term = sn->prior_sigmas_term, diff, t1, t2;
  float prior_mean_sigma = sn->prior_mean_sigma;
  float log_pi = tparm->log_pi, w_j = tparm->w_j, w_j_known, t_w_j_known;
  double temp;

  w_j_known = sn->known_wt;
  t_w_j_known = max(w_j_known, 0.75 * ABSOLUTE_MIN_CLASS_WT);
  diff = sn->weighted_mean - prior_mean_mean;
  if ((float) fabs((double) diff) <=
      (prior_mean_sigma *
       (float) sqrt( 2.0 * LEAST_POSITIVE_SINGLE_FLOAT)))
    t1 = 0.0;
  else t1 = -0.5 * square(diff / prior_mean_sigma);
  if (w_j_known == t_w_j_known)
    t2 = 1.0;
  else
    t2 = w_j_known / t_w_j_known;

  temp = (double) log_pi + (-1.0 * (log_gamma( (double) (w_j + 1.0), FALSE))) +
    log_gamma( (double) (0.5 + (w_j - t_w_j_known)), FALSE) +
      log_gamma( (double) (0.5 + t_w_j_known), FALSE) +
        log_gamma( (double) (0.5 * (t_w_j_known - 1.0)), FALSE) + (double) t1 +
	  (double) (t_w_j_known * log_att_delta) +
            (-0.5 * (double) t_w_j_known *
	     safe_log( M_PI * (double) t_w_j_known)) +
               (-0.5 * (double) (t_w_j_known - 1.0) *
                safe_log((double) max( LEAST_POSITIVE_SINGLE_FLOAT, sn->weighted_var))) +
                  prior_sigmas_term;
  return( ((double) t2) * temp);
}


/* SINGLE_NORMAL_CM_UPDATE_PARAMS
   27nov94 wmt: use percent_equal for float tests
   20dec94 wmt: return type to void

   When called within the environment of Update-Params-fn, this updates the
   param-set of a Single-Normal-cn term.  See
   Single-Normal-cn-Update-Params-term-caller.
   Revised 12Feb90 JCS
   Use of double precision for weighted calculations will triple the runtime. 
*/
void single_normal_cm_update_params( tparm_DS tparm, int known_parms_p)
{
  int n_att = tparm->n_att, n_data = tparm->n_data;
  struct sn_cm_param *sn_cm=&(tparm->ptype.sn_cm);
  float prior_sigma_min_2 = sn_cm->prior_sigma_min_2;
  float prior_sigma_max_2 = sn_cm->prior_sigma_max_2;
  float prior_mean_mean = sn_cm->prior_mean_mean;
  float prior_mean_var = sn_cm->prior_mean_var;
  float prior_known_prior = sn_cm->prior_known_prior;
  float **data = tparm->data, *wts = tparm->wts, prob_known, var_ratio;
  float class_wt = tparm->class_wt, class_wt_1;
  float ignore1, known, mean, variance, skewness, kurtosis;

  class_wt_1 = class_wt + 1;
  if (class_wt > 0.0) {         /* Zero class-wt implies null class */
    /* Update the class statistics from class-DS-wts & database */
    /* If not collect?, we proceed with the previous values. */
    if (tparm->collect == TRUE) {
      central_measures_x(data, n_data, n_att, wts, 
                         percent_equal( (double) sn_cm->mean, FLOAT_UNKNOWN,
                                       REL_ERROR) ?
                         (double) prior_mean_mean : (double) sn_cm->mean,
                         &ignore1, &known, &mean, &variance, &skewness, &kurtosis);
      if (known == 0.0) {
        sn_cm->known_wt = 0.0;
        sn_cm->weighted_mean = prior_mean_mean;
        sn_cm->weighted_var = prior_mean_var;
      }
      else {
        sn_cm->known_wt = known;
        /* commented fprintf(stderr, "Setting known_wt to known %f\n", known);*/
        sn_cm->weighted_mean = mean;
        sn_cm->weighted_var =
          max(prior_sigma_min_2, min(prior_sigma_max_2, variance));
        sn_cm->skewness = skewness;
        sn_cm->kurtosis = kurtosis;
      }
    }
  }
  else {
    sn_cm->known_wt = 0.0;
    sn_cm->weighted_mean = prior_mean_mean;
    sn_cm->weighted_var = prior_mean_var;
  }
  if (known_parms_p != TRUE) {
    prob_known = max(LEAST_POSITIVE_SINGLE_FLOAT,
                     (sn_cm->known_wt + prior_known_prior) / class_wt_1);
    sn_cm->known_prob = prob_known;
    sn_cm->known_log_prob = (float) safe_log((double) prob_known);
    sn_cm->unknown_log_prob =
      (float) safe_log((double) max( LEAST_POSITIVE_SINGLE_FLOAT,
                               1.0 - prob_known));
    var_ratio = sn_cm->weighted_var / (class_wt_1 * prior_mean_var);
    sn_cm->mean = (sn_cm->weighted_mean * (1.0 - var_ratio)) +
      (prior_mean_mean * var_ratio);
    sn_cm->variance = max(sn_cm->weighted_var * (class_wt / class_wt_1),
                          (float) sqrt( LEAST_POSITIVE_SINGLE_FLOAT));
    sn_cm->sigma = (float) sqrt((double) sn_cm->variance);
    sn_cm->log_variance = (float) safe_log((double) sn_cm->variance);
    sn_cm->log_sigma = 0.5 * sn_cm->log_variance;
    sn_cm->inv_variance = 1.0 / sn_cm->variance;
    sn_cm->ll_min_diff =
      max((float) sqrt( LEAST_POSITIVE_SINGLE_FLOAT),
          (sn_cm->variance *
	   (float) sqrt( LEAST_POSITIVE_SINGLE_FLOAT)));
  }
  /* return(class_wt); */
}


/* When called within the environment of Class-Equivalence-fn, this tests for a
   difference of means less than sigma=ratio times MIN of sigmas. */

int single_normal_cm_class_equivalence( tparm_DS tparm1,tparm_DS tparm2, double sigma_ratio)
{
   struct sn_cm_param *sn1 = &(tparm1->ptype.sn_cm);
   struct sn_cm_param *sn2 = &(tparm2->ptype.sn_cm);

   if (fabs((double) (sn1->mean - sn2->mean)) <
       (sigma_ratio * (double) min(sn1->sigma, sn2->sigma)))
     return(TRUE);
   else
     return(FALSE);
}


/* SINGLE_NORMAL_CM_CLASS_MERGED_MARGINAL
   20dec94 wmt: return type to void

   When called within the environment of Class-Merged-Marginal-fn, this
   generates the sufficient statistics of Single-Normal-cn term equivalent
   to the weighted merging of params-0 and params-1, storing same in
   params-m. 
*/
void single_normal_cm_class_merged_marginal( tparm_DS tparm0,tparm_DS tparm1,tparm_DS tparmm)
{

   struct sn_cm_param *sn0=&(tparm0->ptype.sn_cm);
   struct sn_cm_param *sn1=&(tparm1->ptype.sn_cm);
   struct sn_cm_param *snm=&(tparmm->ptype.sn_cm);
   float prior_sigma_min_2 = sn0->prior_sigma_min_2;
   float kwt0, kwt1, kwtm;

   kwt0 = sn0->known_wt;     
   kwt1 = sn1->known_wt;     
   kwtm = kwt0 + kwt1;
   snm->known_wt = kwtm;
   fprintf(stderr, "Set known_wt to %f\n", kwtm);
   if (kwtm != 0.0) {
      snm->weighted_mean =
	 ((kwt0 * sn0->weighted_mean) + (kwt1 * sn1->weighted_mean)) / kwtm;
      snm->weighted_var =
	 max(prior_sigma_min_2,
	      (((kwt0 * (square(sn0->weighted_mean) + sn0->weighted_var)) +
	        (kwt1 * (square(sn1->weighted_mean) + sn1->weighted_var))) /
	       kwtm) -
	      square(snm->weighted_mean));
      /* return (snm->weighted_var);  **************/
   }
   /* else return (FLOAT_UNKNOWN); ****************** nil? */
}
