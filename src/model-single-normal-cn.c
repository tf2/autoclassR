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


/* SN_CN_PARAMS_INFLUENCE_FN
   01feb95 wmt: 2.0 * global_variance (from ac-x)
   */
void sn_cn_params_influence_fn( model_DS model, tparm_DS tparm, int term_index, int n_att,
                               float *v, float *class_mean, float *class_sigma,
                               float *global_mean, float *global_sigma)
{
  struct sn_cn_param *param;
  tparm_DS *p;
  float global_variance;

  param = &(tparm->ptype.sn_cn);
  *class_mean = param->mean;
  *class_sigma = param->sigma;
  p = model_global_tparms(model);
  *global_mean = p[term_index]->ptype.sn_cn.mean;
  *global_sigma = p[term_index]->ptype.sn_cn.sigma;
  global_variance = p[term_index]->ptype.sn_cn.variance;
  *v = (float) log ((double) (*global_sigma / *class_sigma)) + 
    ((((square(*class_mean - *global_mean) +
      (param->variance - global_variance))) / 2.0) / global_variance);
}


/* BUILD_SN_CN_PRIORS
   30jul95 wmt: change log calls to safe_log to prevent "log: SING error"
   		error messages.

   Builds an SN-CN prior from the information in a fully instantiated att
   structure of the real type.
   */
static priors_DS build_sn_cn_priors( att_DS att)
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
      n = att->warnings_and_errors->num_expander_errors;
      att->warnings_and_errors->num_expander_errors += 1;
      if (n == 0)
         att->warnings_and_errors->model_expander_errors =
            (fxlstr *) malloc(sizeof(fxlstr));
      else att->warnings_and_errors->model_expander_errors =
         (fxlstr *) realloc(att->warnings_and_errors->model_expander_errors,
                            (n+1) * sizeof(fxlstr));
      strcpy(att->warnings_and_errors->model_expander_errors[n],
             "          single_normal_cn is faulty due to large error-to-range\n"
             "          ratio on sigma priors.\n");
   } 
   priors = (priors_DS) malloc(sizeof(struct priors));
   priors->sigma_min = sigma_min;
   priors->sigma_max = sigma_max;
   priors->known_prior = 0.0;   /* not used, but needed for priors struct init */
   priors->mean_mean = statistics->mean;
   priors->mean_sigma =
      max((range / 2.0),
           (SN_CM_SIGMA_SAFETY_FACTOR /
                (float) sqrt( ABSOLUTE_MIN_CLASS_WT)) * sigma_min);
   priors->mean_var = square(priors->mean_sigma);
   priors->minus_log_log_sigmas_ratio =
     - (float) safe_log( max( safe_log( 1.0 / (1.0 - SINGLE_FLOAT_EPSILON)),
                        max( safe_log((double) (priors->sigma_max / priors->sigma_min)),
                            LEAST_POSITIVE_SHORT_FLOAT)));
   priors->minus_log_mean_sigma = - (float) safe_log( (double) priors->mean_sigma);
   return(priors);
}


