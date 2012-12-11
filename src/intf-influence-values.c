#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#ifndef _WIN32
#include <sys/param.h>
#endif 
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


/* COMPUTE_INFLUENCE_VALUES
   03feb95 wmt: new

   create & fill arrays of classification attribute influence values
   */
void compute_influence_values( clsf_DS clsf)
{
  int n_atts = clsf->database->n_atts, n_classes = clsf->n_classes, n_class, n_att;
  class_DS class;
  rpt_DS reports = clsf->reports;
  float curr_influence_value, class_influence_value_max, influence_sum,
	global_influence_value_max = 0.0, *influence_sums,
        *all_classes_influence_value_max;
  char *att_type;
  void *influence_struct_DS = NULL, **attribute_array;

  influence_sums = (float *) malloc( n_atts * sizeof( float));
  all_classes_influence_value_max = (float *) malloc( n_atts * sizeof( float));
  for (n_att=0; n_att<n_atts; n_att++) {
    influence_sums[n_att] = 0.0;
    all_classes_influence_value_max[n_att] = 0.0;
  }
  for (n_class=0; n_class<n_classes; n_class++) {
    class = clsf->classes[n_class];
    if (class->w_j >= clsf->min_class_wt) {
      attribute_array = (void **) malloc( n_atts * sizeof( void *));
      influence_sum = 0.0;
      class_influence_value_max = 0.0;
      for (n_att=0; n_att<n_atts; n_att++) {
        att_type = report_att_type( clsf, n_class, n_att);
        if ((eqstring( att_type, "discrete") == TRUE) ||
            (eqstring( att_type, "integer") == TRUE) ||
            (eqstring( att_type, "real") == TRUE)) {

          curr_influence_value = (float) influence_value( n_class, n_att, clsf, att_type,
                                                         &influence_struct_DS);
          if (eqstring( att_type, "discrete") == TRUE) 
            attribute_array[n_att] = (i_discrete_DS) influence_struct_DS;
          if (eqstring( att_type, "integer") == TRUE) 
            attribute_array[n_att] = (i_integer_DS) influence_struct_DS;
          if (eqstring( att_type, "real") == TRUE) 
            attribute_array[n_att] = (i_real_DS) influence_struct_DS;       
        }
        else if (eqstring( att_type, "ignore") == TRUE)
          curr_influence_value = 0.0;
        else {
          fprintf( stderr, "ERROR: attribute type %s not handled\n", att_type);
          abort();
        }
        influence_sum += curr_influence_value;
	influence_sums[n_att] += curr_influence_value;
        if (curr_influence_value > all_classes_influence_value_max[n_att])
          all_classes_influence_value_max[n_att] = curr_influence_value;
        if (curr_influence_value > class_influence_value_max)
          class_influence_value_max = curr_influence_value;
        if (curr_influence_value > global_influence_value_max)
          global_influence_value_max = curr_influence_value;
      }
      class->max_i_value = class_influence_value_max;
      class->i_values = attribute_array;
      class->i_sum = influence_sum;
      /* printf("n_class %d, class_influence_value_max %f, influence_sum %f\n",
             n_class, class_influence_value_max, influence_sum); */
    }
  }
  reports->max_i_value = global_influence_value_max;
  reports->att_max_i_values = all_classes_influence_value_max;
  reports->att_i_sums = influence_sums;
  reports->att_max_i_sum = 0.0;
  for (n_att=0; n_att<n_atts; n_att++)
    if (influence_sums[n_att] > reports->att_max_i_sum)
      reports->att_max_i_sum = influence_sums[n_att];
  /* printf( "max_i_value %f, att_max_i_sum %f\n", reports->max_i_value,
         reports->att_max_i_sum);
  for (n_att=0; n_att<n_atts; n_att++) 
    printf( "n_att %d, att_max_i_values %f, att_i_sums %f\n",
           n_att, reports->att_max_i_values[n_att], reports->att_i_sums[n_att]); */
}


/* INFLUENCE_VALUE
   06feb95 wmt: new
   27apr97 wmt: do not process attribute values which have null translations.
                this occurs when user supplies excessive an excessive range
                value in .hd2, and ignores warning to correct it.

   Compute influence value for real or discrete attribute
   */
