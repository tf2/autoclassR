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


/* With non-nill dbmodel this replaces the database and model structures
   referenced in the clsf-DS and class-DS levels with the corresponding
   compressed structures.  With non-nill wts, it substitutes nil's for
   the class-DS-w_j.  The origional values are returned as multiple values
   for possible restoration in With-Compressed-Clsf and
   With-Compressed-Clsf-Seq. */

void compress_clsf( clsf_DS clsf, model_DS dbmodel, int want_wts_p)
{
  int i;
  class_DS *classes;
  database_DS database;
  model_DS *models;

  database = clsf->database;
  models = clsf->models;
  classes = clsf->classes;
  /* wts vector not returned so why save? JTP
     wts_vector = (float **) malloc(clsf->n_classes * sizeof(float *));
     for (i=0; i<clsf->n_classes; i++)
     wts_vector[i] = classes[i]->wts;
     ********commented */
  /* DJC - Removed code that checks for previous compression */
  if (want_wts_p == FALSE)
    for (i=0; i<clsf->n_classes; i++){
      if(classes[i]->wts != NULL)
        free(classes[i]->wts);
      classes[i]->wts = NULL;
    }
}


/* EXPAND_CLSF
   22jan95 wmt: rework code according to ac-x version
   
   Restores a previously compressed classification to near origional form.
   With 'wts or 'update-wts and other keys nil, this does a full expansion
   ('wts) or expansion and update ('update-wts).  With both nil, the
   class-DS-wts are not expanded.  The other keys provide a mechanism for
   restoring a classification to its previous condition in with-compressed-clsf
   and with-compressed-clsf-seq by providing preferred choices for the
   expansion.  Database & models may sequences of compressed structures, and
   wts-vector may be a sequence of nils, allowing restoration to a
   (partially) compressed state.
   */
clsf_DS expand_clsf( clsf_DS clsf, int want_wts_p, int update_wts_p)
{
  int i_model, i_class, num_wts = 0, n_terms, i_term;
  float **wts_vector = NULL;
  tparm_DS tparm;

  /* If needed, restore the database and model pointers: */
  if (clsf->database->compressed_p == TRUE) {

    clsf->database = expand_database( clsf->database);

    /* reset database ptrs */
    for (i_model=0; i_model<clsf->num_models; i_model++)
      clsf->models[i_model]->database = clsf->database;

    expand_clsf_models( clsf);

    check_errors_and_warnings( clsf->database, clsf->models, clsf->num_models);
  }

  /* reset class tparms att_indices ptrs to expanded model struct */
  for (i_class=0; i_class<clsf->n_classes; i_class++) {
    n_terms = clsf->classes[i_class]->model->n_terms;
    for (i_term=0; i_term<n_terms; i_term++) {
      tparm = clsf->classes[i_class]->tparms[i_term];
      tparm->att_indices = clsf->classes[i_class]->model->terms[i_term]->att_list;
    }
  }

  /* If needed, restore the class weights vectors: */
  if ((want_wts_p == TRUE) || (update_wts_p == TRUE))
    expand_clsf_wts( clsf, wts_vector, num_wts);

  if ((update_wts_p == TRUE) && (wts_vector == NULL))
    update_wts( clsf, clsf);

  return(clsf);
}


/* EXPAND_CLSF_MODELS
   23jan95 wmt: new

   expand models, reading model file and expanding models, if necessary
   */
void expand_clsf_models( clsf_DS clsf)
{
  int i_model, i_class, force = FALSE;
  model_DS *models, *comp_models;
  FILE *log_file_fp = NULL, *stream = NULL;

  comp_models = clsf->models;
  models = (model_DS *) malloc( clsf->num_models * sizeof( model_DS));

  for (i_model=0; i_model<clsf->num_models; i_model++) 
    models[i_model] = expand_model( comp_models[i_model]);

  clsf->models = models;

  for (i_model=0; (i_model < clsf->num_models); i_model++)
    conditional_expand_model_terms( clsf->models[i_model], force, log_file_fp,
                                   stream);
  /* reset model ptrs */
  for (i_class=0; i_class<clsf->n_classes; i_class++) {
    for (i_model=0; i_model<clsf->num_models; i_model++) {
      if (clsf->classes[i_class]->model->file_index ==
          clsf->models[i_model]->file_index) {
        clsf->classes[i_class]->model = clsf->models[i_model];
        break;
      }
    }
  }
  /* free comp models */
  for (i_model=0; i_model<clsf->num_models; i_model++) 
    free_model_DS( comp_models[i_model], i_model);
  free( comp_models); 
}


/*
  EXPAND_CLSF_WTS
  13feb98 wmt: check for malloc/realloc failures

  Normally called from Expand-clsf.  This first checks to see if the
  classification weights need expansion.  It then checks to see if wts_vector
  is provided and suitable, optionally using it.  Otherwise wts vectors of
  n_data length are generated and filled with zeros.  Note that a n_classes
  wts_vector of nils is accepted as a special case, allowing use of the
  with-compressed-clsf-seq macro with classes having previously compressed
  weights. 
*/
void expand_clsf_wts( clsf_DS clsf, float **wts_vector, int num_wts)
{
  int i, j, n_data, found, n_classes;
  float *cl_wts, sum;
  class_DS *classes = clsf->classes;

  n_classes=clsf->n_classes;
  n_data = clsf->database->n_data;

  /* printf("\n###in expand_clsf_wts with ndata,numwts=%d %d\n",n_data,num_wts); jtpDBG */

  found=0;
  i=0;
  while(found==0 && i<n_classes){
    cl_wts = classes[i]->wts;
    if( cl_wts==NULL)
      found = 1;
    else{
      sum = 0.0;
      for (j=0; j<classes[i]->num_wts; j++)
        sum += cl_wts[j];
      if ((classes[i]->num_wts != n_data) || (classes[i]->w_j != sum))
        found = 1;
    }/*end if else*/
    i++;
  }
  if (found == 0) {   /* Check if class-DS-wts exist and have correct totals. */
    found = 1;
    if (num_wts == n_classes  && wts_vector != NULL) {
	                        /* Check wts lengths and sums against classes */
      for (i=0; i<num_wts; i++) {
        sum = 0;
        /* this looks JTP supspicious for calc of length 

           length = sizeof(wts_vector[i]) / sizeof(float);
           for (j=0; j<length; j++)
           sum += wts_vector[i][j];
           if ((wts_vector[i] != NULL) &&
           ((length != n_data) || (classes[i]->w_j != sum)))
           found = 0;
           ***************** commented JTP */
        /* above section recoded just let bad sum catch bad length*/
        for (j=0; j<n_data; j++)
          sum += wts_vector[i][j];
        if ((wts_vector[i] != NULL) &&  (classes[i]->w_j != sum))
          found = 0;


      }
      if (found == 0) {                              /* Use the given wts. */
        for (i=0; i<num_wts; i++) {
          if (classes[i]->wts != NULL)
            free(classes[i]->wts);
          /* note that wts_vector is used as is not copied */
          classes[i]->wts = wts_vector[i];
        }
      }
      else
        for (i=0; i<n_classes; i++){
          if(classes[i]->wts == NULL) {
            classes[i]->wts = (float *) malloc(n_data * sizeof(float));
            if (classes[i]->wts == NULL) {
              fprintf( stderr,
                       "ERROR: expand_clsf_wts(1): out of memory, malloc returned NULL!\n");
              exit(1);
            }
          }
          for (j=0;j<n_data;j++)
            classes[i]->wts[j]=0.0;
        }
    }
    else
      for (i=0; i<n_classes; i++){
        if(classes[i]->wts == NULL) {
          classes[i]->wts = (float *) malloc(n_data * sizeof(float));
          if (classes[i]->wts == NULL) {
            fprintf( stderr,
                     "ERROR: expand_clsf_wts(2): out of memory, malloc returned NULL!\n");
            exit(1);
          }
        }
        for (j=0;j<n_data;j++)
          classes[i]->wts[j]=0.0;
      }
  }
}
 

/* SAVE_CLSF_SEQ
   12nov94 wmt: activated this function
   18feb95 wmt: write file as temporary and rename after completion
                (G_safe_file_writing_p should be true only for unix systems,
                since this does system calls for mv and rm shell commands)
   13mar95 wmt: add binary file capability
   27apr95 wmt: Solaris 2.4 fails open, unless fopen/fclose is done first
   02may95 wmt: double size of str - prevent "Premature end of file
                reading symbol table" error.
   16may95 wmt: converted binary i/o to ANSI

   This saves a sequence of classifications to a file. 
   */
void save_clsf_seq( clsf_DS *clsf_seq, int num, char *save_file_ptr,
                   unsigned int save_compact_p, char *results_or_chkpt)
{
  FILE *save_file_fp = NULL;
  static fxlstr temp_save_file, save_file; 
  shortstr ext_type, temp_ext_type;
  int str_length = 2 * sizeof( fxlstr);
  char caller[] = "save_clsf_seq", *str;

  str = (char *) malloc( str_length);
 
  if (eqstring( results_or_chkpt, "results")) {
    if (save_compact_p == TRUE) {
      strcpy( ext_type, "results_bin");
      strcpy( temp_ext_type, "results_tmp_bin");  
    }
    else {
      strcpy( ext_type, "results");
      strcpy( temp_ext_type, "results_tmp");
    }
  }
  else if (eqstring( results_or_chkpt, "chkpt")) {
    if (save_compact_p == TRUE) {
      strcpy( ext_type, "checkpoint_bin");
      strcpy( temp_ext_type, "checkpoint_tmp_bin");
    }
    else {
      strcpy( ext_type, "checkpoint");
      strcpy( temp_ext_type, "checkpoint_tmp");
    }
  }
  else {
    fprintf( stderr, "ERROR: save file extension type %s not handled\n",
            results_or_chkpt);
    abort();
  }
  temp_save_file[0] = '\0';
  save_file[0] = '\0';
  make_and_validate_pathname( ext_type, save_file_ptr, &save_file, FALSE);

  if (G_safe_file_writing_p == TRUE) {
    make_and_validate_pathname( temp_ext_type, save_file_ptr, &temp_save_file, FALSE);
    save_file_fp = fopen( temp_save_file, (save_compact_p) ? "wb" : "w");
  }
  else
    save_file_fp = fopen( save_file, (save_compact_p) ? "wb" : "w");

  if (save_compact_p)
    dump_clsf_seq( clsf_seq, num, save_file_fp);
  else 
    write_clsf_seq( clsf_seq, num, save_file_fp);
  fclose( save_file_fp);
  
  if (G_safe_file_writing_p == TRUE) {
    if ((save_file_fp = fopen( save_file, "r")) != NULL) {
      fclose( save_file_fp);
      safe_sprintf( str, str_length, caller, "rm %s", save_file);
      system( str);
    }
    safe_sprintf( str, str_length, caller, "mv %s %s", temp_save_file, save_file);
    system( str);
  }
  free( str);
}


