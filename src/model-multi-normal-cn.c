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

 
/* MN_CN_PARAMS_INFLUENCE_FN
   23feb95 wmt: new (from ac-x)

   Compute the influence value of Multi Normal model term with no missing values.
    13 Dec 91 JCS: CURRENTLY UNDER DEVELOPMENT: This version finds the influence value for 
    the term rather then the specified attribute.  
    It is not clear how to do the later, since the single attribute's parameters are mixed 
    with those of all the other attributes through the two covarience terms.  An attribute is 
    mathmatically seperable IFF we have zeros in the off diagonal elements of the corresponding 
    row and column of both covariance matrices. This is not normally expected, particularly not 
    for all classes of a classification.  In short, a covariantly modeled attribute only rarly 
    has any independent influence upon a class, let alone the classification.  To ask what that
    independent influence is, is probably not a usefull question.
    The current solution is to return, for each attribute, the term cross entropy 
    divided by the number of attributes modeled in the term.  This will allow entorpy 
    sums to add up correctly, but we really do need to consider returning model term 
    information when multiple attribute terms are present.
    6 May 93 JCS: Return the individual attribute mean and standard deviation w.r.t. self as
    is done for single normal model terms.  Add the full covariance matrix as a third value .
    Note that since all attributes in a multiple term have common influence, they are printed 
    together in the influence report.  The covariance matrix can then follow them to show their
    interaction.
   */
void mn_cn_params_influence_fn( model_DS model, tparm_DS tparm, int term_index, int n_att,
                               float *v_ptr, float *class_mean_ptr, float *class_sigma_ptr,
                               float *global_mean_ptr, float *global_sigma_ptr,
                               float **term_att_list_ptr, int *n_term_att_list_ptr,
                               float ***class_covar_ptr)   
{ 
  struct mn_cn_param *param;
  tparm_DS *p;
  int i, term_att_index;
  float *delta_means, **global_inv_covar;

  param = &(tparm->ptype.mn_cn);
  p = model_global_tparms( model);
  *class_covar_ptr = param->covariance;
  *n_term_att_list_ptr = tparm->n_att_indices;
  *term_att_list_ptr = tparm->att_indices;

  for (i=0; i<*n_term_att_list_ptr; i++)
    if ((*term_att_list_ptr)[i] == n_att)
      break;
  term_att_index = i;

  *global_mean_ptr = p[term_index]->ptype.mn_cn.means[term_att_index];
  *global_sigma_ptr = (float) sqrt( (double) p[term_index]->
                                   ptype.mn_cn.covariance[term_att_index][term_att_index]);
  global_inv_covar = p[term_index]->ptype.mn_cn.temp_m;
  delta_means = p[term_index]->ptype.mn_cn.temp_v;

  invert_factored_square_matrix( p[term_index]->ptype.mn_cn.factor, global_inv_covar,
                                *n_term_att_list_ptr);
  setf_v_v( delta_means, param->means, *n_term_att_list_ptr);

  decf_v_v( delta_means, p[term_index]->ptype.mn_cn.means, *n_term_att_list_ptr);

  *v_ptr = (((param->ln_root - p[term_index]->ptype.mn_cn.ln_root) +
             (0.5 * (
                     (float) star_vmv( global_inv_covar, delta_means, *n_term_att_list_ptr) +
                
                     (float) trace_star_mm( *class_covar_ptr, global_inv_covar,
                                           *n_term_att_list_ptr) - 
                     *n_term_att_list_ptr)))
            / *n_term_att_list_ptr);

  *class_mean_ptr = param->means[term_att_index];
  *class_sigma_ptr =
    (float) sqrt( (double) (*class_covar_ptr)[term_att_index][term_att_index]);
} 


/* MAKE_MN_CN_PARAM
   21nov94 wmt: initialize all slots in tparm
   23dec94 wmt: initialize vectors and arrays
   12nov97 jcs: sorted to reflect ordering in new_term_params definition
   */