/* SINGLE_NORMAL_CN_MODEL_TERM_BUILDER
   21nov94 wmt: initialize all slots in tparm
   18dec94 wmt: finish "missing" error msg
   30jul95 wmt: change log calls to safe_log to prevent "log: SING error"
   		error messages.

   Funcalled from Expand-Model-Terms.  This constructs parameter, prior, and
   intermediate results structures appropriate to a single-normal likelihood
   term, and places them in the model.  Constructs corresponding log-likelihood
   and parameter update function elements and saves them on the model for later
   compilation. 
*/
void single_normal_cn_model_term_builder( model_DS model, term_DS term, int n_term)
   /* model_DS model;         The model-DS to which this term will contribute. */
   /* term_DS term;          The singleton term-DS definig this attributes use. */
   /* int n_term;           The term index for various model-DS substructures. */
{
   void ***att_trans_data;
   int n, n_att, n_att_trans_data;
   float error, log_att_delta, log_delta_div_root_2pi;
   database_DS data_base;
   att_DS att;
   priors_DS prior_set;
   tparm_DS tparm;
   struct sn_cn_param *sn;

   n_att = term->att_list[0];                  /* index for this SN attribute */
   data_base = model->database;
   att = data_base->att_info[n_att];                 /* Attribute description */
   error = att->error;

   att_trans_data = (void ***) get("single_normal_cn", "att_trans_data");
   n_att_trans_data = ((int *) get("single_normal_cn", "n_att_trans_data"))[0];
   if (getf(att_trans_data, att->type, n_att_trans_data) == NULL)
      fprintf( stderr, "Attribute %d not one allowed for single_normal_cn terms\n",
		       n_att);
   if ( att->missing) {
      if( (n = att->warnings_and_errors->num_expander_errors ++) == 0)
	 att->warnings_and_errors->model_expander_errors =
	    (fxlstr *) malloc(sizeof(fxlstr));
      else 
	 att->warnings_and_errors->model_expander_errors =
            (fxlstr *) realloc(att->warnings_and_errors->model_expander_errors,
			    (n+1) * sizeof(fxlstr));
      strcpy(att->warnings_and_errors->model_expander_errors[n],
             "          using single_normal_cn model on attribute with missing values\n");
   }
   if (term->n_atts != 1)
      fprintf( stderr,
             "Attribute %d using single_normal_cn model in non-singleton set\n",
              n_att);
   if (error <= 0.0)
      fprintf( stderr,
	  "Attribute %d using single_normal_cn model with non-positive error\n",
          n_att);
   log_att_delta = (float) safe_log((double) error);
   log_delta_div_root_2pi = LN_1_DIV_ROOT_2PI + log_att_delta;

		                                /* Allocate parameters struct */

   term->tparm = tparm = (tparm_DS) malloc(sizeof(struct new_term_params));
   tparm->tppt=SN_CN;

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
   tparm->log_pi = -1.0 * (float) safe_log( M_PI);

   sn = &(tparm->ptype.sn_cn);
			                            /* Allocate & SET priors. */
   prior_set = model->priors[n_term] = build_sn_cn_priors(att);

   sn->weighted_mean = sn->weighted_var = sn->mean = sn->sigma =0.0;
   sn->log_sigma = sn->variance = sn->log_variance = sn->inv_variance= 0.0;
   sn->ll_min_diff = sn->skewness = sn->kurtosis = 0.0;

   sn->prior_mean_mean = prior_set->mean_mean;
   sn->prior_mean_var = prior_set->mean_var;
   sn->prior_mean_sigma = prior_set->mean_sigma;
   sn->prior_sigmas_term = (-1.5 * (float) safe_log( 2.0)) +
      prior_set->minus_log_log_sigmas_ratio +
      (-1.0 * (float) safe_log((double) prior_set->mean_sigma));
                                               /* Generate function elements: */
   sn->prior_sigma_min_2 = square(prior_set->sigma_min);
   sn->prior_sigma_max_2 = square(prior_set->sigma_max);

/* function elements are no longer genereated. preprocessed constands are 
	stored in the params and calls are made with parameter lists in call*/
}


/* SINGLE_NORMAL_CN_LOG_LIKELIHOOD
   20dec94 wmt: return type to double 
   When called within the environment of Log-Likelihood-fn, this calculates the
   probability of a Single-Normal-cn term in 'datum given 'params. */

double single_normal_cn_log_likelihood( tparm_DS tparm)
{
  int n_att = tparm->n_att;
  float log_delta = tparm->log_delta, *datum = tparm->datum;
  struct sn_cn_param *sn = &(tparm->ptype.sn_cn);

  return (log_delta + (-0.5 * (sn->log_variance +
                               (square(sn->mean - datum[n_att]) * 
                                sn->inv_variance))));
}


