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


/* SAFE_FWRITE
   13mar95 wmt: new
   16may95 wmt: converted to ANSI i/o

   this is a wrapper to check for error returns from write
   */
void safe_fwrite( FILE *save_fp, char *data, int data_length,
                int data_type, char *caller)
{
  int return_cnt, header[2];
  int header_length = sizeof( header);

  /* wmtdebug 
  fprintf( stderr, "safe_fwrite: save_fp %p, data_length %d, data %p, caller %s\n",
           save_fp, data_length, data, caller); fflush( stderr);
           */
  if (data_type == CHAR_TYPE)
    data_length++;            /* include '\0' */
  header[0] = data_type;
  header[1] = data_length;
  return_cnt = fwrite( (char *) &header, sizeof( char), header_length, save_fp);

  if (return_cnt != header_length) {
    fprintf( stderr, "ERROR: fwrite failed -- called by %s\n", caller);
    abort();
  }
  if (data_length > 0) {
    return_cnt = fwrite( data, sizeof( char), data_length, save_fp);

    if (return_cnt != data_length) {
      fprintf( stderr, "ERROR: write failed -- called by %s\n",
              caller);
      abort();
    }
  }
}


/* CHECK_LOAD_HEADER
   17mar95 wmt: new
   */
void check_load_header( int header_type, int expected_type, char *caller)
{
  if (header_type != expected_type) {
    fprintf( stderr, "ERROR: in %s, expecting data type %d, found %d \n", caller,
            expected_type, header_type);
    abort();
  }
}

/* DUMP_CLSF_SEQ
   13mar95 wmt: support binary clsf write
   */
void dump_clsf_seq( clsf_DS *clsf_seq, int num, FILE *results_fp)
{
  int i;
  fxlstr str;
  char caller[] = "dump_clsf_seq";

  sprintf( str, "# ordered sequence of clsf_DS's: 0 -> %d", num - 1);
  safe_fwrite( results_fp, str, strlen( str), CHAR_TYPE, caller);

  for (i=0; i<num; i++) {
    sprintf( str, "# clsf_DS %d: log_a_x_h = %.7e", i,
            clsf_seq[i]->log_a_x_h);
    safe_fwrite( results_fp, str, strlen( str), CHAR_TYPE, caller);
  }

  sprintf( str, "ac_version %s", G_ac_version);
  safe_fwrite( results_fp, str, strlen( str), CHAR_TYPE, caller);

  for (i=0; i<num; i++)
     dump_clsf_DS( clsf_seq[i], results_fp, i);

  /* end of clsfs marker -- data_length = 0 */
  safe_fwrite( results_fp, str, 0, CLASSIFICATION_TYPE, caller);
}


/* DUMP_CLSF_DS
   15mar95 wmt: new

   write binary clsf
*/
void dump_clsf_DS( clsf_DS clsf, FILE *results_fp, int clsf_num)
{
  int i;
  char caller[] = "dump_clsf_DS", db_string[] = "database_DS_ptr";
  char model_string[] = "model_DS_ptr";
  shortstr model_num_string;

  safe_fwrite( results_fp, (char *) clsf, sizeof( struct classification),
             CLASSIFICATION_TYPE, caller);

  if (clsf_num == 0)
    dump_database_DS( clsf->database, results_fp);
  else
    safe_fwrite( results_fp, db_string, strlen( db_string), CHAR_TYPE, caller);

  for (i=0; i<clsf->num_models; i++) {
    if (clsf_num == 0)
      dump_model_DS( clsf->models[i], i, clsf->database, results_fp);
    else {
      model_num_string[0] = '\0';
      sprintf( model_num_string, "%s %d", model_string, i);
      safe_fwrite( results_fp, model_num_string, strlen( model_num_string),
                 CHAR_TYPE, caller);
    }
  }

  dump_class_DS_s( clsf->classes, clsf->n_classes, results_fp);

  /* clsf->reports is only used for report generation - do not output */

  safe_fwrite( results_fp, (char *) clsf->checkpoint, sizeof( struct checkpoint),
             CHECKPOINT_TYPE, caller);
}


/* DUMP_DATABASE_DS
   15mar95 wmt: new

   write compressed database_DS contents in binary
   */  
void dump_database_DS( database_DS database, FILE *results_fp)
{
  int i;
  char caller[] = "dump_database_DS";

  safe_fwrite( results_fp, (char *) database, sizeof( struct database),
             DATABASE_TYPE, caller);

  /* Ordered N-atts vector of att_DS describing the attributes. */
  for (i=0; i<database->n_atts; i++)
    dump_att_DS( database->att_info[i], i, results_fp);
}


/* DUMP_ATT_DS
   15mar95 wmt: new

   write att_DS contents in binary
   */  
