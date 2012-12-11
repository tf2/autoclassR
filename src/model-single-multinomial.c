#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
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


/* SM_PARAMS_INFLUENCE_FN
   06feb95 wmt: add length_ptr; realloc( 3 *  => realloc( 3 *  (i+1)
   22jul95 wmt: replace if (global_att_prob != 0.0) with better test

   Compute the influence value for a Single Multinomial model term.
   This is the sum over value-index l of P_jkl * log( P_jkl / P_kl), where P_jkl is the
   the probability for value l of attribute k in class j and P_kl is the probability of
   value l of attribute k for a single class classification of the database.
   */
void sm_params_influence_fn( model_DS model, tparm_DS term_params, int term_index,
                            int n_att, float *influence_value,
                            float **class_div_global_att_prob_list_ptr, int *length_ptr)
{
   struct sm_param *param;
   tparm_DS *p;
   fptr class_probs, global_probs;
   int i, ii;
   float class_att_prob, global_att_prob, *class_div_global_att_prob_list;

   param = &(term_params->ptype.sm);
   class_probs = param->val_probs;
   p = model_global_tparms( model);
   global_probs = p[term_index]->ptype.sm.val_probs;
   *length_ptr = 0;
   *influence_value = 0.0;
   class_div_global_att_prob_list = NULL;
   ii = 0;

   for (i=0; i < param->range; i++) {
      class_att_prob = class_probs[i];
      global_att_prob = global_probs[i];
      if (class_div_global_att_prob_list == NULL)  
         class_div_global_att_prob_list = (float *) malloc( 3 * sizeof( float)); 
      else
         class_div_global_att_prob_list = (float *)
                realloc( class_div_global_att_prob_list, 3 * (i+1) * sizeof( float)); 
      class_div_global_att_prob_list[ii] = (float) i;
      ii++;
      class_div_global_att_prob_list[ii] = class_att_prob;  
      ii++;
      class_div_global_att_prob_list[ii] = global_att_prob;  
      ii++;
      (*length_ptr)++;
      if ((global_att_prob > LEAST_POSITIVE_SINGLE_FLOAT) &&
          ((class_att_prob / global_att_prob) < MOST_POSITIVE_SINGLE_FLOAT))
         *influence_value += class_att_prob *  
            (float) log ((double) (class_att_prob / global_att_prob));
   }
   *class_div_global_att_prob_list_ptr = class_div_global_att_prob_list;
}

 
/* SINGLE_MULTINOMIAL_MODEL_TERM_BUILDER
   24oct94 wmt: initialize val_wts, val_probs, val_log_probs
   21nov94 wmt: initialize all slots in tparm

   Funcalled from Expand-Model-Terms.  This constructs parameter, prior, and
   intermediate results structures appropriate to a Single-Multinomial
   likelihood term, and places them in the model.  Constructs corresponding
   log-likelihood and parameter update function elements and saves them on the
   model for later compilation. */

void single_multinomial_model_term_builder( model_DS model, term_DS term, int n_term)
{
   int  n_att, range, i;
   att_DS att;
   struct sm_param *p;
   tparm_DS tparm;
 
   n_att = term->att_list[0];
   att = model->database->att_info[n_att];
   range = att->d_statistics->range;
                                                 /* Allocate data structures: */
						 /* Allocate parameters. */
   term->tparm = tparm = (tparm_DS) malloc(sizeof(struct new_term_params));
   tparm->n_atts=range;
   tparm->collect=0;
   tparm->n_att = tparm->n_term=0;
   tparm->n_att_indices = tparm->n_datum = tparm->n_data = 0;
   tparm->wts = tparm->att_indices = tparm->datum = NULL;
   tparm->data = NULL;
   tparm->w_j = tparm->ranges = tparm->class_wt = 0.0;
   tparm->disc_scale = tparm->log_pi = tparm->log_att_delta = 0.0;
   tparm->log_delta = tparm->wt_m = tparm->log_marginal = 0.0;
   tparm->tppt=SM;
   p = &(tparm->ptype.sm) ;

   p->val_wts = (float *) malloc(range * sizeof(float));
   for (i=0; i<range; i++)
      p->val_wts[i] = 0.0;
   p->val_probs = (float *) malloc(range * sizeof(float));
   for (i=0; i<range; i++)
      p->val_probs[i] = 0.0;
   p->val_log_probs = (float *) malloc(range * sizeof(float));
   for (i=0; i<range; i++)
      p->val_log_probs[i] = 0.0;
   /* Generate function elements: */
   /* function elements no longer used */

   p->range = range;
   p->inv_range =1.0/p->range;
   p->range_m1 = p->range - 1.0;
   p->range_factor = p->range_m1 / p->range;
   p->gamma_term = (float) log_gamma( (double) p->range_m1, FALSE) -
	       (range * (float) log_gamma( (double) p->range_factor, FALSE) );

   tparm->n_term = n_term;
   tparm->n_att = n_att;
}



/* SINGLE_MULTINOMIAL_LOG_LIKELIHOOD
   20dec94 wmt: return type to double
   
   Calculates the log-probability of the datum's N-att'th attributes' value when
   called within the environment of Log-Likelihood-fn.
   */
double single_multinomial_log_likelihood( tparm_DS tparm)
{
  int n_att = tparm->n_att;
  /*    int n_term = tparm->n_term; */
  float *datum = tparm->datum;
  struct sm_param *sm=&(tparm->ptype.sm);

  return(sm->val_log_probs[(int) datum[n_att]]);
}


