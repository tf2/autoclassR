#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#ifndef _MSC_VER
#include <sys/param.h>
#endif
#include "autoclass.h"
#include "globals.h"


/* AUTOCLASS_PREDICT
   18may95 wmt: adapted from ac-x::predict-class
   10apr97 wmt: add database->n_data to copy_class_DS call
   30jun00 wmt: allocate separate storage for test_clsf->reports->class_wt_ordering 
   
   use an autoclass "training" classification to predict 
   class membership of cases in a "test" data base.
   */
clsf_DS autoclass_predict( char *data_file_ptr, clsf_DS training_clsf,
                           clsf_DS test_clsf, FILE *log_file_fp,
                           char *log_file_ptr)
{
  FILE *stream = stdout;
  FILE *header_file_fp = NULL, *model_file_fp = NULL;
  char *header_file_ptr, *model_file_ptr;
  shortstr start_fn_type = "block";
  int want_wts_p = TRUE, n_classes = training_clsf->n_classes;
  int num_classes = 1, reread_p = FALSE, regenerate_p = FALSE, n_class;
  int restart_p = FALSE, initial_cycles_p = FALSE, n_data = 0;
  int start_j_list_from_s_params = FALSE;

  /* ------------------------------------------------------------*/

  G_training_clsf = training_clsf;
  G_prediction_p = TRUE;
  if (test_clsf != NULL) {      /* do not print out input checking again */
    stream = NULL;
    G_stream = NULL;
    log_file_fp = NULL;
    log_file_ptr = NULL;
  }
  /* get test database */
  header_file_ptr = training_clsf->database->header_file;
  model_file_ptr = training_clsf->models[0]->model_file;
  if (eqstring( header_file_ptr, "") != TRUE)
    header_file_fp = fopen( header_file_ptr, "r");
  if (eqstring( model_file_ptr, "") != TRUE)
    model_file_fp = fopen( model_file_ptr, "r");
  test_clsf = generate_clsf( num_classes, header_file_fp, model_file_fp,
                            log_file_fp, stream, reread_p, regenerate_p,
                            data_file_ptr, header_file_ptr, model_file_ptr,
                            log_file_ptr, restart_p, start_fn_type,
                            initial_cycles_p, n_data, start_j_list_from_s_params);
  if (header_file_fp != NULL)
    fclose( header_file_fp);
  if (model_file_fp != NULL)
    fclose( model_file_fp);

  init_clsf_for_reports( test_clsf, G_prediction_p);
 
  if (test_clsf != NULL)
    G_stream = stdout;

  /* use weight ordering from training clsf */
  test_clsf->reports->n_class_wt_ordering = training_clsf->reports->n_class_wt_ordering; 
  /* test_clsf->reports->class_wt_ordering = training_clsf->reports->class_wt_ordering; */
  /* allocate separate storage */
  test_clsf->reports->class_wt_ordering =  get_class_weight_ordering( training_clsf);
  /* create training classes in test_clsf in order to store the predicted weights */
  test_clsf->classes =
    (class_DS *) realloc( test_clsf->classes, n_classes * sizeof( class_DS));
  test_clsf->n_classes = n_classes;
  for (n_class=num_classes; n_class<n_classes; n_class++)
    test_clsf->classes[n_class] = copy_class_DS( test_clsf->classes[0],
                                                 test_clsf->database->n_data,
                                                 want_wts_p);

  if (same_model_and_attributes( test_clsf, training_clsf) == FALSE) {
    fprintf( stdout, "ERROR: training classification & test data have different "
            "models and/or different attributes \n");
    exit (1);
  }

  update_wts( training_clsf, test_clsf);

  return (test_clsf);
}


/* SAME_MODEL_AND_ATTRIBUTES
   20may95 wmt: new

   check if two clsfs have the same model and attributes --
   used by autoclass_predict
   */
int same_model_and_attributes( clsf_DS clsf1, clsf_DS clsf2)
{   
  int i;
  model_DS model1, model2;
  database_DS db1, db2;
  att_DS att1, att2;

  if ((clsf1->num_models != 1) || (clsf2->num_models != 1)) {
    fprintf( stderr, "ERROR: -predict assumes only one model\n");
    exit (1);
  }
  model1 = clsf1->models[0];
  model2 = clsf2->models[0];
  db1 = clsf1->database;
  db2 = clsf2->database;
  if ((eqstring( model1->model_file, model2->model_file)) &&
      (model1->file_index == model2->file_index) &&
      (db1->n_atts == db2->n_atts)) {
    for (i=0; i<db1->n_atts; i++) {
      att1 = db1->att_info[i];
      att2 = db2->att_info[i];
      if (eqstring( att1->type, att2->type) == FALSE)
        return(FALSE);
      if (eqstring( att1->sub_type, att2->sub_type) == FALSE)
        return(FALSE);
      if (eqstring( att1->dscrp, att2->dscrp) == FALSE)
        return(FALSE);
      if (att1->n_props != att2->n_props)
        return(FALSE);
    }
    return(TRUE);
  }
  else
    return(FALSE);
}