tparm_DS make_mn_cn_param( int n_atts) 
{
  int i, j;
  tparm_DS tparm;
  struct mn_cn_param *mn_cn;

  tparm = (tparm_DS) malloc(sizeof(struct new_term_params));

  tparm->n_atts  = n_atts;
  tparm->tppt = MN_CN;
  /* tparam->ptype.mn_cn  substructure initialized below */
  tparm->collect = FALSE;
  tparm->n_term = 0; 
  tparm->n_att = 0;
  tparm->n_att_indices = n_atts;
  tparm->n_datum = 0;
  tparm->n_data = 0;
  tparm->w_j = 0.0;
  tparm->ranges = 0.0;
  tparm->class_wt = 0.0;
  tparm->disc_scale = 0.0;
  tparm->log_pi = 0.0;
  tparm->log_att_delta = 0.0;
  tparm->log_delta = 0.0;
  tparm->wts = NULL;            /* (float *) */
  tparm->datum = NULL;          /* (float *) */
  tparm->att_indices = NULL;    /* (float *) */
  tparm->data = NULL;           /* (float **) */
  tparm->wt_m = 0.0;
  tparm->log_marginal = 0.0;
  
  /* initializing the tparm->ptype.mn_cn substructure */

  mn_cn = &( tparm->ptype.mn_cn);

  mn_cn->ln_root = 0.0;
  mn_cn->log_ranges = 0.0;

  mn_cn->emp_means = (float *) malloc(n_atts * sizeof(float));
  for (i=0; i<n_atts; i++)
    mn_cn->emp_means[i] = 0.0;

  mn_cn->emp_covar = (fptr *) malloc(n_atts * sizeof(fptr));
  for (i=0; i<n_atts; i++) {
    mn_cn->emp_covar[i] = (float *) malloc(n_atts * sizeof(float));
    for (j=0; j<n_atts; j++)
      mn_cn->emp_covar[i][j] = 0.0;
  }
  mn_cn->means = (float *) malloc(n_atts * sizeof(float));
  for (i=0; i<n_atts; i++)
    mn_cn->means[i] = 0.0;

  mn_cn->covariance = (fptr *) malloc(n_atts * sizeof(fptr));
  for (i=0; i<n_atts; i++) {
    mn_cn->covariance[i] = (float *) malloc(n_atts * sizeof(float));
    for (j=0; j<n_atts; j++)
      mn_cn->covariance[i][j] = 0.0;
  }
  mn_cn->factor = (fptr *) malloc(n_atts * sizeof(fptr));
  for (i=0; i<n_atts; i++) {
    mn_cn->factor[i] = (float *) malloc(n_atts * sizeof(float));
    for (j=0; j<n_atts; j++)
      mn_cn->factor[i][j] = 0.0;
  }
  mn_cn->values = (float *) malloc(n_atts * sizeof(float));
  for (i=0; i<n_atts; i++)
    mn_cn->values[i] = 0.0;

 
  mn_cn->temp_v = (float *) malloc(n_atts * sizeof(float));
  for (i=0; i<n_atts; i++)
    mn_cn->temp_v[i] = 0.0;

  mn_cn->temp_m = (fptr *) malloc(n_atts * sizeof(fptr));
  for (i=0; i<n_atts; i++) {
    mn_cn->temp_m[i] = (float *) malloc(n_atts * sizeof(float));
    for (j=0; j<n_atts; j++)
      mn_cn->temp_m[i][j] = 0.0;   
  }
  mn_cn->min_sigma_2s = NULL;   /* (float *) set in multi_normal_cn_model_term_builder*/

return(tparm);
}


/* MULTI_NORMAL_CN_MODEL_TERM_BUILDER
   30jul95 wmt: change log calls to safe_log to prevent "log: SING error"
   		error messages.

   Funcalled from Expand-Model-Terms.  This constructs parameter, prior, and
   intermediate results structures appropriate to a single-normal likelihood
   term, and places them in the model.  Constructs corresponding log-likelihood
   and parameter update function elements and saves them on the model for later
   compilation.
   */