/* SINGLE_MULTINOMIAL_UPDATE_L_APPROX
   20dec94 wmt: return type to double
   29mar95 wmt: calculation to double
   
   This calculates log-a<w_j.S_j/H_j.theta_j>_k, the approximate log likelihood
   of observing the weighted statistics given the class hypothesis and current
   parameters, when called within the environment of Update-L-Approx-Fn.
   See Multinomial Likelihood (eqn 21) in math paper
   */
double single_multinomial_update_l_approx( tparm_DS tparm)
{
   struct sm_param *sm = &(tparm->ptype.sm);
   int i, range = sm->range;
/*    int n_term = tparm->n_term; */
   float *log_probs, *wts;
   double log_a;

   wts = sm->val_wts;
   log_probs = sm->val_log_probs;
   log_a = 0.0;
   for (i=0; i<range; i++)
      log_a += (double) (wts[i] * log_probs[i]);
   return( log_a);
}


/* SINGLE_MULTINOMIAL_UPDATE_M_APPROX
   20dec94 wmt: return type to double
   29mar95 wmt: calculation to double

   When called within the environment of Update-M-Approx-fn with constant
   arguments cached by the xxx-Update-M-approx-term-caller, this calculates
   log-a<w_j.S_j/H_j>_k, the approximate the log marginal likelihood of
   observing the weighted statistics given the class hypothesis alone.
   See Marginalized Likelihood (eqn 24) in math paper. 
*/
double single_multinomial_update_m_approx( tparm_DS tparm)
{
   int i;
/*    int n_term = tparm->n_term; */
   struct sm_param *sm = &(tparm->ptype.sm);
   float gamma_term = sm->gamma_term, range_m1 = sm->range_m1;
   float range_factor = sm->range_factor, w_j = tparm->w_j;
   float *val_wts;
   double temp = 0.0;

   val_wts = sm->val_wts;
   for (i=0; i<sm->range; i++)
      temp += log_gamma( (double) (val_wts[i] + range_factor), FALSE);
   return( ((double) gamma_term) - log_gamma( (double) (w_j + range_m1),
                                             FALSE) + temp);
}


/* SINGLE_MULTINOMIAL_UPDATE_PARAMS
   20dec94 wmt: return type to void

   Updates the parameters of a Single-Multinomial term when called from the
   environment of Update-Params-fn.
   */
void single_multinomial_update_params( tparm_DS tparm, int known_parms_p)
{
   int collect = tparm->collect;
   int i, n_att = tparm->n_att, n_data = tparm->n_data;
/*    int n_term = tparm->n_term; */
   struct sm_param *sm=&(tparm->ptype.sm);
   int n_atts = tparm->n_atts, range = sm->range;
   float inv_range = sm->inv_range, wt_i;
   float **data = tparm->data, *wts = tparm->wts, class_wt = tparm->class_wt;
   float disc_scale = tparm->disc_scale, *val_wts, *val_probs, *val_log_probs;

   val_wts = sm->val_wts;
   val_probs = sm->val_probs;
   val_log_probs = sm->val_log_probs;
   if (collect == TRUE) {            /* Collect & regenerate class statistics */
      for (i=0; i<n_atts; i++)
	 val_wts[i] = 0.0;
      if (class_wt > 0.0) {
	 for (i=0; i<n_data; i++) {
	    wt_i = wts[i];
	    if (wt_i > 0.0)
	       val_wts[(int) data[i][n_att]] += wt_i;
	 }
      }
   }
   if (known_parms_p != TRUE)               /* Update class parameters */
      for (i=0; i<range; i++) {
	 val_probs[i] = disc_scale * (inv_range + val_wts[i]);
	 val_log_probs[i] = (float) safe_log((double) val_probs[i]);
      }
   /* return(1.0); */
}

 
int single_multinomial_class_equivalence( tparm_DS tparm1,tparm_DS tparm2,
                                         double percent_ratio)
{
   struct sm_param *sm1 =&( tparm1->ptype.sm);
   struct sm_param *sm2 =&( tparm2->ptype.sm);
   int i, n_atts = tparm1->n_atts;
/*    int n_term = tparm1->n_term; */
   float *v1=sm1->val_probs, *v2=sm2->val_probs;;

   for (i=0; i<n_atts; i++)
      if (percent_equal( (double) v1[i], (double) v2[i], percent_ratio) == FALSE)
	 return(FALSE);
   return(TRUE);
}


/* SINGLE_MULTINOMIAL_CLASS_MERGED_MARGINAL
   20dec94 wmt: return type changed from float to void

   When called within the environment of a Class-Merged-Marginal-Fn, this
   collects sufficient statistics for a Single-Multinomial term in params-m,
   representing the merger of params-0 and params-1. */

void single_multinomial_class_merged_marginal( tparm_DS tparm1,tparm_DS tparm2,tparm_DS tparmm)
{
struct sm_param *sm1 =&( tparm1->ptype.sm);
   struct sm_param *sm2 = &(tparm2->ptype.sm);
   struct sm_param *smm = &(tparmm->ptype.sm);
   int i, range = sm1->range;
/*    int n_term = tparm1->n_term; */
   float *v1=sm1->val_wts, *v2=sm2->val_wts, *vm=smm->val_wts;
   for (i=0; i<range; i++)
      vm[i] = v1[i] + v2[i];
   /* return (vm[i]);   ************/
}
