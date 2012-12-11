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


/* This searches the *model-list* for one with the same source file, file index,
   and database. */

model_DS find_similar_model( char *model_file, int file_index, database_DS database)
{
   int i;
   model_DS model;

   for (i=0; i<G_m_length; i++) {
      model = G_model_list[i];
      if ((eqstring(model->model_file, model_file)) &&
	  (model->file_index == file_index) &&
	  (db_DS_equal_p(model->database, database)))
         return(model);
   }
   return(NULL);
}


/* MODEL_DS_EQUAL_P
   18may95 wmt: Remove "db_ds_same_source_p" and use "db_same_source_p", instead.

   This extends the concept of model equality to compressed models.
   */
int model_DS_equal_p( model_DS m1, model_DS m2)
{
   if ((m1->model_file == m2->model_file) &&
       (m1->file_index == m2->file_index) &&
       (db_same_source_p( m1->database, m2->database)))
      return(TRUE);
   else return(FALSE);
}

/* EXPAND_MODEL
   23jan95 wmt: new

   create uncompressed model from compressed model
   */
model_DS expand_model( model_DS comp_model)
{
  FILE *model_file_fp, *log_file_fp = NULL, *stream = NULL;
  static fxlstr model_file;
  int file_index, expand_p = TRUE, regenerate_p = FALSE, newlength = 0;
  database_DS database;
  model_DS model = NULL;

  model_file[0] = '\0';
  
  if (comp_model->compressed_p == FALSE)
    return (comp_model);

  if (make_and_validate_pathname( "model", comp_model->model_file, &model_file,
                                 TRUE) != TRUE)
    exit(1);
  file_index = comp_model->file_index;
  database = expand_database( comp_model->database);

  if ((model = find_model( model_file, file_index, database)) != NULL)
    return (model);
  model_file_fp = fopen( model_file, "r");
  if (read_model_file( model_file_fp, log_file_fp, database, regenerate_p, expand_p,
                      stream, &newlength, model_file) != NULL) 
    model = find_model( model_file, file_index, database);
  fclose( model_file_fp);
  return (model);
}


/* FIND_MODEL
   23jan95 wmt: new

   find model in global store
   */
model_DS find_model( char *model_file_ptr, int file_index, database_DS database)
{
  int i;
  model_DS temp;

  for (i=0; i<G_m_length; i++) {
    temp = G_model_list[i];
    if ((eqstring( temp->model_file, model_file_ptr) == TRUE) &&
        (temp->file_index == file_index) &&
        (temp->database == database))
      return (temp);
  }
  return (NULL);
}


/* FREE_MODEL_DS
   25feb95 wmt: new
   */
void free_model_DS( model_DS model, int i_model)
{

  int n_prior, n_term;
  term_DS term;

  if (G_clsf_storage_log_p == TRUE)
    fprintf(stdout, "free_model(%d): %p\n", i_model, (void *) model);

  if (model->terms != NULL) {
    for (n_term=0; n_term < model->n_terms ; n_term++) {
      term = model->terms[n_term];
      free ( term->att_list);
      free_tparm_DS( term->tparm);
      free( term);
    }
    free( model->terms);
  }

  if (model->att_locs != NULL)
    free( model->att_locs);
  if (model->att_ignore_ids != NULL)
    free( model->att_ignore_ids);

  if (model->priors != NULL) {
    for (n_prior=0; n_prior<model->num_priors ; n_prior++)
      if (model->priors[n_prior] != NULL)
        free( model->priors[n_prior]);
    free( model->priors);
  }
  free( model);
}