/* SINGLE_NORMAL_CN_UPDATE_L_APPROX
   20dec94 wmt: return type to double
   
   When called within the environment of Update-L-Approx-fn, this calculates the
   approximate log marginal likelihood log-a<w_j.S_j/H_j>_k of observing the
   weighted statistics given the class hypothesis alone.  See
   Single-Normal-cn-Update-M-approx-term-caller.
   A LOG-LINEAR APPROXIMATION IS USED IN THE REGION WHERE
   0 <= w_j-known <= (* .75 *absolute-min-class-wt*)
   */
double single_normal_cn_update_l_approx( tparm_DS tparm)
{
   float log_delta = tparm->log_delta, diff, t1, t2, w_j = tparm->w_j;
   struct sn_cn_param *sn = &(tparm->ptype.sn_cn);

   diff = sn->weighted_mean - sn->mean;
   t1 = ( fabs((double) diff) <= sqrt( LEAST_POSITIVE_SINGLE_FLOAT))
      ? 0.0 : square(diff);

   t2 = w_j * (log_delta - sn->log_sigma) +
       (-0.5 * w_j * (sn->weighted_var + t1) / sn->variance);
   return (t2);
}


/* SINGLE_NORMAL_CN_UPDATE_M_APPROX
   20dec94 wmt: return type to double 
   29mar95 wmt: calculation to double

   When called within the environment of Update-M-Approx-fn, this calculates
   the approximate log marginal likelihood log-a<w_j.S_j/H_j>_k of observing
   the weighted statistics given the class hypothesis alone.  See
   Single-Normal-cn-Update-M-approx-term-caller.
   */
double single_normal_cn_update_m_approx( tparm_DS tparm)
{
  struct sn_cn_param *sn = &(tparm->ptype.sn_cn);
  float log_att_delta = tparm->log_att_delta;
  float prior_mean_mean = sn->prior_mean_mean;
  float prior_mean_sigma = sn->prior_mean_sigma, diff, t1;
  float prior_sigmas_term = sn->prior_sigmas_term, w_j = tparm->w_j;
  double t2;

  diff = sn->weighted_mean - prior_mean_mean;
  t1 = ( fabs((double) diff) <=
        (prior_mean_sigma * sqrt( 2.0 * LEAST_POSITIVE_SINGLE_FLOAT)))
    ? 0.0 : -0.5 * square( diff / prior_mean_sigma);

  t2 =  log_gamma( (double) (0.5 * (w_j - 1.0)), FALSE) + 
         (double) t1 +
         (double) (w_j * log_att_delta) +
         (-0.5 * (double) w_j * safe_log((double) (M_PI * w_j))) +
         (-0.5 * (double) (w_j - 1.0) *
          safe_log( (double) max( LEAST_POSITIVE_SINGLE_FLOAT,
                            sn->weighted_var))) +
         (double) prior_sigmas_term;
  return (t2);
}


/* SINGLE_NORMAL_CN_UPDATE_PARAMS
   27nov94 wmt: use percent_equal for float tests
   20dec94 wmt: return type to void
   */