/* WRITE_CLSF_SEQ
   18nov94 wmt: output clsf header prior to outputting the clsfs

   Performs Write-Clsf on a sequence of classifications.
   Since we must do a dynamic traversal of the clsf nested structures in
   order to write them out, we will do the "compress-clsf/compress-database/
   compress-models" functions as a part of the writing -- thus no compress-clsf,
   write-clsfs, and then expand clsf (wmt).
   */
void write_clsf_seq( clsf_DS *clsf_seq, int num, FILE *stream)
{
   int i;
   char caller[] = "write_clsf_seq";

   safe_fprintf( stream, caller, "# ordered sequence of clsf_DS's: 0 -> %d\n", num - 1);

   for (i=0; i<num; i++)
      safe_fprintf( stream, caller, "# clsf_DS %d: log_a_x_h = %.7e\n", i,
                   clsf_seq[i]->log_a_x_h);

   safe_fprintf( stream, caller, "ac_version %s\n", G_ac_version);

   for (i=0; i<num; i++)
      write_clsf_DS( clsf_seq[i], stream, i);
}


/* WRITE_CLSF_DS
   12nov94 wmt: implement this function

   Normally writes a compressed form of the classification where class weights
   are discarded and the database and models are referenced by file name.
   There is no net alteration of the classification. 
*/
void write_clsf_DS( clsf_DS clsf, FILE *stream, int clsf_num)
{
  int i;
  char caller[] = "write_clsf_DS";

  safe_fprintf(stream, caller, "clsf_DS %d\n", clsf_num);
  safe_fprintf(stream, caller, "log_p_x_h_pi_theta, log_a_x_h\n");
  safe_fprintf(stream, caller, "%.7e %.7e\n", clsf->log_p_x_h_pi_theta, clsf->log_a_x_h);

  if (clsf_num == 0)
    write_database_DS( clsf->database, stream);
  else
    safe_fprintf(stream, caller, "database_DS_ptr\n");

  safe_fprintf(stream, caller, "num_models\n");
  safe_fprintf(stream, caller, "%d\n", clsf->num_models);

  for (i=0; i<clsf->num_models; i++) {
    if (clsf_num == 0)
      write_model_DS( clsf->models[i], i, clsf->database, stream);
    else
      safe_fprintf(stream, caller, "model_DS_ptr %d\n", i);
  }

  safe_fprintf(stream, caller, "n_classes\n");
  safe_fprintf(stream, caller, "%d\n", clsf->n_classes);

  write_class_DS_s( clsf->classes, clsf->n_classes, stream);

  safe_fprintf(stream, caller, "min_class_wt\n");
  safe_fprintf(stream, caller, "%.7f\n", clsf->min_class_wt);

  /* clsf->reports is only used for report generation - do not output */

  safe_fprintf(stream, caller, "chkpt_DS\n");
  safe_fprintf(stream, caller, "accumulated_try_time, current_try_j_in, current_cycle\n");
  safe_fprintf(stream, caller, "%d %d %d\n", clsf->checkpoint->accumulated_try_time,
          clsf->checkpoint->current_try_j_in, clsf->checkpoint->current_cycle);
}


/* WRITE_DATABASE_DS
   13nov94 wmt: new

   write compressed database_DS contents in ascii
   */  
void write_database_DS( database_DS database, FILE *stream)
{
  int i;
  char caller[] = "write_database_DS";

  safe_fprintf(stream, caller, "database_DS\n");
  safe_fprintf(stream, caller, "data_file, header_file\n");
  safe_fprintf(stream, caller, "%s %s\n", database->data_file, database->header_file);
  safe_fprintf(stream, caller, "n_data, n_atts, input_n_atts\n");
  safe_fprintf(stream, caller, "%d %d %d\n", database->n_data, database->n_atts,
          database->input_n_atts);
  /* Ordered N-atts vector of att_DS describing the attributes. */
  for (i=0; i<database->n_atts; i++)
    write_att_DS( database->att_info[i], i, stream);
}


/* WRITE_ATT_DS
   13may02 wmt: for warnings_and_errors->num_expander_warnings and
                warnings_and_errors->num_expander_errors,
                write char by char, removing \n characters.
                Make sure that msg is only 1 line, since that is
                how it is read in.
   13nov94 wmt: new

   write att_DS contents in ascii
   */  
void write_att_DS( att_DS att_info, int n_att, FILE *stream)
{
  real_stats_DS real_stats;
  discrete_stats_DS discrete_stats;
  warn_err_DS warnings_and_errors;
  fxlstr line;
  int i, j;
  char caller[] = "write_att_DS";

  safe_fprintf(stream, caller, "att_DS %d\n", n_att);
  safe_fprintf(stream, caller, "type, subtype, dscrp\n");
  safe_fprintf(stream, caller, "%s %s \"%s\" \n", att_info->type, att_info->sub_type,
               att_info->dscrp);

  if (eqstring(att_info->type, "real")) {
    real_stats = att_info->r_statistics;
    safe_fprintf(stream, caller, "real_stats_DS\n");
    safe_fprintf(stream, caller, "count, max, min, mean, var\n");
    safe_fprintf(stream, caller, "%d %.7e %.7e %.7e %.7e\n", real_stats->count,
                 real_stats->mx, real_stats->mn, real_stats->mean, real_stats->var);
  }
  else if (eqstring(att_info->type, "discrete")) {
    discrete_stats = att_info->d_statistics;
    safe_fprintf(stream, caller, "discrete_stats_DS\n");
    safe_fprintf(stream, caller, "range, n_observed\n");
    safe_fprintf(stream, caller, "%d %d\n", discrete_stats->range,
                 discrete_stats->n_observed);
    for(i=0; i < discrete_stats->range; i++)
      safe_fprintf(stream, caller, "%d %d\n", i, discrete_stats->observed[i]);
  }
  else if (eqstring(att_info->type, "dummy")) {
    safe_fprintf(stream, caller, "dummy_stats_DS\n");
    safe_fprintf(stream, caller, "%d\n", NULL);
  }
  else {
    fprintf(stderr, "\nERROR: att_info->type %s not handled\n",
            att_info->type);
    exit(1);
  }

  if (! eqstring(att_info->type, "dummy")) {
    safe_fprintf(stream, caller, "n_props, range, zero_point, n_trans\n");
    safe_fprintf(stream, caller, "%d %d %f %d\n", att_info->n_props, att_info->range,
            att_info->zero_point, att_info->n_trans);
    safe_fprintf(stream, caller, "translations_DS\n");
    if (att_info->translations != NULL)
      for (i=0; i < att_info->n_trans; i++)
        safe_fprintf(stream, caller, "%d %s\n", i, att_info->translations[i]);
    else
      safe_fprintf(stream, caller, "%d\n", NULL);

    safe_fprintf(stream, caller, "props_DS\n");
    if (att_info->props != NULL)
      for (i=0; i < att_info->n_props; i++) {
        if (eqstring( (char *) att_info->props[i][2], "int") == TRUE)
          safe_fprintf(stream, caller, "%s %s %d\n", (char *) att_info->props[i][0],
                  (char *) att_info->props[i][2], *((int *) att_info->props[i][1]));
        else if (eqstring( (char *) att_info->props[i][2], "flt") == TRUE)
          safe_fprintf(stream, caller, "%s %s %f\n", (char *) att_info->props[i][0],
                  (char *) att_info->props[i][2], *((float *) att_info->props[i][1]));
        else if (eqstring( (char *) att_info->props[i][2], "str") == TRUE)
          safe_fprintf(stream, caller, "%s %s %s\n", (char *) att_info->props[i][0],
                  (char *) att_info->props[i][2], (char *) att_info->props[i][1]);
        else {
          fprintf( stderr, "ERROR: property list type %s, not handled!\n",
                  (char *) att_info->props[i][2]);
          abort();
        }
      }
    else
      safe_fprintf(stream, caller, "%d\n", NULL);

    warnings_and_errors = att_info->warnings_and_errors;
    safe_fprintf(stream, caller, "warn_err_DS\n");
    safe_fprintf(stream, caller, "unspecified_dummy_warning, single_valued_warning, "
            "num_expander_warnings, num_expander_errors\n");
    safe_fprintf(stream, caller, "%s %s %d %d\n",
            eqstring(warnings_and_errors->unspecified_dummy_warning, "")
            ? "NULL" : warnings_and_errors->unspecified_dummy_warning,
            eqstring(warnings_and_errors->single_valued_warning, "")
            ? "NULL" : warnings_and_errors->single_valued_warning,
            warnings_and_errors->num_expander_warnings,
            warnings_and_errors->num_expander_errors);
    /* float *unused_translators_warning; discrete translations not implementated */
    for (i=0; i < warnings_and_errors->num_expander_warnings; i++) {
      /* safe_fprintf(stream, caller, "%s\n", warnings_and_errors->model_expander_warnings[i]);
         write char by char, removing \n characters */
      strcpy( line, warnings_and_errors->model_expander_warnings[i]);
      for (j=0; j < strlen( line); j++) {
        if (line[j] != '\n')
          putc( line[j], stream);
      }
      putc( '\n', stream);
    }
    for (i=0; i < warnings_and_errors->num_expander_errors; i++) {
      /* safe_fprintf(stream, caller, "%s\n", warnings_and_errors->model_expander_errors[i]);
         write char by char, removing \n characters */
      strcpy( line, warnings_and_errors->model_expander_errors[i]);
      for (j=0; j < strlen( line); j++) {
        if (line[j] != '\n')
          putc( line[j], stream);
      }
      putc( '\n', stream);
    }

    safe_fprintf(stream, caller, "rel_error, error, missing\n");
    safe_fprintf(stream, caller, "%.7e %.7e %d\n", att_info->rel_error, att_info->error,
            att_info->missing);
  }
}


/* WRITE_MODEL_DS
   17nov94 wmt: new

   write model_DS contents in ascii - one or more models -- in compressed form
   */  
