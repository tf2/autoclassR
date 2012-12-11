#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifndef _MSC_VER
#include <sys/param.h>
#endif 

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

/* IO-READ-DATA.C - FUNCTIONS DEFINED */
/* check_stop_processing 
   define_data_file_format
   process_data_header_model_files
   log_header
   read_database
   check_for_non_empty
   check_data_base
   output_warning_msgs
   output_error_msgs
   output_message_summary
   output_messages
   output_db_error_messages
   read_data
   define_attribute_definitions
   process_attribute_definitions
   process_attribute_def
   create_att_DS
   create_warn_err_DS
   define_discrete_translations
   expand_att_list
   find_str_in_list
   process_discrete_translations
   process_translation_msgs
   read_data_doit
   translate_instance
   translate_real
   translate_discrete
   get_line_tokens
   read_from_string
   read_line
   find_att_statistics
   find_real_stats
   store_real_stats
   find_discrete_stats
   output_created_translations
*/

/* CHECK_STOP_PROCESSING

   after log file is closed, determine if run is to proceed:
   error msgs => (error ...) warning msgs => (y-or-n-p ..)
   if *break-on-warnings* is t.  called by generate-clsf
   */
void check_stop_processing( int total_error_cnt, int total_warning_cnt,
                           FILE *log_file_fp, FILE *stream)
{
  fxlstr str;

  if (total_error_cnt > 0) {
    if (stream == NULL) {
      stream = stdout;
      to_screen_and_log_file("\nERRORs have occurred!", log_file_fp, stream, TRUE);
    }
    to_screen_and_log_file("\nThere is NO continuation possible\n",
                           log_file_fp, stream, TRUE);
    exit(1);
  }
  else if (total_warning_cnt > 0) {
    /*      fprintf(stderr, "\ntotal_warning_cnt is %d\n", total_warning_cnt); */
    if (G_break_on_warnings == TRUE) {
      if (stream == NULL) {
        stream = stdout;
        to_screen_and_log_file("\nWARNINGs have occurred!", log_file_fp, stream, TRUE);
      }
      sprintf( str, "\nDo you want to EXIT - {y/n}? ");
      to_screen_and_log_file( str, log_file_fp, NULL, TRUE);
      if (y_or_n_p( str)) {
        to_screen_and_log_file("\nEXIT due to warning messages at user's request\n",
                               log_file_fp, stream, TRUE); 
        exit(1);
      }
      else
        to_screen_and_log_file("\n", log_file_fp, NULL, TRUE);
    }
    else
      to_screen_and_log_file("\nRun continues, even though warnings were found\n\n",
                             log_file_fp, stream, TRUE);
  }
}


/* DEFINE_DATA_FILE_FORMAT
   28nov94 wmt: data_syntax will be line -- discard Lisp syntaxes: :list & :vector;
                put descriptor (number_of_data_file_format_defs) prior to integer value;
                check for comment chars in first column of each line
   30nov94 wmt: read characters as '?', rather than ?, in order to handle the blank char

   error checks & processes data file format parameters.
   User places this into xxxx.hd2 
   DATA-SYNTAX: line   => 1 2 3 3 4 5 #\\return. 
*/
void define_data_file_format( FILE *header_file_fp, FILE *log_file_fp, FILE *stream)
{
  int i, num, num_definition_names = 4;
  fxlstr str, def_name_string;
  database_DS data_base = G_input_data_base;
  char caller[] = "define_data_file_format";

  data_base->input_n_atts = data_base->n_atts = 0;
  data_base->separator_char = ' ';
  data_base->comment_char = ';';
  data_base->unknown_token = '?';

  discard_comment_lines(header_file_fp);
  fscanf(header_file_fp, "%s %d\n", def_name_string, &num);
  for (i=0; i < min(num, num_definition_names); i++) {
    discard_comment_lines(header_file_fp);
    fscanf(header_file_fp, "%s", def_name_string);
    if (eqstring(def_name_string, "number_of_attributes"))
      fscanf(header_file_fp, "%d\n", &(data_base->input_n_atts));
    else if (eqstring(def_name_string, "separator_char"))
      data_base->separator_char =
        (char) read_char_from_single_quotes("separator_char", header_file_fp);
    else if (eqstring(def_name_string, "comment_char"))
      data_base->comment_char =
        (char) read_char_from_single_quotes("comment_char", header_file_fp);
    else if (eqstring(def_name_string, "unknown_token"))
      data_base->unknown_token =
        (char) read_char_from_single_quotes("unknown_token", header_file_fp);
    else {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[2]: invalid data file format definition name: %s\n",
                   def_name_string);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
    }
  }
  if (log_file_fp != NULL) {
    to_screen_and_log_file("ADVISORY[2]: data_file_format settings:\n             ",
                           log_file_fp, stream, TRUE);
    safe_sprintf( str, sizeof( str), caller,
                 "separator_char = '%c', comment_char = '%c', unknown_token = '%c'\n",
                 data_base->separator_char, data_base->comment_char,
                 data_base->unknown_token);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  }
  if ( (data_base->n_atts = data_base->input_n_atts) <= 0) {
    safe_sprintf( str, sizeof( str), caller, 
                 "ERROR[2]: the number of attributes %d should be a positive integer.\n",
                 data_base->n_atts);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
    exit(1);
  }
}


/* PROCESS_DATA_HEADER_MODEL_FILES
   30nov94 wmt: move log_header to generate_clsf
   18dec94 wmt: always expand models -- do not check for errors or warnings first

   Reads data, header, and model files, expands model terms if no
   error messages, then outputs error and warning msgs 
*/
void process_data_header_model_files( FILE *log_file_fp, int regenerate_p, FILE *stream,
	     database_DS db, model_DS *models, int num_models, int *total_error_cnt_ptr,
             int *total_warning_cnt_ptr)
{
  int i;
  char output_msg_type[8] = ":read";

  output_messages(db, models, num_models, log_file_fp, stream, total_error_cnt_ptr,
                  total_warning_cnt_ptr, output_msg_type);
  check_stop_processing(*total_error_cnt_ptr, *total_warning_cnt_ptr, log_file_fp,
                        stream);

  for (i=0; (i < num_models); i++)
    conditional_expand_model_terms(models[i], regenerate_p, log_file_fp, stream);

  strcpy( output_msg_type, ":expand");
  output_messages(db, models, num_models, log_file_fp, stream, total_error_cnt_ptr,
                  total_warning_cnt_ptr, output_msg_type);
  
  to_screen_and_log_file("\n############ Input Check Concluded ##############\n",
                         log_file_fp, stream, TRUE);
}
 
 
/* LOG_HEADER
   12oct94 wmt: modified
   30nov94 wmt: add [1], [2], [3]

   output appropriate header for log messages
   */
