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


/* GENERATE_CLSF

   15nov94 wmt: pass n_data = 0, rather than 300 to read_database;
        use apply_search_start_fn
   25apr95 wmt: add binary data file capability
   20may95 wmt: added G_prediction_p

   Generates and returns a classification from data-base and model(s).
   Checks for data-base and model(s) already available in *db-list* and
   *model-list* and if found uses them, otherwise it generates data-base
   and model(s) by combining information from data-file, header-file, and
   model-file.  Optionally initializes the classification using the start-fn.
   N-CLASSES is the initial value for the initialized classification.
   DATA-FILE is fully qualified pathname (file type forced to *data-file-type*).
   HEADER-FILE can be omitted (gets the same file name as data-file), be a file
   name (gets the same root as data-file), or be a fully qualified pathname.
   In all cases, the file type is forced to *header-file-type*.  MODEL-FILE has
   same behavior as header-file. The file type is forced to *model-file-type*.
   LOG-FILE-P can be nil (no log file produced). If t the file type is forced
   to be *log-file-type*.  OUTPUT-FILES-DEFAULT (names the log file) can be
   omitted (gets the same root as data-file and file-name of <data-file file
   name>&<header-file file name>& <model-file file name>, be a file name
   (gets the same root as data-file), or be a fully qualified pathname.
   (REREAD_P T) forces a re-read of data-file, and (REGENERATE_P T) forces a
   re-generation of the model(s) even if they are found in *db-list* and
   *model-list*.  CLSF-INIT-FUN specifies the classification initialization
   function.  (DISPLAY-WTS T) will display class weights produced by the
   initialization.  *package* is bound for interns by read.
   */
clsf_DS generate_clsf( int n_classes, FILE *header_file_fp, FILE *model_file_fp,
        FILE *log_file_fp, FILE *stream, int reread_p, int regenerate_p,
        char *data_file_ptr, char *header_file_ptr, char *model_file_ptr,
        char *log_file_ptr, int restart_p, char *start_fn_type,
        unsigned int initial_cycles_p, int max_data,
                       int start_j_list_from_s_params)
{
  int total_error_cnt, total_warning_cnt, num_models = 0;
  clsf_DS clsf;
  database_DS db;
  model_DS *models;
  int expand_p = FALSE;

  /* read_database & read_model_file are done here because jtp designed
     it this way, moving them to process_data_header_model_files breaks
     the program  - wmt */

  log_header(log_file_fp, stream, data_file_ptr, header_file_ptr, model_file_ptr,
             log_file_ptr);

  db = read_database( header_file_fp, log_file_fp, data_file_ptr, header_file_ptr,
                     max_data, reread_p, stream);

  models = read_model_file(model_file_fp, log_file_fp, db, regenerate_p, expand_p,
                           stream, &num_models, model_file_ptr); 

  process_data_header_model_files( log_file_fp, regenerate_p, stream, db, models,
                                  num_models, &total_error_cnt, &total_warning_cnt);

  if ((G_prediction_p == FALSE) && (start_j_list_from_s_params == FALSE) &&
      (restart_p == FALSE) && (db->n_data > 1000)) {
    to_screen_and_log_file("\nWARNING: the default start_j_list may not find the correct\n"
                           "         number of classes in your data set!\n",
                           log_file_fp, stream, TRUE);
    total_warning_cnt++;
  }
 
  check_stop_processing( total_error_cnt, total_warning_cnt, log_file_fp, stderr);

  if ((G_prediction_p == TRUE) && (G_training_clsf != NULL))
    models = G_training_clsf->models;

  clsf = set_up_clsf( n_classes, db, models, num_models);

  if ((restart_p == FALSE) && (G_prediction_p == FALSE))
    apply_search_start_fn (clsf, start_fn_type, initial_cycles_p, n_classes,
                           log_file_fp, stream);
  return(clsf);
} 


/* RANDOM_SET_CLSF
   10nov94 wmt: initialize n_used_list
   05jan95 wmt: upgrade calculations to ac-x code
   17may95 wmt: Convert from srand/rand to srand48/lrand48

   This resets the classification 'clsf to size n-classes and uses
   choose-random-prototype to randomly initializes any added classes, or all
   classes with 'random-start.  Classes are initialized by choosing one class
   as a prototype and adding 1% from two others for variance.  The function
   modifies the classification and returns the number of classes and the
   marginal probability.
   */