void dump_att_DS( att_DS att_info, int n_att, FILE *results_fp){

  discrete_stats_DS discrete_stats;
  warn_err_DS warnings_and_errors;
  int i;
  char caller[] = "dump_att_DS";
  fxlstr props_string;

  safe_fwrite( results_fp, (char *) att_info, sizeof( struct att), ATT_TYPE, caller);

  if (eqstring(att_info->type, "real")) 
    safe_fwrite( results_fp, (char *) att_info->r_statistics, sizeof( struct real_stats),
               REAL_STATS_TYPE, caller);
  else if (eqstring(att_info->type, "discrete")) {
    discrete_stats = att_info->d_statistics;
    safe_fwrite( results_fp, (char *) discrete_stats, sizeof( struct discrete_stats),
               DISCRETE_STATS_TYPE, caller);
    safe_fwrite( results_fp, (char *) discrete_stats->observed,
               discrete_stats->range * sizeof( int), INT_TYPE, caller);
  }
  else if (eqstring(att_info->type, "dummy"))
    safe_fwrite( results_fp, props_string, 0, DUMMY_STATS_TYPE, caller);
  else {
    fprintf(stderr, "ERROR: att_info->type %s not handled\n",
            att_info->type);
    exit(1);
  }

  if (! eqstring(att_info->type, "dummy")) {
    for (i=0; i < att_info->n_trans; i++)
      safe_fwrite( results_fp, (char *) att_info->translations[i],
                 strlen( att_info->translations[i]), CHAR_TYPE, caller);

    for (i=0; i < att_info->n_props; i++) {
      props_string[0] = '\0';
      if (eqstring( (char *) att_info->props[i][2], "int") == TRUE) {
        sprintf( props_string, "%s %s %d", (char *) att_info->props[i][0],
                (char *) att_info->props[i][2], *((int *) att_info->props[i][1]));
        safe_fwrite( results_fp, props_string, strlen( props_string), CHAR_TYPE, caller);
      }
      else if (eqstring( (char *) att_info->props[i][2], "flt") == TRUE) {
        sprintf( props_string, "%s %s %f", (char *) att_info->props[i][0],
                (char *) att_info->props[i][2], *((float *) att_info->props[i][1]));
        safe_fwrite( results_fp, props_string, strlen( props_string), CHAR_TYPE, caller);
      }
      else if (eqstring( (char *) att_info->props[i][2], "str") == TRUE) {
        sprintf( props_string, "%s %s %s", (char *) att_info->props[i][0],
                (char *) att_info->props[i][2], (char *) att_info->props[i][1]);
        safe_fwrite( results_fp, props_string, strlen( props_string), CHAR_TYPE, caller);
      }
      else {
        fprintf( stderr, "property list type %s, not handled!\n",
                (char *) att_info->props[i][2]);
        abort();
      }
    }
    
    warnings_and_errors = att_info->warnings_and_errors;
    safe_fwrite( results_fp, (char *) warnings_and_errors, sizeof( struct warn_err),
               WARN_ERR_TYPE, caller);
    safe_fwrite( results_fp,
               eqstring( warnings_and_errors->unspecified_dummy_warning, "")
               ? "NULL" : warnings_and_errors->unspecified_dummy_warning,
               eqstring( warnings_and_errors->unspecified_dummy_warning, "")
               ? 4 : strlen( warnings_and_errors->unspecified_dummy_warning),
               CHAR_TYPE, caller);
    safe_fwrite( results_fp,
               eqstring( warnings_and_errors->single_valued_warning, "")
               ? "NULL" : warnings_and_errors->single_valued_warning,
               eqstring( warnings_and_errors->single_valued_warning, "")
               ? 4 : strlen( warnings_and_errors->single_valued_warning),
               CHAR_TYPE, caller);
    /* float *unused_translators_warning; discrete translations not implementated */
    for (i=0; i < warnings_and_errors->num_expander_warnings; i++)
      safe_fwrite( results_fp, warnings_and_errors->model_expander_warnings[i],
                 strlen( warnings_and_errors->model_expander_warnings[i]),
                 CHAR_TYPE, caller);
    for (i=0; i < warnings_and_errors->num_expander_errors; i++)
      safe_fwrite( results_fp, warnings_and_errors->model_expander_errors[i],
                 strlen( warnings_and_errors->model_expander_errors[i]),
                 CHAR_TYPE, caller);
  }
}


/* DUMP_MODEL_DS
   15mar95 wmt: new

   write model_DS contents in binary - one or more models -- in compressed form
   */  
void dump_model_DS( model_DS model, int model_num, database_DS database,
                  FILE *results_fp)
{
  char caller[] = "dump_model_DS";

  safe_fwrite( results_fp, (char *) model, sizeof( struct model), MODEL_TYPE, caller);
}