void log_header( FILE *log_file_fp, FILE *stream, char *data_file_ptr,
                char *header_file_ptr, char *model_file_ptr, char *log_file_ptr)
{
  fxlstr str;
  char caller[] = "log_header";

  safe_sprintf( str, sizeof( str), caller,
               "\n##############  Starting Input Check  ###############\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  if (log_file_fp != NULL) {
    safe_sprintf( str, sizeof( str), caller, "\nTo log file: \n   %s%s\n",
            (log_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, log_file_ptr);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  }
  safe_sprintf( str, sizeof( str), caller,
               "During loading of: \n   [1] %s%s,\n",
               (data_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, data_file_ptr);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  safe_sprintf( str, sizeof( str), caller, "   [2] %s%s,\n",
               (header_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, header_file_ptr);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  safe_sprintf( str, sizeof( str), caller, "   [3] %s%s.\n",
               (model_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, model_file_ptr);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  safe_sprintf( str, sizeof( str), caller,
               "\n   [Attribute #, value #, and datum # are zero based.]\n\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
}


/* READ_DATABASE
   21oct94 wmt: modified
   16nov94 wmt: pass in data, header file names to be saved in database_DS
   28nov94 wmt: add param log_file_fp, and add call to check_data_base
   25apr95 wmt: add binary data file capability
   27apr95 wmt: Solaris 2.4 fails open, unless fopen/fclose is done first 
   10may95 wmt: move output_created_translations call to output_att_statistics;
                call output_att_statistics
   16may95 wmt: converted binary i/o to ANSI
   20may95 wmt: added prediction_p

   Reads and returns the data base defined by data-file and header-file.
   N-data, if supplied, will overide the value in the data file. returns d-base
   */
database_DS read_database( FILE *header_file_fp, FILE *log_file_fp,
                          char *data_file_ptr, char *header_file_ptr,
                          int max_data, int reread_p, FILE *stream)
{
  int n_att;
  FILE *data_file_fp = NULL;
  att_DS *att_info;
  database_DS d_base;
  warn_err_DS errors;

  if ((d_base = find_database(data_file_ptr, header_file_ptr, max_data)) != NULL) {
    if (reread_p == FALSE) {
      /* When database is already loaded we normally skip rereading it. */
      to_screen_and_log_file( "Skipping a reread of the database\n",
                              log_file_fp, stream, TRUE);
    } else {
      d_base->invalid_value_errors = NULL;
      d_base->num_invalid_value_errors = 0;
      d_base->incomplete_datum = NULL;
      d_base->num_incomplete_datum = 0;
      d_base->compressed_p = FALSE;
      att_info = d_base->att_info;
      for (n_att=0; n_att<d_base->n_atts; n_att++) {
        errors = att_info[n_att]->warnings_and_errors;
        strcpy(errors->unspecified_dummy_warning, "");
        strcpy(errors->single_valued_warning, "");
        /******************** strcpy(errors->model_expander_warnings, "");
	 ******* strcpy(errors->model_expander_errors, ""); changed 6/29 JTP */
        errors->unused_translators_warning = NULL;
        errors->model_expander_warnings = NULL;
        errors->num_expander_warnings = 0;
        errors->model_expander_errors = NULL;
        errors->num_expander_errors = 0;
      }
    }
  } else {
    d_base = create_database();
    d_base->n_data = 0;
    strcpy(d_base->data_file, data_file_ptr);
    strcpy(d_base->header_file, header_file_ptr);
    d_base->compressed_p = FALSE;
  }

  G_input_data_base = d_base;

  define_data_file_format( header_file_fp, log_file_fp, stream);
  define_attribute_definitions( header_file_fp, header_file_ptr, log_file_fp,
                               stream);
  /* defining discrete translations in the header file is not supported -
     define_discrete_translations(NULL, 0, G_input_data_base);
     - so just output discrete translations built in translate_discrete
     from the data read (after the data is read in 02dec94 wmt */
  if (eqstring( G_data_file_format, "binary") == TRUE)
    data_file_fp = fopen( data_file_ptr, "rb");
  else if (eqstring( G_data_file_format, "ascii") == TRUE)
    data_file_fp = fopen( data_file_ptr, "r");
  else {
    fprintf( stderr, "ERROR: G_data_file_format \"%s\" not handled\n", G_data_file_format);
    exit(1);
  }
  read_data( d_base, data_file_fp, max_data, data_file_ptr, log_file_fp, stream);
  
  fclose( data_file_fp);

  check_data_base( d_base, d_base->n_data);

  if (G_prediction_p == FALSE) { 
    find_att_statistics( d_base, log_file_fp, stream);

    output_att_statistics( d_base, log_file_fp, stream);  
  }
  if (find_database_p( d_base, G_db_list, G_db_length) == FALSE) {
    G_db_length++;
    if (G_db_list == NULL)
      G_db_list = (database_DS *) malloc(G_db_length * sizeof(database_DS));
    else
      G_db_list =
        (database_DS *) realloc(G_db_list, G_db_length * sizeof(database_DS));
    G_db_list[G_db_length-1] = d_base;
  }
  return(d_base);
}


int check_for_non_empty( att_DS *atts, int n_atts)
{
   int i;

   for (i=0; i<n_atts; i++)
      if (atts[i] != NULL)
	 return(TRUE);
   return(FALSE);
}


/* CHECK_DATA_BASE
   28nov94 wmt: new

   Checks d-base to assure the correct number of elements in each datum;
   adds entry to db errors
   */
void check_data_base( database_DS d_base, int n_data)
{
  int n_atts = d_base->n_atts, n_datum, datum_length;
  int *datum_length_list = d_base->datum_length;
  int num_errors;
  float *datum, **data = d_base->data;
  incomplete_datum_DS incomp_datum;

  for (n_datum=0; n_datum<n_data; n_datum++) {
    datum = data[n_datum];
    datum_length = datum_length_list[n_datum];
    if (datum_length != n_atts) {
      d_base->num_incomplete_datum++;
      num_errors = d_base->num_incomplete_datum;
      if (d_base->incomplete_datum == NULL) 
        d_base->incomplete_datum =
          (incomplete_datum_DS *) malloc(num_errors * sizeof(incomplete_datum_DS));
      else
        d_base->incomplete_datum =
          (incomplete_datum_DS *) realloc(d_base->incomplete_datum,
                                          (num_errors * sizeof(incomplete_datum_DS)));
      incomp_datum = (incomplete_datum_DS) malloc(sizeof(struct incomplete_datum));
      incomp_datum->n_datum = n_datum;
      incomp_datum->datum_length = datum_length;
      d_base->incomplete_datum[num_errors - 1] = incomp_datum;
    }
  }
}


/* OUTPUT_WARNING_MSGS
   23mar92 - WMT - single-valued functionality changed from error to warning
   -error slot retained for backward compatability to existing .results files 
   03dec94 wmt: changed unused_translators & single_valued errors 
   25may95 wmt: replaced sizeof(msg) with msg_length in first safe_sprintf
   18feb98 wmt: for number of translators less than .hd2 range, reduce the
   range and output advisory, rather than outputting warning -- REMOVED 
   13mar98 JCS due to incompatablility with previous results files

   format all warning messages into returned string 
*/
char *output_warning_msgs( int n_att, att_DS att, database_DS db, model_DS model)
{
  char *msg, caller[] = "output_warning_msgs";
  fxlstr warning_msg;
  shortstr *att_ignore_ids = model->att_ignore_ids;
  int i, msg_length = 4 * sizeof(fxlstr);
  warn_err_DS errors = att->warnings_and_errors;

  msg = (char *) malloc( msg_length);
  msg[0]='\0';

  if (strlen(errors->unspecified_dummy_warning) != 0) 
    safe_sprintf( msg, msg_length, caller,
                 "WARNING[2]: attribute #%d definition has not been specified --"
                  " type set to dummy\n", n_att);
  if (errors->unused_translators_warning != NULL) { 
    safe_sprintf( warning_msg, sizeof( warning_msg), caller, 
                  "WARNING[2]: attribute #%d: \"%s\"\n", n_att, att->dscrp); 
    strcat(msg, warning_msg); 

    /* commented since defined translators are not implemented 03dec94 wmt */
    /*     length = 0; */
    /*     while( errors->unused_translators_warning[length] > MOST_NEGATIVE_SINGLE_FLOAT) { */
    /*       strncat(msg, "No occurrences observed for value %f\n", */
    /*               errors->unused_translators_warning[length]); */
    /*       length++; */
    /*     } */
    /*     safe_sprintf(warning_msg, sizeof( warning_msg), caller, */
    /*             "To improve sensitivity of classification, reduce range by %d\n", */
    /*             length); */
    /*     for (i=0; i<db->num_tsp; i++) */
    /*       if (db->translations_supplied_p[i] == n_att) { */
    /*         strcat(warning_msg, */
    /*                "and redefine its translations to eliminate non-occurring entries\n"); */
    /*         break; */
    /*       } */
    /* do this instead */

    safe_sprintf( warning_msg, sizeof( warning_msg), caller, 
                  "            to improve sensitivity of classification, reduce range to %d.\n",
                  (int) errors->unused_translators_warning[0]); 
    strcat(msg, warning_msg); 
  } 
  if (eqstring(att_ignore_ids[n_att], "model_term_not_specified") == TRUE) {
    safe_sprintf( warning_msg, sizeof( warning_msg), caller,
                 "WARNING[3]: attribute #%d: \"%s\"\n", n_att, att->dscrp);
    strcat( msg, warning_msg);
    strcat( msg,
           "            model term type has not been specified and is set to ignore\n");
  }
  if (errors->model_expander_warnings != NULL)
    for (i=errors->num_expander_warnings-1; i>=0; i--) {
      safe_sprintf( warning_msg, sizeof( warning_msg), caller,
                   "WARNING[3]: attribute #%d: \"%s\"\n", n_att, att->dscrp);
      strcat( msg, warning_msg);
      strcat( msg, errors->model_expander_warnings[i]);
    }
  if (strlen(errors->single_valued_warning) != 0) {
    safe_sprintf( warning_msg, sizeof( warning_msg), caller,
                 "WARNING[3]: attribute #%d: \"%s\"\n", n_att, att->dscrp);
    strcat( msg, warning_msg);
    safe_sprintf( warning_msg, sizeof( warning_msg), caller,
            "            has only one unique value. Change model term type to ignore.\n");
    strcat( msg, warning_msg);
  }
  if ((int) strlen( msg) > (msg_length - 1)) {
    fprintf( stderr, "ERROR: %s produced %d chars (max number is %d)\n",
            caller, (int) strlen( msg), (msg_length - 1));
    abort();
  }
  return (msg);
}


/* OUTPUT_ERROR_MSGS
   18dec94 wmt: complete error msg
   
   format all error messages into returned string
*/
char *output_error_msgs( int n_att, att_DS att)
{
  char *msg, caller[] = "output_error_msgs";
  fxlstr str;
  int i, msg_length = 2;
  warn_err_DS errors = att->warnings_and_errors;


  msg = (char *) malloc( msg_length * sizeof( char));
  strcpy(msg, "");
  for (i=errors->num_expander_errors-1; i>=0; i--) {
    safe_sprintf( str, sizeof( str), caller,
                 "ERROR[2]: attribute #%d: \"%s\"\n", n_att, att->dscrp);
    msg_length = strlen( msg) + strlen( str) +
      strlen( errors->model_expander_errors[i]) + 1;
    msg = (char *) realloc( msg, msg_length * sizeof( char));
    strcat( msg, str);
    strcat( msg, errors->model_expander_errors[i]);
  }
  if ((int) strlen( msg) > (msg_length - 1)) {
    fprintf( stderr, "ERROR: %s produced %d chars (max number is %d)\n",
            caller, (int) strlen( msg), (msg_length - 1));
    abort();
  }
  return (msg);
}


/* 
   OUTPUT_MESSAGE_SUMMARY 

   18feb98 wmt: for number of translators less than .hd2 range, reduce the
                range and output advisory, rather than outputting warning -- REMOVED 
                13mar98 JCS due to incompatablility with previous results files

   output warning/error message summary and return error & warning counts
   23mar92 - WMT - single-valued functionality changed from error to warning
   -error slot retained for backward compatability to existing .results files */

void output_message_summary( int unspecified_dummy_warning_cnt,
                            int ignore_model_term_warning_cnt,
                            int unused_translators_warning_cnt,
                            int incomplete_errors_cnt,
                            int single_valued_warnings_cnt,
                            int invalid_value_errors_cnt,
                            int model_expander_warning_cnt,
                            int model_expander_error_cnt,
                            int *total_error_cnt_ptr,
                            int *total_warning_cnt_ptr,
                            FILE *log_file, FILE *stream, int output_p)
{
  fxlstr str;
  char caller[] = "output_message_summary";

  *total_error_cnt_ptr = incomplete_errors_cnt + invalid_value_errors_cnt +
    model_expander_error_cnt;
  *total_warning_cnt_ptr = unspecified_dummy_warning_cnt +
    ignore_model_term_warning_cnt + unused_translators_warning_cnt +
      model_expander_warning_cnt + single_valued_warnings_cnt;

  if ((*total_error_cnt_ptr + *total_warning_cnt_ptr) > 0) {
    safe_sprintf( str, sizeof( str), caller,
                 "\n*******  SUMMARY OF ALL ERROR AND WARNING MESSAGES *******\n\n");
    to_screen_and_log_file(str, log_file, stream, output_p);
    if (*total_warning_cnt_ptr > 0) {
      safe_sprintf( str, sizeof( str), caller,
                   "%d WARNING message(s) occured:\n", *total_warning_cnt_ptr);
      to_screen_and_log_file(str, log_file, stream, output_p);
      safe_sprintf( str, sizeof( str), caller,
                   "  %d due to unspecified attribute type set to dummy\n",
                   unspecified_dummy_warning_cnt);
      to_screen_and_log_file(str, log_file, stream, output_p);
      safe_sprintf( str, sizeof( str), caller,  
                    "  %d due to excess type = discrete range(s)\n", 
                    unused_translators_warning_cnt); 
      to_screen_and_log_file(str, log_file, stream, output_p); 
      safe_sprintf( str, sizeof( str), caller,
                    "  %d due to unspecified model term type set to ignore\n",
                    ignore_model_term_warning_cnt);
      to_screen_and_log_file(str, log_file, stream, output_p);
      safe_sprintf( str, sizeof( str), caller,
                   "  %d due to single valued attribute(s)\n", single_valued_warnings_cnt);
      to_screen_and_log_file(str, log_file, stream, output_p);
      safe_sprintf( str, sizeof( str), caller,
                   "  %d due to model term type expansion\n", model_expander_warning_cnt);
      to_screen_and_log_file(str, log_file, stream, output_p);
      /* safe_sprintf(str, "\nAbort and change appropriate file\n"); 05dec94 wmt */
      /* to_screen_and_log_file(str, log_file, stream, output_p); */
      /* safe_sprintf(str, "or continue, accepting possible anomalies.\n"); */
      /* to_screen_and_log_file(str, log_file, stream, output_p); */
    }
    if (*total_error_cnt_ptr > 0) {
      safe_sprintf( str, sizeof( str), caller,
                   "\n%d ERROR message(s) occured:\n", *total_error_cnt_ptr);
      to_screen_and_log_file(str, log_file, stream, output_p);
      safe_sprintf( str, sizeof( str), caller, "  %d due to incomplete datum\n",
                   incomplete_errors_cnt);
      to_screen_and_log_file(str, log_file, stream, output_p);
      safe_sprintf( str, sizeof( str), caller,
                   "  %d due to invalid type = real attribute value(s)\n",
                   invalid_value_errors_cnt);
      to_screen_and_log_file(str, log_file, stream, output_p);
      safe_sprintf( str, sizeof( str), caller,
                   "  %d due to model term type expansion\n",
                   model_expander_error_cnt);
      to_screen_and_log_file(str, log_file, stream, output_p);
    }
  }
}


/* OUTPUT_MESSAGES
   02dec94 wmt: free mallocs passed in from output_warning_msgs & output_error_msgs
   
   output warning & error msgs for incomplete datum and attributes whose
   att-ignore-id is not 'ignore-model
   23mar92 - WMT - single-valued functionality changed from error to warning
   -error slot retained for backward compatability to existing .results file */

void output_messages( database_DS db, model_DS *models, int num_models, FILE *log_file,
                     FILE *stream, int *total_error_cnt_ptr, int *total_warning_cnt_ptr,
                     char *output_msg_type_ptr)
{
  int i, n_att, msg_header_p = FALSE, output_p = TRUE;
  char *warning_msgs = NULL, *error_msgs = NULL, caller[] = "output_messages";
  shortstr *att_ignore_ids;
  att_DS att;
  model_DS model;
  warn_err_DS errors;
  att_DS *att_info = db->att_info;
  int n_atts = db->n_atts;
  int unused_translators_warning_cnt = 0;
  int unspecified_dummy_warning_cnt = 0;
  int ignore_model_term_warning_cnt = 0;
  int model_expander_warning_cnt = 0;
  int incomplete_datum_cnt = 0;
  int invalid_value_errors_cnt = 0;
  int single_valued_warnings_cnt = 0;
  int model_expander_error_cnt = 0;
  fxlstr str;

  invalid_value_errors_cnt = db->num_invalid_value_errors;
  incomplete_datum_cnt = db->num_incomplete_datum;
  if (((invalid_value_errors_cnt > 0) || (incomplete_datum_cnt > 0)) &&
      (output_p == TRUE))
    output_db_error_messages(db, log_file, stream, output_p);

  for (i=0; i<num_models; i++) {
    model = models[i];
    msg_header_p = FALSE;
    att_ignore_ids = model->att_ignore_ids;
    for (n_att=0; n_att<n_atts; n_att++) {
      if (eqstring(att_ignore_ids[n_att], "ignore_model") == FALSE) {
        att = att_info[n_att];
        errors = att->warnings_and_errors;
        if (strlen(errors->unspecified_dummy_warning) != 0) 
          unspecified_dummy_warning_cnt++;
        if (eqstring(att_ignore_ids[n_att], "model_term_not_specified"))
          ignore_model_term_warning_cnt++;
        if (errors->unused_translators_warning != NULL)
          unused_translators_warning_cnt++;
        if (errors->model_expander_warnings != NULL )
          model_expander_warning_cnt +=errors->num_expander_warnings;
        if (strlen(errors->single_valued_warning) != 0)
          single_valued_warnings_cnt++;
        warning_msgs = output_warning_msgs(n_att, att, db, model);
        if (errors->model_expander_errors != NULL)
          model_expander_error_cnt += errors->num_expander_errors;
        error_msgs = output_error_msgs(n_att, att);
        if (((int) strlen(warning_msgs) > 0) || ((int) strlen(error_msgs) > 0)) {
          if ((msg_header_p == FALSE) && (output_p == TRUE)) {
            if (eqstring( output_msg_type_ptr, ":read"))
              safe_sprintf( str, sizeof( str), caller,
                           "\n****** Error & Warning Messages from READING Model Index"
                           " = %d ******\n\n", i);
            else
              safe_sprintf( str, sizeof( str), caller,
                           "\n** Error & Warning Messages from READING & "
                           "EXPANDING Model Index = %d **\n\n", i);
            to_screen_and_log_file(str, log_file, stream, output_p);
            msg_header_p = TRUE;            
          }
          if ((int) strlen(warning_msgs) > 0)
            to_screen_and_log_file(warning_msgs, log_file, stream, output_p);
          if ((int) strlen(error_msgs) > 0)
            to_screen_and_log_file(error_msgs, log_file, stream, output_p);
        }
        free(warning_msgs);
        free(error_msgs);
      }
    }
  }
  if (eqstring( output_msg_type_ptr, ":read") &&
      ((incomplete_datum_cnt + invalid_value_errors_cnt) == 0))
    output_p = FALSE;
  output_message_summary(unspecified_dummy_warning_cnt,
                         ignore_model_term_warning_cnt, unused_translators_warning_cnt,
                         incomplete_datum_cnt, single_valued_warnings_cnt,
                         invalid_value_errors_cnt, model_expander_warning_cnt,
                         model_expander_error_cnt, total_error_cnt_ptr,
                         total_warning_cnt_ptr, log_file, stream, output_p);
}


/* OUTPUT_DB_ERROR_MESSAGES
   02dec94 wmt: add more detail to messages

   output error msgs for incomplete datum and invalid value tokens
   */
void output_db_error_messages( database_DS db, FILE *log_file_fp, FILE *stream,
                              int output_p)
{
  fxlstr str;
  int i, n_att;
  char caller[] = "output_db_error_messages";

  safe_sprintf( str, sizeof( str), caller,
               "\n**********  Error Messages from Data Base ***********\n\n");
  to_screen_and_log_file(str, log_file_fp, stream, output_p);
  if (db->invalid_value_errors != NULL) {
    for (i=0; i<db->num_invalid_value_errors; i++) {
      n_att = db->invalid_value_errors[i]->n_att;
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[1]: in datum #%d, type = real attribute #%d: \"%s\" has\n",
                   db->invalid_value_errors[i]->n_datum, n_att,
                   db->att_info[n_att]->dscrp);
      to_screen_and_log_file(str, log_file_fp, stream, output_p);
      safe_sprintf( str, sizeof( str), caller, "           non-number value, %s\n",
                   db->invalid_value_errors[i]->value);
      to_screen_and_log_file(str, log_file_fp, stream, output_p);
    }
  }
  if (db->incomplete_datum != NULL) {
    for (i=0; i<db->num_incomplete_datum; i++) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[1]: datum #%d is incomplete: it has %d attributes, "
                   "instead of %d.\n", db->incomplete_datum[i]->n_datum,
                   db->incomplete_datum[i]->datum_length, db->n_atts);
      to_screen_and_log_file(str, log_file_fp, stream, output_p);
    }
  }
}


/* READ_DATA
   15nov94 wmt: max_data check changed & add DATA_ALLOC_INCREMENT
        & do incremental allocation of data array
   27nov94 wmt: pass instance_length to read_data_doit & translate_instance
   28nov94 wmt: add datum_length,
   03dec94 wmt: pass comment chars to read_data_doit
   25apr95 wmt: add binary data file capability
   09may95 wmt: test on (max_data - 1), rather than max_data
   16may95 wmt: converted binary i/o to ANSI
   13feb98 wmt: check for malloc/realloc failures
   */
void read_data( database_DS d_base, FILE *data_file_fp, int max_data,
               char *data_file_ptr, FILE *log_file_fp, FILE *stream)
{
  int n, data_allocated = 0, instance_length = 0, *datum_length = NULL;
  int n_comment_chars = 3, binary_instance_length;
  int input_binary_instance_length, num_header_chars = 8;
  char **instance, db2_bin_header[10] = "";
  float **data = NULL;
  char *str;
  int str_length = 2 * sizeof( fxlstr);
  char comment_chars[4], caller[] = "read_data";
  float *binary_instance = NULL;

  str = (char *) malloc( str_length);
  binary_instance_length = d_base->n_atts * sizeof( float);
  if (eqstring( G_data_file_format, "binary")) {
    fread( &db2_bin_header, sizeof( char), num_header_chars, data_file_fp);
    fread( &input_binary_instance_length, sizeof( char), sizeof( float),
          data_file_fp);
    if ((eqstring( db2_bin_header, ".db2-bin") != TRUE) ||
        (input_binary_instance_length != binary_instance_length)) {
      fprintf( stderr, "ERROR: %s%s, \n       either \"%s\" is not the correct "
              "header string(\".db2-bin\") or\n"
              "       %d is not the correct case length (%d)\n",
              (data_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, data_file_ptr,
              db2_bin_header, input_binary_instance_length, binary_instance_length);
      exit(1);
    }
  }
  /* treat blank lines are comment lines */
  comment_chars[0] = d_base->comment_char; comment_chars[1] = ' ';
  comment_chars[2] = '\n'; comment_chars[3] = '\0';
  instance = read_data_doit( d_base, data_file_fp, TRUE, &instance_length,
                            n_comment_chars, comment_chars, binary_instance_length,
                            &binary_instance);
  for (n=0; (instance != NULL) || (binary_instance != NULL); n++) {
    d_base->n_data += 1;
    if (d_base->n_data > data_allocated) {
      data_allocated += DATA_ALLOC_INCREMENT;
      if (d_base->data == NULL) {
        d_base->data = (fptr *) malloc(data_allocated * sizeof(float *));
        if (d_base->data == NULL) {
          fprintf( stderr,
                   "ERROR: read_data(1): out of memory, malloc returned NULL!\n");
          exit(1);
        }
      }
      else {
        d_base->data = (fptr *) realloc(d_base->data, data_allocated * sizeof(float *));
        if (d_base->data == NULL) {
          fprintf( stderr,
                   "ERROR: read_data(2): out of memory, realloc returned NULL!\n");
          exit(1);
        }
      }
      data = d_base->data;
      if (d_base->datum_length == NULL) {
        d_base->datum_length = (int *) malloc(data_allocated * sizeof(int));
        if (d_base->datum_length == NULL) {
          fprintf( stderr,
                   "ERROR: read_data(3): out of memory, malloc returned NULL!\n");
          exit(1);
        }
      }
      else {
        d_base->datum_length = (int *) realloc(d_base->datum_length,
                                               data_allocated * sizeof(int));
        if (d_base->datum_length == NULL) {
          fprintf( stderr,
                   "ERROR: read_data(4): out of memory, realloc returned NULL!\n");
          exit(1);
        }
      }
      datum_length = d_base->datum_length;
    }
    if (eqstring( G_data_file_format, "ascii"))
      data[(d_base->n_data-1)] = translate_instance(d_base, instance, instance_length,
                                                    n, log_file_fp, stream);
    else
      data[(d_base->n_data-1)] = binary_instance;
    datum_length[(d_base->n_data-1)] = instance_length;
    instance_length = 0;
    /* fprintf( stderr, " read_data n %d\n", n); */
    instance = read_data_doit( d_base, data_file_fp, FALSE, &instance_length,
                              n_comment_chars, comment_chars, binary_instance_length,
                              &binary_instance);
    if ((max_data != 0) && (n+1 >= max_data))
      break;
  }
  if ((max_data != 0) && (n+1 < max_data))
    safe_sprintf( str, str_length, caller,
                 "\nWARNING[1]: read_data found *ONLY* %d datum in\n"
                 "            \"%s%s\"\n\n", n,
                 (data_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, data_file_ptr);
  else if (n == 0) {
    safe_sprintf( str, str_length, caller,
                 "\nERROR[1]: no data read by read_data from\n"
                 "          \"%s%s\"\n",
                 (data_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, data_file_ptr);
    to_screen_and_log_file( str, G_log_file_fp, G_stream, TRUE);
    exit(1);
  }
  else
    safe_sprintf( str, str_length, caller,
                 "ADVISORY[1]: %s %d datum from \n             %s%s\n",
                 (eqstring( G_data_file_format, "ascii")) ? "read" : "loaded",
                 d_base->n_data, (data_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                 data_file_ptr);
  to_screen_and_log_file( str, G_log_file_fp, G_stream, TRUE);
  free( str);
}


/* error checks & processes attribute definitions.  User places this into
   xxxx.hd2 (*header-file-type*). */

void define_attribute_definitions( FILE *header_file_fp, char *header_file_ptr,
                                  FILE *log_file_fp, FILE *stream)
{
   database_DS data_base = G_input_data_base;

   process_attribute_definitions( data_base, header_file_fp, header_file_ptr,
                                 log_file_fp, stream);
}


/* PROCESS_ATTRIBUTE_DEFINITIONS
   28nov94 wmt: pass att_num to create_att_DS, and exit if input errors are found
   30nov94 wmt: check for invalid att_num
   12dec94 wmt: read header file using get_line_tokens, rather than scanf -- which
                reads past \n etc, causing all kinds of grief
   28feb95 wmt: realloc att_info if n_atts > allo_n_atts

   checks the attribute descriptions, and sets the
   N-atts, & Att-info slots of the d-base data-base-DS structure.
   */
void process_attribute_definitions( database_DS d_base, FILE *header_file_fp,
                                   char *header_file_ptr, FILE *log_file_fp,
                                   FILE *stream)
{
  int i, j, att_num, n_atts, first_read = FALSE, num_tokens, n_comment_chars = 5;
  att_DS *att_info;
  int input_error = FALSE, integer_p, n_atts_read = 0, str_length = 2 * sizeof( fxlstr);
  char *str;
  char **tokens = NULL, caller[] = "process_attribute_definitions";
  char separator_char = ' ', comment_chars[6];

  str = (char *) malloc( str_length);
  /* allow general comment lines anywhere in model file */
  comment_chars[0] = '!'; comment_chars[1] = '#'; comment_chars[2] = ';';
  comment_chars[3] = ' '; comment_chars[4] = '\n'; comment_chars[5] = '\0';

  n_atts = d_base->n_atts;
  if (n_atts > d_base->allo_n_atts) {
    d_base->allo_n_atts = n_atts;
    d_base->att_info = (att_DS *) realloc( d_base->att_info,
                                          d_base->allo_n_atts * sizeof( att_DS));
  }
  att_info = d_base->att_info;
  for (i=0; i<n_atts; i++) 
    att_info[i] = d_base->att_info[i] = NULL;

  for (i=0; i<n_atts; i++) {

    if ((tokens = get_line_tokens(header_file_fp, (int) separator_char, n_comment_chars,
                               comment_chars, first_read, &num_tokens)) == NULL) {
      break;
    }
    att_num = atoi_p(tokens[0], &integer_p);
    if (integer_p != TRUE) {
      safe_sprintf( str, str_length, caller,
                   "ERROR[2]: expecting integer attribute index %d, read %s\n",
                   i, tokens[0]);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      input_error = TRUE;
      break;
    }
    att_info[att_num] = process_attribute_def( att_num, &input_error, tokens, num_tokens,
                                              log_file_fp, stream);
    n_atts_read++;
    for (j=0; j<num_tokens; j++)
      free( tokens[j]);
    free( tokens);
  }
  if (input_error == TRUE)
    exit(1);
  safe_sprintf( str, str_length, caller,
               "ADVISORY[2]: read %d attribute defs from \n             %s%s\n",
               n_atts_read,
               (header_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, header_file_ptr);
  to_screen_and_log_file( str, G_log_file_fp, G_stream, TRUE);

  for (att_num=0; att_num<n_atts; att_num++) {
    if (att_info[att_num] == NULL) {
      att_info[att_num] = create_att_DS( att_num, &input_error, 0, 0.0, 0.0, 0.0,
                                        "dummy", "nil", "unspecified_attribute",
                                        log_file_fp, stream);
      strcpy( att_info[att_num]->warnings_and_errors->unspecified_dummy_warning, "true");
    }
  }
  free( str);
}


/* PROCESS_ATTRIBUTE_DEF
   15dec94 wmt: new - split off from create_att_DS
   21mar97 wmt: check for incomplete discrete/real att defs

   read the defintion from the .hd2 file 
*/
att_DS process_attribute_def( int att_num, int *input_error_ptr, char **tokens,
                             int num_tokens, FILE *log_file_fp, FILE *stream)
{
  fxlstr str;
  int range = 0;                /* changed 6/21/JTP was INT_UNKNOWN;*/
  float rel_error = 0.0, error = 0.0; /* these were -1.0 changed to 0.0 5/21/94 JTP*/
  float zero_point = 0.0;
  int min_discrete_tokens = 6, min_real_location_tokens = 6;
  int min_real_scalar_tokens = 8;
  int min_num_tokens = 4, integer_p, float_p, index_1, index_2;
  char *type_ptr = NULL, *sub_type_ptr = NULL, *dscrp_ptr = NULL;
  char caller[] = "process_attribute_def";

  if (num_tokens < min_num_tokens) {
    safe_sprintf( str, sizeof( str), caller, "ERROR[2]: expected at least %d items: \n"
                 "          <att_num> <att_type> <att_sub_type> <att_description> \n"
                 "          read %d: %s, %s, %s, %s\n",
                 min_num_tokens, num_tokens, tokens[0],
                 (num_tokens >= 2) ? tokens[1] : "",
                 (num_tokens >= 3) ? tokens[2] : "",
                 (num_tokens >= 4) ? tokens[3] : "");
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
    *input_error_ptr = TRUE;
  }
  else {
    type_ptr = tokens[1]; sub_type_ptr = tokens[2]; dscrp_ptr = tokens[3];

    if (eqstring(type_ptr, "discrete") && eqstring(sub_type_ptr, "nominal")) {
      if (num_tokens < min_discrete_tokens) {
        safe_sprintf( str, sizeof( str), caller, "ERROR[2]: expected at least %d items: \n"
                     "          <att_num> <att_type> <att_sub_type> <att_description> \n"
                     "                    range <range_value> \n",
                     min_discrete_tokens);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        safe_sprintf( str, sizeof( str), caller,
                     "          read %d: %s, %s, %s, %s, %s, %s\n",
                     num_tokens, tokens[0],
                     (num_tokens >= 2) ? tokens[1] : "",
                     (num_tokens >= 3) ? tokens[2] : "",
                     (num_tokens >= 4) ? tokens[3] : "",
                     (num_tokens >= 5) ? tokens[4] : "",
                     (num_tokens >= 6) ? tokens[5] : "");
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        *input_error_ptr = TRUE;
      } else {
        if( ! eqstring(tokens[4], "range") ) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[2]: expected parameter range, got %s \n"
                       "          for attribute #%d: \"%s\"\n",
                       tokens[4], att_num, dscrp_ptr);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          *input_error_ptr = TRUE;
        }
        else {
          range = atoi_p(tokens[5], &integer_p);
          if (integer_p != TRUE) {
            safe_sprintf(str, sizeof( str), caller,
                         "ERROR[2]: value of parameter range read, %s, was not an integer\n"
                         "          for attribute #%d: \"%s\"\n", tokens[5],
                         att_num, dscrp_ptr);
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            *input_error_ptr = TRUE;
          }
        }
      }
    }
    else if (eqstring(type_ptr, "real") && eqstring(sub_type_ptr, "scalar")) {
      if (num_tokens < min_real_scalar_tokens) {
        safe_sprintf( str, sizeof( str), caller, "ERROR[2]: expected at least %d items: \n"
                     "          <att_num> <att_type> <att_sub_type> <att_description> \n",
                     min_real_scalar_tokens);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        safe_sprintf( str, sizeof( str), caller, 
                     "                    zero_point <zero_point_value> \n"
                     "                    rel_error <rel_error_value> \n");
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        safe_sprintf( str, sizeof( str), caller, 
                     "          read %d: %s, %s, %s, %s, %s, %s, %s, %s\n",
                     num_tokens, tokens[0],
                     (num_tokens >= 2) ? tokens[1] : "",
                     (num_tokens >= 3) ? tokens[2] : "",
                     (num_tokens >= 4) ? tokens[3] : "",
                     (num_tokens >= 5) ? tokens[4] : "",
                     (num_tokens >= 6) ? tokens[5] : "",
                     (num_tokens >= 7) ? tokens[6] : "",
                     (num_tokens >= 8) ? tokens[7] : "");
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        *input_error_ptr = TRUE;
      } else {
        if( ! eqstring(tokens[4], "zero_point") && !eqstring(tokens[6],"zero_point") ) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[2]: expected parameter zero_point, got %s and %s\n"
                       "          for attribute #%d: \"%s\"\n",
                       tokens[4], tokens[6], att_num, dscrp_ptr);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          *input_error_ptr = TRUE;
        }
        else if( ! eqstring(tokens[4], "rel_error") && !eqstring(tokens[6], "rel_error") ) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[2]: expected parameter rel_error, got %s and %s\n"
                       "          for attribute #%d: \"%s\"\n",
                       tokens[4], tokens[6], att_num, dscrp_ptr);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          *input_error_ptr = TRUE;
        }
        else {
          if (eqstring(tokens[4], "zero_point")) {
            index_1 = 5; index_2 = 7;
          }
          else {
            index_1 = 7; index_2 = 5;
          }
          zero_point = (float) atof_p(tokens[index_1], &float_p);
          if (float_p != TRUE) {
            safe_sprintf( str, sizeof( str), caller,
                         "ERROR[2]: value of parameter zero_point read, %s, was not a float\n"
                         "          for attribute #%d: \"%s\"\n", tokens[index_1], att_num,
                         dscrp_ptr);
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            *input_error_ptr = TRUE;
          }
          rel_error = (float) atof_p(tokens[index_2], &float_p);
          if (float_p != TRUE) {
            safe_sprintf( str, sizeof( str), caller,
                         "ERROR[2]: value of parameter rel_error read, %s, was not a float\n"
                         "          for attribute #%d: \"%s\"\n", tokens[index_2], att_num,
                         dscrp_ptr);
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            *input_error_ptr = TRUE;
          }
        }
      }
    }
    else if (eqstring(type_ptr, "real") && eqstring(sub_type_ptr, "location")) {
      if (num_tokens < min_real_location_tokens) {
        safe_sprintf( str, sizeof( str), caller, "ERROR[2]: expected at least %d items: \n"
                     "          <att_num> <att_type> <att_sub_type> <att_description> \n"
                     "                    error <error_value> \n",
                     min_real_location_tokens);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        safe_sprintf( str, sizeof( str), caller, 
                     "          read %d: %s, %s, %s, %s, %s, %s\n",
                     num_tokens, tokens[0],
                     (num_tokens >= 2) ? tokens[1] : "",
                     (num_tokens >= 3) ? tokens[2] : "",
                     (num_tokens >= 4) ? tokens[3] : "",
                     (num_tokens >= 5) ? tokens[4] : "",
                     (num_tokens >= 6) ? tokens[5] : "");
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        *input_error_ptr = TRUE;
      } else {
        if( ! eqstring(tokens[4], "error") ) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[2]: expected parameter error, got %s \n"
                       "          for attribute #%d: \"%s\"\n",
                       tokens[4], att_num, dscrp_ptr);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          *input_error_ptr = TRUE;
        }
        else {
          error = atof_p(tokens[5], &float_p);
          if (float_p != TRUE) {
            safe_sprintf( str, sizeof( str), caller,
                         "ERROR[2]: value of parameter error read, %s, was not a float\n"
                         "          for attribute #%d: \"%s\"\n", tokens[5], att_num,
                         dscrp_ptr);
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            *input_error_ptr = TRUE;
          }
        }
      }
    }
    else if (eqstring(type_ptr, "dummy") ) {
      if( !eqstring(sub_type_ptr, "nil") && !eqstring(sub_type_ptr,"none") ) {
        safe_sprintf( str, sizeof( str), caller,
                     "ERROR[2]: expected sub_type nil or none, got %s\n"
                     "          for attribute #%d: \"%s\"\n", sub_type_ptr,
                     att_num, dscrp_ptr);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        *input_error_ptr = TRUE;
      }
    }
    else {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[2]: unknown type/sub_type = %s/%s\n"
                   "          for attribute #%d: \"%s\"\n",
                   type_ptr, sub_type_ptr, att_num, dscrp_ptr);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      *input_error_ptr = TRUE;
    }
  }

  return( create_att_DS( att_num, input_error_ptr, range, (double) rel_error,
                        (double) error, (double) zero_point, type_ptr,
                        sub_type_ptr, dscrp_ptr, log_file_fp, stream));
}