void write_model_DS( model_DS model, int model_num, database_DS database,
                  FILE *stream)
{
  char caller[] = "write_model_DS";

  safe_fprintf(stream, caller, "model_DS %d\n", model_num);
  safe_fprintf(stream, caller, "id, file_index\n");
  safe_fprintf(stream, caller, "%s %d\n", model->id, model->file_index);
  safe_fprintf(stream, caller, "model_file\n");
  safe_fprintf(stream, caller, "%s \n", model->model_file);
  /* output the data_file, header file, and n-data as the compressed database_DS */
  safe_fprintf(stream, caller, "data_file, header_file, n_data\n");
  safe_fprintf(stream, caller, "%s %s %d\n", database->data_file, database->header_file,
          database->n_data);
}


/* WRITE_TERM_DS
   18nov94 wmt: new

   write term_DS to ascii file
   */
void write_term_DS( term_DS term, int n_term, FILE *stream)
{
  int parm_num = 0;
  char caller[] = "write_term_DS";

  safe_fprintf(stream, caller, "term_DS %d\n", n_term);

  safe_fprintf(stream, caller, "n_atts, type\n");
  safe_fprintf(stream, caller, "%d %s\n", term->n_atts, term->type);

  write_vector_float( term->att_list, term->n_atts, stream);

  write_tparm_DS( term->tparm, parm_num, stream);
}


/* WRITE_TPARM_DS
   18nov94 wmt: new

   write tparm_DS (term params) to ascii file
   */
void write_tparm_DS( tparm_DS term_param, int parm_num, FILE *stream)

{
  char caller[] = "write_tparm_DS";

  safe_fprintf(stream, caller, "tparm_DS %d\n", parm_num);
  safe_fprintf(stream, caller, "n_atts, tppt(type)\n");
  safe_fprintf(stream, caller, "%d %d ", term_param->n_atts, term_param->tppt);

  switch(term_param->tppt) {
  case  SM:
    write_sm_params( &(term_param->ptype.sm), term_param->n_atts, stream);
    break;
  case  SN_CM:
    write_sn_cm_params( &(term_param->ptype.sn_cm), stream);
    break;
  case  SN_CN:
    write_sn_cn_params( &(term_param->ptype.sn_cn), stream);
    break;
  case MM_D:
    write_mm_d_params( &(term_param->ptype.mm_d), term_param->n_atts, stream);
    break;
  case MM_S:
    write_mm_s_params( &(term_param->ptype.mm_s), term_param->n_atts, stream);
    break;
  case MN_CN:
    write_mn_cn_params( &(term_param->ptype.mn_cn), term_param->n_atts, stream);
    break;
  default:
    printf("\n write_tparms_DS: unknown type of enum MODEL_TYPES=%d\n",
           term_param->tppt);
    abort();
  }

  safe_fprintf(stream, caller, "n_term, n_att, n_att_indices, n_datum, n_data\n");
  safe_fprintf(stream, caller, "%d %d %d %d %d\n", term_param->n_term,
          term_param->n_att, term_param->n_att_indices, term_param->n_datum,
          term_param->n_data);
  safe_fprintf(stream, caller, "w_j, ranges, class_wt, disc_scale\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e %.7e\n", term_param->w_j, term_param->ranges,
          term_param->class_wt, term_param->disc_scale);
  safe_fprintf(stream, caller, "log_pi, log_att_delta, log_delta, wt_m, log_marginal\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e %.7e %.7e\n", term_param->log_pi,
          term_param->log_att_delta, term_param->log_delta, term_param->wt_m,
          term_param->log_marginal);

  /* term_param->wts, term_param->datum, term_param->att_indices, term_param->data
     are just ptrs to other structs -- no need to output */
}


/* WRITE_MM_D_PARAMS
   21nov94 wmt: new

   write mm_d params to ascii file
   */
void write_mm_d_params( struct mm_d_param *param, int n_atts, FILE *stream)
{
  int i, m = 0;
  char caller[] = "write_mm_d_params";

  safe_fprintf(stream, caller, "mm_d_params\n");

  for(i=0; i<n_atts; i++) {
    m = param->sizes[i]; 
    printf("row %d, size %d\n", i, m);
    safe_fprintf(stream, caller, "wts\n");
    write_vector_float(param->wts[i], m, stream);
    safe_fprintf(stream, caller, "probs\n");
    write_vector_float(param->probs[i], m, stream);
    safe_fprintf(stream, caller, "log_probs\n");
    write_vector_float(param->log_probs[i], m, stream);            
  }

  safe_fprintf(stream, caller, "wts_vec\n");
  write_vector_float(param->wts_vec, m, stream);
  safe_fprintf(stream, caller, "probs_vec\n");
  write_vector_float(param->probs_vec, m, stream);
  safe_fprintf(stream, caller, "log_probs_vec\n");
  write_vector_float(param->log_probs_vec, m, stream);            
}


/* WRITE_MM_S_PARAMS
   21nov94 wmt: new

   write mm_s_params to ascii file -- incomplete
   */
void write_mm_s_params( struct mm_s_param *param, int n_atts, FILE *stream)
{
  char caller[] = "write_mm_s_params";

  safe_fprintf(stream, caller, "mm_s_params\n");

  safe_fprintf(stream, caller, "count, wt, prob, log_prob\n");
  safe_fprintf(stream, caller, "%d %.7e %.7e %.7e\n", param->count, param->wt, param->prob,
          param->log_prob);
}


/* WRITE_MN_CN_PARAMS
   21nov94 wmt: new

   write mn_cn_params to ascii file
   */
void write_mn_cn_params( struct mn_cn_param *param, int n_atts, FILE *stream)
{
  char caller[] = "write_mn_cn_params";

  safe_fprintf(stream, caller, "mn_cn_params\n");

  safe_fprintf(stream, caller, "ln_root log_ranges\n");
  safe_fprintf(stream, caller, "%.7e %.7e\n", param->ln_root, param->log_ranges);

  safe_fprintf(stream, caller, "emp_means\n");
	
  write_vector_float(param->emp_means, n_atts, stream);
  safe_fprintf(stream, caller, "emp_covar\n");
  write_matrix_float(param->emp_covar, n_atts, n_atts, stream);

  safe_fprintf(stream, caller, "means\n");
  write_vector_float(param->means, n_atts, stream);
  safe_fprintf(stream, caller, "covariance\n");
  write_matrix_float(param->covariance, n_atts, n_atts, stream);
  safe_fprintf(stream, caller, "factor\n");
  write_matrix_float(param->factor, n_atts, n_atts, stream);

  /* values, temp_v & temp_m are temporary storage - do not save, just reinit to 0.0 */

  safe_fprintf(stream, caller, "min_sigma_2s\n");
  write_vector_float(param->min_sigma_2s, n_atts, stream);
}


/* WRITE_SM_PARAMS
   21nov94 wmt: new

   write sm_params to ascii file

   n_atts is actually n_vals -- an overloaded slot definition
   */
void write_sm_params( struct sm_param *param, int n_atts, FILE *stream)
{
  char caller[] = "write_sm_params";

  safe_fprintf(stream, caller, "sm_params\n");

  safe_fprintf(stream, caller, "gamma_term, range, range_m1, inv_range, "
               "range_factor\n");
  safe_fprintf(stream, caller, "%.7e %d %.7e %.7e %.7e \n", param->gamma_term,
               param->range, param->range_m1, param->inv_range, param->range_factor);
  safe_fprintf(stream, caller, "val_wts\n");
  write_vector_float(param->val_wts, n_atts, stream);
  safe_fprintf(stream, caller, "val_probs\n");
  write_vector_float(param->val_probs, n_atts, stream);
  safe_fprintf(stream, caller, "val_log_probs\n");
  write_vector_float(param->val_log_probs, n_atts, stream); 
}


/* WRITE_SN_CM_PARAMS
   21nov94 wmt: new

   write sn_cm_params to ascii file
   */
void write_sn_cm_params( struct sn_cm_param *param, FILE *stream)
{
  char caller[] = "write_sn_cm_params";

  safe_fprintf(stream, caller, "sn_cm_params\n");

  safe_fprintf(stream, caller, "known_wt, known_prob, known_log_prob, unknown_log_prob\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e %.7e\n", param->known_wt, param->known_prob,
          param->known_log_prob, param->unknown_log_prob);  
   
  safe_fprintf(stream, caller, "weighted_mean, weighted_var, mean\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e\n", param->weighted_mean,
               param->weighted_var, param->mean);
   
  safe_fprintf(stream, caller, "sigma, log_sigma, variance, log_variance, inv_variance\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e %.7e %.7e\n", param->sigma, param->log_sigma,
          param->variance, param->log_variance, param->inv_variance);
   
  safe_fprintf(stream, caller, "ll_min_diff, skewness, kurtosis\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e\n", param->ll_min_diff, param->skewness,
               param->kurtosis);

  safe_fprintf(stream, caller, "prior_sigma_min_2, prior_mean_mean, prior_mean_sigma\n");
  safe_fprintf(stream, caller, " %.7e %.7e %.7e\n", param->prior_sigma_min_2,
               param->prior_mean_mean, param->prior_mean_sigma);
    
  safe_fprintf(stream, caller, "prior_sigmas_term, prior_sigma_max_2, prior_mean_var, "
               "prior_known_prior\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e %.7e\n", param->prior_sigmas_term,
               param->prior_sigma_max_2, param->prior_mean_var, param->prior_known_prior);
}


/* WRITE_SN_CN_PARAMS
   21nov94 wmt: new

   write sn_cn_params to ascii file
   */
void write_sn_cn_params( struct sn_cn_param *param, FILE *stream)
{
  char caller[] = "write_sn_cn_params";

  safe_fprintf(stream, caller, "sn_cn_params\n");

  safe_fprintf(stream, caller, "weighted_mean, weighted_var, mean\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e\n", param->weighted_mean,
               param->weighted_var, param->mean);
   
  safe_fprintf(stream, caller, "sigma, log_sigma, variance, log_variance, inv_variance\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e %.7e %.7e\n", param->sigma, param->log_sigma,
          param->variance, param->log_variance, param->inv_variance);

  safe_fprintf(stream, caller, "ll_min_diff, skewness, kurtosis\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e\n", param->ll_min_diff, param->skewness,
               param->kurtosis);

  safe_fprintf(stream, caller, "prior_sigma_min_2, prior_mean_mean, prior_mean_sigma\n");
  safe_fprintf(stream, caller, " %.7e %.7e %.7e\n", param->prior_sigma_min_2,
               param->prior_mean_mean, param->prior_mean_sigma);
    
  safe_fprintf(stream, caller, "prior_sigmas_term, prior_sigma_max_2, prior_mean_var, "
               "prior_known_prior\n");
  safe_fprintf(stream, caller, "%.7e %.7e %.7e\n", param->prior_sigmas_term,
               param->prior_sigma_max_2, param->prior_mean_var);
}