/* DUMP_TERM_DS
   15mar95 wmt: new

   write term_DS to binary file
   */
void dump_term_DS( term_DS term, int n_term, FILE *results_fp)
{
  int parm_num = 0;
  char caller[] = "dump_term_DS";

  safe_fwrite( results_fp, (char *) term, sizeof( struct term), TERM_TYPE, caller);

  safe_fwrite( results_fp, (char *) term->att_list, term->n_atts * sizeof( float),
             FLOAT_TYPE, caller);

  dump_tparm_DS( term->tparm, parm_num, results_fp);
}


/* DUMP_TPARM_DS
   15mar95 wmt: new

   write tparm_DS (term params) to binary file
   */
void dump_tparm_DS( tparm_DS term_param, int parm_num, FILE *results_fp)

{
  char caller[] = "dump_tparm_DS";

  safe_fwrite( results_fp, (char *) term_param, sizeof( struct new_term_params),
             TPARM_TYPE, caller);

  switch(term_param->tppt) {
  case  SM:
    dump_sm_params( &(term_param->ptype.sm), term_param->n_atts, results_fp);
    break;
  case  SN_CM:
    /* nothing to do
       dump_sn_cm_params( &(term_param->ptype.sn_cm), results_fp); */
    break;
  case  SN_CN:
    /* nothing to do
       dump_sn_cn_params( &(term_param->ptype.sn_cn), results_fp); */
    break;
  case MM_D:
    dump_mm_d_params( &(term_param->ptype.mm_d), term_param->n_atts, results_fp);
    break;
  case MM_S:
    dump_mm_s_params( &(term_param->ptype.mm_s), term_param->n_atts, results_fp);
    break;
  case MN_CN:
    dump_mn_cn_params( &(term_param->ptype.mn_cn), term_param->n_atts, results_fp);
    break;
  default:
    printf("\n dump_tparms_DS: unknown type of ENUM MODEL_TYPES =%d\n", term_param->tppt);
    abort();
  }
}


/* DUMP_MM_D_PARAMS
   15mar95 wmt: new

   write mm_d params to binary file - not converted from write_mm_d_params
   */
void dump_mm_d_params( struct mm_d_param *param, int n_atts, FILE *results_fp)
{
  /* int i, m;
  char caller[] = "dump_mm_d_params"; */

  /*
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
  */
}


/* DUMP_MM_S_PARAMS
   15mar95 wmt: new

   write mm_s_params to ascii file -- incomplete
   */
void dump_mm_s_params( struct mm_s_param *param, int n_atts, FILE *results_fp)
{
  /* char caller[] = "dump_mm_s_params"; */

  /*
  safe_fprintf(stream, caller, "mm_s_params\n");

  safe_fprintf(stream, caller, "count, wt, prob, log_prob\n");
  safe_fprintf(stream, caller, "%d %.7e %.7e %.7e\n", param->count, param->wt,
        param->prob, param->log_prob);
          */
}


/* DUMP_MN_CN_PARAMS
   15mar95 wmt: new

   write mn_cn_params to binary file
   */
void dump_mn_cn_params( struct mn_cn_param *param, int n_atts, FILE *results_fp)
{
  char caller[] = "dump_mn_cn_params";
  int i;

  safe_fwrite( results_fp, (char *) param->emp_means, n_atts * sizeof( float),
             FLOAT_TYPE, caller);
  for (i=0; i<n_atts; i++)
    safe_fwrite( results_fp, (char *) param->emp_covar[i], n_atts * sizeof( float),
               FLOAT_TYPE, caller);
  safe_fwrite( results_fp, (char *) param->means, n_atts * sizeof( float), FLOAT_TYPE,
             caller);
  for (i=0; i<n_atts; i++)
    safe_fwrite( results_fp, (char *) param->covariance[i], n_atts * sizeof( float),
               FLOAT_TYPE, caller);
  for (i=0; i<n_atts; i++)
    safe_fwrite( results_fp, (char *) param->factor[i], n_atts * sizeof( float),
               FLOAT_TYPE, caller);
  safe_fwrite( results_fp, (char *) param->min_sigma_2s, n_atts * sizeof( float),
             FLOAT_TYPE, caller);
  /* values, temp_v & temp_m are temporary storage - do not save, just reinit to 0.0 */
}


/* DUMP_SM_PARAMS
   15mar95 wmt: new

   write sm_params to binary file

   n_atts is actually n_vals -- an overloaded slot definition
   */