/* CREATE_ATT_DS
   15nov94 wmt: initialize n_props & props & missing
   28nov94 wmt: pass att_num to create_att_DS, reword error msgs
   12dec94 wmt: read header file using get_line_tokens, rather than scanf
   15dec94 wmt: split this into read_attribute_definitions & create_att_DS
   23may95 wmt: add G_prediction_p

   Create an att-DS from a descriptor form (index type sub-type
   description-string prop-list).
   Any translations placed on the prop-list are processed later. 
*/
att_DS create_att_DS( int att_num, int *input_error_ptr, int range,
                     double rel_error, double error, double zero_point,
                     char *type_ptr, char *sub_type_ptr, char *dscrp_ptr,
                     FILE *log_file_fp, FILE *stream)
{
  att_DS att;
  int i;
  fxlstr str;
  char caller[] = "create_att_DS";

  att = (att_DS) malloc(sizeof(struct att));

  if (*input_error_ptr == FALSE) {
    strcpy(att->type, type_ptr);
    strcpy(att->sub_type, sub_type_ptr);
    if ((int) strlen( dscrp_ptr) >= SHORT_STRING_LENGTH) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[2]: length of attribute #%d description \"%s\"\n"
                   "          is longer than %d characters\n", att_num, dscrp_ptr,
                   SHORT_STRING_LENGTH - 1);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
    }
    strcpy(att->dscrp, dscrp_ptr);
    att->range = range;
    att->zero_point = (float) zero_point;
    att->translations = NULL;
    att->rel_error = (float) rel_error;
    att->error = (float) error;
    att->n_trans = 0;
    att->n_props = 0;
    att->props = NULL;
    att->missing = FALSE;
    if (eqstring(type_ptr, "real")) {
      att->d_statistics = NULL;
      if ((G_prediction_p == TRUE) && (G_training_clsf != NULL)) {
        /* force the "test" database to use the same statistics
           as the "training" database. */
        att->r_statistics = G_training_clsf->database->att_info[att_num]->r_statistics;
        att->missing = G_training_clsf->database->att_info[att_num]->missing;
        att->range = G_training_clsf->database->att_info[att_num]->range;
      }
      else
        att->r_statistics = (real_stats_DS) malloc(sizeof(struct real_stats));
    }
    else if (eqstring(type_ptr, "discrete")) {
      att->r_statistics = NULL;
      att->d_statistics =
        (discrete_stats_DS) malloc(sizeof(struct discrete_stats));
      att->d_statistics->range = range;
      att->d_statistics->observed = (int *) malloc(range * sizeof(int));
      att->d_statistics->n_observed = range;
      for (i=0; i<range; i++)
        att->d_statistics->observed[i] = 0;
      /* force the "test" database to use the same discrete translations
        as the "training" database. */
      if ((G_prediction_p == TRUE) && (G_training_clsf != NULL)) {
        att->n_trans = G_training_clsf->database->att_info[att_num]->n_trans;
        att->translations = G_training_clsf->database->att_info[att_num]->translations;
      }
    }
    else {
      att->r_statistics = NULL;
      att->d_statistics = NULL;
    }
    att->warnings_and_errors = create_warn_err_DS();  
  }
  return(att);
}