void multi_normal_cn_model_term_builder( model_DS model, term_DS term, int n_term)
   /* model_DS model;         The model-DS to which this term will contribute. */
   /* term_DS term;           The singleton term-DS definig this attributes use. */
   /* int n_term;            The term index for various model-DS substructures. */
{
   int i, n_atts;
   float *errors, log_att_delta, log_ranges, log_delta_div_root_2pi_k, mn;
   float *att_indices;
   att_DS att;
   database_DS data_base;
   real_stats_DS stats;
   tparm_DS tparm;
   struct mn_cn_param *mn_cn;

   n_atts = term->n_atts;
   if (n_atts <= 1) {
      fprintf(stderr, "multi_normal_cn: attempt to apply to non-multiple set\n");
      exit(1);
    }

   att_indices = term->att_list;           /* Attribute indices for this term */
   data_base = model->database;

   errors = (float *) malloc(n_atts * sizeof(float)); /* not freed used for min_sigma_2s */
   
   for (i=0; i<n_atts; i++){
      att = data_base->att_info[(int) att_indices[i]];
      errors[i] = att->error;
   }
	 
   log_att_delta = 0.0;
   for (i=0; i<n_atts; i++)
      log_att_delta += (float) safe_log((double) errors[i]);


   log_ranges = 0.0;
   for (i=0; i<n_atts; i++) {
      att = data_base->att_info[(int) att_indices[i]];
      stats = att->r_statistics;
      log_ranges += (float) safe_log((double) (stats->mx - stats->mn));
   }

   log_delta_div_root_2pi_k = log_att_delta + (n_atts * LN_1_DIV_ROOT_2PI);

					       /* Allocate parameters struct. */
   term->tparm = tparm = make_mn_cn_param(n_atts);
   mn_cn=&(tparm->ptype.mn_cn);

/*******strcpy(model->priors[n_term], "implicit-prior");*/

   /*   JCS 2/98  Change minimum estimated sigma limiter to 
        ~sqrt(LEAST_POSITIVE_SINGLE_FLOAT).  This is still a hack, and not a 
        satisfactory prior, but it eliminated the catastrophic cycling found 
        with large n_atts and properly enforces the underflow constraint. 
   mn = safe_exp( 1.0 + LEAST_POSITIVE_SINGLE_LOG /  n_atts);  */
   mn = safe_exp( LEAST_POSITIVE_SINGLE_LOG / 2);
   for (i=0; i<n_atts; i++) {
      errors [i] =  max(errors[i] * errors[i], mn);
   }

   tparm->n_term = n_term;
   tparm->att_indices = att_indices;
   tparm->log_delta = log_delta_div_root_2pi_k;
   tparm->log_att_delta = log_att_delta;
   tparm->n_atts = n_atts;
   mn_cn->log_ranges = log_ranges;
   mn_cn->min_sigma_2s = errors;  
}


/* MULTI_NORMAL_CN_LOG_LIKELIHOOD
   20dec94 wmt: return type to double

   When called within the environment of Log-Likelihood-fn, this calculates the
   probability of a Multi-normal-cn term over the indexed attributes. */

double multi_normal_cn_log_likelihood( tparm_DS tparm)
{
  struct mn_cn_param *mn_cn =&(tparm->ptype.mn_cn);
  float *att_indices = tparm->att_indices, log_delta = tparm->log_delta;
  float *datum = tparm->datum, *values, *temp_v, temp;

  values = mn_cn->values;
  temp_v = mn_cn->temp_v;
  /* sets values to the indexed data values */
  collect_indexed_values(values, att_indices, datum,tparm->n_atts);
  /* subtracts the means from the values */
  decf_v_v(values, mn_cn->means, tparm->n_atts);
  /*print_vector_f(values,tparm->n_atts,"values in log_likelihood"); dbg*/
  setf_v_v(temp_v, values, tparm->n_atts);
  /*print_vector_f(temp_v,tparm->n_atts,"temp_v in log_likelihood"); dbg*/
  /*print_matrix_f(mn_cn->factor,tparm->n_atts,tparm->n_atts,"factor matrix"); dbg*/
  solve(mn_cn->factor,temp_v,tparm->n_atts),
  /*print_vector_f(temp_v,tparm->n_atts,"temp_v after solve in log_likelihood"); dbg*/
  /*printf(" dotvv =%f\n", dot_vv(values,temp_v,tparm->n_atts)); dbg*/

  temp = log_delta + mn_cn->ln_root +
    (-0.5 * (float) dot_vv( values, temp_v, tparm->n_atts)); 
  return (temp);
}


/* MULTI_NORMAL_CN_UPDATE_L_APPROX
   20dec94 wmt: return type to double

   When called within the environment of Update-L-Approx-fn, this calculates
   the approximate log likelihood term (in log-a<w_j.S_j/H_j.theta_j>) for
   observing the weighted statistics given the class hypothesis and current
   parameters.
   */
double multi_normal_cn_update_l_approx( tparm_DS tparm)
{
  struct mn_cn_param *mn_cn =&(tparm->ptype.mn_cn);
  float log_delta = tparm->log_delta, w_j = tparm->w_j, t1;

  t1 = -0.5 * (float) dot_mm(mn_cn->emp_covar,
                             invert_factored_square_matrix(mn_cn->factor,
                                                           mn_cn->temp_m,
                                                           tparm->n_atts),
                             tparm->n_atts);
  /* Contribution from empherical mean: zero when E<mean> = emp-mean */
  t1 = (w_j * (log_delta + mn_cn->ln_root + t1));
  return (t1);
}


/* MULTI_NORMAL_CN_UPDATE_M_APPROX
   20dec94 wmt: return type to double

   When called within the environment of Update-M-Approx-fn, this calculates
   the approximate log marginal likelihood log-a<w_j.S_j/H_j>_k of observing
   the weighted statistics given the class hypothesis alone.
   */