void dump_sm_params( struct sm_param *param, int n_atts, FILE *results_fp)
{
  char caller[] = "dump_sm_params";

  safe_fwrite( results_fp, (char *) param->val_wts, n_atts * sizeof( float),
             FLOAT_TYPE, caller);
  safe_fwrite( results_fp, (char *) param->val_probs, n_atts * sizeof( float),
             FLOAT_TYPE, caller);
  safe_fwrite( results_fp, (char *) param->val_log_probs, n_atts * sizeof( float),
             FLOAT_TYPE, caller);
}


/* DUMP_CLASS_DS_S
   16mar95 wmt: new

   write class_DS to binary file. 
   */
void dump_class_DS_s( class_DS *classes, int n_classes, FILE *results_fp)
{
  int i, j;
  char caller[] = "dump_class_DS_s";

  for (i=0; i < n_classes; i++) {

    safe_fwrite( results_fp, (char *) classes[i], sizeof( struct class), CLASS_TYPE,
               caller);
   
    for (j=0; j<classes[i]->num_tparms; j++) 
      dump_tparm_DS( classes[i]->tparms[j], j, results_fp);

    /* num_i_values, i_values, i_sum, & max_i_value used only in reports -
       do not output */
    safe_fwrite( results_fp, (char *) &classes[i]->model->file_index, sizeof( int),
               INT_TYPE, caller);
  }
}


/* LOAD_CLSF_SEQ
   13mar95 wmt: read binary file
   16may95 wmt: converted binary i/o to ANSI
   20sep98 wmt: strip of win/unx from ac_version, starting with version 3.3
   27nov98 wmt: check for win/unx before stripping -- backward compatible
   */
clsf_DS *load_clsf_seq( FILE *results_file_fp, char * results_file_ptr, int expand_p,
                       int want_wts_p, int update_wts_p, int *n_best_clsfs_ptr,
                       int_list expand_list) 
{
  int length = 0, file_ac_version, float_p, str_length = 2 * sizeof( fxlstr);
  int header[2], header_length = 2 * sizeof( int), token_length;
  clsf_DS clsf, first_clsf = NULL, *seq = NULL;
  shortstr token1, token2;
  fxlstr line;
  char *str;
  char caller[] = "load_clsf_seq";

  str = (char *) malloc( str_length);
  do {                  /* read comment lines */
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], CHAR_TYPE, caller);
    fread( &line, sizeof( char), header[1], results_file_fp);
  }
  while (line[0] == '#');

  sscanf( line, "%s %s", token1, token2);
  /* strip of win/unx from ac_version, if present */
  if (strstr( token2, "unx") || strstr( token2, "win")) {
    token_length = strlen( token2);
    token2[token_length - 3] = '\0';
  }
  if ((eqstring( token1, "ac_version") != TRUE) ||
      ((file_ac_version = atof_p( token2, &float_p)) < 1.0) ||
      (float_p != TRUE)) {
    fprintf( stderr, "ERROR: expecting \"ac_version n.n\", found \"%s %s\" \n",
            token1, token2);
    abort();    
  }

  while (1) {
    clsf = load_clsf( results_file_fp, expand_p, want_wts_p, update_wts_p, length,
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
  safe_sprintf( str, str_length, caller,
               "ADVISORY: loaded %d classification%s from \n          %s%s\n",
               length, (length == 1) ? "" : "s",
               (results_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
               results_file_ptr);
  to_screen_and_log_file( str, G_log_file_fp, G_stream, TRUE);
  free( str);
  return(seq);
}


/* LOAD_CLSF
   16mar95 wmt: add loading capability
   16may95 wmt: converted binary i/o to ANSI

   Intended to read a compactly represented classification as written by
   write_clsf_DS, and to optionally expand it to standard form.  Anything else,
   including non-compact classifications, are returned without modification.
   Compact classification are identified by the presence of a list of filenames
   in the database field, instead of a database structure.  With Expand, Wts or
   Update_Wts regenerates the wts vectors. update_wts also updates wts.
   */
clsf_DS load_clsf( FILE *results_file_fp, int expand_p, int want_wts_p, int update_wts_p,
                  int clsf_index, clsf_DS first_clsf, int file_ac_version,
                  int_list expand_list)
{
  clsf_DS clsf = NULL;
  shortstr token1;
  fxlstr line;
  int model_index;
  model_DS *models;
  int header[2], header_length = 2 * sizeof( int);
  char caller[] = "load_clsf";

  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], CLASSIFICATION_TYPE, caller);
  if (header[1] > 0) {
    clsf = (clsf_DS) malloc( sizeof( struct classification));
    fread( clsf, sizeof( char), header[1], results_file_fp);

    if (first_clsf == NULL)
      clsf->database = load_database_DS( clsf, results_file_fp, file_ac_version);
    else {
      fread( &header, sizeof( char), header_length, results_file_fp);
      check_load_header( header[0], CHAR_TYPE, caller);
      fread( line, sizeof( char), header[1], results_file_fp);
      sscanf( line, "%s", token1);
      if (eqstring( token1, "database_DS_ptr") != TRUE) {
        fprintf( stderr, "ERROR: expecting \"database_DS_ptr\", found \"%s\"\n", line);
        abort();
      }
      clsf->database = first_clsf->database;
    }

    if (first_clsf == NULL)
      models = (model_DS *) malloc( clsf->num_models * sizeof( model_DS));
    else
      models = first_clsf->models;
    clsf->models = models;

    for (model_index=0; model_index<clsf->num_models; model_index++) {
      if (first_clsf == NULL)
        models[model_index] = load_model_DS( clsf, model_index, results_file_fp,
                                            file_ac_version);
      else {
        fread( &header, sizeof( char), header_length, results_file_fp);
        check_load_header( header[0], CHAR_TYPE, caller);
        fread( line, sizeof( char), header[1], results_file_fp);
        sscanf( line, "%s", token1);
        if (eqstring( token1, "model_DS_ptr") != TRUE) {
          fprintf( stderr, "ERROR: expecting \"model_DS_ptr\", found \"%s\"\n", line);
          abort();
        }
      }
    }

    load_class_DS_s( clsf, clsf->n_classes, results_file_fp,
                    (first_clsf == NULL) ? clsf : first_clsf,
                    file_ac_version);

    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], CHECKPOINT_TYPE, caller);
    clsf->checkpoint = (chkpt_DS) malloc( sizeof( struct checkpoint));
    fread( clsf->checkpoint, sizeof( char), header[1], results_file_fp);

    clsf->next = NULL;
    if ((expand_p == TRUE) && ((expand_list[0] == END_OF_INT_LIST) ||
                             ((expand_list[0] != END_OF_INT_LIST) &&
                              (member_int_list( clsf_index+1, expand_list) == TRUE)))) {
      expand_clsf( clsf, want_wts_p, update_wts_p);
 
      /* fprintf( stderr, "clsf index %d expanded\n", clsf_index); */
    }
  }
  return (clsf);
}