/* WRITE_PRIORS_DS
   18nov94 wmt: new

   write priors_DS to ascii file. only sn-cn & sn-cm models are non-NULL
   not used
   */
void write_priors_DS(priors_DS priors, int n_prior, FILE *stream)
{
  char caller[] = "write_priors_DS";

  safe_fprintf(stream, caller, "priors_DS %d\n", n_prior); 

  if (priors != NULL) {
    safe_fprintf(stream, caller, "known_prior, sigma_min, sigma_max\n");
    safe_fprintf(stream, caller, "%.7e %.7e %.7e\n",
            priors->known_prior, priors->sigma_min, priors->sigma_max);   
    safe_fprintf(stream, caller, "mean_mean, mean_sigma, mean_var\n");
    safe_fprintf(stream, caller, "%.7e %.7e %.7e\n",
            priors->mean_mean, priors->mean_sigma, priors->mean_var);   
    safe_fprintf(stream, caller, "minus_log_log_sigmas_ratio, minus_log_mean_sigma\n");
    safe_fprintf(stream, caller, "%.7e %.7e\n",
            priors->minus_log_log_sigmas_ratio, priors->minus_log_mean_sigma);
  }
  else
    safe_fprintf(stream, caller, "NULL\n");
}


/* WRITE_CLASS_DS_S
   22nov94 wmt: new

   write class_DS to ascii file. 
   */
void write_class_DS_s( class_DS *classes, int n_classes, FILE *stream)
{
  int i, j;
  char caller[] = "write_class_DS_s";

  for (i=0; i < n_classes; i++) {
    safe_fprintf(stream, caller, "class_DS %d\n", i);

    safe_fprintf(stream, caller, "w_j, pi_j\n");
    safe_fprintf(stream, caller, "%.7e %.7e\n", classes[i]->w_j, classes[i]->pi_j);

    safe_fprintf(stream, caller, "log_pi_j, log_a_w_s_h_pi_theta, log_a_w_s_h_j\n");
    safe_fprintf(stream, caller, "%.7e %.7e %.7e\n", classes[i]->log_pi_j,
            classes[i]->log_a_w_s_h_pi_theta, classes[i]->log_a_w_s_h_j);

    safe_fprintf(stream, caller, "known_parms_p, num_tparms\n");
    safe_fprintf(stream, caller, "%d %d\n", classes[i]->known_parms_p,
                 classes[i]->num_tparms);
   
    for (j=0; j<classes[i]->num_tparms; j++) 
      write_tparm_DS( classes[i]->tparms[j], j, stream);

    /* num_i_values, i_values, i_sum, & max_i_value used only in reports -
       do not output */

    safe_fprintf(stream, caller, "num_wts\n%d\n", classes[i]->num_wts);

    safe_fprintf(stream, caller, "model_DS_ptr %d\n", classes[i]->model->file_index);
  }
}
 

/* MAKE_AND_VALIDATE_PATHNAME
   06oct94 wmt
   01may95 wmt: change type of file_ptr: char * => fxlstr *
   18may95 wmt: added "-predict" mode
   13jun95 wmt: only do fclose, if fopen returns non-NULL
   07sep95 wmt: check for presence of '.'
   13feb98 wmt: change strchr to strrchr to handle `../ac.xxx'

   create pathname with forced extension and optionally validate its existence
   */
int make_and_validate_pathname (char *type, char *file_arg, fxlstr *file_ptr,
                                int validate_p)
{
  FILE *file_fp; 
  char *file_arg_ext, *file_ext;
  char dot_char = '.';
  int file_valid_p = TRUE;
  
  if (eqstring(type, "data"))
    file_ext = DATA_FILE_TYPE;
  else if (eqstring(type, "header"))
    file_ext = HEADER_FILE_TYPE;
  else if (eqstring(type, "model"))
    file_ext = MODEL_FILE_TYPE;
  else if (eqstring(type, "search params"))
    file_ext = SEARCH_PARAMS_FILE_TYPE;
  else if (eqstring(type, "reports params"))
    file_ext = REPORTS_PARAMS_FILE_TYPE;
  else if (eqstring(type, "search"))
    file_ext = SEARCH_FILE_TYPE;
  else if (eqstring(type, "search_tmp"))
    file_ext = TEMP_SEARCH_FILE_TYPE;
  else if (eqstring(type, "results"))
    file_ext = RESULTS_FILE_TYPE;
  else if (eqstring(type, "results_tmp"))
    file_ext = TEMP_RESULTS_FILE_TYPE;
  else if (eqstring(type, "results_bin"))
    file_ext = RESULTS_BINARY_FILE_TYPE;
  else if (eqstring(type, "results_tmp_bin"))
    file_ext = TEMP_RESULTS_BINARY_FILE_TYPE;
  else if (eqstring(type, "log"))
    file_ext = SEARCH_LOG_FILE_TYPE;
  else if (eqstring(type, "rlog"))
    file_ext = REPORT_LOG_FILE_TYPE;
  else if (eqstring(type, "checkpoint"))
    file_ext = CHECKPOINT_FILE_TYPE;
  else if (eqstring(type, "checkpoint_tmp"))
    file_ext = TEMP_CHECKPOINT_FILE_TYPE;
  else if (eqstring(type, "checkpoint_bin"))
    file_ext = CHECKPOINT_BINARY_FILE_TYPE;
  else if (eqstring(type, "checkpoint_tmp_bin"))
    file_ext = TEMP_CHECKPOINT_BINARY_FILE_TYPE;
  else if (eqstring(type, "influ_vals"))
    file_ext = INFLU_VALS_FILE_TYPE;
  else if (eqstring(type, "xref_class"))
    file_ext = XREF_CLASS_FILE_TYPE;
  else if (eqstring(type, "xref_case"))
    file_ext = XREF_CASE_FILE_TYPE;
  else if (eqstring(type, "predict"))
    file_ext = PREDICT_FILE_TYPE;
  else {
    fprintf(stderr, "ERROR: file type: %s not handled! \n", type);
    exit(1);
  }

  if ((int) strlen( file_arg) > (STRLIMIT - 1)) {
    fprintf ( stderr, "ERROR: pathname %s is greater than %d chars -- see autoclass.h \n",
             file_arg, (STRLIMIT - 1));
    exit(1);
  }
  file_arg_ext = strrchr( file_arg, dot_char);
  if (file_arg_ext == NULL) {
    fprintf (stderr, "ERROR: pathname %s does not contain '.' character \n", file_arg);
    file_valid_p = FALSE;
  }
  else {
    strncat( *file_ptr, file_arg, strlen( file_arg) - strlen( file_arg_ext)); 
    strncat( *file_ptr, file_ext, strlen( file_ext));
    /*    printf("file %s\n", *file_ptr); */
    if (validate_p == TRUE) {
      file_fp = fopen( *file_ptr, "r");
      if (file_fp == NULL) {
        fprintf( stderr, "ERROR: %s file: %s%s not found! \n", type,
                ((*file_ptr)[0] == G_slash) ? "" : G_absolute_pathname, *file_ptr);
        file_valid_p = FALSE;
      }
      else
        fclose(file_fp);
    }
  }
  return(file_valid_p);
}


/* VALIDATE_RESULTS_PATHNAME
   17mar95 wmt: new
   27apr95 wmt: Solaris 2.4 fails open, unless fopen/fclose is done first 
   01may95 wmt: change type of file_ptr: char * => fxlstr *
   16may95 wmt: converted binary i/o to ANSI
   13jun95 wmt: only do fclose, if fopen returns non-NULL
   06sep95 wmt: prefer the user supplied file extension, and only attempt
                to open ".result-bin", and then ".results", if no extension
                or an invalid extension is supplied.
   25jun96 wmt: use binary_file, rather than file, were it is intended
   18apr97 wmt: handle checkpoint files similarly to results files =>
                determine if they are ascii or binary, rather than assuming
                they are binary.
   13feb98 wmt: change strchr to strrchr to handle `../ac.results'

   check for either binary or ascii "results" or "checkpoint" files
   */