double multi_normal_cn_update_m_approx( tparm_DS tparm)

{
  struct mn_cn_param *mn_cn = &(tparm->ptype.mn_cn);
  int k, n_atts = tparm->n_atts;
  float log_att_delta = tparm->log_att_delta, log_ranges = mn_cn->log_ranges;
  float w_j = tparm->w_j, h_cb, temp, wh, h1, a;
  fptr *g_matrix, *emp_covar;
  double sum;

  emp_covar = mn_cn->emp_covar; 
  g_matrix = mn_cn->temp_m;
  h_cb = n_atts;
  extract_diagonal_matrix(emp_covar, g_matrix, n_atts);
  
  h1 = h_cb +1;
  wh = w_j + h_cb;
  sum = 0.0;

  for (k=0; k<n_atts; k++) {
    a = k + 1;
    sum += log_gamma( (double) ((wh - a) / 2.0), FALSE) -
      log_gamma( (double) ((h1 - a) / 2.0), FALSE);
  }  
  temp  = 
    + (-0.5 * n_atts * (float) safe_log((double) w_j)) 
      + (-0.5 * n_atts * (w_j - 1.0) * LN_SINGLE_PI)  
        + (float) sum 
          + (0.5 * h_cb * (float) safe_log( diagonal_product(g_matrix, n_atts))) 
            /* lisp has some commented code here*/
            + ((w_j + h_cb - 1.0) *
               (mn_cn->ln_root + (-0.5 * n_atts * (float) safe_log( (double) (w_j - 2.0)))))
              + (w_j * log_att_delta)  
                + (- log_ranges);
  return (temp);
}


/* MULTI_NORMAL_CN_UPDATE_PARAMS
   20dec94 wmt: return type to void
   08mar95 wmt: det to double
   */
void multi_normal_cn_update_params( tparm_DS tparm, int known_params_p)
{
   struct mn_cn_param *mn_cn=&(tparm->ptype.mn_cn);
   int collect = tparm->collect;
   int n_data = tparm->n_data;
   float *att_indices = tparm->att_indices;
   float **data = tparm->data, *wts = tparm->wts, class_wt = tparm->class_wt;
   float *emp_means = mn_cn->emp_means, 
	*means = mn_cn->means,
   	*min_sigma_2s = mn_cn->min_sigma_2s,
   	*values = mn_cn->values;
   fptr *emp_covar = mn_cn->emp_covar, 
	*covar = mn_cn->covariance,
	*factor = mn_cn->factor, 
	*temp_m = mn_cn->temp_m;
   float scale;
   double det;

   if (class_wt > 0.0) {                 /* Zero class-wt implies null class. */
      if (collect == TRUE) {         /* Collect & regenerate class statistics */
	             /* If not collect?, we proceed with the previous values. */
                             /* Updates emp-means & emp-covar, alters values: */
	 update_means_and_covariance(data, n_data, att_indices, wts, means,
	    emp_means, emp_covar, values, tparm->n_atts);
	 /* Do we want to do any limiting on the means or covariance? Following
	    limits the covariance diagonals to square of attribute error: */
	 limit_min_diagonal_values(emp_covar, min_sigma_2s, tparm->n_atts); 
      }
   }
   if (known_params_p != TRUE) {             /* Update class parameters */
      setf_v_v(means, emp_means,tparm->n_atts);/* Estimated value of means is the emp_means */
      scale = 1.0 / (class_wt - 2.0);
/*printf(" scale=%f\n",scale); dbg*/

/*    print_matrix_f(emp_covar,tparm->n_atts,tparm->n_atts," in update p emp covar");  */

      setf_m_ms(covar, emp_covar, (double) (class_wt * scale), tparm->n_atts);
/*print_matrix_f(covar,tparm->n_atts,tparm->n_atts," in update p covar"); dbg*/

      extract_diagonal_matrix(emp_covar, temp_m, tparm->n_atts);
/*print_matrix_f(temp_m,tparm->n_atts,tparm->n_atts," in update p diag ele of emp covar"); dbg*/

      incf_m_ms(covar,temp_m, (double) scale, tparm->n_atts);
/*print_matrix_f(covar,tparm->n_atts,tparm->n_atts," in update p scaled covar"); dbg*/

      copy_to_matrix(covar, factor, tparm->n_atts);
/*print_matrix_f(factor,tparm->n_atts,tparm->n_atts," in update p factor"); dbg*/

      compute_factor(factor, tparm->n_atts);
/*print_matrix_f(factor,tparm->n_atts,tparm->n_atts," in update p factored factor "); dbg*/

      det = determinent_f(factor, tparm->n_atts);

      mn_cn->ln_root = -0.5 *
		( (det > 0.0) ? (float) safe_log( det) : LEAST_POSITIVE_SINGLE_LOG);
/*printf(" det=%f ln_root=%f\n",det,mn_cn->ln_root); dbg*/
   }
   /* return(class_wt); */
}


