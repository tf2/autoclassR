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


/* INITIALIZE_REPORTS_FROM_RESULTS_PATHNAME
   02feb95 wmt: new
   17feb95 wmt: add clsf_n_list to get_clsf_seq arg list
   26may95 wmt: added prediction_p

   Get classifications from results_file. Add reports strucuture
   Create global classification and compute influence values.
   Returns clsf list
   */
clsf_DS *initialize_reports_from_results_pathname( char *results_file_ptr,
                                                  int_list clsf_n_list,
                                                  int *num_clsfs_found_ptr,
                                                  int prediction_p)
{
  clsf_DS *clsf_list = NULL, *clsf_seq, clsf;
  int expand_p = TRUE, want_wts_p = TRUE, update_wts_p = TRUE;
  int n_best_clsfs, i_clsf, i;
  shortstr file_type = "results";

  if (prediction_p == TRUE)
    expand_p = want_wts_p = update_wts_p = FALSE;

  clsf_seq = get_clsf_seq( results_file_ptr, expand_p, want_wts_p, update_wts_p,
                          file_type, &n_best_clsfs, clsf_n_list);

  for (i=0; clsf_n_list[i] != END_OF_INT_LIST; i++)
    if (clsf_n_list[i] > n_best_clsfs)
      fprintf( stdout, "\nWARNING: requested clsf number %d not found -- max number "
              "is %d\n\n", clsf_n_list[i], n_best_clsfs);

  *num_clsfs_found_ptr = 0;
  for (i_clsf=0; i_clsf<n_best_clsfs; i_clsf++) {
    if (member_int_list( i_clsf + 1, clsf_n_list) == TRUE) {
      (*num_clsfs_found_ptr)++;
      if (clsf_list == NULL)
        clsf_list = (clsf_DS *) malloc( (*num_clsfs_found_ptr) * sizeof( clsf_DS));
      else
        clsf_list = (clsf_DS *) realloc( clsf_list, (*num_clsfs_found_ptr) *
                                        sizeof( clsf_DS));
      clsf = clsf_seq[i_clsf];

      clsf_list[(*num_clsfs_found_ptr) - 1] =
        init_clsf_for_reports( clsf, prediction_p);

      strcpy( clsf->reports->current_results, results_file_ptr);
    }
    else
      free_clsf_DS( clsf_seq[i_clsf]);
  }
  return (clsf_list);
}


/* INIT_CLSF_FOR_REPORTS
   02feb95 wmt: new
   26may95 wmt: added prediction_p

   do expand_clsf in initialize_reports_from_results_pathname because of
   peculiarities of the expansion code
   */
clsf_DS init_clsf_for_reports( clsf_DS clsf, int prediction_p)
{
  int n_classes;
  float *class_strength_list = NULL, max_strength = MOST_NEGATIVE_SINGLE_FLOAT;
  int i_class;
  class_DS *classes;

  n_classes = clsf->n_classes;
  classes = clsf->classes;
  if (prediction_p == FALSE) {
    for (i_class=0; i_class<n_classes; i_class++) {
      if (class_strength_list == NULL)
        class_strength_list = (float *) malloc( (i_class + 1) * sizeof( float));
      else
        class_strength_list = (float *) realloc( class_strength_list, (i_class + 1) *
                                                sizeof( float));
      update_l_approx_fn( classes[i_class]);
 
      class_strength_list[i_class] = (float) class_strength_measure( classes[i_class]);

      if (class_strength_list[i_class] > max_strength)
        max_strength = class_strength_list[i_class];
    }
  }
  /* allocate reports struct */
  clsf->reports = (rpt_DS) malloc( sizeof( struct reports));
  if (prediction_p != TRUE) {
    clsf->reports->att_model_term_types = get_attribute_model_term_types( clsf);
    clsf->reports->max_class_strength = max_strength;
    clsf->reports->class_strength = class_strength_list;
  }
  else {
    clsf->reports->att_model_term_types = NULL;
    clsf->reports->max_class_strength = 0.0;
    clsf->reports->class_strength = NULL;
  }
  clsf->reports->n_class_wt_ordering = n_classes;
  clsf->reports->class_wt_ordering = get_class_weight_ordering( clsf);
  clsf->reports->datum_class_assignment = NULL;
  clsf->reports->att_i_sums = NULL;
  clsf->reports->att_max_i_sum = 0.0;
  clsf->reports->att_max_i_values = NULL;
  clsf->reports->max_i_value = 0.0;

  if (prediction_p != TRUE)
    compute_influence_values( clsf);

  return (clsf);
}


/* GET_CLASS_WEIGHT_ORDERING
   03feb95 wmt: new

   compute ordering of classes by class membership (weight)
   */