int validate_results_pathname( char *file_pathname, fxlstr *found_file_ptr,
                               char *type, int exit_if_error_p, int silent_p)
{
  FILE *file_fp, *binary_file_fp; 
  char *file_arg_ext;
  static fxlstr file, binary_file;
  char dot_char = '.', user_extension[10] = "";
  int file_valid_p = FALSE, file_arg_ext_length;

  file[0] = binary_file[0] = '\0';
  if ((int) strlen( file_pathname) > (STRLIMIT - 1)) {
    fprintf (stderr, "ERROR: pathname %s is greater than %d chars -- see autoclass.h \n",
             file_pathname, (STRLIMIT - 1));
    exit(1);
  }
  file_arg_ext = strrchr( file_pathname, dot_char);
  if (file_arg_ext == NULL) {
    fprintf (stderr, "ERROR: results file pathname %s does not contain '.' character \n",
             file_pathname);
    if (exit_if_error_p == TRUE)
      exit(1);
    else
      return (file_valid_p);
  }
  file_arg_ext_length = (int) strlen( file_arg_ext);
  if (eqstring( type, "results")) {
    if (strstr( file_pathname, RESULTS_FILE_TYPE) &&
        (file_arg_ext_length == (int) strlen( RESULTS_FILE_TYPE))) {
      strcpy( file, file_pathname);
      strcpy (user_extension, "ascii");
    }
    else if (strstr( file_pathname, RESULTS_BINARY_FILE_TYPE) &&
             (file_arg_ext_length == (int) strlen( RESULTS_BINARY_FILE_TYPE))) {        
      strcpy( binary_file, file_pathname);
      strcpy (user_extension, "binary");
    }
  }
  else if (eqstring( type, "checkpoint")) {
    if (strstr( file_pathname, CHECKPOINT_FILE_TYPE) &&
        (file_arg_ext_length == (int) strlen( CHECKPOINT_FILE_TYPE))) {
      strcpy( file, file_pathname);
      strcpy (user_extension, "ascii");
    }
    else if (strstr( file_pathname, CHECKPOINT_BINARY_FILE_TYPE) &&
             (file_arg_ext_length == (int) strlen( CHECKPOINT_BINARY_FILE_TYPE))) {        
      strcpy( binary_file, file_pathname);
      strcpy (user_extension, "binary");
    }
  }
  else {
    fprintf( stderr, "ERROR: type %s not handled by validate_results_pathname\n",
             type);
    abort();
  }
  if (eqstring( user_extension, "")) {
    strncat( file, file_pathname, strlen( file_pathname) - file_arg_ext_length); 
    strncat( binary_file, file_pathname, strlen( file_pathname) - file_arg_ext_length);
    if (eqstring( type, "results")) {
      strncat( file, RESULTS_FILE_TYPE, strlen( RESULTS_FILE_TYPE));
      strncat( binary_file, RESULTS_BINARY_FILE_TYPE, strlen( RESULTS_BINARY_FILE_TYPE));
    } 
    else if (eqstring( type, "checkpoint")) {
      strncat( file, CHECKPOINT_FILE_TYPE, strlen( CHECKPOINT_FILE_TYPE));
      strncat( binary_file, CHECKPOINT_BINARY_FILE_TYPE, strlen( CHECKPOINT_BINARY_FILE_TYPE));
    }
  }
  /*    printf("file %s\n", file); */
  /*    printf("binary-file %s\n", binary_file); */

  if ((eqstring( user_extension, "")) || (eqstring( user_extension, "binary"))) {
    binary_file_fp = fopen( binary_file, "rb");
    if (binary_file_fp != NULL) {
      file_valid_p = TRUE;
      strcpy( *found_file_ptr, binary_file);
      fclose( binary_file_fp);
    }
  }
  if ((file_valid_p == FALSE) && ((eqstring( user_extension, "")) ||
                                  (eqstring( user_extension, "ascii")))) {
    file_fp = fopen( file, "r");
    if (file_fp != NULL) {
      file_valid_p = TRUE;
      strcpy( *found_file_ptr, file);
      fclose( file_fp);
    }
  } 
  if ((silent_p != TRUE) && (file_valid_p == FALSE)) {
    if (eqstring( user_extension, "")) {
      fprintf(stderr, "ERROR: neither \n       %s%s, \n       nor \n       "
              "%s%s \n       were found! \n",
              (binary_file[0] == G_slash) ? "" : G_absolute_pathname, binary_file,
              (file[0] == G_slash) ? "" : G_absolute_pathname, file);
    } else if (eqstring( user_extension, "ascii")) {
      fprintf(stderr, "ERROR: %s%s, \n       was not found! \n",
              (file[0] == G_slash) ? "" : G_absolute_pathname, file);
    } else {
      fprintf(stderr, "ERROR: %s%s, \n       was not found! \n",
              (binary_file[0] == G_slash) ? "" : G_absolute_pathname, binary_file);
    }
    if (exit_if_error_p == TRUE)
      exit(1);
  }
  return (file_valid_p);
}


/* VALIDATE_DATA_PATHNAME
   24apr95 wmt: new
   27apr95 wmt: Solaris 2.4 fails open, unless fopen/fclose is done first 
   01may95 wmt: change type of file_ptr: char * => fxlstr *
   16may95 wmt: converted binary i/o to ANSI
   13jun95 wmt: only do fclose, if fopen returns non-NULL
   06sep95 wmt: prefer the user supplied file extension, and only attempt
                to open ".db2", and then ".db2-bin", if no extension
                or an invalid extension is supplied.
   25jun96 wmt: use binary_file, rather than file, were it is intended
   13feb98 wmt: change strchr to strrchr to handle `../ac.db2'

   check for either binary or ascii "data" files
   */
int validate_data_pathname( char *file_pathname, fxlstr *found_file_ptr,
                               int exit_if_error_p, int silent_p)
{
  FILE *file_fp, *binary_file_fp; 
  static fxlstr file, binary_file;
  char dot_char = '.', user_extension[10] = "", *file_arg_ext;
  int file_valid_p = FALSE, file_arg_ext_length;
 
  file[0] = binary_file[0] = '\0';
  if ((int) strlen( file_pathname) > (STRLIMIT - 1)) {
    fprintf (stderr, "ERROR: data file pathname %s is greater than %d chars -- "
             "see autoclass.h \n",
             file_pathname, (STRLIMIT - 1));
    exit(1);
  }
  file_arg_ext = strrchr( file_pathname, dot_char);
  if (file_arg_ext == NULL) {
    fprintf (stderr, "ERROR: data file pathname %s does not contain '.' character \n",
             file_pathname);
    if (exit_if_error_p == TRUE)
      exit(1);
    else
      return (file_valid_p);
  }
  file_arg_ext_length = (int) strlen( file_arg_ext);
  if (strstr( file_pathname, DATA_FILE_TYPE) &&
      (file_arg_ext_length == (int) strlen( DATA_FILE_TYPE))) {
    strcpy( file, file_pathname);
    strcpy (user_extension, "ascii");
  }
  else if (strstr( file_pathname, DATA_BINARY_FILE_TYPE) &&
           (file_arg_ext_length == (int) strlen( DATA_BINARY_FILE_TYPE))) {        
    strcpy( binary_file, file_pathname);
    strcpy (user_extension, "binary");
  }
  else {
    strncat( file, file_pathname, strlen( file_pathname) - file_arg_ext_length); 
    strncat( binary_file, file_pathname, strlen( file_pathname) - file_arg_ext_length);
    strncat( file, DATA_FILE_TYPE, strlen( DATA_FILE_TYPE));
    strncat( binary_file, DATA_BINARY_FILE_TYPE, strlen( DATA_BINARY_FILE_TYPE));
  }
  /*    printf("file %s\n", file); */
  /*    printf("binary-file %s\n", binary_file); */

  if ((eqstring( user_extension, "")) || (eqstring( user_extension, "ascii"))) {
    file_fp = fopen( file, "r");
    if (file_fp != NULL) {
      file_valid_p = TRUE;
      strcpy( G_data_file_format, "ascii");
      strcpy( *found_file_ptr, file);
      fclose( file_fp);
    }
  }
  if ((file_valid_p == FALSE) && ((eqstring( user_extension, "")) ||
                                  (eqstring( user_extension, "binary")))) {
    binary_file_fp = fopen( binary_file, "rb");
    if (binary_file_fp != NULL) {
      file_valid_p = TRUE;
      strcpy( G_data_file_format, "binary");
      strcpy( *found_file_ptr, binary_file);
      fclose( binary_file_fp);
    }
  }
   
  if ((silent_p != TRUE) && (file_valid_p == FALSE)) {
    if (eqstring( user_extension, "")) {
      fprintf(stderr, "ERROR: neither \n       %s%s, \n       nor \n       "
              "%s%s \n       were found! \n",
              (binary_file[0] == G_slash) ? "" : G_absolute_pathname, binary_file,
              (file[0] == G_slash) ? "" : G_absolute_pathname, file);
    } else if (eqstring( user_extension, "ascii")) {
      fprintf(stderr, "ERROR: %s%s, \n       was not found! \n",
              (file[0] == G_slash) ? "" : G_absolute_pathname, file);
    } else {
      fprintf(stderr, "ERROR: %s%s, \n       was not found! \n",
              (binary_file[0] == G_slash) ? "" : G_absolute_pathname, binary_file);
    }
    if (exit_if_error_p == TRUE)
      exit(1);
  }
  return (file_valid_p);
}


/* GET_CLSF_SEQ
   13jan95 wmt: modified, add save_file_ptr, and n_best_clsfs_ptr
   17feb95 wmt: add expand_list
   27apr95 wmt: Solaris 2.4 fails open, unless fopen/fclose is done first 
   16may95 wmt: converted binary i/o to ANSI
   07sep95 wmt: simplify the test for "ascii" or "binary" results file format --
                also more portable.
   18apr97 wmt: handle checkpoint files properly
   13feb98 wmt: change strchr to strrchr to handle `../ac.results-bin'

   This tries to read save-file.
   */
clsf_DS *get_clsf_seq( char *results_file_ptr, int expand_p, int want_wts_p, int update_wts_p,
                      char *file_type, int *n_best_clsfs_ptr, int_list expand_list)
{
  FILE *results_file_fp;
  int dot_char = '.';
  char *file_ext_addr;
  clsf_DS *clsf_seq;

  file_ext_addr = strrchr( results_file_ptr, dot_char);

  if (eqstring( file_type, "results")) {
    if (strstr( results_file_ptr, RESULTS_BINARY_FILE_TYPE) &&
        ((int) strlen( file_ext_addr) == (int) strlen( RESULTS_BINARY_FILE_TYPE))) { 
      results_file_fp = fopen( results_file_ptr,  "rb");

      clsf_seq = load_clsf_seq( results_file_fp, results_file_ptr, expand_p, want_wts_p,
                                update_wts_p, n_best_clsfs_ptr, expand_list);
    }
    else {
      results_file_fp = fopen( results_file_ptr, "r");

      clsf_seq = read_clsf_seq( results_file_fp, results_file_ptr, expand_p, want_wts_p,
                                update_wts_p, n_best_clsfs_ptr, expand_list);
    }
  }
  else if (eqstring( file_type, "checkpoint")) {
    if (strstr( results_file_ptr, CHECKPOINT_BINARY_FILE_TYPE) &&
        ((int) strlen( file_ext_addr) == (int) strlen( CHECKPOINT_BINARY_FILE_TYPE))) { 
      results_file_fp = fopen( results_file_ptr,  "rb");

      clsf_seq = load_clsf_seq( results_file_fp, results_file_ptr, expand_p, want_wts_p,
                                update_wts_p, n_best_clsfs_ptr, expand_list);
    }
    else {
      results_file_fp = fopen( results_file_ptr, "r");

      clsf_seq = read_clsf_seq( results_file_fp, results_file_ptr, expand_p, want_wts_p,
                                update_wts_p, n_best_clsfs_ptr, expand_list);
    }
  }
  else {
    fprintf( stderr, "ERROR: file_type %s not handled by get_clsf_seq\n",
             file_type);
    abort();
  }
  fclose( results_file_fp);

  return (clsf_seq);
}


/* READ_CLSF_SEQ
   09dec 94 wmt: correct malloc/relloc
   13jan95 wmt: add n_best_clsfs_ptr
   17feb95 wmt: add expand_list
   20sep98 wmt: strip of win/unx from ac_version, starting with version 3.3
   27nov98 wmt: check for win/unx before stripping -- backward compatible

   Performs a sequence of Read_Clsf.
   */