/* LOAD_DATABASE_DS
   16mar95 wmt: new
   16may95 wmt: converted binary i/o to ANSI
   21jun95 wmt: initialize realloc'ed att_info

   load database_DS from results file - for first clsf only
   */
database_DS load_database_DS( clsf_DS clsf, FILE *results_file_fp, int file_ac_version)
{
  int n_att, i;
  database_DS d_base;
  int header[2], header_length = 2 * sizeof( int);
  char caller[] = "load_database_DS";


  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], DATABASE_TYPE, caller);
  d_base = (database_DS) malloc( sizeof( struct database));
  fread( d_base, sizeof( char), header[1], results_file_fp);
  d_base->att_info = (att_DS *) malloc( d_base->allo_n_atts * sizeof( att_DS));
  if (d_base->n_atts > d_base->allo_n_atts) {
    d_base->allo_n_atts = d_base->n_atts;
    d_base->att_info = (att_DS *) realloc( d_base->att_info,
                                          d_base->allo_n_atts * sizeof( att_DS));
    for (i=0; i<d_base->n_atts; i++) 
      d_base->att_info[i] = NULL;
  }  
  /* Ordered N-atts vector of att_DS describing the attributes. */
  for (n_att=0; n_att<d_base->n_atts; n_att++)
    load_att_DS( d_base, n_att, results_file_fp, file_ac_version);

  d_base->compressed_p = TRUE;

  return( d_base);
}


/* LOAD_ATT_DS
   16mar95 wmt: new

   load att_DS from results file and allocate storage
   16may95 wmt: converted binary i/o to ANSI
   */