int random_set_clsf( clsf_DS clsf, int n_classes, int delete_duplicates,
                    int  display_wts, unsigned int initial_cycles_p,
                    FILE *log_file_fp, FILE *stream)
{
  int n, i, n_data, index_0, *used_list, n_used_list = 0, num_atts = 0, n_others;
  int index_1, *used_cls_list, n_used_cls_list = 0, m;
  class_DS *classes, class;
  float proto_wt, wt_0, wt_1;

  n_classes =
    max(1, min((int) ceil((double) n_classes), clsf_DS_max_n_classes(clsf)));

  adjust_clsf_DS_classes(clsf, n_classes);

  n_data = clsf->database->n_data;
  for (i=0; i<clsf->database->n_atts; i++)
    if (eqstring( clsf->database->att_info[i]->type, "dummy") != TRUE)
      num_atts++;
  classes = clsf->classes;
  proto_wt = (float) n_data / n_classes;
  wt_0 = 0.95 * proto_wt;
  n_others = min( num_atts, (int) ceil( (double) (n_data / 2)));
  wt_1 = (0.05 * proto_wt) / n_others;
  used_list = (int *) malloc( n_classes * sizeof(int));
  used_cls_list = (int *) malloc( (n_others + 1) * sizeof(int));
  for (i=0; i<n_classes; i++)
    used_list[i] = 0;                                   /* primary cases for all classes */
  srand48( (long) (get_universal_time()));              /* re-init random number generator */
  for (n=0; n<n_classes; n++) {
    class = classes[n];
    index_0 = new_random( n_data, used_list, n_used_list);
    used_list[n_used_list] = index_0;
    n_used_list++;
    for (i=0; i<n_others; i++)
      used_cls_list[i] = 0;                               /* secondary cases for each class */
    used_cls_list[0] = index_0;
/*  printf("\nclass-index %d index_0 %d\n", n, index_0); */
    n_used_cls_list = 1;
    class->wts = fill( class->wts, 0.0, class->num_wts, n_data);
    class->wts[index_0] = wt_0;
    for (m=0; m<n_others; m++) {
      index_1 = new_random( n_data, used_cls_list, n_used_cls_list);
/*  printf("index_1 %d  ", index_1); */
      used_cls_list[n_used_cls_list] = index_1;
      n_used_cls_list++;
      class->wts[index_1] = wt_1;
    }
/*  printf("\n"); */
/*  for (i=0; i<class->num_wts; i++) */
/*    printf("%f  ", class->wts[i]); */
    class->w_j = max( proto_wt, clsf->min_class_wt);
  }
  free(used_list);
  free(used_cls_list);
  
  return(initialize_parameters(clsf, display_wts, delete_duplicates, initial_cycles_p,
                               log_file_fp, stream));
}


/* SET_UP_CLSF

   Sets up a classification-DS of n-classes, using the model-set and database.
   If the model-set contains two or more model-DS, the classes are evenly
   distributed between them.  The classification is not initialized.  Use one
   of the *start-fn-list* functions.

   10apr97 wmt: add database->n_data to get_class_DS & copy_class_DS calls
   */
clsf_DS set_up_clsf( int n_classes, database_DS database, model_DS *model_set,
                    int n_models)
{
   int i, n_class;
   class_DS *classes;
   clsf_DS clsf;
   int force_p = FALSE, want_wts_p = TRUE, check_model_p = TRUE;
   FILE *log_file_p = NULL;

   for (i=0; i<n_models; i++)
     conditional_expand_model_terms(model_set[i], force_p, log_file_p, G_stream);
   clsf = get_clsf_DS(n_classes);
   classes = clsf->classes;

   if (every_db_DS_same_source_p(database, model_set) == FALSE) {
      fprintf(stderr, "ERROR: Model has database other than the current database.\n");
      abort();
    }                                            /* Get a set of prototype classes */
   for (n_class=0; n_class<n_models; n_class++)
     classes[n_class] = get_class_DS(model_set[n_class], database->n_data,
                                     want_wts_p, check_model_p);
   /* Fill remaining class slots. */
   for (n_class=n_models; n_class<n_classes; n_class++)
     classes[n_class] = copy_class_DS( classes[n_class%n_models], database->n_data,
                                       want_wts_p);

   clsf->database = database;
   clsf->num_models = n_models;
   clsf->models = model_set;
   clsf->min_class_wt = max(ABSOLUTE_MIN_CLASS_WT,
      (MIN_CLASS_WT_FACTOR * (float) database->n_data) / (float) n_classes);
   return(clsf);
}