clsf_DS *read_clsf_seq( FILE *results_file_fp, char *results_file_ptr, int expand_p,
                       int want_wts_p, int update_wts_p, int *n_best_clsfs_ptr,
                       int_list expand_list)
{
  int length = 0, file_ac_version, float_p, token_length;
  clsf_DS clsf, first_clsf = NULL, *seq = NULL;
  shortstr token1, token2;
  fxlstr line;
  char caller[] = "read_clsf_seq";

  do {                  /* read comment lines */
    fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%s %s", token1, token2);
  }
  while (eqstring( token1, "#"));

  /* strip of win/unx from ac_version, if present */
  if (strstr( token2, "unx") || strstr( token2, "win")) {
    token_length = strlen( token2);
    token2[token_length - 3] = '\0';
  }
  if ((eqstring( token1, "ac_version") != TRUE) ||
      ((file_ac_version = atof_p( token2, &float_p)) < 1.0) ||
      (float_p != TRUE)) {
    fprintf( stderr, "ERROR: expecting \"ac_version n.n\", found \"%s\" \n", line);
    abort();    
  }

  while (1) {
    clsf = read_clsf( results_file_fp, expand_p, want_wts_p, update_wts_p, length,
                     first_clsf, file_ac_version, expand_list);
    if (clsf != NULL) {
      if (length == 0)
        first_clsf = clsf;
      length++;
      if (seq == NULL)
        seq = (clsf_DS *) malloc( length * sizeof(clsf_DS));
      else
        seq = (clsf_DS *) realloc(seq, length * sizeof(clsf_DS));
      seq[length - 1] = clsf;
    }
    else
      break;
  }
  *n_best_clsfs_ptr = length;
  safe_sprintf( line,  sizeof( line), caller,
               "ADVISORY: read %d classifications from \n          %s%s\n", length,
               (results_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
               results_file_ptr);
  to_screen_and_log_file( line, G_log_file_fp, G_stream, TRUE);
  return(seq);
}


/* READ_CLSF
   13jan95 wmt: add reading capability
   17feb95 wmt: add expand_list for use by initialize_reports_from_results_pathname

   Intended to read a compactly represented classification as written by
   write_clsf_DS, and to optionally expand it to standard form.  Anything else,
   including non-compact classifications, are returned without modification.
   Compact classification are identified by the presence of a list of filenames
   in the database field, instead of a database structure.  With Expand, Wts or
   Update_Wts regenerates the wts vectors. update_wts also updates wts.
   */
clsf_DS read_clsf( FILE *results_file_fp, int expand_p, int want_wts_p, int update_wts_p,
                  int clsf_index, clsf_DS first_clsf, int file_ac_version,
                  int_list expand_list)
{
  clsf_DS clsf = NULL;
  shortstr token1, token2;
  fxlstr line;
  int integer_p, file_clsf_index, i, model_index;
  model_DS *models;

  if (fgets( line, sizeof(line), results_file_fp) != NULL) {
    sscanf( line, "%s %s", token1, token2);
    if ((eqstring( token1, "clsf_DS") == TRUE) &&
        ((file_clsf_index = atoi_p( token2, &integer_p)) == clsf_index) &&
        (integer_p == TRUE)) {

      clsf = create_clsf_DS();

      for (i=0; i<2; i++)
        fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%le %le\n", &clsf->log_p_x_h_pi_theta, &clsf->log_a_x_h);

      if (first_clsf == NULL)
        clsf->database = read_database_DS( clsf, results_file_fp, file_ac_version);
      else {
        fgets( line, sizeof(line), results_file_fp);
        sscanf( line, "%s", token1);
        if (eqstring( token1, "database_DS_ptr") != TRUE) {
          fprintf( stderr, "ERROR: expecting \"database_DS_ptr\", found \"%s\"\n", line);
          abort();
        }
        clsf->database = first_clsf->database;
      }
      fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%s", token1);
      if (eqstring( token1, "num_models") != TRUE) {
        fprintf( stderr, "ERROR: expecting \"num_models\", found \"%s\" \n", line);
        abort();
      }
      fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%d", &clsf->num_models);

      if (first_clsf == NULL)
        models = (model_DS *) malloc( clsf->num_models * sizeof( model_DS));
      else
        models = first_clsf->models;
      clsf->models = models;

      for (model_index=0; model_index<clsf->num_models; model_index++) {
        if (first_clsf == NULL)
          models[model_index] = read_model_DS( clsf, model_index, results_file_fp,
                                              file_ac_version);
        else {
          fgets( line, sizeof(line), results_file_fp);
          sscanf( line, "%s", token1);
          if (eqstring( token1, "model_DS_ptr") != TRUE) {
            fprintf( stderr, "ERROR: expecting \"model_DS_ptr\", found \"%s\"\n", line);
            abort();
          }
        }
      }
      fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%s", token1);
      if (eqstring( token1, "n_classes") != TRUE) {
        fprintf( stderr, "ERROR: expecting \"n_classes\", found \"%s\" \n", line);
        abort();
      }
      fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%d", &clsf->n_classes);

      read_class_DS_s( clsf, clsf->n_classes, results_file_fp,
                      (first_clsf == NULL) ? clsf : first_clsf,
                      file_ac_version);

      for (i=0; i<2; i++)
        fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%f", &clsf->min_class_wt);

      fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%s", token1);
      if (eqstring( token1, "chkpt_DS") != TRUE) {
        fprintf( stderr, "ERROR: expecting \"chkpt_DS\", found \"%s\" \n", line);
        abort();
      }
      for (i=0; i<2; i++)
        fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%d %d %d\n", &clsf->checkpoint->accumulated_try_time,
             &clsf->checkpoint->current_try_j_in, &clsf->checkpoint->current_cycle);

      clsf->next = NULL;
    }
    else {
      fprintf( stderr, "ERROR: expecting clsf_DS index %d, found \"%s\" \n", clsf_index, line);
      abort();
    }

    if ((expand_p == TRUE) && ((expand_list[0] == END_OF_INT_LIST) ||
                             ((expand_list[0] != END_OF_INT_LIST) &&
                              (member_int_list( clsf_index+1, expand_list) == TRUE)))) {
      expand_clsf( clsf, want_wts_p, update_wts_p);

      /* fprintf( stderr, "clsf index %d expanded\n", clsf_index); */
    }
  }
  return (clsf);
}


/* READ_DATABASE_DS
   17jan95 wmt: new
   06mar95 wmt: expand att_info, if needed
   21jun95 wmt: initialize realloc'ed att_info

   read database_DS from results file - for first clsf only
   */
database_DS read_database_DS( clsf_DS clsf, FILE *results_file_fp, int file_ac_version)
{
  int i, n_data, n_atts, input_n_atts, n_att;
  fxlstr line, data_file, header_file;
  shortstr token;
  database_DS d_base;

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s\n", token);
  if (eqstring( token, "database_DS") != TRUE) {
    fprintf( stderr, "ERROR: expecting \"database_DS\", found \"%s\"\n", line);
    abort();
  }
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s %s", data_file, header_file);
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%d %d %d", &n_data, &n_atts, &input_n_atts);

  d_base = create_database();
  d_base->n_data = 0;
  strcpy(d_base->data_file, data_file);
  strcpy(d_base->header_file, header_file);
  d_base->n_data = n_data;
  d_base->n_atts = n_atts;
  d_base->input_n_atts = input_n_atts;
  if (n_atts > d_base->allo_n_atts) {
    d_base->allo_n_atts = n_atts;
    d_base->att_info = (att_DS *) realloc( d_base->att_info,
                                          d_base->allo_n_atts * sizeof( att_DS));
    for (i=0; i<n_atts; i++) 
      d_base->att_info[i] = NULL;
  }  
  /* Ordered N-atts vector of att_DS describing the attributes. */
  for (n_att=0; n_att<d_base->n_atts; n_att++)
    read_att_DS( d_base, n_att, results_file_fp, file_ac_version);

  d_base->compressed_p = TRUE;
  d_base->separator_char = ' ';
  d_base->comment_char = ';';
  d_base->unknown_token = '?';

  return( d_base);
}


/* READ_MODEL_DS
   17jan95 wmt: new

   read model_DS from results file and allocate storage - first clsf only
   */
model_DS read_model_DS( clsf_DS clsf, int model_index, FILE *results_file_fp,
                       int file_ac_version)
{
  int i, file_model_index;
  shortstr token1;
  model_DS model;
  fxlstr line;

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s %d", token1, &file_model_index);
  if ((eqstring( token1, "model_DS") != TRUE) || (file_model_index != model_index)) {
    fprintf( stderr, "ERROR: expecting \"model_DS\" and model_index = %d, found \"%s\"\n",
            model_index, line);
    abort();
  }
  model = (model_DS) malloc( sizeof( struct model));
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s %d", model->id, &model->file_index);
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s", model->model_file);
  /* the data_file, header file, n_data are the compressed database_DS */
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s %s %d", model->data_file, model->header_file,
         &model->n_data);
  model->compressed_p = TRUE;
  model->database = clsf->database;

  /* since this is compressed model, set everthing to null */
  model->expanded_terms = FALSE;
  model->n_terms = model->num_priors = 0;
  model->terms = NULL;
  model->priors = NULL;
  model->n_att_locs = 0; 
  model->att_locs = NULL;
  model->n_att_ignore_ids = 0;
  model->att_ignore_ids = NULL;
  model->class_store = NULL; 
  model->num_class_store = 0;
  model->global_clsf = NULL;
         
  return( model);
}


/* READ_CLASS_DS_S
   17jan95 wmt: new

   read class_DS from results file and allocate storage
   */