/* MULTI_NORMAL_CN_CLASS_EQUIVALENCE
   27feb95 wmt: free mallocs from vector_root_diagonal_matrix

   When called within the environment of class-equivalence-fn, this tests for
   equivalence of the  parameters.
   */
int multi_normal_cn_class_equivalence( tparm_DS tparm1, tparm_DS tparm2,
                                      double sigma_ratio)
{
  int c, r, n_atts = tparm1->n_atts;
  float *m1, *m2, *s1, *s2, *col1, *col2, sigma1, sigma2, covar1, covar2;
  fptr *c1, *c2;
  struct mn_cn_param *mn_cn1, *mn_cn2;

  mn_cn1 = &(tparm1->ptype.mn_cn);     mn_cn2 =&( tparm2->ptype.mn_cn);
  m1 = mn_cn1->means;              m2 = mn_cn2->means;
  c1 = mn_cn1->covariance;         c2 = mn_cn2->covariance;
  s1 = vector_root_diagonal_matrix(c1, tparm1->n_atts);
  s2 = vector_root_diagonal_matrix(c2, tparm2->n_atts);
  for (c=0; c<n_atts; c++) {
    col1 = c1[c];   col2 = c2[c];
    sigma1 = s1[c];         sigma2 = s2[c];
    if ((m1[c] - m2[c]) >= (float) (sigma_ratio * (double) min(sigma1, sigma2)))
      return(FALSE);
    /* Comparing diagonal covariances */
    if (percent_equal( (double) sigma1, (double) sigma2, (double) 1.0) == FALSE)
      return(FALSE);
    /* Comparing off diagonal covarainces */
    for (r=c+1; r<n_atts; r++) {
      covar1 = col1[r];
      covar2 = col2[r];
      if ((covar1 != covar2) &&
          (percent_equal( (double) ((covar1 / sigma1) / s1[r]),
                         (double) ((covar2 / sigma2) / s2[r]), 1.0) == FALSE)) {
        free( s1);
        free( s2);
        return(FALSE);
      }
    }
  }
  free( s1);
  free( s2);
  return(TRUE);
}


/* MULTI_NORMAL_CN_CLASS_MERGED_MARGINAL -
   modified 22oct94 wmt: add args float wt_0, float wt_1, float wt_m
   20dec94 wmt: return type to void

   When called within the environment of Merged-Marginal-fn, this generates the
   sufficient statistics of a Multi-Normal-cn term equivalent to the weighted
   merging of params-0 and params-1, storing same in params-m. */
void multi_normal_cn_class_merged_marginal( tparm_DS tparm0, tparm_DS tparm1, tparm_DS tparmm,
                                            float wt_0, float wt_1, float wt_m)
{
   float *em0, *em1, *emm;
   fptr *ecovarm;
   struct mn_cn_param *mn_cn0, *mn_cn1, *mn_cnm;

   mn_cn0 = &(tparm0->ptype.mn_cn);       mn_cn1 = &(tparm1->ptype.mn_cn);
   mn_cnm = &(tparmm->ptype.mn_cn);

   em0 = mn_cn0->emp_means;  em1 = mn_cn1->emp_means;
   emm = mn_cnm->emp_means;

   ecovarm = mn_cnm->emp_covar;
			             /* Calculate the merged empherical mean: */
   setf_v_vs(emm, em0, (double) wt_0, tparm0->n_atts);
   incf_v_vs(emm, em1, (double) wt_1, tparm0->n_atts);
   n_sv( (double) (wt_m / 1.0), emm, tparm0->n_atts);
	                       /* Calculate the merged empherical covariance:
	                         W(S + m*m) = W0(S0 + m0*m0) + W1(S1 + m1*m1) */
   setf_m_ms(ecovarm, mn_cn0->emp_covar, (double) wt_0, tparm0->n_atts);
   incf_m_ms(ecovarm, mn_cn1->emp_covar, (double) wt_1, tparm0->n_atts);
   incf_m_vvs(ecovarm, em0, em0, (double) wt_0, tparm0->n_atts);
   incf_m_vvs(ecovarm, em1, em1, (double) wt_1, tparm0->n_atts);
   n_sm( (double) (1.0 / wt_m), ecovarm, tparm0->n_atts);
   incf_m_vvs(ecovarm, emm, emm, (double) -1.0, tparm0->n_atts);

   /* return(wt_0); */
}