void single_normal_cn_update_params( tparm_DS tparm, int known_parms_p)
{
  int n_att = tparm->n_att, n_data = tparm->n_data;
  struct sn_cn_param *sn=&(tparm->ptype.sn_cn);

  float prior_sigma_min_2 = sn->prior_sigma_min_2;
  float prior_sigma_max_2 = sn->prior_sigma_max_2;
  float prior_mean_mean = sn->prior_mean_mean;
  float prior_mean_var = sn->prior_mean_var;
  float **data = tparm->data, *wts = tparm->wts;
  float class_wt = tparm->class_wt, class_wt_1;
  float ignore1, ignore2, mean, variance, skewness, kurtosis, var_ratio;

  class_wt_1 = class_wt + 1;
  if (class_wt > 0.0) {         /* Zero class-wt implies null class */
    if ( tparm->collect == TRUE) {
      /* If not collect?, we proceed with the previous values. */
      central_measures_x(data, n_data, n_att, wts,
                         percent_equal( (double) sn->mean, FLOAT_UNKNOWN,
                                       REL_ERROR) ?
                         (double) prior_mean_mean : (double) sn->mean,
                         &ignore1, &ignore2, &mean, &variance, &skewness, &kurtosis);

      sn->weighted_mean = mean;
      /* Limit the weighted variance, rather than sigma, to get
         consistent results: */
      sn->weighted_var =
        max(prior_sigma_min_2,
            min(prior_sigma_max_2,
                min(
		    class_wt * SN_CN_SIGMA_SAFETY_FACTOR * prior_mean_var,
		    variance)));
      sn->skewness = skewness;
      sn->kurtosis = kurtosis;
    }
    else {
      sn->weighted_mean = prior_mean_mean;
      sn->weighted_var = prior_mean_var;
    }
  }
  if (known_parms_p != TRUE) {  /* Update class parameters */
    var_ratio = sn->weighted_var / (class_wt_1 * prior_mean_var);
    sn->mean = (sn->weighted_mean * (1.0 - var_ratio)) +
      (prior_mean_mean * var_ratio);
    sn->variance =  max(sn->weighted_var * (class_wt / class_wt_1),
                        (float) sqrt( LEAST_POSITIVE_SINGLE_FLOAT));
    sn->sigma = (float) sqrt((double) sn->variance);
    sn->log_variance = (float) safe_log((double) sn->variance);
    sn->log_sigma = 0.5 * sn->log_variance;
    sn->inv_variance = 1.0 / sn->variance;
    sn->ll_min_diff =
      max((float) sqrt( LEAST_POSITIVE_SINGLE_FLOAT),
          (sn->variance *
           (float) sqrt( LEAST_POSITIVE_SINGLE_FLOAT)));
  }
  /* return(class_wt); */
}


/* When called within the environment of Class-Equivalence-fn, this tests for a
   difference of means less than sigma=ratio times MIN of sigmas. */

int single_normal_cn_class_equivalence( tparm_DS tparm1,tparm_DS tparm2,
                                       double sigma_ratio)
{
  struct sn_cn_param *sn1 = &(tparm1->ptype.sn_cn), *sn2 = &(tparm2->ptype.sn_cn);

  if (fabs((double) (sn1->mean - sn2->mean)) <
      (sigma_ratio * (double) min(sn1->sigma, sn2->sigma)))
    return(TRUE);
  else
    return(FALSE);
}


/* SINGLE_NORMAL_CN_CLASS_MERGED_MARGINAL
   20dec94 wmt: return type to void

   When called within the environment of Class-Merged-Marginal-fn, this
   generates the sufficient statistics of Single-Normal-cn term equivalent
   to the weighted merging of params-0 and params-1, storing same in
   params-m. 
   dont know what wt1 and wt0 and wtm are supposed to be. should snm be
   returned maybe and let caller get snm->...
        */
void single_normal_cn_class_merged_marginal( tparm_DS tparm0, tparm_DS tparm1,
                                            tparm_DS tparmm)
{
   float wt_0=.5, wt_1=.5, wt_m=1.0;
   struct sn_cn_param   *sn0=&(tparm0->ptype.sn_cn), 
			*sn1=&(tparm1->ptype.sn_cn),
			*snm=&(tparmm->ptype.sn_cn);
   float prior_sigma_min_2 = sn0->prior_sigma_min_2;

   snm->weighted_mean =
      ((wt_0 * sn0->weighted_mean) + (wt_1 * sn1->weighted_mean)) / wt_m;
   snm->weighted_var =
      max(prior_sigma_min_2,
         (((wt_0 * (square(sn0->weighted_mean) + sn0->weighted_var)) +
           (wt_1 * (square(sn1->weighted_mean) + sn1->weighted_var))) /
          wt_m) -
         square(snm->weighted_mean));
   /* return (snm->weighted_var); */
}