int *get_class_weight_ordering( clsf_DS clsf)
{
  int i_class, *class_weight_ordering;
  sort_cell_DS sort_list, temp_sort_list;
  int (* comp_func) () = float_sort_cell_compare_gtr;

  sort_list = (sort_cell_DS) malloc( clsf->n_classes * sizeof( struct sort_cell));
  class_weight_ordering = (int *) malloc( clsf->n_classes * sizeof( int));
  temp_sort_list = sort_list;
  for (i_class=0; i_class<clsf->n_classes; i_class++) {
    temp_sort_list->float_value = clsf->classes[i_class]->w_j;
    temp_sort_list->int_value = i_class;
    /* printf ("before: order-index %d class-index %d class-weight %f\n", i_class, */
    /*         temp_sort_list->int_value, temp_sort_list->float_value); */
    temp_sort_list++;
  }
  qsort( (char *) sort_list, clsf->n_classes, sizeof( struct sort_cell), comp_func);

  temp_sort_list = sort_list;
  for (i_class=0; i_class<clsf->n_classes; i_class++) {
    class_weight_ordering[i_class] = temp_sort_list->int_value;
    /* printf ("after: order-index %d class-index %d class-weight %f\n", i_class, */
    /*         temp_sort_list->int_value, temp_sort_list->float_value); */
    temp_sort_list++;
  }

  free( sort_list);
  return (class_weight_ordering);
}


/* GET_ATTRIBUTE_MODEL_TERM_TYPES
   03feb95 wmt: new
   
   Build array of classes by attributes containing attribute model term types
   */
char ***get_attribute_model_term_types( clsf_DS clsf)
{
  char ***model_term_type_array, **att_model_term_type_array;
  int n_classes = clsf->n_classes, n_atts = clsf->database->n_atts;
  int i_class, i_att, term_index, integer_p;
  model_DS model;

  model_term_type_array = (char ***) malloc( n_classes * sizeof( char **));
  for (i_class=0; i_class<n_classes; i_class++) {
    att_model_term_type_array = ( char **) malloc( n_atts * sizeof( char *));
    model = clsf->classes[i_class]->model;
    for (i_att=0; i_att<n_atts; i_att++) {
      integer_p = TRUE;
      term_index = atoi_p( model->att_locs[i_att], &integer_p);
      att_model_term_type_array[i_att] = ( char *) malloc( sizeof( shortstr));
      strcpy( att_model_term_type_array[i_att], integer_p ?
             model->terms[term_index]->type : "ignore");
      /* printf( "i_class %d i_att %d model_term_type %s\n", i_class, i_att, */
      /*         att_model_term_type_array[i_att]); */
    }
    model_term_type_array[i_class] = att_model_term_type_array;
  }
  return (model_term_type_array);
}
 

/*  REPORT_ATT_TYPE
    06feb95 wmt: new

    attribute type, with models applied, of attribute n-att in class n-class
    (clsf numbering), e.g. "real"
    */
char *report_att_type( clsf_DS clsf, int n_class, int n_att)
{
  char * att_type;
  
  att_type = clsf_att_type( clsf, n_att);
  if (eqstring( rpt_att_model_term_type( clsf, n_class, n_att), "ignore") == TRUE)
    att_type = "ignore";
  return (att_type);
}


/* RPT_ATT_MODEL_TERM_TYPE
   06feb95 wmt: new

   attribute model term type stringof attribute n_att 
   in class n_class (clsf numbering), e.g. "single_normal_cn"
   */
char *rpt_att_model_term_type( clsf_DS clsf, int n_class, int n_att)
{

  return (clsf->reports->att_model_term_types[n_class][n_att]);
}
  

/* GET_MODELS_SOURCE_INFO
   07feb95 wmt: new
   14may97 wmt: add comment_data_headers_p

   Get classification model info & format into list of strings.  
   */
void get_models_source_info( model_DS *models, int num_models,
                             FILE *xref_case_text_fp,
                             unsigned int comment_data_headers_p)
{
  int i_model;
  char blank = ' ';

  for (i_model=0; i_model<num_models ;i_model++)
    fprintf( xref_case_text_fp, "%s%8c%s%s - index = %d\n",
             (comment_data_headers_p == TRUE) ? "#" : "", blank,
            (models[i_model]->model_file[0] == G_slash) ? "" : G_absolute_pathname,
            models[i_model]->model_file, models[i_model]->file_index);
}


/* GET_CLASS_MODEL_SOURCE_INFO
   14feb95 wmt: new
   14may97 wmt: add comment_data_headers_p

   Formats the model file pathname and index for a class
   */
void get_class_model_source_info( class_DS class, char *class_model_source,
                                  unsigned int comment_data_headers_p)
{
  char *source = class->model->model_file;
  int index = class->model->file_index;
  char caller[] = "get_class_model_source_info";

  safe_sprintf( class_model_source, sizeof( fxlstr), caller, "%s  %s%s  -  index = %d",
                (comment_data_headers_p == TRUE) ? "#" : "",
               (source[0] == G_slash) ? "" : G_absolute_pathname, source, index);
}