void read_class_DS_s( clsf_DS clsf, int n_classes, FILE *results_file_fp,
                     clsf_DS first_clsf, int file_ac_version)
{
  int i, n_parm, n_class, file_n_class, file_model_file_index;
  shortstr token1;
  fxlstr line;
  class_DS *classes;

  classes = (class_DS *) malloc( n_classes * sizeof( class_DS));
  clsf->classes = classes;

  for (n_class=0; n_class < n_classes; n_class++) {
    fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%s %d", token1, &file_n_class);
    if ((eqstring( token1, "class_DS") != TRUE) || (file_n_class != n_class)) {
      fprintf( stderr, "ERROR: expecting \"class_DS\" and n_class = %d, found \"%s\"\n",
              n_class, line);
      abort();
    }
    classes[n_class] = (class_DS) malloc( sizeof( struct class));
    for (i=0; i<2; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%e %e", &classes[n_class]->w_j, &classes[n_class]->pi_j);
    for (i=0; i<2; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%e %le %le", &classes[n_class]->log_pi_j,
           &classes[n_class]->log_a_w_s_h_pi_theta, &classes[n_class]->log_a_w_s_h_j);
    for (i=0; i<2; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%d %d", &classes[n_class]->known_parms_p, &classes[n_class]->num_tparms);
   
    classes[n_class]->tparms =
      (tparm_DS *) malloc( classes[n_class]->num_tparms * sizeof( tparm_DS));
    for (n_parm=0; n_parm<classes[n_class]->num_tparms; n_parm++) {
      classes[n_class]->tparms[n_parm] = (tparm_DS) malloc( sizeof( struct new_term_params));
      read_tparm_DS( classes[n_class]->tparms[n_parm], n_parm, results_file_fp,
                    file_ac_version);
    }
    classes[n_class]->num_i_values = 0;
    classes[n_class]->i_values = NULL;
    classes[n_class]->i_sum = 0.0;
    classes[n_class]->max_i_value = 0.0;

    for (i=0; i<2; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%d", &classes[n_class]->num_wts);
    classes[n_class]->wts = (float *) malloc( classes[n_class]->num_wts * sizeof( float));
    for (i=0; i<classes[n_class]->num_wts; i++)
      classes[n_class]->wts[i] = 0.0;

    if (G_clsf_storage_log_p == TRUE) {
      fprintf( stdout, "\nread_class_DS_s: %p, num_wts %d, wts:%p, wts-len:%d\n",
              (void *) classes[n_class], classes[n_class]->num_wts,
              (void *) classes[n_class]->wts,
              classes[n_class]->num_wts * (int) sizeof( float));
      if (G_n_freed_classes > 0)
        G_n_create_classes_after_free++;
    }

    fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%s %d", token1, &file_model_file_index);
    if (eqstring( token1, "model_DS_ptr") != TRUE) {
      fprintf( stderr, "ERROR: expecting \"model_DS_ptr\" and file_index, found \"%s\"\n",
              line);
      abort();
    }
    for (i=0; i<first_clsf->num_models; i++) {
      if (first_clsf->models[i]->file_index == file_model_file_index) {
        classes[n_class]->model = first_clsf->models[i];
        break;
      }
    }
    classes[n_class]->next = NULL;
  }
}


/* READ_ATT_DS
   17jan95 wmt: new
   24jan02 wmt: do not assume that translations length < shortstr

   read att_DS from results file and allocate storage
   */
void read_att_DS( database_DS d_base, int n_att, FILE *results_file_fp, int file_ac_version)
{
  int i, file_n_att, index, *int_value, n_comment_chars = 0, token_list_length;
  int int_token, expected_token_list_length = 3, n_props;
  float *float_value;
  char *string_value, **token_list = NULL, comment_chars[1], separator_char = ' ';
  char *token_ptr;
  fxlstr line, token1, token2, token3;
  att_DS att;
  real_stats_DS real_stats;
  discrete_stats_DS discrete_stats;
        
  comment_chars[0] = '\0';

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s %d", token1, &file_n_att);
  if ((eqstring( token1, "att_DS") != TRUE) || (file_n_att != n_att)) {
    fprintf( stderr, "ERROR: expecting \"att_DS\" and n_att = %d, found \"%s\"\n",
            n_att, line);
    abort();
  }
  att = (att_DS) malloc(sizeof(struct att));
  d_base->att_info[n_att] = att;
  fgets( line, sizeof(line), results_file_fp);
  /* use get_line_tokens because of imbedded blanks in att->dscrp */
  token_list = get_line_tokens( results_file_fp, (int) separator_char, n_comment_chars,
                             comment_chars, FALSE, &token_list_length);
  if (token_list_length != expected_token_list_length)
    abort();
  strcpy( att->type, token_list[0]);
  strcpy( att->sub_type, token_list[1]);
  strcpy( att->dscrp, token_list[2]);
  for (i=0; i<expected_token_list_length; i++)
    free( token_list[i]);
  free( token_list);
  
  if (eqstring(att->type, "real")) {
    att->r_statistics = (real_stats_DS) malloc(sizeof(struct real_stats));
    att->d_statistics = NULL;
    real_stats = att->r_statistics;
    for (i=0; i<3; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%d %e %e %e %e", &real_stats->count, &real_stats->mx,
           &real_stats->mn, &real_stats->mean, &real_stats->var);
  }
  else if (eqstring(att->type, "discrete")) {
    att->r_statistics = NULL;
    att->d_statistics = (discrete_stats_DS) malloc(sizeof(struct discrete_stats));
    discrete_stats = att->d_statistics;
    for (i=0; i<3; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line,"%d %d", &discrete_stats->range, &discrete_stats->n_observed);
    att->d_statistics->observed = (int *) malloc(discrete_stats->range * sizeof(int));
    for(i=0; i < discrete_stats->range; i++) {
      fgets( line, sizeof(line), results_file_fp);
      sscanf( line, "%d %d\n", &index, &discrete_stats->observed[i]);
      if (index != i) {
        fprintf( stderr, "ERROR: expecting observed[%d], found \"%s\"\n",
                i, line);
        abort();
      }
    }
  }
  else if (eqstring( att->type, "dummy")) {
    att->r_statistics = NULL; att->d_statistics = NULL;
    att->props = NULL; att->n_props = 0;
    att->translations = NULL; att->n_trans = 0;
    att->range = att->missing = 0;
    att->error = att->rel_error = att->zero_point = 0.0;
    att->warnings_and_errors = NULL;
    for (i=0; i<2; i++)
      fgets( line, sizeof( line), results_file_fp);
  }
  else {
    fprintf(stderr, "\nERROR: att_info->type %s not handled\n",
            att->type);
    abort();
  }

  if (! eqstring( att->type, "dummy")) {
    for (i=0; i<2; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%d %d %f %d\n", &n_props, &att->range, &att->zero_point, &att->n_trans);
    att->n_props = 0;
    /* att->n_props is incremented by add_to_plist */
    fgets( line, sizeof(line), results_file_fp); /* translations_DS */
    if (att->n_trans > 0) {
      att->translations = (char **) malloc( att->n_trans * sizeof(char *));
      for (i=0; i < att->n_trans; i++) {
        fgets( line, sizeof(line), results_file_fp);
        sscanf( line, "%d %s", &int_token, token2);
        att->translations[i] = (char *) malloc( strlen( token2) + 1);
        strcpy( att->translations[i], token2);
      }
    }
    else {
      att->translations = NULL;
      fgets( line, sizeof(line), results_file_fp);      /* read the zero */
    }

    fgets( line, sizeof(line), results_file_fp);        /* props_DS */
    if (n_props > 0) {
      for (i=0; i < n_props; i++) {
        fgets( line, sizeof(line), results_file_fp);
        sscanf( line, "%s %s %s", token1, token2, token3);
        token_ptr = (char *) malloc( strlen( token1) + 1);
        strcpy( token_ptr, token1);
        if (eqstring( token2, "int") == TRUE) {
          int_value = (int *) malloc( sizeof( int));
          *int_value = atoi( token3);
          add_to_plist( att, token_ptr, int_value, "int");
        }
        else if (eqstring( token2, "flt") == TRUE) {
          float_value = (float *) malloc( sizeof( float));
          *float_value = atof( token3);
          add_to_plist( att, token_ptr, float_value, "flt");
        }
        else if (eqstring( token2, "str") == TRUE) {
          string_value = (char *) malloc( strlen( token3) + 1);
          strcpy( string_value, token3);
          add_to_plist( att, token_ptr, string_value, "str");
        }
        else {
          fprintf( stderr, "property list type %s, not handled!\n", token2);
          abort();
        }
      }
    }
    else {
      att->props = NULL;
      fgets( line, sizeof(line), results_file_fp);      /* read the zero */
    }

    att->warnings_and_errors = create_warn_err_DS();  
    fgets( line, sizeof(line), results_file_fp);        /* warn_err_DS */
    for (i=0; i<2; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%s %s %d %d", token1, token2,
           &att->warnings_and_errors->num_expander_warnings,
           &att->warnings_and_errors->num_expander_errors);
    if (eqstring( token1, "NULL") == TRUE)
      strcpy( att->warnings_and_errors->unspecified_dummy_warning, "");
    else 
      strcpy( att->warnings_and_errors->unspecified_dummy_warning, token1);
    if (eqstring( token2, "NULL") == TRUE)
      strcpy( att->warnings_and_errors->single_valued_warning, "");
    else 
      strcpy( att->warnings_and_errors->single_valued_warning, token2);
    /* float *unused_translators_warning; discrete translations not implementated */
    if (att->warnings_and_errors->num_expander_warnings > 0)
      att->warnings_and_errors->model_expander_warnings =
        (fxlstr *) malloc( att->warnings_and_errors->num_expander_warnings * sizeof( fxlstr));
    else
      att->warnings_and_errors->model_expander_warnings = NULL;
    for (i=0; i < att->warnings_and_errors->num_expander_warnings; i++) {
      fgets( line, sizeof(line), results_file_fp);
      strcpy( att->warnings_and_errors->model_expander_warnings[i], line);
    }
    if (att->warnings_and_errors->num_expander_errors > 0)
      att->warnings_and_errors->model_expander_errors =
        (fxlstr *) malloc( att->warnings_and_errors->num_expander_errors * sizeof( fxlstr));
    else
      att->warnings_and_errors->model_expander_errors = NULL;    
    for (i=0; i < att->warnings_and_errors->num_expander_errors; i++) {
      fgets( line, sizeof(line), results_file_fp);
      strcpy( att->warnings_and_errors->model_expander_errors[i], line);
    }

    for (i=0; i<2; i++)
      fgets( line, sizeof(line), results_file_fp);
    sscanf( line, "%e %e %d", &att->rel_error, &att->error, &att->missing);
  }
}


/* READ_TPARM_DS
   18jan95 wmt: new

   read and allocate space for tparms
   */
void read_tparm_DS( tparm_DS tparm, int n_parm, FILE *results_file_fp, int file_ac_version)
{
  int i, file_n_parm, tppt;
  fxlstr line;
  shortstr token1;

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s %d", token1, &file_n_parm);
  if ((eqstring( token1, "tparm_DS") != TRUE) || (file_n_parm != n_parm)) {
    fprintf( stderr, "ERROR: expecting \"tparm_DS\" and n_parm = %d, found \"%s\"\n",
            n_parm, line);
    abort();
  }
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%d %d", &tparm->n_atts, &tppt);
  tparm->tppt = (enum MODEL_TYPES) tppt;
  tparm->collect = 0;
  tparm->wts = NULL;
  tparm->data = NULL;
  tparm->att_indices = NULL;
  tparm->datum = NULL;

  switch(tparm->tppt) {
  case  SM:
    read_sm_params( &(tparm->ptype.sm), tparm->n_atts, results_file_fp,
                   file_ac_version);
    break;
  case  SN_CM:
    read_sn_cm_params( &(tparm->ptype.sn_cm), results_file_fp, file_ac_version);
    break;
  case  SN_CN:
    read_sn_cn_params( &(tparm->ptype.sn_cn), results_file_fp, file_ac_version);
    break;
  case MM_D:
    read_mm_d_params( &(tparm->ptype.mm_d), tparm->n_atts, results_file_fp,
                     file_ac_version);
    break;
  case MM_S:
    read_mm_s_params( &(tparm->ptype.mm_s), tparm->n_atts, results_file_fp,
                     file_ac_version);
    break;
  case MN_CN:
    read_mn_cn_params( &(tparm->ptype.mn_cn), tparm->n_atts, results_file_fp,
                      file_ac_version);
    break;
  default:
    printf("\n read_tparms_DS: unknown type of ENUM MODEL_TYPES =%d\n", tparm->tppt);
    abort();
  }

  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%d %d %d %d %d", &tparm->n_term, &tparm->n_att, &tparm->n_att_indices,
         &tparm->n_datum, &tparm->n_data);
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e %e", &tparm->w_j, &tparm->ranges, &tparm->class_wt,
         &tparm->disc_scale);
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e %e %e", &tparm->log_pi, &tparm->log_att_delta,
         &tparm->log_delta, &tparm->wt_m, &tparm->log_marginal);
}


/* READ_MM_D_PARAMS
   19jan95 wmt: new

   read mm_d params from ascii file
   */
void read_mm_d_params( struct mm_d_param *param, int n_atts, FILE *results_file_p,
                      int file_ac_version)
{
  /* int i, m; */

  fprintf( stderr, "read_mm_d_params not converted from write_mm_d_params\n");
  abort();

/*   fprintf(results_file_p, "mm_d_params\n"); */

/*   for(i=0; i<n_atts; i++) { */
/*     m = param->sizes[i];  */
/*     printf("row %d, size %d\n", i, m); */
/*     fprintf(results_file_p, "wts\n"); */
/*     write_vector_float(param->wts[i], m, results_file_p); */
/*     fprintf(results_file_p, "probs\n"); */
/*     write_vector_float(param->probs[i], m, results_file_p); */
/*     fprintf(results_file_p, "log_probs\n"); */
/*     write_vector_float(param->log_probs[i], m, results_file_p);             */
/*   } */

/*   fprintf(results_file_p, "wts_vec\n"); */
/*   write_vector_float(param->wts_vec, m, results_file_p); */
/*   fprintf(results_file_p, "probs_vec\n"); */
/*   write_vector_float(param->probs_vec, m, results_file_p); */
/*   fprintf(results_file_p, "log_probs_vec\n"); */
/*   write_vector_float(param->log_probs_vec, m, results_file_p);             */
}


/* READ_MM_S_PARAMS
   19jan95 wmt: new

   write mm_s_params to ascii file -- incomplete
   */
void read_mm_s_params( struct mm_s_param *param, int n_atts, FILE *results_file_p,
                      int file_ac_version)
{
    fprintf( stderr, "read_mm_s_params not converted from write_mm_s_params\n");
    abort();

/*   fprintf(results_file_p, "mm_s_params\n"); */

/*   fprintf(results_file_p, "count, wt, prob, log_prob\n"); */
/*   fprintf(results_file_p, "%d %.7e %.7e %.7e\n", param->count, param->wt, param->prob, */
/*           param->log_prob); */
}


/* READ_MN_CN_PARAMS
   19jan95 wmt: new

   write mn_cn_params to ascii file, allocating storage
   */
void read_mn_cn_params( struct mn_cn_param *param, int n_atts, FILE *results_file_fp,
                       int file_ac_version)
{
  int i, j;
  fxlstr line;
  shortstr token1;
  
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e", &param->ln_root, &param->log_ranges);

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "emp_means") != TRUE) {
    fprintf( stderr, "read_mn_cn_params expected \"emp_means\", read \"%s\"\n",
            line);
    abort();
  }
  param->emp_means = (float *) malloc( n_atts * sizeof( float));
  read_vector_float( param->emp_means, n_atts, results_file_fp);

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "emp_covar") != TRUE) {
    fprintf( stderr, "read_mn_cn_params expected \"emp_covar\", read \"%s\"\n",
            line);
    abort();
  }
  param->emp_covar = (fptr *) malloc( n_atts * sizeof( fptr));
  for (i=0; i<n_atts; i++) 
    param->emp_covar[i] = (float *) malloc( n_atts * sizeof( float));
  read_matrix_float( param->emp_covar, n_atts, n_atts, results_file_fp);

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "means") != TRUE) {
    fprintf( stderr, "read_mn_cn_params expected \"means\", read \"%s\"\n",
            line);
    abort();
  }
  param->means = (float *) malloc( n_atts * sizeof( float));
  read_vector_float( param->means, n_atts, results_file_fp);
  
  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "covariance") != TRUE) {
    fprintf( stderr, "read_mn_cn_params expected \"covariance\", read \"%s\"\n",
            line);
    abort();
  }
  param->covariance = (fptr *) malloc( n_atts * sizeof( fptr));
  for (i=0; i<n_atts; i++) 
    param->covariance[i] = (float *) malloc( n_atts * sizeof( float));
  read_matrix_float(param->covariance, n_atts, n_atts, results_file_fp);

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "factor") != TRUE) {
    fprintf( stderr, "read_mn_cn_params expected \"factor\", read \"%s\"\n",
            line);
    abort();
  }
  param->factor = (fptr *) malloc( n_atts * sizeof( fptr));
  for (i=0; i<n_atts; i++) 
    param->factor[i] = (float *) malloc( n_atts * sizeof( float));
  read_matrix_float(param->factor, n_atts, n_atts, results_file_fp);

    param->values = (float *) malloc( n_atts * sizeof( float));
  for (i=0; i<n_atts; i++)
    param->values[i] = 0.0;

  param->temp_v = (float *) malloc( n_atts * sizeof( float));
  for (i=0; i<n_atts; i++)
    param->temp_v[i] = 0.0;

  param->temp_m = (fptr *) malloc( n_atts * sizeof( fptr));
  for (i=0; i<n_atts; i++) {
    param->temp_m[i] = (float *) malloc( n_atts * sizeof( float));
    for (j=0; j<n_atts; j++)
      param->temp_m[i][j] = 0.0;
  }

  fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "min_sigma_2s") != TRUE) {
    fprintf( stderr, "read_mn_cn_params expected \"min_sigma_2s\", read \"%s\"\n",
            line);
    abort();
  }
  param->min_sigma_2s = (float *) malloc( n_atts * sizeof( float));
  read_vector_float( param->min_sigma_2s, n_atts, results_file_fp);
}