double influence_value( int n_class, int n_att, clsf_DS clsf, char *att_type,
                       void **influence_struct_DS_ptr)
{
  class_DS class = NULL;
  int term_index, n_att_prob_list, n_term_att_list = 0;  
  tparm_DS term_params;
  float influence_value, *class_div_global_att_prob_list = NULL;
  float class_mean, class_sigma, class_known_prob, global_mean, global_sigma;
  float global_known_prob, *term_att_list = NULL, **class_covar = NULL;
  i_discrete_DS i_discrete_struct;
  i_real_DS i_real_struct;
  att_DS att;
  /* int i; */

  term_index = find_attribute_modeling_class( clsf, n_class, n_att, &class);

  if (class == NULL) {
    *influence_struct_DS_ptr = NULL; 
    return (0.0);
  }
  else {
    term_params = class->tparms[term_index];
    switch(term_params->tppt) {
    case  MN_CN:
      mn_cn_params_influence_fn( class->model, term_params, term_index, n_att,
                                &influence_value, &class_mean, &class_sigma,
                                &global_mean, &global_sigma, &term_att_list,
                                &n_term_att_list, &class_covar);
      break;
    case  SM:
      sm_params_influence_fn( class->model, term_params, term_index, n_att,
                             &influence_value, &class_div_global_att_prob_list,
                             &n_att_prob_list);
      break;
    case  SN_CM:
      sn_cm_params_influence_fn( class->model, term_params, term_index, n_att,
                                &influence_value, &class_mean, &class_sigma,
                                &class_known_prob, &global_mean, &global_sigma,
                                &global_known_prob);
      break;
    case  SN_CN:
      sn_cn_params_influence_fn( class->model, term_params, term_index, n_att,
                                &influence_value, &class_mean, &class_sigma,
                                &global_mean, &global_sigma);
      break;

    case MM_D: 
    case MM_S:
    default:
      fprintf(stderr, "ERROR: unknown type of ENUM MODEL_TYPES in influence_value: %d\n",
              term_params->tppt);
      abort();
    }
    /* Use the magnitude of the cross entropy value */
    influence_value = (float) fabs( (double) influence_value);
    if (eqstring( att_type, "discrete") == TRUE) {
      i_discrete_struct = (i_discrete_DS) malloc( sizeof( struct i_discrete));
      i_discrete_struct->influence_value = influence_value;
      att = clsf->database->att_info[n_att];
      if (n_att_prob_list > att->n_trans)
        n_att_prob_list = att->n_trans;
      i_discrete_struct->n_p_p_star_list = 3 * n_att_prob_list;
      i_discrete_struct->p_p_star_list = class_div_global_att_prob_list;
      *influence_struct_DS_ptr = (void *) i_discrete_struct;
      /* printf( "n_class %d, n_att %d, influence_value %f n_att_prob_list %d\n",
             n_class, n_att, influence_value, n_att_prob_list);
      for (i=0;i<n_att_prob_list; i++)
        printf( "index %f local %f global %f\n",
               class_div_global_att_prob_list[i*3],
               class_div_global_att_prob_list[(i*3)+1],
               class_div_global_att_prob_list[(i*3)+2]); */       
    }
    else if (eqstring( att_type, "integer") == TRUE) {
      abort();
      /* 		     (make_i_integer :value influence_value */
      /* 				      :mean_sigma_list influence_parameters))) */
    }
    else if (eqstring( att_type, "real") == TRUE) {
      i_real_struct = (i_real_DS) malloc( sizeof( struct i_real));
      i_real_struct->influence_value = influence_value;
      i_real_struct->mean_sigma_list =
        (float *) malloc( ((term_params->tppt == SN_CM) ? 6 : 4) * sizeof( float));
      i_real_struct->n_mean_sigma_list = (term_params->tppt == SN_CM) ? 6 : 4;
      i_real_struct->mean_sigma_list[0] = class_mean;
      i_real_struct->mean_sigma_list[1] = class_sigma;
      i_real_struct->mean_sigma_list[2] = global_mean;
      i_real_struct->mean_sigma_list[3] = global_sigma;
      if (term_params->tppt == SN_CM) {
        i_real_struct->mean_sigma_list[4] = class_known_prob;
        i_real_struct->mean_sigma_list[5] = global_known_prob;
      }
      i_real_struct->n_term_att_list = n_term_att_list;
      i_real_struct->class_covar = class_covar;
      i_real_struct->term_att_list = term_att_list; 
      *influence_struct_DS_ptr = (void *) i_real_struct;

    }
    else {
      fprintf( stderr, "influence_value called with unknown attribute_type: %s\n",
              att_type);
      abort(); 
    }
  }
  return (influence_value);
}


/* FIND_ATTRIBUTE_MODELING_CLASS
   06feb95 wmt: new

   attribute model is in the class
   */
int find_attribute_modeling_class( clsf_DS clsf, int n_class, int n_att, class_DS *class_ptr)
{
  class_DS class;
  
  class = clsf->classes[n_class];
  *class_ptr = class;
  return ( atoi( class->model->att_locs[n_att]));
}