/* CREATE_WARN_ERR_DS
   16may95 wmt: move malloc out of declaration
   */
warn_err_DS create_warn_err_DS(void)
{
   warn_err_DS weds;

   weds= (warn_err_DS) malloc( sizeof( struct warn_err));
   weds->model_expander_warnings = NULL;
   weds->num_expander_warnings = 0;
   weds->model_expander_errors = NULL;
   weds->num_expander_errors = 0;
   strcpy(weds->unspecified_dummy_warning, "");
   strcpy(weds->single_valued_warning, "");
   weds->unused_translators_warning = NULL;

   return(weds);
}


/* error checks & processes type = discrete attribute translations. 

void define_discrete_translations( char ***discrete_translations, int num, 
                                  database_DS data_base) 
{ 
  int nlength;  

  process_discrete_translations 
    (data_base, expand_att_list(discrete_translations, num, &nlength), 
     nlength); 
}
*/

/* This takes a list of possibly compact attribute descriptions or translations
   and expands it into a list of standard forms.  In the standard forms,
   attribute indices are required to be non-negative integers or 'default.
   Expand-Att-List allows the use of a compact representation which substitutes
   lists of attribute indices for the single index.  The expanded list is
   returned.  Any duplication of indices will cause an error. */

char ***expand_att_list( char ***att_list, int num, int *nlength)
{
   return(att_list);
}