/* READ_SM_PARAMS
   19jan95 wmt: new

   write sm_params to ascii file, allocating storage

   n_atts is actually n_vals -- an overloaded slot definition
   */
void read_sm_params( struct sm_param *param, int n_atts, FILE *results_file_fp,
                    int file_ac_version)
{
  int i;
  fxlstr line;
  shortstr token1;

  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %d %e %e %e", &param->gamma_term, &param->range,
          &param->range_m1, &param->inv_range, &param->range_factor);

  fgets( line, sizeof( line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "val_wts") != TRUE) {
    fprintf( stderr, "read_sm_params expected \"val_wts\", read \"%s\"\n",
            line);
    abort();
  }
  param->val_wts = (float *) malloc( n_atts * sizeof( float));
  read_vector_float( param->val_wts, n_atts, results_file_fp);

  fgets( line, sizeof( line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "val_probs") != TRUE) {
    fprintf( stderr, "read_sm_params expected \"val_probs\", read \"%s\"\n",
            line);
    abort();
  }
  param->val_probs = (float *) malloc( n_atts * sizeof( float));
  read_vector_float( param->val_probs, n_atts, results_file_fp);

  fgets( line, sizeof( line), results_file_fp);
  sscanf( line, "%s", token1);
  if (eqstring( token1, "val_log_probs") != TRUE) {
    fprintf( stderr, "read_sm_params expected \"val_log_probs\", read \"%s\"\n",
            line);
    abort();
  }
  param->val_log_probs = (float *) malloc( n_atts * sizeof( float));
  read_vector_float( param->val_log_probs, n_atts, results_file_fp); 
}


/* READ_SN_CM_PARAMS
   19jan95 wmt: new

   write sn_cm_params to ascii file, allocating storage
   */
void read_sn_cm_params( struct sn_cm_param *param, FILE *results_file_fp,
                       int file_ac_version)
{
  int i;
  fxlstr line;

  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e %e", &param->known_wt, &param->known_prob,
         &param->known_log_prob, &param->unknown_log_prob);  
   
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e", &param->weighted_mean, &param->weighted_var,
          &param->mean);
   
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e %e %e", &param->sigma, &param->log_sigma,
          &param->variance, &param->log_variance, &param->inv_variance);
   
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e\n", &param->ll_min_diff, &param->skewness, &param->kurtosis);

  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, " %e %e %e", &param->prior_sigma_min_2, &param->prior_mean_mean,
          &param->prior_mean_sigma);
    
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e %e", &param->prior_sigmas_term, &param->prior_sigma_max_2,
          &param->prior_mean_var, &param->prior_known_prior);
}


/* READ_SN_CN_PARAMS
   19jan95 wmt: new

   write sn_cn_params to ascii file
   */
void read_sn_cn_params( struct sn_cn_param *param, FILE *results_file_fp,
                       int file_ac_version)
{
  int i;
  fxlstr line;

  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e", &param->weighted_mean, &param->weighted_var,
         &param->mean);
   
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e %e %e", &param->sigma, &param->log_sigma,
         &param->variance, &param->log_variance, &param->inv_variance);

  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e", &param->ll_min_diff, &param->skewness, &param->kurtosis);

  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, " %e %e %e", &param->prior_sigma_min_2, &param->prior_mean_mean,
         &param->prior_mean_sigma);
    
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), results_file_fp);
  sscanf( line, "%e %e %e", &param->prior_sigmas_term, &param->prior_sigma_max_2,
         &param->prior_mean_var);
}