void load_att_DS( database_DS d_base, int n_att, FILE *results_file_fp, int file_ac_version)
{
  int i, *int_value, n_props;
  int header[2], header_length = 2 * sizeof( int);
  float *float_value;
  char *string_value, *token_ptr, caller[] = "load_att_DS";

  fxlstr line, token1, token2, token3;
  att_DS att;
  discrete_stats_DS discrete_stats;
        
  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], ATT_TYPE, caller);
  att = (att_DS) malloc(sizeof(struct att));
  fread( att, sizeof( char), header[1], results_file_fp);
  d_base->att_info[n_att] = att;
  n_props = att->n_props;

  if (eqstring(att->type, "real")) {
    att->r_statistics = (real_stats_DS) malloc( sizeof( struct real_stats));
    att->d_statistics = NULL;
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], REAL_STATS_TYPE, caller);
    fread( att->r_statistics, sizeof( char), header[1], results_file_fp);
  }
  else if (eqstring(att->type, "discrete")) {
    att->r_statistics = NULL;
    att->d_statistics = (discrete_stats_DS) malloc( sizeof( struct discrete_stats));
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], DISCRETE_STATS_TYPE, caller);
    fread( att->d_statistics, sizeof( char), header[1], results_file_fp);
    discrete_stats = att->d_statistics;
    att->d_statistics->observed = (int *) malloc( discrete_stats->range * sizeof( int));
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], INT_TYPE, caller);
    fread( discrete_stats->observed, sizeof( char), header[1], results_file_fp);
  }
  else if (eqstring( att->type, "dummy")) {
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], DUMMY_STATS_TYPE, caller);
    att->r_statistics = NULL; att->d_statistics = NULL;
    att->props = NULL; 
    att->translations = NULL; 
    att->warnings_and_errors = NULL;
  }
  else {   
    fprintf(stderr, "\nERROR: att_info->type %s not handled\n",
            att->type);
    abort();
  }

  if (! eqstring( att->type, "dummy")) {
    att->n_props = 0;
    att->props = NULL;
    /* att->n_props is incremented by add_to_plist */
    att->translations = NULL; 
    if (att->n_trans > 0) {
      att->translations = (char **) malloc( att->n_trans * sizeof( char *));
      for (i=0; i < att->n_trans; i++) {
        att->translations[i] = (char *) malloc( sizeof( shortstr));
        fread( &header, sizeof( char), header_length, results_file_fp);
        check_load_header( header[0], CHAR_TYPE, caller);
        fread( att->translations[i], sizeof( char), header[1], results_file_fp);
      }
    }
    /* props_DS - att->n_props is incremented by add_to_plist */
    if (n_props > 0) {
      for (i=0; i < n_props; i++) {
        fread( &header, sizeof( char), header_length, results_file_fp);
        check_load_header( header[0], CHAR_TYPE, caller);
        fread( &line, sizeof( char), header[1], results_file_fp);
        sscanf( line, "%s %s %s", token1, token2, token3);
        token_ptr = (char *) malloc( sizeof( shortstr));
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
          string_value = (char *) malloc( sizeof( shortstr));
          strcpy( string_value, token3);
          add_to_plist( att, token_ptr, string_value, "str");
        }
        else {
          fprintf( stderr, "property list type %s, not handled!\n", token2);
          abort();
        }
      }
    }

    att->warnings_and_errors = (warn_err_DS) malloc( sizeof( struct warn_err));
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], WARN_ERR_TYPE, caller);
    fread( att->warnings_and_errors, sizeof( char), header[1], results_file_fp);

    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], CHAR_TYPE, caller);
    fread( &token1, sizeof( char), header[1], results_file_fp);
    if (eqstring( token1, "NULL") == TRUE)
      strcpy( att->warnings_and_errors->unspecified_dummy_warning, "");
    else 
      strcpy( att->warnings_and_errors->unspecified_dummy_warning, token1);

    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], CHAR_TYPE, caller);
    fread( &token2, sizeof( char), header[1], results_file_fp);
    if (eqstring( token2, "NULL") == TRUE)
      strcpy( att->warnings_and_errors->single_valued_warning, "");
    else 
      strcpy( att->warnings_and_errors->single_valued_warning, token2);

    /* float *unused_translators_warning; discrete translations not implementated */
    if (att->warnings_and_errors->num_expander_warnings > 0)
      att->warnings_and_errors->model_expander_warnings =
        (fxlstr *) malloc( att->warnings_and_errors->num_expander_warnings *
                          sizeof( fxlstr));
    else
      att->warnings_and_errors->model_expander_warnings = NULL;
    for (i=0; i < att->warnings_and_errors->num_expander_warnings; i++) {
      fread( &header, sizeof( char), header_length, results_file_fp);
      check_load_header( header[0], CHAR_TYPE, caller);
      fread( &att->warnings_and_errors->model_expander_warnings[i], sizeof( char),
           header[1], results_file_fp);
    }

    if (att->warnings_and_errors->num_expander_errors > 0)
      att->warnings_and_errors->model_expander_errors =
        (fxlstr *) malloc( att->warnings_and_errors->num_expander_errors *
                          sizeof( fxlstr));
    else
      att->warnings_and_errors->model_expander_errors = NULL;    
    for (i=0; i < att->warnings_and_errors->num_expander_errors; i++) {
      fread( &header, sizeof( char), header_length, results_file_fp);
      check_load_header( header[0], CHAR_TYPE, caller);
      fread( &att->warnings_and_errors->model_expander_errors[i], sizeof( char),
           header[1], results_file_fp);
    }
  }
}


/* LOAD_MODEL_DS
   17mar95 wmt: new
   16may95 wmt: converted binary i/o to ANSI

   load model_DS from results file and allocate storage - first clsf only
   */