/* FIND_STR_IN_LIST
   02dec94 wmt: ***translations => **translations
   */
int find_str_in_list( char *str, char **translations, int num)
{
   int i;

   for (i=0; i<num; i++)
      if (eqstring(translations[i], str) == TRUE)
	 return(i);
   return(-1);
}


/* For each att-description requiring a translation, tries to find and check
   such description, placing, the result in the att-description props. */
/*
void process_discrete_translations( database_DS d_base, char ***value_translations, 
			      int vlength) 
{ 
   int i, plength = 0; */
/*    int n_att, pos, tplength = 0, tnplength = 0; */
/*    int range, *translations_provided, *translations_not_provided; */
/*    char **att_translation; */
   char **default_translation, ***processed;
/*    fxlstr temp, str; */
/*    att_DS att, *att_info = d_base->att_info; */
/*
   default_translation = NULL;
   processed = NULL;
   for (i=0; i<vlength; i++)
      if (eqstring(value_translations[i][0], "default")) {
	 default_translation = (char **) malloc(2 * sizeof(char *));
	 default_translation[0] = (char *) malloc(sizeof(fxlstr));
	 default_translation[1] = (char *) malloc(sizeof(fxlstr));
	 strcpy(default_translation[0], value_translations[i][0]);
	 strcpy(default_translation[1], value_translations[i][1]);
	 break;
      }
   if (default_translation != NULL) {
      default_translation = process_translation(d_base, 0, NULL, 1, default_translation);
      plength++;
      processed = (char ***) malloc(plength * sizeof(char **));
      processed[plength-1] = (char **) malloc(2 * sizeof(char *));
      processed[plength-1][0] = (char *) malloc(sizeof(fxlstr));
      strcpy(processed[plength-1][0], default_translation[0]);
      strcpy(processed[plength-1][1], default_translation[1]);
   }
*/
   /* ----- For DISCRETES, check and/or generate translations, placing them in
      the props list: 
   for (n_att=0; n_att<d_base->n_atts; n_att++) {
      att = att_info[n_att];
      if (eqstring(att->type, "discrete")) {
	 att_translations = att->translations;
	 if (att_translations == NULL)
	    sprintf(temp, "%d", n_att);
	 pos = find_str_in_lisT(temp, value_translations, vlength);
	 att_translation = (char **) malloc(2 * sizeof(char *));
	 att_translation[0] = value_translations[pos][1];
	 att_translation[1] = NULL;
	 range = att->range;
	 if (att_translation != NULL) {
	    att_translation = process_translation(d_base, n_att,att, 1, att_translation);
	    plength++;
	    if (processed == NULL)
	       processed = (char ***) malloc(plength * sizeof(char **));
	    else processed =
		    (char ***) realloc(processed, plength * sizeof(char **));
	    processed[plength-1] = (char **) malloc(2 * sizeof(char *));
	    processed[plength-1][0] = (char *) malloc(sizeof(fxlstr));
	    processed[plength-1][1] = (char *) malloc(sizeof(fxlstr));
	    strcpy(processed[plength-1][0], temp);
	    strcpy(processed[plength-1][1], att_translation[0]);
	    tplength++;
	    if (translations_provided == NULL)
	       translations_provided =
		  (int *) malloc(tplength * sizeof(int));
	    else
	       translations_provided = (int *) realloc(translations_provided,
						       tplength * sizeof(int));
	    translations_provided[tplength - 1] = n_att;
	 }
	 else {
	    tnplength++;
	    if (translations_not_provided == NULL)
	       translations_not_provided =
		  (int *) malloc(tnplength * sizeof(int));
	    else
	       translations_not_provided =
		  (int *) realloc(translations_not_provided,
				  tnplength * sizeof(int));
	    translations_not_provided[tnplength - 1] = n_att;
	    att_translation = default_translation;
	 }
	 att->range = max(range, att->n_trans);
	 fprintf( stream, "Reset range to %d\n", range);
	 if ((att_translation != NULL) && (range != att->n_trans)) {
	    sprintf(str,
		  "ERROR[2]: attribute range %d is not equal to number of translations\n",
		  range);
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
	    exit(1);
	 }
         att->n_trans += 1;
         if (att->translations == NULL)
	    att->translations =
	       (char ***) malloc(att->n_trans * sizeof(char **));
	 else att->translations =
	    (char ***) realloc(att->translations, att->trans * sizeof(char **));
	 if (att_translation != NULL)
	    att->translations[att->n_trans] = att_translation;
	 else {
	    n = att->n_trans - 1;
	    att->translations[n] = (char **) malloc(2 * sizeof(char *));
	    att->translations[n][0] = (char *) malloc(sizeof(fxlstr));
	    sprintf(att->translations[n][0], "%d", range);
	    att->translations[n][0] = NULL;
	 }
      }
   }
   d_base->translations_supplied_p = translations_provided; 
   if (translations_not_provided)
      process_translations_msgs(translations_not_provided, default_translation,
				att_info, stream); */