/* BLOCK_SET_CLSF
   15dec94 wmt: added params FILE *log_file_fp, FILE *stream
   05jan95 wmt: upgrade to ac-x code
   22mar95 wmt: ceil -> floor for n_classes & block_size; only full blocks

   Initalize a classification by dividing the database into sequential blocks,
   assigning each block to a class, and updating the class parameters.
   Blocks are are specified by giving n-classes or block-size (which overides
   n-classes when specified).  This is useful for checking the `correct'
   classification in artifical databases formed of blocks of data
   generated from some model such as those implimented through
   #'gen-formatted-data.  The function modifies the classification and
   returns the number of classes and the marginal probability.
   */
void block_set_clsf( clsf_DS clsf, int n_classes, int block_size, 
                    int delete_duplicates, int display_wts,
                    unsigned int initial_cycles_p, FILE *log_file_fp, FILE *stream)
{
  int i, n_class, n_data, base, limit, num_wts, count;
  float *wts;
  class_DS class, *classes;

  n_data = clsf->database->n_data;
  base = 0;
  limit = 0;
  if (block_size != 0) {
    block_size = max(1, min(block_size, n_data));
    n_classes = (int) floor((double) ((float) n_data / (float) block_size));
  }
  else if (n_classes != 0) {
    n_classes = max(1, min(n_classes, clsf_DS_max_n_classes( clsf)));
    block_size = (int) floor((double) ((float) n_data / (float) n_classes));
  }
  else {
    n_classes = clsf->n_classes;
    block_size = (int) floor((double) ((float) n_data / (float) n_classes));
  }

  adjust_clsf_DS_classes(clsf, n_classes);

  classes = clsf->classes;
  for (n_class = 0; n_class < n_classes; n_class++) {
    class = classes[n_class];
    num_wts = class->num_wts;
    wts = class->wts;
    count = 0;
    limit = min( limit + block_size, n_data);
    for (i=0; i<num_wts; i++)
      if ((i < base) || (i >= limit))
        wts[i] = 0.0;
      else {
        wts[i] = 1.0;
        count++;
      }
    class->w_j = max( clsf->min_class_wt, (float) count);
    base += count;
  }

  initialize_parameters(clsf, display_wts, delete_duplicates, initial_cycles_p,
                        log_file_fp, stream);
}


/* INITIALIZE_PARAMETERS
   15dec94 wmt: add params FILE *log_file_fp, FILE *stream, exit for
                "Too many classes for available data"

   This is to be applied to a classification which has just had its wts reset.
   It updates the parameters and does two base-cycle's to assure that the
   weights and parameters are in approximate agreement.  With delete-duplicates,
   it then eliminates any obvious duplicates.
   */
int initialize_parameters( clsf_DS clsf, int display_wts, int  delete_duplicates,
                          unsigned int initial_cycles_p, FILE *log_file_fp, FILE *stream)
{
  int converge_cycle_p = FALSE;

  if (clsf->n_classes > clsf_DS_max_n_classes(clsf)) {
    to_screen_and_log_file("ERROR: too many classes for available data!!!\n",
                           log_file_fp, stream, TRUE);
    exit(1);
  }
  clsf->log_p_x_h_pi_theta = 0.0;

  update_parameters(clsf);

  update_ln_p_x_pi_theta(clsf, FALSE);

  update_approximations(clsf);

  if (display_wts == TRUE)
    display_step( clsf, stream);

  if (initial_cycles_p == TRUE) {

    base_cycle(clsf, stream, display_wts, converge_cycle_p);

    if (delete_duplicates == TRUE)
      while (class_duplicatesp(clsf->n_classes, clsf->classes) == TRUE) {
        if (stream != NULL)
          fprintf( stream, "Before class_duplicatesp, n_classes is %d\n", clsf->n_classes);
        clsf->classes =
          delete_class_duplicates(&(clsf->n_classes), clsf->classes);
        if (stream != NULL)
          fprintf( stream, "After class_duplicatesp, n_classes is %d\n", clsf->n_classes);
        base_cycle(clsf, stream, display_wts, converge_cycle_p);
      }
  }
  return(clsf->n_classes);
}


class_DS *delete_class_duplicates( int *num, class_DS *classes)
{
   int i, j, k, newlength = *num;

   for (i=0; i<newlength; i++)
      for (j=i+1; j<newlength; j++)
	 if (classes[i] == classes[j]) {
	    for (k=j; k<(newlength - 1); k++)
	       classes[k] = classes[k+1];
	    newlength--;
	 }
   /*for (i=newlength; i<num; i++)
      free(classes[i]; */
   *num = newlength;
   return(classes);
}