model_DS load_model_DS( clsf_DS clsf, int model_index, FILE *results_file_fp,
                       int file_ac_version)
{
  int header[2], header_length = 2 * sizeof( int);
  model_DS model;
  char caller[] = "load_model_DS";

  model = (model_DS) malloc( sizeof( struct model));

  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], MODEL_TYPE, caller);
  fread( model, sizeof( char), header[1], results_file_fp);

  model->compressed_p = TRUE;
  model->database = clsf->database;

  /* since this is compressed model, set everything to null */
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


/* LOAD_CLASS_DS_S
   17mar95 wmt: new
   16may95 wmt: converted binary i/o to ANSI
   13feb98 wmt: check for malloc/realloc failures

   load class_DS from results file and allocate storage
   */
void load_class_DS_s( clsf_DS clsf, int n_classes, FILE *results_file_fp,
                     clsf_DS first_clsf, int file_ac_version)
{
  int i, n_parm, n_class, file_model_file_index;
  int header[2], header_length = 2 * sizeof( int);
  class_DS *classes;
  char caller[] = "load_class_DS_s";

  classes = (class_DS *) malloc( n_classes * sizeof( class_DS));
  clsf->classes = classes;

  for (n_class=0; n_class < n_classes; n_class++) {

    classes[n_class] = (class_DS) malloc( sizeof( struct class));
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], CLASS_TYPE, caller);
    fread( classes[n_class], sizeof( char), header[1], results_file_fp);

    classes[n_class]->tparms = (tparm_DS *) 
    malloc( classes[n_class]->num_tparms * sizeof( tparm_DS));

    for (n_parm=0; n_parm<classes[n_class]->num_tparms; n_parm++) {

      classes[n_class]->tparms[n_parm] = 
      (tparm_DS) malloc( sizeof( struct new_term_params));

      load_tparm_DS( classes[n_class]->tparms[n_parm], 
                     n_parm, 
		     results_file_fp,
                     file_ac_version);
    }

    classes[n_class]->wts = (float *) 
    malloc( classes[n_class]->num_wts * sizeof( float));

    if (classes[n_class]->wts == NULL) {
      fprintf( stderr,
               "ERROR: load_class_DS_s: out of memory, malloc returned NULL!\n");
      exit(1);
    }
    for (i=0; i<classes[n_class]->num_wts; i++)
      classes[n_class]->wts[i] = 0.0;

    if (G_clsf_storage_log_p == TRUE) {
      fprintf( stdout, "\nload_class_DS: %p, num_wts %d, wts:%p, wts-len:%d\n",
              (void *) classes[n_class], classes[n_class]->num_wts,
              (void *) classes[n_class]->wts,
              classes[n_class]->num_wts * (int) sizeof( float));
      if (G_n_freed_classes > 0)
        G_n_create_classes_after_free++;
    }   
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], INT_TYPE, caller);
    fread( &file_model_file_index, sizeof( char), header[1], results_file_fp);
    for (i=0; i<first_clsf->num_models; i++) {
      if (first_clsf->models[i]->file_index == file_model_file_index) {
        classes[n_class]->model = first_clsf->models[i];
        break;
      }
    }
    classes[n_class]->next = NULL;
  }
}


/* LOAD_TPARM_DS
   18jan95 wmt: new
   16may95 wmt: converted binary i/o to ANSI

   read and allocate space for tparms
   */
void load_tparm_DS( tparm_DS tparm, int n_parm, FILE *results_file_fp, int file_ac_version)
{
  int header[2], header_length = 2 * sizeof( int);
  char caller[] = "load_tparm_DS";

  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], TPARM_TYPE, caller);
  fread( tparm, sizeof( char), header[1], results_file_fp);

  tparm->collect = 0;
  tparm->wts = NULL;
  tparm->data = NULL;
  tparm->att_indices = NULL;
  tparm->datum = NULL;

  switch(tparm->tppt) {
  case  SM:
    load_sm_params( &(tparm->ptype.sm), tparm->n_atts, results_file_fp,
                   file_ac_version);
    break;
  case  SN_CM:
    /* nothing to do
       load_sn_cm_params( &(tparm->ptype.sn_cm), results_file_fp, file_ac_version); */
    break;
  case  SN_CN:
    /* nothing to do
       load_sn_cn_params( &(tparm->ptype.sn_cn), results_file_fp, file_ac_version); */
    break;
  case MM_D:
    load_mm_d_params( &(tparm->ptype.mm_d), tparm->n_atts, results_file_fp,
                     file_ac_version);
    break;
  case MM_S:
    load_mm_s_params( &(tparm->ptype.mm_s), tparm->n_atts, results_file_fp,
                     file_ac_version);
    break;
  case MN_CN:
    load_mn_cn_params( &(tparm->ptype.mn_cn), tparm->n_atts, results_file_fp,
                      file_ac_version);
    break;
  default:
    printf("\n load_tparms_DS: unknown type of ENUM MODEL_TYPES =%d\n", tparm->tppt);
    abort();
  }
}