/* } */

/* output advisory msgs for translations not provided */


void process_translation_msgs( int *translations_not_provided, int num,
			   char *default_translation, att_DS *att_info, FILE *stream)
{
   int i;

   if (default_translation != NULL)
      fprintf(stream, "ADVISORY:  the default translation will be used");
   else fprintf(stream, "ADVISORY:  no translations were provided");
   fprintf(stream, " for these type = discrete attributes\n");
   for (i=0; i<num; i++) {
      fprintf(stream, " #%d \"%s\"", translations_not_provided[i],
	      att_info[i]->dscrp);
   }
}


char **process_translation( database_DS d_base, int n_att, att_DS att_dscrp,
					   int nat,   char **att_translation)
{
   fprintf(stderr,
           "ERROR: process_translation called with commented code in io-read-data.c\n");
   exit(1);

   /***************************
   char *att_props[3][STRLIMIT], missing_value_token, **value_trans;
   char ***unknown_translators, **translator;
   int i, range, ulength = 0;

   att_props = att_dscrp->props;
   range = att_dscrp->range;
   missing_value_token = db->unknown_token;
   return(att_translation);
   for (i=0; i<nat; i++) {
      value_trans = att_translation[i];
      if ((value_trans[0][0] == missing_value_token) &&
          (find_str_in_list(value_trans[0], unknown_translators, ulength) == -1))
	 ulength++;
	 if (unknown_translators == NULL)
	    unknown_translators = (char ***) malloc(unlength * sizeof(char **));
	 else unknown_translators =
	    (char ***) realloc(unknown_translators, unlength * sizeof(char **));
	 unknown_translators[ulength-1] = value_trans;
      }
   }
   att_translation = qsort(att_translation, nat, sizeof(int), int_cmp);
   for (i=0; i<ulength; i++) {
      translator = unknown_translators[i];
   }
   *********************************/
   return(att_translation);
}


/* READ_DATA_DOIT
   23nov94 wmt: first_read looks for first non-comment line
   27nov94 wmt: pass instance length to get_line_tokens
   29nov94 wmt: discard :list & :vector data_syntax
   25apr95 wmt: add binary data file capability
   16may95 wmt: converted binary i/o to ANSI

   reads datum with appropriate syntax and returns it
 */
char **read_data_doit( database_DS d_base, FILE *data_file_fp, int first_read,
                      int *instance_length_ptr, int n_comment_chars,
                      char *comment_chars, int binary_instance_length,
                      float **binary_instance_ptr)
{
  char **instance = NULL;
  int read_return_value;

  if (eqstring( G_data_file_format, "binary") == TRUE) {
    *binary_instance_ptr = (fptr) malloc( binary_instance_length);
    read_return_value = fread( *binary_instance_ptr, sizeof( char),
                             binary_instance_length, data_file_fp);
    if (read_return_value == 0) {       /* EOF */
      free( *binary_instance_ptr);
      *binary_instance_ptr = NULL;
    }
    *instance_length_ptr = d_base->n_atts;
  }
  else
    instance = get_line_tokens(data_file_fp, (int) d_base->separator_char,
                               n_comment_chars, comment_chars, first_read,
                               instance_length_ptr);
  return (instance);
}


/* TRANSLATE_INSTANCE
   27nov94 wmt: use minimum of num_atts & instance_length; free instance

   read in datum, even if incomplete. After total database
   is read in, Check-Data-Base finds all incomplete datum at one time
   updated by WMT 04 Sep 91 to allow excess length datum thru
   */
float *translate_instance( database_DS d_base, char **instance, int instance_length,
                          int n_datum, FILE *log_file_fp, FILE *stream)
{
   int n_att, num_atts = d_base->n_atts, i;
   float *new_instance;
   att_DS *att_info, attribute;

   new_instance = (fptr) malloc(num_atts * sizeof(float));
   att_info = d_base->att_info;
   for (n_att=0; n_att < min(num_atts, instance_length); n_att++) {
      attribute = att_info[n_att];
      if (eqstring(attribute->type, "real"))
	 new_instance[n_att] = (float) translate_real(d_base, n_datum, n_att,
                                                      instance[n_att]);
      else if (eqstring(attribute->type, "discrete"))
	 new_instance[n_att] =
	    (float) translate_discrete(d_base, n_att, attribute, instance[n_att],
                                       log_file_fp, stream);
      else if (eqstring(attribute->type, "dummy") ) 
	 new_instance[n_att] = 0.0;
      else{
	 fprintf(stderr, "ERROR[2]: unknown attribute type: %s\n",
                 attribute->type);
	 exit(1);
      }
   }
   for (i=0; i<instance_length; i++)
     free( instance[i]);
   free( instance);
   return (new_instance);
}


/* TRANSLATE_REAL
   01dec94 wmt: check value (string token) for invalid real numbers
   20dec94 wmt: return type to double 

   translate unknown symbols to (float) INT_UNKNOWN
   */
double translate_real( database_DS d_base, int n_datum, int n_att, char *value)
{
  int float_p, num_errors;
  double num;
  invalid_value_errors_DS invalid_error;
  
  if ((strlen(value) == 1) && (value[0] == d_base->unknown_token))
    num = FLOAT_UNKNOWN;
  else {
    num = atof_p(value, &float_p);
    if (float_p != TRUE) {
      d_base->num_invalid_value_errors++;
      num_errors = d_base->num_invalid_value_errors;
      if (d_base->invalid_value_errors == NULL) 
        d_base->invalid_value_errors =
          (invalid_value_errors_DS *) malloc(num_errors * sizeof(invalid_value_errors_DS));
      else
        d_base->invalid_value_errors =
          (invalid_value_errors_DS *) realloc(d_base->invalid_value_errors,
                                              (num_errors * sizeof(invalid_value_errors_DS)));
      invalid_error = (invalid_value_errors_DS) malloc(sizeof(struct invalid_value_errors));
      invalid_error->n_datum = n_datum;
      invalid_error->n_att = n_att;
      strcpy(invalid_error->value, value);
      d_base->invalid_value_errors[num_errors - 1] = invalid_error;
    }
  }
  return(num);
}


/* TRANSLATE_DISCRETE
   27nov94 wmt: initialize stats->observed when length is increased
   02dec94 wmt: revised building of translations: ***translations => **translations
   19jan96 wmt: allocate space for translations using (strlen( value) + 1),
                rather than sizeof(shortstr) -- prevents corruption of
                translation tables. Use very_long_str to print translators.

   Translate string tokens of printable characters to integers.
   Translate unknown symbols to INT_UNKNOWN, translate other values using:
   a: supplied translation table, and expand translation table if
   necessary => advisories; or b: translation table built as needed
   => no advisories. 
   */
int translate_discrete( database_DS d_base, int n_att, att_DS attribute,
                       char *value, FILE *log_file_fp, FILE *stream)
{
  int val, i;
  discrete_stats_DS stats;
  fxlstr long_str;
  very_long_str very_long_str;
  char caller[] = "translate_discrete";

  val = find_str_in_list(value, attribute->translations, attribute->n_trans);
  if (val == -1) {            /* not in attribute->translations yet */
    val = attribute->n_trans;
    attribute->n_trans += 1;
    if (attribute->translations == NULL)
      attribute->translations =
        (char **) malloc(attribute->n_trans * sizeof(char *));
    else attribute->translations =
      (char **) realloc(attribute->translations,
                         attribute->n_trans * sizeof(char *));
    /* attribute->translations[val] = (char *) malloc(sizeof(shortstr)); */
    attribute->translations[val] = (char *) malloc( strlen( value) + 1);
    strcpy(attribute->translations[val], value);
    stats = attribute->d_statistics;
    if (attribute->n_trans > stats->range) {
      /* fprintf(stderr, "Setting range to %d\n", attribute->n_trans); 27nov94 wmt */
      if (attribute->n_trans > stats->n_observed) {
        stats->n_observed = attribute->n_trans;
        stats->observed =
          (int *) realloc(stats->observed,
                          stats->n_observed * sizeof(int));
        for (i = stats->range; i < stats->n_observed; i++)
          stats->observed[i] = 0;
      }
      stats->range = attribute->n_trans;
      attribute->range = attribute->n_trans;
      /*       if (member_int(n_att, */
      /*         d_base->translations_supplied_p, d_base->num_tsp)) 02dec94 wmt */
      safe_sprintf( long_str, sizeof( long_str), caller,
                   "ADVISORY[2]: for attribute #%d: \"%s\" range increased to %d,\n",
                   n_att, attribute->dscrp, attribute->n_trans);
      to_screen_and_log_file(long_str, log_file_fp, stream, TRUE);
      safe_sprintf( very_long_str, sizeof( very_long_str), caller,
                   "             for value %d -- translator (%d %s).\n", val, val,
                   attribute->translations[val]);
      to_screen_and_log_file(very_long_str, log_file_fp, stream, TRUE);
    }
  }
  return(val);
}


/* GET_LINE_TOKENS
   21oct94 wmt: modified
   23nov94 wmt: handle comment lines
   27nov94 wmt: set instance_length_ptr; use very_long_str
   30nov94 wmt: add comment_char functionality
   28feb95 wmt: pass VERY_LONG_STRING_LENGTH to read_line
   19jan96 wmt: add length checking for "form"; make it and length check
                for datum_string explicit
   */
char **get_line_tokens( FILE *stream, int separator_char, int n_comment_chars,
                       char *comment_chars, int first_read, int *instance_length_ptr)
{
  int position=0, length=0;
  char **line_tokens = NULL;
  char form[VERY_LONG_TOKEN_LENGTH];
  char datum_string[VERY_LONG_STRING_LENGTH];
  int datum_string_first_char;

  do {
    position = 0;
    *instance_length_ptr = 0;
    strcpy(form, "");
    if (read_line( datum_string, VERY_LONG_STRING_LENGTH, stream) == TRUE) {
      /* fprintf( stderr, "datum_string1: '%s'\n", datum_string); */
      while ((first_read == TRUE) &&
             (datum_string_first_char = (int) datum_string[0]) &&
             ((datum_string_first_char == '#') || (datum_string_first_char == '!') ||
              (datum_string_first_char == ';') || (datum_string_first_char == ' ') ||
              (datum_string_first_char == '\n'))) {
        read_line( datum_string, VERY_LONG_STRING_LENGTH, stream);
        /* fprintf( stderr, "datum_stringn: '%s'\n", datum_string); */
      }
      position = read_from_string( datum_string, form, VERY_LONG_TOKEN_LENGTH,
                                  separator_char, n_comment_chars, comment_chars,
                                  position);
      /* fprintf( stderr, "form1: %s\n", form); */
      while ((eqstring(form, "eof") == FALSE) && (eqstring(form, "comment") == FALSE)) {
        length++;
        if (line_tokens == NULL)
          line_tokens = (char **) malloc(length * sizeof(char *));
        else line_tokens =
          (char **) realloc(line_tokens, length * sizeof(char *));
        /* allow for end of string char */
        line_tokens[length - 1] =
          (char *) malloc((strlen(form) + 1) * sizeof(char)); 
        strcpy(line_tokens[length - 1], form);
        strcpy(form, "");
        position = read_from_string(datum_string, form, VERY_LONG_TOKEN_LENGTH,
                                    separator_char, n_comment_chars, comment_chars,
                                    position);
        /* fprintf( stderr, "formn: %s\n", form); */
      }
      *instance_length_ptr = length;
      /* fprintf( stderr, "instance_length = %d\n", length); */
      if ((eqstring(form, "comment") == FALSE) && (eqstring(form, "eof") == FALSE) &&
          (length <= 1)) {
        fprintf(stderr,
                "ERROR[1]: data is of type :vector or :list, but only :line is handled\n");
        exit(1);
      }
    }
  }
  while (eqstring(form, "comment") == TRUE);

   /* if (length != 0) */
   /*   printf("1st token: %s\n", line_tokens[0]); */

  return(line_tokens);
}


/* READ_FROM_STRING
   30nov94 wmt: add comment_char functionality
   03dec94 wmt: pass multiple comment chars
   13dec94 wmt: allow separator_char, space, & tab  to be repeated between tokens
   30dec94 wmt: discard double quotes around string tokens and keep imbedded blanks
   19jan96 wmt: add length checking for s2
   */
int read_from_string( char *s1, char *s2, int string_limit, int separator_char,
                     int n_comment_chars, char *comment_chars, int position)
{
  int i = 0, str_len = strlen(s1), n_char, comment_p = FALSE, in_string_p = FALSE;
  char double_quote = '\"';     /* " */

  if (position >= str_len) {
    strcpy(s2, "eof");
    return(position);
  }
  if (s1[position] == EOF) {
    strcpy(s2, "eof");
    return(position);
  }
  if (position == 0) {
    for (n_char=0; n_char < n_comment_chars; n_char++) {
      if (s1[0] == comment_chars[n_char]) {
        comment_p = TRUE;
        break;
      }
    }
    if (comment_p == TRUE) {
      strcpy(s2, "comment");
      return(position);
    }
  }
  /* read past multiple separator chars, spaces or tabs */
  while ((s1[position] == (char) separator_char) || (s1[position] == ' ') ||
         (s1[position] == '\t'))
    position++;

  while ((in_string_p == TRUE) ||
         ((s1[position] != ' ') && (s1[position] != '\n') &&
          (s1[position] != '\t') && (s1[position] != EOF) &&
          (s1[position] != (char) separator_char) && (i < str_len))) {
    /*       printf("%d:%d:%c ", i, (int) s1[position], s1[position]);  */
    if (s1[position] != double_quote) {      /* discard double_quotes around strings */
      if (i >= (string_limit - 1)) {
        fprintf( stderr, "ERROR: read_from_string read a token longer than %d characters\n",
            string_limit-1);
        abort();
      }    
      s2[i] = s1[position];
      i++;
    }
    if (s1[position] == double_quote) {
      if (i == 0) 
        in_string_p = TRUE;
      else
        in_string_p = FALSE;
    }
    position++;
  }
  s2[i] = '\0';

  if (i == 0) {
    strcpy(s2, "eof");
    return(position);
  }
  return(position + 1);
}


/* READ_LINE
   28feb95 wmt: pass string_limit as parameter; check for excess line length
   05mar97 wmt: only return FALSE if no chars have been read -- allows
                last line with no new-line to be read
   */
int read_line( char *s, int string_limit, FILE *stream)
{
  int c = 0, i;

  for (i=0; i<(string_limit-1) && ((c=fgetc(stream)) != EOF) && (c != '\n'); i++)
    s[i] = c;
  if (c == '\n') {
    s[i] = c;
    i++;
  }
  s[i] = '\0';
  if ((c != '\n') && (c != EOF)) {
    fprintf( stderr,
            "ERROR: read_line read a line longer than %d characters\n",
            string_limit-1);
    abort();
  }

  /* fprintf( stderr, "read_line: i %d string %s\n", i, s); */


  if ((c == EOF) && (i == 0))
    return(FALSE);
  else
    return(TRUE);
}


/* Checks d-base to assure the correct number of elements in each datum.
   return error-cnt
   updated by wmt 03 Sep 91 to print out bad datum
   18mar92 WMT - added *print-level*, *print-length* bindings */


/* return count of supplied attribute translators not used */

void find_att_statistics( database_DS d_base, FILE *log_file_fp, FILE *stream)
{
   int n_att;
   att_DS att, *att_info;

   att_info = d_base->att_info;
   for (n_att=0; n_att<d_base->n_atts; n_att++) {
      att = att_info[n_att];
      if (find_str_in_table(att->type, G_att_type_data, NUM_ATT_TYPES) > -1) {
	 if (eqstring(att->type, "real") == TRUE)
	    find_real_stats(d_base, n_att, log_file_fp, stream);
	 else if (eqstring(att->type, "discrete") == TRUE)
	    find_discrete_stats(d_base, n_att);
      }
      else {
	 fprintf(stderr, "ERROR: (find_att_statistics) unknown attribute type: %s\n",
                 att->type);
	 abort();
      }
   }
}


/* FIND_REAL_STATS
   27nov94 wmt: use percent_equal for float tests; skip incomplete datum
   15dec94 wmt: redo advisory message
   27dec94 wmt: use direct variance calculation to prevent numerical problems

   compute means, etc for real attributes & return count of invalid values
*/
void find_real_stats( database_DS d_base, int n_att, FILE *log_file_fp, FILE *stream)
{
  int n_datum, count = 0, missing = 0, percent_error;
  float mn = MOST_POSITIVE_SINGLE_FLOAT, mx = MOST_NEGATIVE_SINGLE_FLOAT;
  float val, error;
  double sum = 0.0, sum_sq = 0.0, mean = 0.0, variance = 0.0, double_val;
  double float_unknown = FLOAT_UNKNOWN, rel_error = REL_ERROR;
  float **data = d_base->data;
  att_DS att = d_base->att_info[n_att];
  real_stats_DS stats = att->r_statistics;
  int *datum_length = d_base->datum_length;
  fxlstr str;
  char caller[] = "find_real_stats";

  error = att->error;
  for (n_datum=0; n_datum < d_base->n_data; n_datum++) {
    if (n_att < datum_length[n_datum]) {
      val = data[n_datum][n_att];
      if (percent_equal( (double) val, float_unknown, rel_error))
        missing++;
      else {
        count++;
        mn = min(mn, val);
        mx = max(mx, val);
        sum += val;
      }
    }
  }
  if (count > 0) {
    mean = sum / ((double) count);
    for (n_datum=0; n_datum < d_base->n_data; n_datum++) {
      if (n_att < datum_length[n_datum]) {
        double_val = (double) data[n_datum][n_att];
        if (! percent_equal( double_val, float_unknown, rel_error))
          sum_sq += square( double_val - mean);
      }
    }
    variance = sum_sq / ((double) count);
  }
  if ((count == 0) ||           /* All values are missing */
      (mx == mn)) {             /* All known values are identical */
    strcpy(att->warnings_and_errors->single_valued_warning, "true");
  }
  else if ((error != 0.0) && (error > (.1 * (mx - mn)))) {
    percent_error = iround( (double) (100.0 * (error / (mx - mn))));
    safe_sprintf( str, sizeof( str), caller,
                 "ADVISORY[2]: attribute #%d: \"%s\", the error %f is %d%% \n"
                 "             of the range %f.\n",
                 n_att, att->dscrp, att->error, percent_error, (mx - mn));
    to_screen_and_log_file( str, log_file_fp, stream, TRUE);
  }
  store_real_stats(stats, att, count, mean, variance, missing, (double) mx, (double) mn);
}