/* LOAD_MM_D_PARAMS
   17mar95 wmt: new
   16may95 wmt: converted binary i/o to ANSI

   load mm_d params from ascii file
   */
void load_mm_d_params( struct mm_d_param *param, int n_atts, FILE *results_file_fp,
                      int file_ac_version)
{
  /* int i, m; */

  fprintf( stderr, "load_mm_d_params not converted from dump_mm_d_params\n");
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


/* LOAD_MM_S_PARAMS
   17mar95 wmt: new

   load mm_s_params to ascii file -- incomplete
   */
void load_mm_s_params( struct mm_s_param *param, int n_atts, FILE *results_file_fp,
                      int file_ac_version)
{
    fprintf( stderr, "load_mm_s_params not converted from dump_mm_s_params\n");
    abort();

/*   fprintf(results_file_p, "mm_s_params\n"); */

/*   fprintf(results_file_p, "count, wt, prob, log_prob\n"); */
/*   fprintf(results_file_p, "%d %.7e %.7e %.7e\n", param->count, param->wt, param->prob, */
/*           param->log_prob); */
}


/* LOAD_MN_CN_PARAMS
   17mar95 wmt: new
   16may95 wmt: converted binary i/o to ANSI

   load mn_cn_params to ascii file, allocating storage
   */
void load_mn_cn_params( struct mn_cn_param *param, int n_atts, FILE *results_file_fp,
                       int file_ac_version)
{
  int i, j;
  int header[2], header_length = 2 * sizeof( int);
  char caller[] = "load_mn_cn_params";
  
  param->emp_means = (float *) malloc( n_atts * sizeof( float));
  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], FLOAT_TYPE, caller);
  fread( param->emp_means, sizeof( char), header[1], results_file_fp);

  param->emp_covar = (fptr *) malloc( n_atts * sizeof( fptr));
  for (i=0; i<n_atts; i++) {
    param->emp_covar[i] = (float *) malloc( n_atts * sizeof( float));
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], FLOAT_TYPE, caller);
    fread( param->emp_covar[i], sizeof( char), header[1], results_file_fp);
  }

  param->means = (float *) malloc( n_atts * sizeof( float));
  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], FLOAT_TYPE, caller);
  fread( param->means, sizeof( char), header[1], results_file_fp);
  
  param->covariance = (fptr *) malloc( n_atts * sizeof( fptr));
  for (i=0; i<n_atts; i++) {
    param->covariance[i] = (float *) malloc( n_atts * sizeof( float));
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], FLOAT_TYPE, caller);
    fread( param->covariance[i], sizeof( char), header[1], results_file_fp);
  }

  param->factor = (fptr *) malloc( n_atts * sizeof( fptr));
  for (i=0; i<n_atts; i++) {
    param->factor[i] = (float *) malloc( n_atts * sizeof( float));
    fread( &header, sizeof( char), header_length, results_file_fp);
    check_load_header( header[0], FLOAT_TYPE, caller);
    fread( param->factor[i], sizeof( char), header[1], results_file_fp);
  }

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
  param->min_sigma_2s = (float *) malloc( n_atts * sizeof( float));
  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], FLOAT_TYPE, caller);
  fread( param->min_sigma_2s, sizeof( char), header[1], results_file_fp);
}


/* LOAD_SM_PARAMS
   17mar95 wmt: new
   16may95 wmt: converted binary i/o to ANSI

   load sm_params to ascii file, allocating storage

   n_atts is actually n_vals -- an overloaded slot definition
   */
void load_sm_params( struct sm_param *param, int n_atts, FILE *results_file_fp,
                    int file_ac_version)
{
  int header[2], header_length = 2 * sizeof( int);
  char caller[] = "load_sm_params";

  param->val_wts = (float *) malloc( n_atts * sizeof( float));
  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], FLOAT_TYPE, caller);
  fread( param->val_wts, sizeof( char), header[1], results_file_fp);

  param->val_probs = (float *) malloc( n_atts * sizeof( float));
  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], FLOAT_TYPE, caller);
  fread( param->val_probs, sizeof( char), header[1], results_file_fp);

  param->val_log_probs = (float *) malloc( n_atts * sizeof( float));
  fread( &header, sizeof( char), header_length, results_file_fp);
  check_load_header( header[0], FLOAT_TYPE, caller);
  fread( param->val_log_probs, sizeof( char), header[1], results_file_fp);
}