/* STORE_REAL_STATS
   19dec94 wmt: make parms double rather than float
   27dec94 wmt: use direct variance calculation to prevent numerical problems

   store real stats in statistics structure
   */
void store_real_stats( real_stats_DS statistics, att_DS att,
	       int count, double mean, double variance, int missing, double  mx,
                      double mn)
{
  statistics->count = count;
  if (count == 0) {
    statistics->mx = 0.0;
    statistics->mn = 0.0;
    statistics->mean = 0.0;
    statistics->var = 0.0;
  }
  else {
    statistics->mx = (float) mx;
    statistics->mn = (float) mn;
    statistics->mean = (float) mean;
    statistics->var = (float) variance;
  }
  att->missing = (missing > 0)?TRUE:FALSE;
}


/* FIND_DISCRETE_STATS
   29nov94 wmt: skip incomplete datum; put ( length < range ) in unused translators
   18feb98 wmt: for number of translators less than .hd2 range, reduce the
                range and output advisory, rather than outputting warning -- REMOVED 
                13mar98 JCS due to incompatablility with previous results files

   check for missing values occurring and non-occurrences of
   specified translations -- return non-occurrence count
   23mar92 - WMT - single-valued functionality changed from error to warning
   -error slot retained for backward compatability to existing .results files 
   */
void find_discrete_stats( database_DS d_base, int n_att)
{
  int i, length, n_datum, missing_value, missing_value_cnt = 0, *accumulator;
  int ulength=0, val;
  float *unused_translators = NULL;
  float *datum, **data = d_base->data;
  att_DS att = d_base->att_info[n_att];
  discrete_stats_DS stats = att->d_statistics;
  int *datum_length = d_base->datum_length;
  /* fxlstr long_str;
     char caller[] = "find_discrete_stats"; */

  missing_value = find_str_in_list("nil", att->translations, att->n_trans);

  accumulator = stats->observed;
  if (accumulator == NULL)
    length = 0;
  else 
    length = stats->n_observed;

  for (i=0; i<length; i++)
    accumulator[i] = 0;

  for (n_datum=0; n_datum<d_base->n_data; n_datum++) {
    if (n_att < datum_length[n_datum]) {
      datum = data[n_datum];
      val = (int) datum[n_att];
      accumulator[val] ++;

      /* this is dealing with translated values so missing_value is 
         the integer value to which missing values are translated*/
      if ((missing_value != -1) && (val == missing_value)) /* count missing values */
        missing_value_cnt++;
    }
  }
  att->missing = (missing_value_cnt > 0)?TRUE:FALSE;

  for (i=0; i<length; i++) {
    if (accumulator[i] == 0) {
      if (ulength++ == 0)
        unused_translators = (float *) malloc(ulength * sizeof(float));
      else unused_translators =
        (float *) realloc(unused_translators, ulength * sizeof(float));
      unused_translators[ulength - 1] = (float) i;
    }
  }
  if ((length - ulength) == 1) {
    strcpy(att->warnings_and_errors->single_valued_warning, "true");

  } else if (ulength > 0) {
     /* Do flag this as a warning -- 
       reducing range renders AC incompatable with results of previous versions. */
    unused_translators = 
      (float *) realloc(unused_translators, (ulength + 1) * sizeof(float));  
    unused_translators[ulength ] = MOST_NEGATIVE_SINGLE_FLOAT;  
    att->warnings_and_errors->unused_translators_warning = unused_translators;  
    /* fprintf( stderr, "find_discrete_stats: stats->range %d att->range %d" 
              " att->n_trans %d length %d\n",   
              stats->range, att->range, att->n_trans, length);  */

    /*  Range reduction: seemed like a good idea. But this leaves AC unable to 
        search, report or predict from results files made by prior AC versions.    
        stats->range = att->n_trans;
        att->range = att->n_trans;
        att->d_statistics->range = att->n_trans;
        att->d_statistics->n_observed = att->n_trans;
        safe_sprintf( long_str, sizeof( long_str), caller,
        "ADVISORY[2]: for attribute #%d: \"%s\" range decreased to %d.\n",
        n_att, att->dscrp, att->n_trans);
        to_screen_and_log_file(long_str, G_log_file_fp, G_stream, TRUE);
    */
  } else if ((length < att->range) && (ulength == 0)) {
  /* do this since .hd2 defined translators are not implemented 03dec94 wmt */
    unused_translators = (float *) malloc(sizeof(float));
    unused_translators[ulength - 1] = (float) length;
  }
}


/* OUTPUT_ATT_STATISTICS
   10may95 wmt: new

   output real & discrete statistics from data read in.
   */
void output_att_statistics( database_DS d_base, FILE *log_file_fp, FILE *stream)
{
  int n_atts = d_base->n_atts, n_att, stats_to_output_p = FALSE;
  /* int i, j; */

  output_created_translations( d_base, log_file_fp, stream);                           

  for (n_att=0; n_att<n_atts; n_att++) {
    if (d_base->att_info[n_att]->r_statistics != NULL) {
      stats_to_output_p = TRUE;
      break;
    }
  }
  if ((stats_to_output_p == TRUE) && (log_file_fp != NULL)) {
    to_screen_and_log_file
      ("ADVISORY[1]: real statistics [ min < (mean : std dev) < max ] built \n"
       "             from input data --\n",
       log_file_fp, stream, TRUE);
    for (n_att=0; n_att<n_atts; n_att++) 
      output_real_att_statistics( d_base, n_att, log_file_fp, stream);
  }
  /* test for landsat binary .db2 file
  for (i=0; i<301; i++) {
    fprintf( stderr, "\npixel_index %d\n", i);
    for (j=0; j<14; j++)
      fprintf ( stderr, "%e  ", d_base->data[i][j]);
  }
  exit(1); */
}


/* OUTPUT_REAL_ATT_STATISTICS
   10may95 wmt: new
   20may97 wmt: added check for variance exceeding infinity

   output real statistics for attribute n_att
   */
void output_real_att_statistics( database_DS d_base, int n_att, FILE *log_file_fp,
                                FILE *stream)
{
  att_DS att;
  real_stats_DS r_statistics;
  fxlstr str;
  char caller[] = "output_real_att_statistics";

  if (log_file_fp != NULL) {
    att = d_base->att_info[n_att];
    r_statistics = att->r_statistics;
    if (att->r_statistics != NULL) {
      safe_sprintf( str, sizeof( str), caller,
                   "   Attribute #%d, \"%s\":\n      [", n_att, att->dscrp);
      to_screen_and_log_file( str, log_file_fp, stream, TRUE);
      safe_sprintf( str, sizeof( str), caller,
                    " %11.4e < (%11.4e : %10.4e) < %11.4e ]\n",
                    r_statistics->mn, r_statistics->mean, sqrt(r_statistics->var),
                    r_statistics->mx);
      to_screen_and_log_file( str, log_file_fp, stream, TRUE);
      if (r_statistics->var > INFINITY) {
        fprintf( stderr, "\nERROR: (output_real_att_statistics) attribute #%d: \"%s\", \n"
                         "       the variance exceeds %e\n",
                 n_att, att->dscrp, INFINITY);
        abort();
      }
    }      
  }
}


/* OUTPUT_CREATED_TRANSLATIONS
   02dec94 wmt: new
   10may95 wmt: add discrete value occurrance count
   19jan96 wmt: increase output string length

   output discrete translations built from the data read in - (see translate_discrete)
   */
void output_created_translations( database_DS d_base, FILE *log_file_fp, FILE *stream)
{
  int n_atts = d_base->n_atts, n_att, n_trans, translations_to_output_p = FALSE;
  att_DS att;
  char str[VERY_LONG_TOKEN_LENGTH];
  char caller[] = "output_created_translations";

  for (n_att=0; n_att<n_atts; n_att++) {
    if (d_base->att_info[n_att]->n_trans > 0) {
      translations_to_output_p = TRUE;
    break;
    }
  }

  if ((translations_to_output_p == TRUE) && (log_file_fp != NULL)) {
    to_screen_and_log_file
      ("ADVISORY[1]: discrete translations [ (internal external):count ... ] built \n"
       "             from input data --\n",
       log_file_fp, stream, TRUE);
    for (n_att=0; n_att<n_atts; n_att++) {
      att = d_base->att_info[n_att];
      if (att->n_trans > 0) {
        safe_sprintf(str, sizeof( str), caller,
                     "   Attribute #%d, \"%s\":\n      [", n_att, att->dscrp);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        for (n_trans=0; n_trans < att->n_trans; n_trans++) {
          safe_sprintf( str, sizeof( str), caller, " (%d %s):%d", n_trans,
                       att->translations[n_trans],
                       att->d_statistics->observed[n_trans]);
          to_screen_and_log_file( str, log_file_fp, stream, TRUE);
        }
        to_screen_and_log_file( " ]\n", log_file_fp, stream, TRUE);
      }      
    }
  } 
}


/* CHECK_ERRORS_AND_WARNINGS
   22jan95 wmt: new
   24mar99 wmt: write warnings/errors to log file

   check for changed or corrupted data/header/model files
   */
void check_errors_and_warnings( database_DS database, model_DS *models, int num_models)
{
  int total_error_cnt = 0, total_warning_cnt = 0;
  FILE *log_file_fp = NULL, *stream = NULL;
  shortstr output_msg_type;

  strcpy( output_msg_type, ":expand");

  output_messages( database, models, num_models, log_file_fp, stream,
                  &total_error_cnt, &total_warning_cnt, output_msg_type);
  if ((total_error_cnt + total_warning_cnt) > 0) {
    output_messages( database, models, num_models, G_log_file_fp, G_stream, &total_error_cnt,
                  &total_warning_cnt, output_msg_type);
    check_stop_processing( total_error_cnt, total_warning_cnt, G_log_file_fp, G_stream);
  }
}
