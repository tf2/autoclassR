#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#ifndef _WIN32
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


/* AUTOCLASS_REPORTS
   01feb95 wmt: new - adapted from ac-x:autoclass-reports-from-results-file
   26apr95 wmt: change NULL to 0 for 6th arg of defparam
   26may95 wmt: add prediction capability
   15oct96 wmt: compute last_classification_p to indicate when database
                should be freed
   17mar97 wmt: added report_mode
   25apr97 wmt: add prediction_p
   17feb98 wmt: write screen output to log file as well
   02dec98 wmt: write params to log file

   "-reports": generate the influence value, class cross-reference & case
   cross-reference reports

   "-predict": with test_data_file non-"", generate the class cross-reference
   & case cross-reference reports for test_data_file prediction
   */
int autoclass_reports( char *results_file_ptr, char *search_file_ptr,
                      char *reports_params_file_ptr, char *influ_vals_file_ptr,
                      char *xref_class_file_ptr, char *xref_case_file_ptr,
                      char *test_data_file, char *log_file_ptr)
{
  /* number of clsfs in the .results file for which to generate reports,
     starting with the first (most probable) */
  int n_clsfs = 1;
  /* if specified, this is a zero-based index list of clsfs in the clsf
     sequence read from the .results file */
  static int clsf_n_list[MAX_CLSF_N_LIST] = {END_OF_INT_LIST};
  /* type of reports to generate: "all", "influence-values", "xref_case", or
     "xref_class" */
  shortstr report_type = "all";
  /* mode of reports to generate. "text" is formatted text layout.  "data"
     is numerical -- suitable for further processing. */
  shortstr report_mode = "text";
  /* The default value does not insert # in column 1 of most report_mode = "data"
     header lines.  If specified as true, the comment character will be inserted
     in most header lines */
  unsigned int comment_data_headers_p = FALSE;
  /* if specified, the number of attributes to list in influence values report.
     if not overridden, all attributes will be output

     the output for report_mode = "text" uses these criteria to make it
     easier to parse with awk:
        remove ":"'s
        remove "..."'s
        all section headers start in column 1
        secondary lines of an item can have leading blanks
     */
  int num_atts_to_list = ALL_ATTRIBUTES;
  /* if specified, a list of attribute numbers (zero-based), whose values will
     be output in the "xref_class" report along with the case probabilities */
  static int xref_class_report_att_list[MAX_CLASS_REPORT_ATT_LIST] = {END_OF_INT_LIST};
  /* The default value lists each class's attributes in descending order of
     attribute influence value, and uses ".influ-o-text-n" as the
     influence values report file type.  If specified as false, then each 
     class's attributes will be listed in ascending order by attribute number.  
     The extension of the file generated will be "influ-no-text-n". */
  unsigned int order_attributes_by_influence_p = TRUE;
  /* The default value asks the user whether to coninue or not when data
     definition warnings are found.  If specified as false, then AutoClass
     will continue, despite warnings -- the warnings will continue to be
     output to the terminal and the log file. */
  unsigned int break_on_warnings_p = TRUE;
  /* The default value tells AutoClass to free the majority of its allocated
     storage.  This is not required, and in the case of DEC Alpha's causes
     core dump.  If specified as false, AutoClass will not attempt to free
     storage. */
  unsigned int free_storage_p = TRUE;
  /* Determines how many lessor class probabilities will be printed for the 
     case and class cross-reference reports.  The default is to print the
     most probable class probability value and up to 4 lessor class prob-
     ibilities.  Note this is true for both the "text" and "data" class
     cross-reference reports, but only true for the "data" case cross-
     reference report.  The "text" case cross-reference report only has the
     most probable class probability. */
  int max_num_xref_class_probs = 5;
  /* sigma_contours_att_list = 
     If specified, a list of real valued attribute indices (from .hd2 file) 
     will be to compute sigma class contour values, when generating 
     influence values report with the data option (report_mode = "data"). 
     If not specified, there will be no sigma class contour output.
     (e.g. sigma_contours_att_list = 3, 4, 5, 8, 15) */
  static int sigma_contours_att_list[MAX_N_SIGMA_CONTOUR_LIST] = {END_OF_INT_LIST};


  /* -------------------------------------------------------------------------*/

  int r_parms_error_cnt, n_params = 0, i, clsf_n_list_from_s_params = FALSE;
  int num_clsf_n_list, num_clsfs_found, n_clsf, clsf_num, prediction_p = FALSE;
  int training_prediction_p = FALSE, last_clsf_p = FALSE;
  int sigma_contours_list_len = 0;
  search_DS search;
  FILE *search_file_fp, *reports_params_file_fp;
  static FILE *log_file_fp = NULL;
  PARAM params[MAXPARAMS];
  clsf_DS *clsf_seq, clsf, test_clsf = NULL, training_clsf;
  xref_data_DS xref_data = NULL;
  fxlstr str, autoclass_mode;
  char caller[] = "autoclass_reports";
  unsigned int log_file_p = TRUE;

  /* -------------------------------------------------------------------------*/

  G_stream = stdout;
  params[0].paramptr = NULL;
  defparam( params, n_params++, "n_clsfs", TINT, &n_clsfs, 0);
  defparam( params, n_params++, "clsf_n_list", TINT_LIST, clsf_n_list,
           MAX_CLSF_N_LIST);
  defparam( params, n_params++, "report_type", TSTRING, report_type,
           SHORT_STRING_LENGTH);
  defparam( params, n_params++, "report_mode", TSTRING, report_mode,
           SHORT_STRING_LENGTH);
  defparam( params, n_params++, "comment_data_headers_p", TBOOL,
           &comment_data_headers_p, 0);
  defparam( params, n_params++, "num_atts_to_list", TINT, &num_atts_to_list, 0);
  defparam( params, n_params++, "xref_class_report_att_list", TINT_LIST,
           xref_class_report_att_list, MAX_CLASS_REPORT_ATT_LIST);
  defparam( params, n_params++, "order_attributes_by_influence_p", TBOOL,
           &order_attributes_by_influence_p, 0);
  defparam( params, n_params++, "break_on_warnings_p", TBOOL,
           &break_on_warnings_p, 0);
  defparam( params, n_params++, "free_storage_p", TBOOL,
           &free_storage_p, 0);
  defparam( params, n_params++, "max_num_xref_class_probs", TINT,
            &max_num_xref_class_probs, 0);
  defparam( params, n_params++, "sigma_contours_att_list", TINT_LIST,
           sigma_contours_att_list, MAX_N_SIGMA_CONTOUR_LIST);

  /* -------------------------------------------------- */

  if (eqstring( test_data_file, "") != TRUE)
    prediction_p = TRUE;
  /* read reports params file */
  fprintf( stdout, "\n\n\n### Starting Check of %s%s\n",
          (reports_params_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
          reports_params_file_ptr);
  reports_params_file_fp = fopen( reports_params_file_ptr, "r");   
  r_parms_error_cnt = getparams( reports_params_file_fp, params);
  fclose( reports_params_file_fp);

  if ((eqstring( report_mode, "text") != TRUE) &&
      (eqstring( report_mode , "data") != TRUE)) {
    fprintf( stderr, "ERROR: report_mode must be  either \"text\", or "
            "\"data\".\n");
    r_parms_error_cnt++;
  }    

  if (prediction_p == TRUE) {
    if ((eqstring( report_type, "all") != TRUE) &&
        (eqstring( report_type , "xref_case") != TRUE) &&
        (eqstring( report_type , "xref_class") != TRUE)) {
      fprintf( stderr, "ERROR: report_type must be  either \"all\", "
               "\"xref_case\", or \"xref_class\".\n");
      r_parms_error_cnt++;
    }
  } else {
    if ((eqstring( report_type, "all") != TRUE) &&
        (eqstring( report_type , "influence_values") != TRUE) &&
        (eqstring( report_type , "xref_case") != TRUE) &&
        (eqstring( report_type , "xref_class") != TRUE)) {
      fprintf( stderr, "ERROR: report_type must be  either \"all\", "
               "\"influence_values\", \"xref_case\", or \"xref_class\".\n");
      r_parms_error_cnt++;
    }
  }

  if ((eqstring( report_mode, "text") == TRUE) &&
      (comment_data_headers_p == TRUE)) {
    fprintf( stderr, "ERROR: report_mode must be \"data\" if "
             "comment_data_headers_p is true.\n");
    r_parms_error_cnt++;
  }

  if ( max_num_xref_class_probs < 1) {
    fprintf( stderr, "ERROR: max_num_xref_class_probs must be greater than 0.\n");
    r_parms_error_cnt++;
  }

  fprintf(stdout, "### Ending Check of   %s%s\n",
          (reports_params_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
          reports_params_file_ptr);
  for (i=0; i<n_params; i++) 
    if (eqstring( params[i].paramname, "clsf_n_list")) {
      if (params[i].overridden_p == TRUE)
        clsf_n_list_from_s_params = TRUE;
      break;
    }
  if (r_parms_error_cnt > 0)
    exit(1);
  G_break_on_warnings = break_on_warnings_p;
  /* end of reports params processing */

  if (log_file_ptr[0] == '\0') {
    G_log_file_fp = NULL;
    log_file_p = FALSE;
  } else {
    log_file_fp = fopen( log_file_ptr, "a");
    G_log_file_fp = log_file_fp;
  }
  safe_sprintf( str, sizeof( str), caller,
               "\n\nAUTOCLASS C (version %s) STARTING at %s \n\n", G_ac_version,
               format_universal_time(get_universal_time()));
  to_screen_and_log_file(str, G_log_file_fp, G_stream, TRUE);
  if (log_file_p == TRUE) {
    if (prediction_p == TRUE)
      strcpy( autoclass_mode, "-PREDICT");
    else
      strcpy( autoclass_mode, "-REPORTS");
    fprintf(log_file_fp, "AUTOCLASS %s default parameters:\n", autoclass_mode);
    putparams(log_file_fp, params, FALSE);
  }
  if (log_file_p == TRUE) {
    fprintf(log_file_fp, "USER supplied parameters which override the defaults:\n");
    putparams(log_file_fp, params, TRUE);
  }

  if (clsf_n_list_from_s_params != TRUE) {
    for (i=(n_clsfs - 1); i>=0; i--) {
      /* index 0 is first on list */
      push_int_list( clsf_n_list, &num_clsf_n_list, i+1, MAX_CLSF_N_LIST);
    }
  }
  clsf_seq = initialize_reports_from_results_pathname( results_file_ptr, clsf_n_list,
                                                      &num_clsfs_found, 
                                                      training_prediction_p);
  search_file_fp = fopen( search_file_ptr, "r");
  search = get_search_from_file( search_file_fp, search_file_ptr);
  fclose( search_file_fp);

  fprintf( G_stream, "\n");
  for (n_clsf=0; n_clsf<num_clsfs_found; n_clsf++) {
    clsf = clsf_seq[n_clsf];
    clsf_num = clsf_n_list[n_clsf];
    if (n_clsf == ( num_clsfs_found - 1)) {
      last_clsf_p = TRUE;
    }
    if (clsf_search_validity_check( clsf, search) < 0) {
      fprintf( stderr, "ERROR: .results[-bin] file and .search file not from the same run\n");
      exit(1);
    }
    if ((eqstring (report_mode, "data") == TRUE) &&
        (sigma_contours_att_list[0] != END_OF_INT_LIST)) {
      for (i=0; sigma_contours_att_list[i] != END_OF_INT_LIST; i++) {
        if ( sigma_contours_att_list[i] > (clsf->database->input_n_atts - 1)) {
          fprintf( stderr, "ERROR: sigma_contours_att_list index %d cannot exceed %d --\n"
                   "       use indices from .hd2 file.\n",
                   sigma_contours_att_list[i], clsf->database->input_n_atts - 1);
          exit(1);
        }
        sigma_contours_list_len++;
      }
      if (sigma_contours_list_len < 2) {
        fprintf( stderr, "ERROR: sigma_contours_att_list length must be >= 2.\n");
        exit(1);
      }
    }
    if (prediction_p == FALSE) {        /* -reports */
      if ((eqstring( report_type, "all") == TRUE) ||
          (eqstring( report_type, "influence_values") == TRUE))
        influence_values_report_streams( clsf, search, num_atts_to_list, report_mode,
                                         influ_vals_file_ptr, results_file_ptr,
                                         clsf_num, test_clsf,
                                         order_attributes_by_influence_p,
                                         comment_data_headers_p,
                                         sigma_contours_att_list);

      if (eqstring( report_type, "all") == TRUE)
        xref_data = case_class_data_sharing( clsf, report_mode, report_type,
                                             xref_class_file_ptr, xref_case_file_ptr,
                                             results_file_ptr,
                                             xref_class_report_att_list,
                                             clsf_num, test_clsf, last_clsf_p,
                                             prediction_p, comment_data_headers_p,
                                             max_num_xref_class_probs);

      if (eqstring( report_type, "xref_case") == TRUE) {
        /* pass empty xref_class_report_att_list */
        xref_class_report_att_list[0] = END_OF_INT_LIST;
        xref_data = xref_get_data( clsf, "case", xref_class_report_att_list,
                                   xref_data, last_clsf_p, prediction_p,
                                   max_num_xref_class_probs);

        case_report_streams( clsf, report_mode, xref_case_file_ptr, results_file_ptr,
                             xref_data, clsf_num, test_clsf, last_clsf_p,
                             comment_data_headers_p, max_num_xref_class_probs);
      }

      if (eqstring( report_type, "xref_class") == TRUE) {
        xref_data = xref_get_data( clsf, "class", xref_class_report_att_list,
                                  xref_data, last_clsf_p, prediction_p,
                                   max_num_xref_class_probs);

        class_report_streams( clsf, report_mode, xref_class_file_ptr, results_file_ptr, 
                              xref_class_report_att_list, xref_data, clsf_num,
                              test_clsf, last_clsf_p, comment_data_headers_p,
                              max_num_xref_class_probs);
      }
    }
    else {              /* -predict */
      training_clsf = clsf;
      test_clsf = autoclass_predict( test_data_file, training_clsf, test_clsf,
                                     log_file_fp, log_file_ptr);

      case_class_data_sharing( clsf, report_mode, report_type, xref_class_file_ptr,
                               xref_case_file_ptr, results_file_ptr,
                               xref_class_report_att_list, clsf_num,
                               test_clsf, last_clsf_p, prediction_p,
                               comment_data_headers_p, max_num_xref_class_probs);    
    }
    /* free storage from this clsf's reports */
    if (xref_data != NULL) {
      for (i=0; i<clsf->database->n_data; i++) {
        if (xref_data[i].discrete_attribute_data != NULL)
          free( xref_data[i].discrete_attribute_data);
        if (xref_data[i].real_attribute_data != NULL)
          free( xref_data[i].real_attribute_data);
        if (xref_data[i].wt_class_pairs != NULL)
          free( xref_data[i].wt_class_pairs);
      }
      free ( xref_data);
      xref_data = NULL;
    }
    if (free_storage_p == TRUE) {
      free_clsf_class_search_storage( clsf,
                                     (n_clsf == (num_clsfs_found - 1)) ? search : NULL,
                                     (n_clsf == (num_clsfs_found - 1)) ? TRUE : FALSE);
      if (prediction_p == TRUE)
        free_clsf_class_search_storage( test_clsf, NULL, FALSE);
    }
  }
  safe_sprintf( str, sizeof( str), caller,
               "\n\nAUTOCLASS C (version %s) STOPPING at %s \n\n", G_ac_version,
               format_universal_time(get_universal_time()));
  to_screen_and_log_file(str, G_log_file_fp, G_stream, TRUE);
  if (log_file_ptr[0] != '\0') {
    fclose( log_file_fp);
  }
  return (0);
}


/* CLSF_SEARCH_VALIDITY_CHECK
   06feb95 wmt: new

   check that clsf is in the trials of search - return position of clsf
   */
int clsf_search_validity_check( clsf_DS clsf, search_DS search)
{
  double clsf_id = clsf->log_a_x_h;
  search_try_DS *tries = search->tries;
  int n_try, position = -1;

  for (n_try=0; n_try<search->n_tries; n_try++)
    if (percent_equal( clsf_id, tries[n_try]->ln_p, SINGLE_FLOAT_EPSILON) == TRUE) {
      position = n_try;
      break;
    }
  return (position);
}


/* INFLUENCE_VALUES_REPORT_STREAMS
   06feb95 wmt: new
   24jul95 wmt: add order_attributes_by_influence_p
   17mar97 wmt: add report_mode, 
   14may97 wmt: add comment_data_headers_p
   24jun97 wmt: add start_sigma_contours_att & stop_sigma_contours_att
   28feb98 wmt: replace start/stop_sigma_contours_att with
                sigma_contours_att_list

   create streams from pathnames for influence values reports

   */
void influence_values_report_streams( clsf_DS clsf, search_DS search, int num_atts_to_list,
                                      shortstr report_mode, char *influ_vals_file_ptr,
                                      char *results_file_ptr, int clsf_num, clsf_DS test_clsf,
                                      unsigned int order_attributes_by_influence_p,
                                      unsigned int comment_data_headers_p,
                                      int_list sigma_contours_att_list)
{
  static fxlstr influence_report_pathname;
  char clsf_num_string[4];
  FILE *influence_report_fp = NULL;
  int header_information_p = FALSE, num_chars_to_trunc = 5, trunc_index;
  fxlstr str;
  char caller[] = "influence_values_report_streams";

  influence_report_pathname[0] = clsf_num_string[0] = '\0';

  strcpy( influence_report_pathname, influ_vals_file_ptr);
  /* truncate "-----.influ-text-" to "----.influ-" */
  trunc_index = (int) strlen( influence_report_pathname) - num_chars_to_trunc;
  influence_report_pathname[trunc_index] = '\0';  
  if (order_attributes_by_influence_p == TRUE)
    strcat( influence_report_pathname, "o-");
  else
    strcat( influence_report_pathname, "no-");

  if (eqstring (report_mode, "text") == TRUE)
    strcat( influence_report_pathname, "text-");
  else if (eqstring (report_mode, "data") == TRUE) 
    strcat( influence_report_pathname, "data-");
  sprintf( clsf_num_string, "%d", clsf_num);
  strcat( influence_report_pathname, clsf_num_string);
  if (num_atts_to_list == ALL_ATTRIBUTES)
    num_atts_to_list = clsf->database->n_atts;
  else
    num_atts_to_list = max( 1, min( num_atts_to_list, clsf->database->n_atts));

  influence_report_fp = fopen( influence_report_pathname, "w");

  autoclass_influence_values_report( clsf, search, num_atts_to_list, results_file_ptr,
                                     header_information_p, influence_report_fp,
                                     report_mode , test_clsf,
                                     order_attributes_by_influence_p,
                                     comment_data_headers_p,
                                     sigma_contours_att_list);

  fclose( influence_report_fp);

  safe_sprintf( str, sizeof( str), caller,
               "\nFile written: %s%s\n",
          (influence_report_pathname[0] == G_slash) ? "" : G_absolute_pathname,
          influence_report_pathname);
  to_screen_and_log_file(str, G_log_file_fp, G_stream, TRUE);
}


/* CASE_CLASS_DATA_SHARING
   06feb95 wmt: new
   30may95 wmt: added test_clsf for prediction
   15oct96 wmt: add last_clsf_p arg
   18mar97 wmt: add report_mode
   25apr97 wmt: add prediction_p
   14may97 wmt: add comment_data_headers_p
   21jun97 wmt: add  max_num_xref_class_probs
   02dec98 wmt: in prediction mode, check report_type

   share xref data when both by case and by class reports are requested
   */
xref_data_DS case_class_data_sharing( clsf_DS clsf, shortstr report_mode,
                                      shortstr report_type, char *xref_class_file_ptr,
                                      char *xref_case_file_ptr, char *results_file_ptr,
                                      int_list xref_class_report_att_list,
                                      int clsf_num, clsf_DS test_clsf,
                                      int last_clsf_p, int prediction_p,
                                      unsigned int comment_data_headers_p,
                                      int max_num_xref_class_probs)
{
  xref_data_DS data = NULL;

  if ((prediction_p == FALSE) ||
      ((prediction_p == TRUE) &&
       ((eqstring( report_type, "all") == TRUE) ||
        (eqstring( report_type, "xref_case") == TRUE)))) {
    data = xref_get_data( (test_clsf == NULL) ? clsf : test_clsf,
                          "case", xref_class_report_att_list, data,
                          last_clsf_p, prediction_p, max_num_xref_class_probs);

    case_report_streams( clsf, report_mode, xref_case_file_ptr, results_file_ptr, data,
                         clsf_num, test_clsf, last_clsf_p, comment_data_headers_p,
                         max_num_xref_class_probs);
  }

  if ((prediction_p == FALSE) ||
      ((prediction_p == TRUE) &&
       ((eqstring( report_type, "all") == TRUE) ||
        (eqstring( report_type, "xref_class") == TRUE)))) {
    class_report_streams( clsf, report_mode, xref_class_file_ptr, results_file_ptr,
                          xref_class_report_att_list,
                          xref_get_data( (test_clsf == NULL) ? clsf : test_clsf,
                                         "class", xref_class_report_att_list, data,
                                         last_clsf_p, prediction_p,
                                         max_num_xref_class_probs),
                          clsf_num, test_clsf, last_clsf_p, comment_data_headers_p,
                          max_num_xref_class_probs);
  }
  return (data);
}


/* CASE_REPORT_STREAMS
   06feb95 wmt: new
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   21jun97 wmt: add  max_num_xref_class_probs

   create streams from pathnames for xref by case reports
   */
xref_data_DS case_report_streams( clsf_DS clsf, shortstr report_mode, char *xref_case_file_ptr,
                                  char *results_file_ptr, xref_data_DS xref_data,
                                  int clsf_num, clsf_DS test_clsf,
                                  int last_clsf_p, unsigned int comment_data_headers_p,
                                  int max_num_xref_class_probs)
{
  static fxlstr xref_case_report_pathname;
  int trunc_index, num_chars_to_trunc = 5;
  char clsf_num_string[4];
  FILE *xref_case_report_fp;
  fxlstr str;
  char caller[] = "case_report_streams";

  xref_case_report_pathname[0] = clsf_num_string[0] = '\0';

  strcpy( xref_case_report_pathname, xref_case_file_ptr);
  /* truncate "-----.case-text-" to "----.case-" */
  trunc_index = (int) strlen( xref_case_report_pathname) - num_chars_to_trunc;
  xref_case_report_pathname[trunc_index] = '\0';  
  if (eqstring (report_mode, "text") == TRUE)
    strcat( xref_case_report_pathname, "text-");
  else if (eqstring (report_mode, "data") == TRUE) 
    strcat( xref_case_report_pathname, "data-");
  sprintf( clsf_num_string, "%d", clsf_num);
  strcat( xref_case_report_pathname, clsf_num_string);
  xref_case_report_fp = fopen( xref_case_report_pathname, "w");

  autoclass_xref_by_case_report( clsf, xref_case_report_fp, report_mode, xref_data,
                                 results_file_ptr, test_clsf,
                                 last_clsf_p, comment_data_headers_p,
                                 max_num_xref_class_probs);
  fclose( xref_case_report_fp);
  safe_sprintf( str, sizeof( str), caller,
                "\nFile written: %s%s\n",
                (xref_case_report_pathname[0] == G_slash) ? "" : G_absolute_pathname,
                xref_case_report_pathname);
  to_screen_and_log_file(str, G_log_file_fp, G_stream, TRUE);
  return (xref_data);
}


/* CLASS_REPORT_STREAMS
   06feb95 wmt: new
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   21jun97 wmt: add  max_num_xref_class_probs
   */
xref_data_DS class_report_streams( clsf_DS clsf, shortstr report_mode,
                                   char *xref_class_file_ptr,
                                   char *results_file_ptr,
                                   int_list xref_class_report_att_list,
                                   xref_data_DS xref_data, int clsf_num,
                                   clsf_DS test_clsf, int last_clsf_p,
                                   unsigned int comment_data_headers_p,
                                   int max_num_xref_class_probs)
{
  static fxlstr xref_class_report_pathname;
  int trunc_index, num_chars_to_trunc = 5;
  char clsf_num_string[4];
  FILE *xref_class_report_fp;
  fxlstr str;
  char caller[] = "class_report_streams";

  xref_class_report_pathname[0] = clsf_num_string[0] = '\0';

  strcpy( xref_class_report_pathname, xref_class_file_ptr);
  /* truncate "-----.class-text-" to "----.class-" */
  trunc_index = (int) strlen( xref_class_report_pathname) - num_chars_to_trunc;
  xref_class_report_pathname[trunc_index] = '\0';  
  if (eqstring (report_mode, "text") == TRUE)
    strcat( xref_class_report_pathname, "text-");
  else if (eqstring (report_mode, "data") == TRUE) 
    strcat( xref_class_report_pathname, "data-");  
  sprintf( clsf_num_string, "%d", clsf_num);
  strcat( xref_class_report_pathname, clsf_num_string);

  xref_class_report_fp = fopen( xref_class_report_pathname, "w");

  autoclass_xref_by_class_report( clsf, xref_class_report_fp, report_mode, xref_data,
                                  xref_class_report_att_list, results_file_ptr,
                                  test_clsf, last_clsf_p, comment_data_headers_p,
                                  max_num_xref_class_probs);
  fclose( xref_class_report_fp);
  safe_sprintf( str, sizeof( str), caller,
               "\nFile written: %s%s\n",
          (xref_class_report_pathname[0] == G_slash) ? "" : G_absolute_pathname,
          xref_class_report_pathname);
  to_screen_and_log_file(str, G_log_file_fp, G_stream, TRUE);
  return (xref_data);
}
 

/* XREF_GET_DATA
   06feb95 wmt: new
   15may95 wmt: use n_real_att-1, rather than i, for index to
                real_attribute_data, etc
   25jun96 wmt: do not do realloc's in increments of 1 -- use DATA_ALLOC_INCREMENT
   04jul96 wmt: check for malloc/realloc returning NULL
   05jul96 wmt: free database allocated memory after it is transferred into report
                data structures
   14oct96 wmt: validity check report_attributes < n_atts --
                xref_class_report_att_list from .r-params file;
                replace free( &datum_array[att_number]) with
                free( &data_array[case_num]) and put it in correct location
   15oct96 wmt: add last_clsf_p to indicate when database
                should be freed
   24apr97 wmt: allocate memory for collector once for each case, rather than
                n_classes times. Eliminate ATTR_ALLOC_INCREMENT, and allocate 
     once for all discrete, and once for all real report attributes, if 
     needed, rather than invoking malloc/realloc for each report attribute.
     Limit probability collector to 5 values.
   25apr97 wmt: add prediction_p; flag "test" cases which are not predicted
                in be in any of the "training" classes.  Put them in class -1.
   21jun97 wmt: add  max_num_xref_class_probs
   01nov97 wmt: allocate more storage for instance class probabilities if there
                are more than MAX_NUM_XREF_CLASS_PROBS, and only save for 
                printing a maximum of MAX_NUM_XREF_CLASS_PROBS classes

   collects data from clsf
   with the ordered set of (wt class) pairs: ((h1 h2 ... h-N) (w1 c1) (w2 c2) ...).
   h1 = datum number, h2 ..h-N are report_attributes values
   */
xref_data_DS xref_get_data( clsf_DS clsf, char *type, int_list report_attributes,
                            xref_data_DS xref_data, int last_clsf_p, int prediction_p,
                            int max_num_xref_class_probs)
{
  int n_classes = clsf->n_classes, n_data = clsf->database->n_data, case_num;
  int n_attribute_data, i, att_number = 0, n_class, n_collector;
  int n_discrete_att = 0, n_real_att = 0, n_atts = clsf->database->n_atts;
  int num_discrete_att = 0, num_real_att = 0;
  int xref_data_allocated = 0, collector_length = 0;
  float **data_array = clsf->database->data, wt, *datum_array;
  sort_cell_DS collector;
  att_DS *att_info = clsf->database->att_info;
  class_DS *classes = clsf->classes;
  shortstr *discrete_attribute_data = NULL;
  float *real_attribute_data = NULL;
  int (* class_wt_sort_func) () = float_sort_cell_compare_gtr;
  int (* class_case_sort_func) () = class_case_sort_compare_lsr;

  /* int rpt_n_class; */

  /* printf( "xref_get_data: type = %s\n", type); */

  /* for (i=0; report_attributes[i] != END_OF_INT_LIST; i++) {
    fprintf( stderr, "xref_get_data report_attributes i %d num %d\n",
            i, report_attributes[i]);
  } */

  if (xref_data == NULL) {
    for (i=0; report_attributes[i] != END_OF_INT_LIST; i++) {
      att_number = report_attributes[i];
      if ((att_number < 0) || (att_number >= n_atts)) {
        fprintf( stderr,
                 "ERROR: .r-params file: xref_class_report_att_list index %d "
                 "not in range: 0<->%d\n", att_number, n_atts - 1);
        exit(1);
      }
      if (att_info[att_number]->translations != NULL) {
        num_discrete_att++;
      } else {
        num_real_att++;
      }
    }
    for (case_num=0; case_num<n_data; case_num++) {
    /* for (case_num=0; case_num<50; case_num++) { */
      datum_array = data_array[case_num];
      n_attribute_data = n_discrete_att = n_real_att = 0;
      if ((case_num + 1) > xref_data_allocated) {
        xref_data_allocated += DATA_ALLOC_INCREMENT;
        if (xref_data == NULL) {
          xref_data = (xref_data_DS) malloc( xref_data_allocated *
                                            sizeof( struct xref_data));
          if (xref_data == NULL) {
            fprintf( stderr,
                   "ERROR: xref_get_data(1): out of memory, malloc returned NULL!\n");
            exit(1);
          }
        }
        else {
          xref_data = (xref_data_DS) realloc( xref_data, xref_data_allocated *
                                             sizeof( struct xref_data));
          if (xref_data == NULL) {
            fprintf( stderr,
                   "ERROR: xref_get_data(1): out of memory, realloc returned NULL!\n");
            exit(1);
          }
        }
      }
      if (num_discrete_att > 0) {
        discrete_attribute_data = (shortstr *) malloc( num_discrete_att *
                                                       sizeof( shortstr));
        if (discrete_attribute_data == NULL) {
          fprintf( stderr,
                   "ERROR: xref_get_data(2): out of memory, malloc returned NULL!\n");
          exit(1);
        }
      }
      if (num_real_att > 0) {
        real_attribute_data = (float *) malloc( num_real_att * sizeof( float));
        if (real_attribute_data == NULL) {
          fprintf( stderr,
                   "ERROR: xref_get_data(3): out of memory, malloc returned NULL!\n");
          exit(1);
        }
      }
      collector = (sort_cell_DS) malloc( max_num_xref_class_probs *
                                         sizeof( struct sort_cell));
      if (collector == NULL) {
        fprintf( stderr,
                 "ERROR: xref_get_data(4): out of memory, malloc returned NULL!\n");
        exit(1);
      }
      xref_data[case_num].n_attribute_data = 0;
      xref_data[case_num].case_number = case_num + 1;
      
      for (i=0; report_attributes[i] != END_OF_INT_LIST; i++) {
        att_number = report_attributes[i];
        xref_data[case_num].n_attribute_data++;
        if (att_info[att_number]->translations != NULL) {
          n_discrete_att++;
          strcpy( discrete_attribute_data[n_discrete_att - 1],
                 att_info[att_number]->translations[(int) (datum_array[att_number])]);
        }
        else {
          n_real_att++;
          real_attribute_data[n_real_att - 1] = datum_array[att_number];        
        }
      }

      xref_data[case_num].discrete_attribute_data = discrete_attribute_data;
      xref_data[case_num].real_attribute_data = real_attribute_data;
      n_collector = 0;
      collector_length = max_num_xref_class_probs;
      for (n_class=0; n_class<n_classes; n_class++) {
        wt = classes[n_class]->wts[case_num];
        if (wt > 0.00099999) { 
          if (n_collector >=  collector_length) {
            /* allocate more storage for instance class probabilities */
            /* fprintf( stderr, "xref_get_data: allocating more storage for instance %d\n",
                     case_num + 1); */
            collector_length += max_num_xref_class_probs;
            collector = (sort_cell_DS) realloc( collector, collector_length *
                                                sizeof( struct sort_cell));
          }
          collector[n_collector].float_value = wt;
          collector[n_collector].int_value = map_class_num_clsf_to_report( clsf, n_class);
          n_collector++;
          /* 
          rpt_n_class = map_class_num_clsf_to_report( clsf, n_class); 
          if (case_num == 144) { 
            fprintf( stderr, "xref_get_data: n_class %d rpt_n_class %d case_num %d wt %f\n", 
                     n_class, rpt_n_class, case_num + 1, wt);  
          } 
          */
        }
      }

      if (prediction_p && (n_collector == 0)) {
        /* put cases which do not fall into any class - put into class 9999 */
        collector[n_collector].int_value = 9999;
        collector[n_collector].float_value = 1.0;
        n_collector = 1;
        fprintf( stderr, "xref_get_data: case_num %d => class 9999\n", case_num);
      }
      /* sort collector by most probable class first */
      qsort( (char *) collector, n_collector, sizeof( struct sort_cell),
            class_wt_sort_func);
      /* 
      if (case_num == 144) { 
        for (i=0; i<n_collector; i++) { 
          fprintf( stderr, "i %d coll %f ", i, collector[i].float_value); 
        } 
        fprintf( stderr, "\n"); 
      } 
      */
      xref_data[case_num].class_case_sort_key =     /* (n_class * num_data) + n_case  */
        (collector[0].int_value * n_data) + (case_num + 1);
      /* only save for printing a maximum of <max_num_xref_class_probs> classes */
      xref_data[case_num].n_collector = min( n_collector, max_num_xref_class_probs);
      xref_data[case_num].wt_class_pairs = collector;
      if (last_clsf_p) {
        /* free database allocated memory after it is transferred into report
           data structures */
        /* fprintf( stderr, "xref_get_data: free case_num %d\n", case_num); */
        free( clsf->database->data[case_num]);
      }
    }
  }
  if (eqstring( type, "class") == TRUE) {
    /* sort xref_data by lowest class first */
    qsort( (char *) xref_data, n_data, sizeof( struct xref_data), class_case_sort_func);
  }
/*   for (case_num=0; case_num<n_data; case_num++) { */
/*     printf( "\ncase_num %d: ", case_num + 1);   */
/*     for (i=0; i<xref_data[case_num].n_collector; i++)  */
/*       printf( "cls: %d, wt: %f; ", xref_data[case_num].wt_class_pairs[i].int_value,  */
/*               xref_data[case_num].wt_class_pairs[i].float_value); */
/*   } */
  return (xref_data);
}


/* MAP_CLASS_NUM_CLSF_TO_REPORT
   07feb95 wmt: new

   map class number from classification to report (weight ordered)
   */
int map_class_num_clsf_to_report( clsf_DS clsf, int clsf_n_class)
{
  int report_n_class;

  for (report_n_class=0; report_n_class<clsf->reports->n_class_wt_ordering;
       report_n_class++)
    if (clsf_n_class == clsf->reports->class_wt_ordering[report_n_class])
      break;
  return ( report_n_class);
}


/* MAP_CLASS_NUM_REPORT_TO_CLSF
   07feb95 wmt: new

   map class number from report (weight ordered) to classification
   */
int map_class_num_report_to_clsf( clsf_DS clsf, int report_n_class)
{
  return ( clsf->reports->class_wt_ordering[report_n_class]);
}


/* AUTOCLASS_XREF_BY_CASE_REPORT
   07feb95 wmt: new
   30may95 wmt: added test_clsf
   04jul96 wmt: check for malloc/realloc returning NULL
   21mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   21jun97 wmt: add  max_num_xref_class_probs

   output cross reference by case number
   */
void autoclass_xref_by_case_report( clsf_DS training_clsf, FILE *xref_case_report_fp,
                                    shortstr report_mode, xref_data_DS xref_data,
                                    char *results_file_ptr,
                                    clsf_DS test_clsf, int last_clsf_p,
                                    unsigned int comment_data_headers_p,
                                    int max_num_xref_class_probs)
{
  int_list xref_class_report_att_list;
  char blank = ' ';
  int reports_initial_line_cnt_max = 46, prediction_initial_line_cnt_max = 42;
  int initial_line_cnt_max, prediction_p = 0;
  clsf_DS clsf;

  if (test_clsf == NULL) {
    initial_line_cnt_max = reports_initial_line_cnt_max;
    clsf = training_clsf;
  }
  else {
    initial_line_cnt_max = prediction_initial_line_cnt_max;
    clsf = test_clsf;
  }
  if (xref_data == NULL) {
    xref_class_report_att_list = (int *) malloc( sizeof( int));
    if (xref_class_report_att_list == NULL) {
      fprintf( stderr,
             "ERROR: autoclass_xref_by_case_report: out of memory, malloc returned NULL!\n");
      exit(1);
    }
    xref_class_report_att_list[0] = END_OF_INT_LIST;
    xref_data = xref_get_data( clsf, "case", xref_class_report_att_list, xref_data,
                              last_clsf_p, prediction_p, max_num_xref_class_probs);
  }
  fprintf( xref_case_report_fp, "%s%6cCROSS REFERENCE   CASE NUMBER => "
          "MOST PROBABLE CLASS\n%s\n%s\n",
           (comment_data_headers_p == TRUE) ? "#" : "", blank,
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");

  classification_header( training_clsf, results_file_ptr, xref_case_report_fp,
                        report_mode, test_clsf, comment_data_headers_p);

  xref_paginate_by_case( xref_data, clsf->database->n_data, xref_case_report_fp,
                        report_mode, initial_line_cnt_max, comment_data_headers_p);
}


/* CLASSIFICATION_HEADER
   07feb95 wmt: new
   30may95 wmt: add prediction header
   21mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p

   output classification identification information
   */
void classification_header( clsf_DS clsf, char *results_file_ptr,
                           FILE *xref_case_report_fp, shortstr report_mode,
                           clsf_DS test_clsf, unsigned int comment_data_headers_p)
{
  char blank = ' ';

  if (eqstring (report_mode, "data") == TRUE)
    fprintf( xref_case_report_fp, "DATA_CLSF_HEADER\n");

  if (test_clsf != NULL) {
    fprintf( xref_case_report_fp, "%s%6cAutoClass PREDICTION for the %d \"TEST\" "
            "cases in\n",
             (comment_data_headers_p == TRUE) ? "#" : "",
            blank, test_clsf->database->n_data);
    fprintf( xref_case_report_fp, "%s%8c%s%s\n\n",
             (comment_data_headers_p == TRUE) ? "#" : "", blank,
            (test_clsf->database->data_file[0] == G_slash) ? "" : G_absolute_pathname,
            test_clsf->database->data_file);          
    fprintf( xref_case_report_fp, "%s%6cbased on the \"TRAINING\" classification of %d "
            "cases in\n", (comment_data_headers_p == TRUE) ? "#" : "",
            blank, clsf->database->n_data);
  }
  else 
    fprintf( xref_case_report_fp, "%s%6cAutoClass CLASSIFICATION for the %d cases in\n",
             (comment_data_headers_p == TRUE) ? "#" : "",
            blank, clsf->database->n_data);

  fprintf( xref_case_report_fp, "%s%8c%s%s\n",
           (comment_data_headers_p == TRUE) ? "#" : "", blank,
          (clsf->database->data_file[0] == G_slash) ? "" : G_absolute_pathname,
          clsf->database->data_file);          
  fprintf( xref_case_report_fp, "%s%8c%s%s\n",
           (comment_data_headers_p == TRUE) ? "#" : "", blank,
          (clsf->database->header_file[0] == G_slash) ? "" : G_absolute_pathname,
          clsf->database->header_file);
  fprintf( xref_case_report_fp, "%s%6cwith log-A<X/H> (approximate marginal likelihood) "
          "= %.3f\n",
           (comment_data_headers_p == TRUE) ? "#" : "", blank, clsf->log_a_x_h);
  fprintf( xref_case_report_fp, "%s%6cfrom classification results file\n",
           (comment_data_headers_p == TRUE) ? "#" : "", blank);
  fprintf( xref_case_report_fp, "%s%8c%s%s\n",
           (comment_data_headers_p == TRUE) ? "#" : "", blank,
          (results_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, results_file_ptr);
  fprintf( xref_case_report_fp, "%s%6cand using models\n",
           (comment_data_headers_p == TRUE) ? "#" : "", blank);

  get_models_source_info( clsf->models, clsf->num_models, xref_case_report_fp,
                          comment_data_headers_p);

}


/* XREF_PAGINATE_BY_CASE
   write output with headers at top of each page
   07feb95 wmt: new
   17mar97 wmt: add report_mode, 
   14may97 wmt: add comment_data_headers_p
   */
void xref_paginate_by_case( xref_data_DS xref_data, int n_data, FILE *xref_case_report_fp,
                            shortstr report_mode, int initial_line_cnt_max,
                            unsigned int comment_data_headers_p)
{
  int line_cnt_max = initial_line_cnt_max, page_num = 0, column_1_index;      
  int column_3_index = 0, page_1_p = TRUE, current_data_index = 0, current_n_data;
  int line_cnt, num_report_attribute_strings = 0, column_2_index = 0, i;
  struct xref_data elt1, elt2, elt3;
  float float_value;
  rpt_att_string_DS *report_attribute_strings = NULL;
  char blank = ' ';

  if (eqstring (report_mode, "data") == TRUE)
    fprintf( xref_case_report_fp, "%s\n%s\nDATA_CASE_TO_CLASS",
             (comment_data_headers_p == TRUE) ? "#" : "",
             (comment_data_headers_p == TRUE) ? "#" : "");
  for (current_n_data=n_data; current_n_data>0; ) {

    xref_output_page_headers( "case", page_1_p, num_report_attribute_strings,
                             report_attribute_strings, xref_case_report_fp,
                             report_mode, comment_data_headers_p);

    column_1_index = current_data_index;
    if (eqstring (report_mode, "text") == TRUE) {
      line_cnt_max = (int) ceil( (double) (min( (line_cnt_max * 3.0), current_n_data)
                                           / 3.0));
      column_2_index =  column_1_index + line_cnt_max;
      column_3_index =  column_2_index + line_cnt_max;
      current_data_index = column_3_index + line_cnt_max;
      current_n_data -= 3 * line_cnt_max;
    } else {
      current_data_index = column_1_index + line_cnt_max;
      current_n_data -= line_cnt_max;
    }
    for (line_cnt=0; line_cnt<line_cnt_max; line_cnt++) {
      if ((column_1_index + line_cnt) < n_data) {
        elt1 = xref_data[column_1_index + line_cnt];
        float_value = elt1.wt_class_pairs[0].float_value;
        if (eqstring (report_mode, "data") == TRUE)
          fprintf( xref_case_report_fp, "%03d  ", elt1.case_number);
        else
          fprintf( xref_case_report_fp, "%11d ", elt1.case_number);

        fprintf( xref_case_report_fp, "%4d   %5.3f    ",
                 elt1.wt_class_pairs[0].int_value,
                 (float_value == 1.0) ? float_value : min( 0.999, float_value));
        if (eqstring (report_mode, "data") == TRUE) {
          for (i=1; i<elt1.n_collector; i++) {
            fprintf( xref_case_report_fp, "%c%4d   %5.3f ", blank,
                     elt1.wt_class_pairs[i].int_value,
                     elt1.wt_class_pairs[i].float_value);
          }
        }
      }
      if (eqstring (report_mode, "text") == TRUE) {
        if ((column_2_index + line_cnt) < n_data) {
          elt2 = xref_data[column_2_index + line_cnt];
          float_value = elt2.wt_class_pairs[0].float_value;
          fprintf( xref_case_report_fp, "%11d %4d   %5.3f    ",
                  elt2.case_number, elt2.wt_class_pairs[0].int_value,
                  (float_value == 1.0) ? float_value : min( 0.999, float_value));
        }
        if ((column_3_index + line_cnt) < n_data) {
          elt3 = xref_data[column_3_index + line_cnt];
          float_value = elt3.wt_class_pairs[0].float_value;
          fprintf( xref_case_report_fp, "%11d %4d   %5.3f",
                  elt3.case_number, elt3.wt_class_pairs[0].int_value,
                  (float_value == 1.0) ? float_value : min( 0.999, float_value));
        }
      }
      if ((column_1_index + line_cnt) < n_data) {
        fprintf( xref_case_report_fp, "\n");
      }
    }
    if (page_num == 0)
      line_cnt_max = G_line_cnt_max;
    if (eqstring (report_mode, "text") == TRUE)
      fprintf( xref_case_report_fp, "\f");          /* new page */
    page_num++;
    page_1_p = FALSE;
  }
}


/* XREF_OUTPUT_PAGE_HEADERS
   the output header for xref reports
   08feb95 wmt: new
   17mar97 wmt: add report_mode, 
   14may97 wmt: add comment_data_headers_p

   */
void xref_output_page_headers( char *type, int page_1_p, int num_report_attribute_strings,
                               rpt_att_string_DS *report_attribute_strings,
                               FILE *xref_report_fp, shortstr report_mode,
                               unsigned int comment_data_headers_p)
{
  char dashed_line[92] = "------------------------------------------------------------"
    "------------------------------\n";
  fxlstr attribute_labels;
  int i, blank_cnt, diff;
  char blank = ' ';
  shortstr divider_format;
  rpt_att_string_DS report_att_string;

  attribute_labels[0] = '\0';
  
  if ((page_1_p == TRUE) && (eqstring (report_mode, "text") == TRUE))
    fprintf( xref_report_fp, "\n\n");
  if ((eqstring (report_mode, "text") == TRUE) ||
      ((eqstring (report_mode, "data") == TRUE) &&
       (page_1_p == TRUE))) { 
    if (eqstring( type, "case") == TRUE) {
      if (eqstring (report_mode, "text") == TRUE) {
        fprintf( xref_report_fp, "\n%sCase# Class  Prob         Case #  Class  "
                 "Prob         Case #  Class  Prob \n",
                 (comment_data_headers_p == TRUE) ? "#" : "");
      } else {
        fprintf( xref_report_fp, "\n%sCase# Class  Prob    (Class  Prob) "
                 "(Class  Prob) (Class  Prob) (Class  Prob) \n",
                 (comment_data_headers_p == TRUE) ? "#" : "");
      }
    } else if (eqstring( type, "class") == TRUE) {
      if (report_attribute_strings == NULL)
        fprintf( xref_report_fp, "%s\n%sCase #      Prob         (Class  Prob) "
                 "(Class  Prob) (Class  Prob) (Class  Prob) \n",
                 (comment_data_headers_p == TRUE) ? "#" : "",
                 (comment_data_headers_p == TRUE) ? "#" : "");
      else {
        fprintf( xref_report_fp, "\n%sCase #",
                 (comment_data_headers_p == TRUE) ? "#" : "");
        for (i=0; i<num_report_attribute_strings; i++) {
          report_att_string = report_attribute_strings[i];
          fprintf( xref_report_fp, "   %s", report_att_string->att_dscrp);
          diff = report_att_string->dscrp_length - strlen( report_att_string->att_dscrp);
          blank_cnt = max( 0, diff);
          if (blank_cnt > 0) {
            sprintf( divider_format, "%%%dc", blank_cnt);
            fprintf( xref_report_fp, divider_format, blank);
          }
        }
        fprintf( xref_report_fp, "   (Cls  Prob)\n");
      }
    }                          
    if (eqstring (report_mode, "text") == TRUE)
      fprintf( xref_report_fp, "%s", dashed_line);
  }
}


/* AUTOCLASS_XREF_BY_CLASS_REPORT
   09feb95 wmt: new
   17mar97 wmt: add report_mode, 
   14may97 wmt: add comment_data_headers_p
   21jun97 wmt: add  max_num_xref_class_probs

   output cross reference sorted by class number.
   report_attributes = list of attribute numbers
   */
void autoclass_xref_by_class_report( clsf_DS training_clsf, FILE *xref_class_report_fp,
                                     shortstr report_mode, xref_data_DS xref_data,
                                     int_list report_attributes,
                                     char *results_file_ptr, clsf_DS test_clsf,
                                     int last_clsf_p, unsigned int comment_data_headers_p,
                                     int max_num_xref_class_probs)
{
  char blank = ' ';
  int reports_initial_line_cnt = 16, prediction_initial_line_cnt = 20;
  int initial_line_cnt, prediction_p = 0;
  clsf_DS clsf;

  if (test_clsf == NULL) {
    initial_line_cnt = reports_initial_line_cnt;
    clsf = training_clsf;
  }
  else {
    initial_line_cnt = prediction_initial_line_cnt;
    clsf = test_clsf;
  }
  if (xref_data == NULL)
    xref_data = xref_get_data( clsf, "class", report_attributes, xref_data,
                              last_clsf_p, prediction_p, max_num_xref_class_probs);

  fprintf( xref_class_report_fp, "%s%6cCROSS REFERENCE    CLASS => "
          "CASE NUMBER MEMBERSHIP\n%s\n%s\n",
           (comment_data_headers_p == TRUE) ? "#" : "", blank,
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");

  classification_header( training_clsf, results_file_ptr, xref_class_report_fp,
                        report_mode, test_clsf, comment_data_headers_p);

  xref_paginate_by_class( clsf, xref_data, report_attributes, xref_class_report_fp,
                         report_mode, initial_line_cnt, comment_data_headers_p);
}


/* XREF_PAGINATE_BY_CLASS
   09feb95 wmt: new
   17mar97 wmt: add report_mode, 
   14may97 wmt: add comment_data_headers_p

   write output with headers at top of each page for autoclass class sort
   */
void xref_paginate_by_class( clsf_DS clsf, xref_data_DS xref_data,
                             int_list report_attributes, FILE *xref_class_report_fp,
                             shortstr report_mode, int initial_line_cnt,
                             unsigned int comment_data_headers_p)
{
  int cnt = initial_line_cnt; /* initial page header */
  int current_class = -1, line_cnt = G_line_cnt_max, n_data = clsf->database->n_data;
  int n_datum, num_wt_class_pairs, i, num_report_att_strings = 0, prob_tab = 0;
  shortstr *attribute_formats = NULL;
  rpt_att_string_DS *report_attribute_strings = NULL;
  struct xref_data xref_datum;
  sort_cell_DS wt_class_pairs;

  if (report_attributes[0] != END_OF_INT_LIST) {
    report_attribute_strings =
      xref_class_report_attributes( clsf, report_attributes, &attribute_formats, &prob_tab);
    for (i=0; report_attributes[i] != END_OF_INT_LIST; i++)
      ;
    num_report_att_strings = i;
  }
  /* elt = xref_data[n_datum]; arg1 = translations_list; arg2 = wt_class_pairs */
  for (n_datum=0; n_datum<n_data; n_datum++) {
    xref_datum = xref_data[n_datum];
    num_wt_class_pairs = xref_datum.n_collector;
    wt_class_pairs = xref_datum.wt_class_pairs;
    /* change in autoclass class */
    if (wt_class_pairs[0].int_value != current_class) {
      current_class = wt_class_pairs[0].int_value;
      xref_paginate_by_class_hdrs( xref_class_report_fp, report_mode, &cnt, line_cnt,
                                  wt_class_pairs,
                                  TRUE, num_report_att_strings,
                                  report_attribute_strings, comment_data_headers_p);
    }
    /* check for multi_line entry and end_of_page */
    if ((cnt + ((report_attribute_strings != NULL) ? num_wt_class_pairs : 1)) >
        line_cnt) {
      xref_paginate_by_class_hdrs( xref_class_report_fp, report_mode, &cnt, line_cnt,
                                  wt_class_pairs,
                                  FALSE, num_report_att_strings,
                                  report_attribute_strings, comment_data_headers_p);
    }

    xref_output_line_by_class( clsf, xref_class_report_fp, report_mode,
                              &attribute_formats, &xref_datum,
                              wt_class_pairs, prob_tab, report_attributes,
                               comment_data_headers_p);

    if (report_attribute_strings != NULL)
      cnt += num_wt_class_pairs;
    else
      cnt++;           
  }
  if (report_attribute_strings != NULL) {
    for (i=0; i<num_report_att_strings; i++)
      free( report_attribute_strings[i]);
    free( report_attribute_strings);
  }
  if (attribute_formats != NULL)
    free( attribute_formats);
}


/* XREF_CLASS_REPORT_ATTRIBUTES
   09feb95 wmt: new
   16may95 wmt: use %g, rather than %f for real data
   04jul96 wmt: check for malloc/realloc returning NULL

   determine additional attribute labels, field widths, and attribute format string
   */
rpt_att_string_DS *xref_class_report_attributes( clsf_DS clsf,
                                                int_list report_attribute_numbers,
                                                shortstr **attribute_formats_ptr,
                                                int *prob_tab_ptr)
{
  shortstr str;
  rpt_att_string_DS *report_attribute_strings = NULL;
  char *att_dscrp, **translations;
  att_DS *all_att_info = clsf->database->att_info;
  int i, att_number, dscrp_length, n_trans, num_report_att_strings;

  *prob_tab_ptr = 6;

  for (i=0; report_attribute_numbers[i] != END_OF_INT_LIST; i++)
    ;
  num_report_att_strings = i;
  if (num_report_att_strings > 0) {
    report_attribute_strings =
      (rpt_att_string_DS *) malloc( num_report_att_strings * sizeof( rpt_att_string_DS));
    if (report_attribute_strings == NULL) {
      fprintf( stderr,
             "ERROR: xref_class_report_attributes(1): out of memory, malloc returned NULL!\n");
      exit(1);
    }
  }
  for (i=0; report_attribute_numbers[i] != END_OF_INT_LIST; i++) {
    report_attribute_strings[i] =
      (rpt_att_string_DS) malloc( sizeof( struct report_attribute_string));
    if (report_attribute_strings[i] == NULL) {
      fprintf( stderr,
             "ERROR: xref_class_report_attributes(2): out of memory, malloc returned NULL!\n");
      exit(1);
    }
    att_number = report_attribute_numbers[i];
    att_dscrp = all_att_info[att_number]->dscrp;
    dscrp_length = strlen( att_dscrp);
    translations = all_att_info[att_number]->translations;
    report_attribute_strings[i]->att_number = att_number;
    strcpy( report_attribute_strings[i]->att_dscrp, att_dscrp);
    if (translations == NULL)
      report_attribute_strings[i]->dscrp_length = dscrp_length + 3;
    else {
      report_attribute_strings[i]->dscrp_length = dscrp_length;
      for (n_trans=0; n_trans<all_att_info[att_number]->n_trans; n_trans++) {
        if ((int) strlen( translations[n_trans]) >
            report_attribute_strings[i]->dscrp_length)
          report_attribute_strings[i]->dscrp_length = strlen( translations[n_trans]);
      }
    }
  }
  for (i=0; i<num_report_att_strings; i++) {
    *prob_tab_ptr += 3 + report_attribute_strings[i]->dscrp_length;
    if (all_att_info[report_attribute_numbers[i]]->translations != NULL)
      sprintf( str, "   %%-%ds", report_attribute_strings[i]->dscrp_length);
    else
      sprintf( str, "   %%-%dg", report_attribute_strings[i]->dscrp_length);
    if ( *attribute_formats_ptr == NULL) {
      *attribute_formats_ptr  = (shortstr *) malloc( sizeof( shortstr));
      if (*attribute_formats_ptr == NULL) {
        fprintf( stderr,
               "ERROR: xref_class_report_attributes(3): out of memory, malloc returned NULL!\n");
        exit(1);
      }
    } else {
      *attribute_formats_ptr = (shortstr *) realloc( *attribute_formats_ptr, (i + 1) *
                                              sizeof( shortstr));
      if (*attribute_formats_ptr == NULL) {
        fprintf( stderr,
               "ERROR: xref_class_report_attributes(3): out of memory, realloc returned NULL!\n");
        exit(1);
      }
    }
    strcpy( (*attribute_formats_ptr)[i], str);
  }
  return (report_attribute_strings);
}


/* XREF_PAGINATE_BY_CLASS_HDRS
   10feb95 wmt: new
   17mar97 wmt: add report_mode, 
   14may97 wmt: add comment_data_headers_p

   output class header - return cnt
   */
void xref_paginate_by_class_hdrs( FILE *xref_class_report_fp, shortstr report_mode,
                                  int *cnt_ptr, int line_cnt,
                                  sort_cell_DS wt_class_pairs, int init,
                                  int num_report_attribute_strings,
                                  rpt_att_string_DS *report_attribute_strings,
                                  unsigned int comment_data_headers_p)
{
  char blank = ' ';
  int page_1_p = TRUE;

  if (eqstring (report_mode, "text") == TRUE) {
    if ((line_cnt - *cnt_ptr) < 10) {
      *cnt_ptr = 0;
      fprintf( xref_class_report_fp, "\f"); /* form feed */
    }
    *cnt_ptr += 8;
  }
  if ((eqstring (report_mode, "text") == TRUE) ||
      ((eqstring (report_mode, "data") == TRUE) && (init == TRUE))) {
    fprintf( xref_class_report_fp, "%s\n%s\n%s\n",
             (comment_data_headers_p == TRUE) ? "#" : "",
             (comment_data_headers_p == TRUE) ? "#" : "",
             (comment_data_headers_p == TRUE) ? "#" : "");
    if (eqstring (report_mode, "data") == TRUE)
      fprintf( xref_class_report_fp, "DATA_CLASS %d\n",
               wt_class_pairs[0].int_value);
    fprintf( xref_class_report_fp, "%s%32c CLASS = %d %s \n%s",
             (comment_data_headers_p == TRUE) ? "#" : "", blank,
             wt_class_pairs[0].int_value, (init == TRUE) ? "" : "(continued)",
             (comment_data_headers_p == TRUE) ? "#" : "");
 
    xref_output_page_headers( "class", page_1_p, num_report_attribute_strings,
                             report_attribute_strings, xref_class_report_fp,
                             report_mode, comment_data_headers_p);
  }
}

   
/* XREF_OUTPUT_LINE_BY_CLASS
   10feb95 wmt: new
   16may95 wmt: handle unknown real values
   17mar97 wmt: add report_mode, 
   14may97 wmt: add comment_data_headers_p
   */
void xref_output_line_by_class( clsf_DS clsf, FILE *xref_class_report_fp,
                                shortstr report_mode, 
                                shortstr **attribute_formats_ptr,
                                xref_data_DS xref_datum_ptr,
                                sort_cell_DS wt_class_pairs, int prob_tab,
                                int_list report_attribute_numbers,
                                unsigned int comment_data_headers_p)    
{
  struct xref_data xref_datum = *xref_datum_ptr;
  int n_discrete_att = 0, n_real_att = 0, i, print_atts_p;
  att_DS *att_info = clsf->database->att_info;
  fxlstr prob_tab_format;
  char blank = ' ', char_s = 's', char_g = 'g', question_mark[] = "?";

  print_atts_p = (xref_datum.n_attribute_data > 0);
  if (print_atts_p == TRUE) {
    prob_tab += 4;
    sprintf( prob_tab_format, "\n%%%dc", prob_tab);
    strcat( prob_tab_format, "%2d  %5.3f");
  }
  if (eqstring (report_mode, "data") == TRUE)
    fprintf( xref_class_report_fp, "%03d", xref_datum.case_number);
  else
    fprintf( xref_class_report_fp, (print_atts_p == TRUE) ? "\n%6d" : "\n%11d",
             xref_datum.case_number);

  for (i=0; i<xref_datum.n_attribute_data; i++) {
    if (att_info[report_attribute_numbers[i]]->translations != NULL) {
      fprintf( xref_class_report_fp, (*attribute_formats_ptr)[i],
              xref_datum.discrete_attribute_data[n_discrete_att]);
      n_discrete_att++;
    }
    else {
      if (percent_equal( (double) xref_datum.real_attribute_data[n_real_att],
                        FLOAT_UNKNOWN, REL_ERROR) == TRUE) {
        /* replace g with s in format directive */
        (*attribute_formats_ptr)[i][strlen( (*attribute_formats_ptr)[i]) -1] =
          char_s;
        fprintf( xref_class_report_fp, (*attribute_formats_ptr)[i], question_mark);
      }
      else {
        (*attribute_formats_ptr)[i][strlen( (*attribute_formats_ptr)[i]) -1] =
          char_g;
        fprintf( xref_class_report_fp, (*attribute_formats_ptr)[i],
                xref_datum.real_attribute_data[n_real_att]);
      }
      n_real_att++;
    }
  }
  fprintf( xref_class_report_fp, "%s     %5.3f%s", (print_atts_p == TRUE) ? "   " : "",
          xref_datum.wt_class_pairs[0].float_value,
          (print_atts_p == TRUE) ? "" : "        ");
  for (i=1; i<xref_datum.n_collector; i++) {
    fprintf( xref_class_report_fp,
            (print_atts_p == TRUE) ? prob_tab_format : "%c%4d   %5.3f ", blank,
            xref_datum.wt_class_pairs[i].int_value,
            xref_datum.wt_class_pairs[i].float_value);
  }
  fprintf( xref_class_report_fp,"\n");
}

 
/* AUTOCLASS_INFLUENCE_VALUES_REPORT
   12feb95 wmt: new
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   24jun97 wmt: add start_sigma_contours_att & stop_sigma_contours_att
   28feb98 wmt: replace start/stop_sigma_contours_att with
                sigma_contours_att_list

   generate text reports for influence values of all classes
   */

void autoclass_influence_values_report( clsf_DS clsf, search_DS search, int num_atts_to_list,
                                        char *results_file_ptr, int header_information_p,
                                        FILE *influence_report_fp, shortstr report_mode,
                                        clsf_DS test_clsf,
                                        unsigned int order_attributes_by_influence_p,
                                        unsigned int comment_data_headers_p,
                                        int_list sigma_contours_att_list)
{
  char blank = ' ';
  int report_class_number, single_class_p = FALSE;
  char class_number_type[5] = "clsf";


  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp,
            "\n\n\n%6cI N F L U E N C E   V A L U E S   R E P O R T   \n"
            "%6c order attributes by influence values = %s\n"
            "%6c=============================================\n\n", blank, blank,
            (order_attributes_by_influence_p == TRUE) ? "true" : "false",
            blank);

  influence_values_header( clsf, search, results_file_ptr, TRUE, influence_report_fp,
                          report_mode, test_clsf, comment_data_headers_p);

  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp, "\f"); /* new page */

  for (report_class_number=0; report_class_number < clsf->n_classes;
       report_class_number++) {
    
    autoclass_class_influence_values_report
      ( clsf, search, class_number_type,
        map_class_num_report_to_clsf( clsf, report_class_number), num_atts_to_list,
        header_information_p, results_file_ptr, single_class_p, influence_report_fp,
        report_mode, test_clsf, order_attributes_by_influence_p, comment_data_headers_p,
        sigma_contours_att_list);

    if ((eqstring (report_mode, "text") == TRUE) &&
        (report_class_number < (clsf->n_classes - 1)))
      fprintf( influence_report_fp, "\f");
  }
}


/* INFLUENCE_VALUES_HEADER
   12feb95 wmt: new
   15jun95 wmt: revise layout and content
   18mar97 wmt: add report_mode 
   14may97 wmt: add comment_data_headers_p
   28aug97 wmt: change fprintf( influence_report_fp, header);
                to fprintf( influence_report_fp, header, "");
                prevent segmentation fault.

   output influence values header
   */
void influence_values_header( clsf_DS clsf, search_DS search, char *results_file_ptr,
                              int header_information_p, FILE *influence_report_fp,
                              shortstr report_mode, clsf_DS test_clsf,
                              unsigned int comment_data_headers_p)
{
  int populated_classes_cnt = 0, clsf_class_number, line_cnt = 11, n_att;
  int max_line_cnt = G_line_cnt_max, n_atts = clsf->database->n_atts;
  char class_number_type[5] = "clsf";
  ordered_influ_vals_DS output = NULL;
  fxlstr header = "\n%s   num                        description                          "
    "I-*k \n%s", str;

  for (clsf_class_number=0; clsf_class_number<clsf->n_classes; clsf_class_number++)
    if (populated_class_p( clsf_class_number, class_number_type, clsf) == TRUE)
        populated_classes_cnt++;
  if (header_information_p == TRUE)
    classification_header( clsf, results_file_ptr, influence_report_fp,
                          report_mode,test_clsf, comment_data_headers_p);

  output = ordered_normalized_influence_values( clsf);

  if (eqstring (report_mode, "text") == TRUE)
    influence_values_explanation( influence_report_fp);

  search_summary( search, influence_report_fp, report_mode, comment_data_headers_p);

  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp, "\f");
  fprintf( influence_report_fp, "%s\n%s\n%s\n", (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");
  if (eqstring (report_mode, "data") == TRUE)
    fprintf( influence_report_fp, "DATA_POP_CLASSES\n");
  fprintf( influence_report_fp, "CLASSIFICATION HAS %d POPULATED CLASSES   "
          "(max global influence value = %5.3f) \n",
           populated_classes_cnt, clsf->reports->max_i_value);

  class_weights_and_strengths( clsf, influence_report_fp, report_mode,
                               comment_data_headers_p);

  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp, "\f");
  fprintf( influence_report_fp, "%s\n%s\n%s\n", (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");
  if (eqstring (report_mode, "data") == TRUE)
    fprintf ( influence_report_fp, "DATA_CLASS_DIVS\n");
  fprintf( influence_report_fp, "%sCLASS DIVERGENCES\n%s",
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");

  class_divergences( clsf, influence_report_fp, report_mode,
                     comment_data_headers_p);
 
  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp, "\f");
  fprintf( influence_report_fp, "\n%s\n%s\n", (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");
  if (eqstring (report_mode, "data") == TRUE)
    fprintf( influence_report_fp, "DATA_NORM_INF_VALS\n");
  fprintf( influence_report_fp, "%sORDERED LIST OF NORMALIZED ATTRIBUTE INFLUENCE "
          "VALUES SUMMED OVER ALL CLASSES\n%s",
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");
  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp, "\n"
            "  This gives a rough heuristic measure of relative influence of each\n"
            "  attribute in differentiating the classes from the overall data set.\n"
            "  Note that \"influence values\" are only computable with respect to the\n"
            "  model terms.  When multiple attributes are modeled by a single\n"
            "  dependent term (e.g. multi_normal_cn), the term influence value is\n"
            "  distributed equally over the modeled attributes.\n");
  fprintf( influence_report_fp, header, (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");

  for (n_att=0; n_att<n_atts; n_att++) {
    if (eqstring (report_mode, "text") == TRUE) {
      if (line_cnt > max_line_cnt) {
        line_cnt = 4;
        fprintf( influence_report_fp, "\f");
        fprintf( influence_report_fp, header, "");
      }
    }
    strcpy( str, "");
    fprintf( influence_report_fp, "\n");
    if (eqstring (report_mode, "text") == TRUE)
      fprintf( influence_report_fp, "   ");
    fprintf( influence_report_fp, "%03d  %-55s   ", output[n_att].n_att,
              strncat( str, output[n_att].att_dscrp_ptr, 55));
    if (output[n_att].model_term_type_ptr == NULL)      
      fprintf( influence_report_fp, " -----");
    else
      fprintf( influence_report_fp, "%6.3f", output[n_att].norm_att_i_sum);

    line_cnt++;
  }

  if (eqstring (report_mode, "text") == TRUE) {
    fprintf( influence_report_fp, "\f\n\nCLASS LISTINGS:\n\n"
            "  These listings are ordered by class weight --\n"
            "    * j is the zero-based class index,\n"
            "    * k is the zero-based attribute index, and\n"
            "    * l is the zero-based discrete attribute instance index.\n\n"
            "  Within each class, the covariant and independent model terms are ordered\n"
            "  by their term influence value I-jk.\n\n");
    fprintf( influence_report_fp,
            "  Covariant attributes and discrete attribute instance values are both\n"
            "  ordered by their significance value.  Significance values are computed\n"
            "  with respect to a single class classification, using the divergence from\n"
            "  it, abs( log( Prob-jkl / Prob-*kl)), for discrete attributes and the\n"
            "  relative separation from it, abs( Mean-jk - Mean-*k) / StDev-jk, for\n" 
            "  numerical valued attributes.  For the SNcm model, the value line is\n"
            "  followed by the probabilities that the value is known, for that class\n");
    fprintf( influence_report_fp,
            "  and for the single class classification.\n\n");
    fprintf( influence_report_fp,
            "  Entries are attribute type dependent, and the corresponding headers are\n"
            "  reproduced with each class.  In these --\n"
            "    * num/t denotes model term number,\n"
            "    * num/a denotes attribute number,\n"
            "    * t     denotes attribute type,\n"
            "    * mtt   denotes model term type, and\n"
            "    * I-jk  denotes the term influence value for attribute k\n"
            "            in class j.  This is the cross entropy or\n"
             "            Kullback-Leibler distance between the class and\n");
    fprintf( influence_report_fp,    
            "            full database probability distributions (see\n"
            "            interpretation-c.text).\n"
            "    * Mean  StDev\n"
            "      -jk   -jk    The estimated mean and standard deviation\n"
            "                   for attribute k in class j.\n"
            "    * |Mean-jk -   The absolute difference between the\n"
            "       Mean-*k|/   two means, scaled w.r.t. the class\n"
            "       StDev-jk    standard deviation, to get a measure\n"
            "                   of the distance between the attribute\n"
              "                   means in the class and full data.\n");
    fprintf( influence_report_fp,
            "    * Mean  StDev  The estimated mean and standard\n"
            "      -*k   -*k    deviation for attribute k when the\n"
            "                   model is applied to the data set\n"
             "                   as a whole.\n");
    fprintf( influence_report_fp,
            "    * Prob-jk is known  1.00e+00   Prob-*k is  known  9.98e-01\n"
            "            The SNcm model allows for the possibility that data\n"
            "            values are unknown, and models this with a discrete\n"
            "            known/unknown probability.  The gaussian normal for\n"
            "            known values is then conditional on the known\n"
            "            probability.   In this instance, we have a class\n"
            "            where all values are known, as opposed to a database\n"
            "            where only 99.8%% of values are known.\n");
  }
  if (eqstring (report_mode, "data") == TRUE)
    fprintf( influence_report_fp, "\n");
  if (output != NULL)
    free( output);
}


/* AUTOCLASS_CLASS_INFLUENCE_VALUES_REPORT
   12feb95 wmt: new
   20jun95 wmt: redo line_cnt computations
   04jul96 wmt: check for malloc/realloc returning NULL
   18mar97 wmt: add report_mode 
   14may97 wmt: add comment_data_headers_p
   24jun97 wmt: call generate_sigma_contours; add start_sigma_contours_att &
                stop_sigma_contours_att 
   28feb98 wmt: replace start/stop_sigma_contours_att with
                sigma_contours_att_list
   20mar98 wmt: outputing of MNcn correlation matrices after last class attribute
                instead of after each term is now done by a call to
                generate_mncn_correlation_matrices from
                autoclass_class_influence_values_report

   Generates influence values report for requested clsf class number
   class_number_type => :report or :clsf. class_number => report or
   classification class number
   */
void autoclass_class_influence_values_report( clsf_DS clsf, search_DS search,
                                              char *class_number_type,
                                              int class_number, int num_atts_to_list,
                                              int header_information_p,
                                              char *results_file_ptr, int single_class_p,
                                              FILE *influence_report_fp,
                                              shortstr report_mode, clsf_DS test_clsf,
                                              unsigned int order_attributes_by_influence_p,
                                              unsigned int comment_data_headers_p,
                                              int_list sigma_contours_att_list)
{
  int clsf_class_number, line_cnt = 8, n_data = clsf->database->n_data, i;
  int n_atts = clsf->database->n_atts, n_att, num_term_types = 0, first_term_type = TRUE;
  int real_atts_header_p = FALSE, discrete_atts_header_p = FALSE, report_class_num;
  float class_wt;
  char title_line_1[2*STRLIMIT] = "", title_line_2[3*STRLIMIT] = "",  *att_type;
  fxlstr class_model_source = "", temp;
  shortstr *term_types = NULL, a_term_type;
  char caller[] = "autoclass_class_influence_values_report";

  clsf_class_number = (eqstring( class_number_type, "clsf")) ? class_number :
    map_class_num_report_to_clsf( clsf, class_number);
  /*
  fprintf( stderr, "autoclass_class_influence_values_report: class_number %d" 
           " class_number_type %s clsf_class_number %d\n", 
           class_number, class_number_type, clsf_class_number); 
  */
  class_wt = clsf->classes[clsf_class_number]->w_j;
  if (num_atts_to_list == ALL_ATTRIBUTES)
    num_atts_to_list = clsf->database->n_atts;
  else
    num_atts_to_list = max( 1, min( num_atts_to_list, clsf->database->n_atts));
  report_class_num = map_class_num_clsf_to_report( clsf, clsf_class_number);
  if (eqstring (report_mode, "data") == TRUE)
    fprintf( influence_report_fp, "%s\n%s\nDATA_CLASS %d\n",
             (comment_data_headers_p == TRUE) ? "#" : "",
             (comment_data_headers_p == TRUE) ? "#" : "", report_class_num);
  safe_sprintf( title_line_1, sizeof( title_line_1), caller,
               "%sCLASS %2d - weight %3d   normalized weight %5.3f   relative "
               "strength %9.2e *******"
               "\n%s                            class cross entropy w.r.t. global class "
               "%9.2e *******",
                (comment_data_headers_p == TRUE) ? "#" : "",
               report_class_num, iround((double) class_wt), (class_wt / n_data),
               (float) safe_exp( (double) (clsf->reports->class_strength[clsf_class_number] -
                                           clsf->reports->max_class_strength)),
                (comment_data_headers_p == TRUE) ? "#" : "",
               clsf->classes[clsf_class_number]->i_sum);
  get_class_model_source_info( clsf->classes[clsf_class_number], class_model_source,
                               comment_data_headers_p);
  safe_sprintf( title_line_2, sizeof( title_line_2), caller,
               "   Model file: %s \n   Numbers: numb/t = model term number; "
               "numb/a = attribute number \n   Model term types (mtt): ",
               class_model_source);
  /* fprintf( stderr, "autoclass_class_influence_values_report: n_atts %d\n", n_atts); */
  for (n_att=0; n_att<n_atts; n_att++) {
    strcpy( a_term_type, rpt_att_model_term_type( clsf, clsf_class_number, n_att));
    att_type = report_att_type( clsf, clsf_class_number, n_att);
    /* fprintf( stderr, "autoclass_class_influence_values_report: n_att %d att_type %s\n",
            n_att, att_type); */
    if ((eqstring( a_term_type, "ignore") != TRUE) &&
        (find_str_in_table( a_term_type, term_types, num_term_types) == -1)) { 
      if (term_types == NULL) {
        term_types =  (shortstr *) malloc( sizeof( shortstr));
        if (term_types == NULL) {
          fprintf( stderr,
                 "ERROR: autoclass_class_influence_values_report: out of memory, malloc returned NULL!\n");
          exit(1);
        }
      } else {
        term_types =  (shortstr *) realloc( term_types, (num_term_types + 1) *
                                           sizeof( shortstr));
        if (term_types == NULL) {
          fprintf( stderr,
                 "ERROR: autoclass_class_influence_values_report: out of memory, realloc returned NULL!\n");
          exit(1);
        }
      }
      strcpy( term_types[num_term_types], a_term_type);
      num_term_types++;
      if (eqstring( att_type, "discrete")) {
        discrete_atts_header_p = TRUE;
        line_cnt += 4;
      }
      if (eqstring( att_type, "real")) {
        real_atts_header_p = TRUE;
        line_cnt += 4;
      }
    }
  }
  for (i=0; i<num_term_types; i++) {
    safe_sprintf( temp, sizeof( temp), caller, "%s(%s %s)\n",
                 (first_term_type) ? "" : "                           ",
                 term_types[i], (char *) get( term_types[i], "print_string"));
    strcat(title_line_2, temp);
    first_term_type = FALSE;
  }
  line_cnt += num_term_types;
	
  text_stream_header( single_class_p, influence_report_fp, report_mode,
                      header_information_p, clsf,
                      search, results_file_ptr, title_line_1, title_line_2,
                      test_clsf, order_attributes_by_influence_p,
                      comment_data_headers_p);
  free( term_types);

  pre_format_attributes( clsf, clsf_class_number, num_atts_to_list, line_cnt,
                        discrete_atts_header_p, real_atts_header_p,
                        influence_report_fp, report_mode,
                        order_attributes_by_influence_p, comment_data_headers_p);

  generate_mncn_correlation_matrices ( clsf, clsf_class_number, report_mode,
                                       comment_data_headers_p,
                                       influence_report_fp );

  if ((eqstring (report_mode, "data") == TRUE) &&
      (sigma_contours_att_list[0] != END_OF_INT_LIST)) {

    generate_sigma_contours ( clsf, clsf_class_number, sigma_contours_att_list,
                              influence_report_fp, comment_data_headers_p);
  }
}


/* POPULATED_CLASS_P
   13feb95 wmt: new

   check that class (classification ordering) has a class-wt >= minimum weight
   (ensure that class is populated)
   class_number_type => "clsf" or "report"
   */
int populated_class_p( int class_number, char *class_number_type, clsf_DS clsf)
{

  if (eqstring( class_number_type, "report") == TRUE)
    class_number = map_class_num_report_to_clsf( clsf, class_number);
   return ((clsf->classes[class_number]->w_j + 0.001) >= clsf->min_class_wt);
}


/* ORDERED_NORMALIZED_INFLUENCE_VALUES
   13feb95 wmt: new
   23jun95 wmt: add model_term_type_ptr to output data structure
   04jul96 wmt: check for malloc/realloc returning NULL

   return ordered normalized influence values summed over all classes
   */
ordered_influ_vals_DS ordered_normalized_influence_values( clsf_DS clsf)
{
  
  ordered_influ_vals_DS output = NULL;
  rpt_DS reports = clsf->reports;
  float max_i_sum, att_i_sum;
  int n_att, n_atts = clsf->database->n_atts, clsf_class_number = 0;
  int (* att_i_sum_sort_func) () = att_i_sum_sort_compare_gtr;

  max_i_sum = reports->att_max_i_sum;

  for (n_att=0; n_att<n_atts; n_att++) {
    if ( output == NULL) {
      output  = (ordered_influ_vals_DS) malloc( sizeof( struct ordered_influence_values));
      if (output == NULL) {
        fprintf( stderr, "ERROR: ordered_normalized_influence_values : out of memory, malloc returned NULL!\n");
        exit(1);
      }
    } else {
      output = (ordered_influ_vals_DS) realloc( output, (n_att + 1) *
                                              sizeof( struct ordered_influence_values));
      if (output == NULL) {
        fprintf( stderr, "ERROR: ordered_normalized_influence_values : out of memory, realloc returned NULL!\n");
        exit(1);
      }
    }
    att_i_sum = reports->att_i_sums[n_att];
    output[n_att].att_i_sum = att_i_sum;
    output[n_att].n_att = n_att;
    output[n_att].att_dscrp_ptr = clsf->database->att_info[n_att]->dscrp;
    output[n_att].norm_att_i_sum = (max_i_sum == 0.0) ? 0.0 : (att_i_sum / max_i_sum);
    /* assumes that for each attribute, all classes have the same model term type */
    output[n_att].model_term_type_ptr =
      (char *) get( rpt_att_model_term_type( clsf, clsf_class_number, n_att),
                   "print_string");
  }
  qsort( (char *) output, n_atts, sizeof( struct ordered_influence_values),
        att_i_sum_sort_func);
  return (output);
}


/* INFLUENCE_VALUES_EXPLANATION
   13feb95 wmt: new

   output explanation of influence value layout
   */
void influence_values_explanation( FILE *influence_report_fp)
{
  fprintf( influence_report_fp,
          "\n\n\n\nORDER OF PRESENTATION:\n\n"
          "  * Summary of the generating search.\n"
          "  * Weight ordered list of the classes found & class strength heuristic.\n"
          "  * List of class cross entropies with respect to the global class.\n"
          "  * Ordered list of attribute influence values summed over all classes.\n"
          "  * Class listings, ordered by class weight.\n");
}


/* SEARCH_SUMMARY
   13feb95 wmt: new
   05may95 wmt: change search->n to search->n_tries to prevent
                segment violation when there are duplicates
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p

   output variation of print_final_report
   */
void search_summary( search_DS search, FILE *influence_report_fp, shortstr report_mode,
                     unsigned int comment_data_headers_p)
{
  search_try_DS *tries = search->tries;
  int n_final_summary = search->n_final_summary, n_save = search->n_save, i;
  int new_line_p = FALSE;
  char *pad = "  ";
  fxlstr dashes = "\n_________________________________________________________________"
    "____________";

  if (eqstring (report_mode, "text") == TRUE) {
    fprintf( influence_report_fp, "%s", "\n\n\n\n\n\n");
    fprintf( influence_report_fp, "%s", dashes);
    fprintf( influence_report_fp, "%s", dashes);
  }
  fprintf( influence_report_fp, "%s\n%s\n", (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");
  if (eqstring (report_mode, "data") == TRUE) {
    fprintf( influence_report_fp, "DATA_SEARCH_SUMMARY\n");
    pad = "";
  }
  fprintf( influence_report_fp, "%sSEARCH SUMMARY %d tries over %s\n",
           (comment_data_headers_p == TRUE) ? "#" : "",
           search->n, format_time_duration((time_t) search->time));
  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp, "\n  _______________ ");
  fprintf( influence_report_fp, "%sSUMMARY OF %d BEST RESULTS",
           (comment_data_headers_p == TRUE) ? "#" : "",
           n_final_summary);
  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp, " _________________________  ##\n");
  else
    fprintf( influence_report_fp, "\n");
  for (i=0; i<n_final_summary; i++) {
    if (i < search->n_tries) {
      print_search_try( influence_report_fp, NULL, tries[i], (i < n_save), new_line_p,
                        pad, FALSE); /* comment_data_headers_p); */
      if (influence_report_fp != NULL) {
        if (i < n_save)
          fprintf( influence_report_fp, "  -%d\n", i + 1);
        else
          fprintf( influence_report_fp, "\n");
      }
    }
  }
  if (eqstring (report_mode, "text") == TRUE) {
    fprintf( influence_report_fp, "\n\n   ## - report filenames suffix");
    fprintf( influence_report_fp, "%s", dashes);
    fprintf( influence_report_fp, "%s", dashes);
  }
} 


/* CLASS_WEIGHTS_AND_STRENGTHS
   14feb95 wmt: new
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   13feb98 wmt: add args to output_title fprintf for new page

   output class weights and strengths
   */
void class_weights_and_strengths( clsf_DS clsf, FILE *influence_report_fp,
                                  shortstr report_mode,
                                  unsigned int comment_data_headers_p)
{
  int clsf_class_number, report_class_number, n_data = clsf->database->n_data;
  int line_cnt = 11, max_line_cnt = G_line_cnt_max;
  float max_strength = clsf->reports->max_class_strength, class_wt, class_strength;
  fxlstr output_title =
    "%s\n%s   Class     Log of class       Relative         Class     Normalized\n"
      "%s    num        strength       class strength     weight    class weight\n%s";
  /* influence_report_fp => stdout */
  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp,
            "\n  We give below a heuristic measure of class strength: the approximate\n"
            "  geometric mean probability for instances belonging to each class,\n"
            "  computed from the class parameters and statistics.  This approximates\n"
            "  the contribution made, by any one instance \"belonging\" to the class,\n"
            "  to the log probability of the data set w.r.t. the classification.  It\n"
            "  thus provides a heuristic measure of how strongly each class predicts\n"
            "  \"its\" instances.\n");

  fprintf( influence_report_fp, output_title,
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");

  for (report_class_number=0; report_class_number<clsf->n_classes;
       report_class_number++) {
    line_cnt++;
    if ((eqstring (report_mode, "text") == TRUE) && (line_cnt > max_line_cnt)) {
      fprintf( influence_report_fp, "\f");
      line_cnt = 7;
      fprintf( influence_report_fp, output_title,
               (comment_data_headers_p == TRUE) ? "#" : "",
               (comment_data_headers_p == TRUE) ? "#" : "");
    }
    clsf_class_number = map_class_num_report_to_clsf( clsf, report_class_number);
    class_wt = clsf->classes[clsf_class_number]->w_j;
    class_strength = clsf->reports->class_strength[clsf_class_number];
    if (eqstring (report_mode, "data") == TRUE)
      fprintf( influence_report_fp, "\n%02d", report_class_number);
    else
      fprintf( influence_report_fp, "\n%6d", report_class_number);
    fprintf( influence_report_fp,
             "        %9.2e         %9.2e      %6d        %6.3f",
             class_strength,
             (float) safe_exp( (double) (class_strength - max_strength)),
             iround( (double) class_wt), class_wt / n_data);
  }
} 


/* CLASS_DIVERGENCES
   22jul95 wmt: new
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   13feb98 wmt: add args to output_title fprintf for new page

   output class divergences
   */
void class_divergences( clsf_DS clsf, FILE *influence_report_fp, shortstr report_mode,
                        unsigned int comment_data_headers_p)
{
  int clsf_class_number, report_class_number, n_data = clsf->database->n_data;
  int line_cnt = 11, max_line_cnt = G_line_cnt_max;
  float class_wt, class_divergence;
  fxlstr output_title =
    "\n%s   Class       class cross entropy       Class     Normalized\n"
    "%s    num        w.r.t. global class       weight    class weight\n%s";


  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp,
            "\n  The class divergence, or cross entropy w.r.t. the single class\n"
            "  classification, is a measure of how strongly the class probability\n"
            "  distribution  function differs from that of the database as a whole. \n"
            "  It is zero for identical distributions, going infinite when two\n"
            "  discrete distributions place probability 1 on differing values of the\n"
            "  same attribute.\n");

  fprintf( influence_report_fp, output_title,
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");

  for (report_class_number=0; report_class_number<clsf->n_classes;
       report_class_number++) {
    line_cnt++;
    if (eqstring (report_mode, "text") && (line_cnt > max_line_cnt)) {
      fprintf( influence_report_fp, "\f");
      line_cnt = 7;
      fprintf( influence_report_fp, output_title,
               (comment_data_headers_p == TRUE) ? "#" : "",
               (comment_data_headers_p == TRUE) ? "#" : "");
    }
    clsf_class_number = map_class_num_report_to_clsf( clsf, report_class_number);
    class_wt = clsf->classes[clsf_class_number]->w_j;
    class_divergence = clsf->classes[clsf_class_number]->i_sum;
    if (eqstring (report_mode, "data") == TRUE)
      fprintf( influence_report_fp, "\n%02d", report_class_number);
    else
      fprintf( influence_report_fp, "\n%6d", report_class_number);
    fprintf( influence_report_fp,
            "              %9.2e          %6d        %6.3f",
             class_divergence, iround( (double) class_wt), class_wt / n_data);
  }
} 


/* TEXT_STREAM_HEADER
   14feb95 wmt: new
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p

   output text stream header for influence values report
   */
void text_stream_header( int single_class_p, FILE *influence_report_fp, shortstr report_mode, 
                         int header_information_p, clsf_DS clsf, search_DS search,
                         char *results_file_ptr, char *title_line_1, char *title_line_2,
                         clsf_DS test_clsf, unsigned int order_attributes_by_influence_p,
                         unsigned int comment_data_headers_p)
{
  char blank = ' ';

  if ((eqstring (report_mode, "text") == TRUE) && (single_class_p))
    fprintf( influence_report_fp,
            "\n\n\n%6cI N F L U E N C E   V A L U E S   R E P O R T   \n"
            "%6c order attributes by influence values = %s\n"
            "%6c=============================================\n\n", blank, blank,
            (order_attributes_by_influence_p == TRUE) ? "true" : "false",
            blank);
  if (header_information_p)
    influence_values_header( clsf, search, results_file_ptr, header_information_p,
                            influence_report_fp, report_mode, test_clsf,
                             comment_data_headers_p); 
  if (eqstring (report_mode, "text") == TRUE)
    fprintf( influence_report_fp, "%s\n%s\n", (comment_data_headers_p == TRUE) ? "#" : "",
           (comment_data_headers_p == TRUE) ? "#" : "");
  fprintf( influence_report_fp, "%s", title_line_1);
  fprintf( influence_report_fp, "\n%s\n",
           (comment_data_headers_p == TRUE) ? "#" : "");
  if (eqstring (report_mode, "text") == TRUE) {
    fprintf( influence_report_fp, "%s", title_line_2);
    fprintf( influence_report_fp, "%s\n", (comment_data_headers_p == TRUE) ? "#" : "");
  }
}


/* PRE_FORMAT_ATTRIBUTES
   14feb95 wmt: new; combine get_attribute_header & print_attribute_header
                into a separate function named print_attribute_header
   12jun95 wmt: sort real attributes, if they are multi_normal_cn model
                since they all have the same influence value.
   20jun95 wmt: add real_atts_p & discrete_atts_p
   24jul95 wmt: add order_attributes_by_influence_p
   04jul96 wmt: check for malloc/realloc returning NULL
   14oct96 wmt: Correlation matrix only printed if MNcn attribute group is
                last - fix it so embedded groups will also be printed
   25feb97 wmt: check for num_terms > 0 prior to calling sort_mncn_attributes.
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   26feb98 wmt: properly order or not order multiple mncn model term groups

   outputs variable/values stored in the accumulater. the values are
   displayed as probabilities, that is weight/total-wt. sort the influence
   values of all attributes
   */
void pre_format_attributes( clsf_DS clsf, int clsf_class_number, int num_atts_to_list,
                            int line_cnt, int discrete_atts_header_p,
                            int real_atts_header_p, FILE *influence_report_fp,
                            shortstr report_mode, 
                            unsigned int order_attributes_by_influence_p,
                            unsigned int comment_data_headers_p)
{
  sort_cell_DS sort_list = NULL;
  int (* class_wt_sort_func) () = float_sort_cell_compare_gtr;
  int number_of_sorted_attributes = 0, n_att, i, term_count = 0;
  int sort_index, n_model_term, current_model_term;
  i_discrete_DS discrete_iv_struct = NULL;
  i_integer_DS integer_iv_struct = NULL;
  i_real_DS real_iv_struct = NULL;
  char *type;
  model_DS model;
  float *mean_sigma_list;
  unsigned int mncn_attributes_p = FALSE;

  for (n_att=0; n_att<clsf->database->n_atts; n_att++) {
    type = report_att_type( clsf, clsf_class_number, n_att);
    if (eqstring( type, "ignore") != TRUE) {
      if (sort_list == NULL) {
        sort_list = (sort_cell_DS) malloc( sizeof( struct sort_cell));
        if (sort_list == NULL) {
          fprintf( stderr,
                 "ERROR: pre_format_attributes: out of memory, malloc returned NULL!\n");
          exit(1);
        }
      } else {
        sort_list = (sort_cell_DS) realloc( sort_list, (number_of_sorted_attributes + 1) *
                                           sizeof( struct sort_cell));
        if (sort_list == NULL) {
          fprintf( stderr,
                 "ERROR: pre_format_attributes: out of memory, realloc returned NULL!\n");
          exit(1);
        }
      }
      if (eqstring( type, "discrete")) {
        discrete_iv_struct =
          (i_discrete_DS) clsf->classes[clsf_class_number]->i_values[n_att];
        sort_list[number_of_sorted_attributes].float_value =
          discrete_iv_struct->influence_value;
      }
      else if (eqstring( type, "integer")) {
        integer_iv_struct =
          (i_integer_DS) clsf->classes[clsf_class_number]->i_values[n_att];
        sort_list[number_of_sorted_attributes].float_value =
          integer_iv_struct->influence_value;
      }
      else if (eqstring( type, "real")) {
        real_iv_struct =
          (i_real_DS) clsf->classes[clsf_class_number]->i_values[n_att];
        sort_list[number_of_sorted_attributes].float_value =
          real_iv_struct->influence_value;
      }
      else {
        fprintf( stdout, "ERROR: attribute type %s not supported\n", type);
        abort();
      }
      sort_list[number_of_sorted_attributes].int_value = n_att;
      number_of_sorted_attributes++;
    }
  }

  if (order_attributes_by_influence_p == TRUE) {
    /* sort sort_list by greatest influence value first */
    qsort( (char *) sort_list, number_of_sorted_attributes, sizeof( struct sort_cell),
          class_wt_sort_func);
  }
  print_attribute_header( discrete_atts_header_p, real_atts_header_p,
                         influence_report_fp, report_mode, comment_data_headers_p);

  /*
  fprintf( stderr, "initial sort: "); 
  for (sort_index=0; sort_index<number_of_sorted_attributes ; sort_index++) { 
    fprintf( stderr, "%d ", sort_list[sort_index].int_value); 
  } 
  fprintf( stderr, "\n"); 
  */
  model = clsf->classes[clsf_class_number]->model; 
  current_model_term = 0;
  for (sort_index=0; sort_index<number_of_sorted_attributes ; sort_index++) {
    n_att = sort_list[sort_index].int_value;
    n_model_term = attribute_model_term_number( n_att, model);
    /* sort attribute in each model term group by |Mean-jk - Mean*-k| / StDev-jk */
    if ((eqstring( report_att_type( clsf, clsf_class_number, n_att), "real")) &&
        (eqstring( ((char *) get( rpt_att_model_term_type( clsf, clsf_class_number, n_att),
                                  "print_string")), "MNcn"))) {
      mncn_attributes_p = TRUE;
      if ((current_model_term == 0) || (current_model_term == n_model_term)) {
        real_iv_struct = (i_real_DS) clsf->classes[clsf_class_number]->i_values[n_att];
        mean_sigma_list = real_iv_struct->mean_sigma_list;
        sort_list[sort_index].float_value =
          ((float) fabs((double) (mean_sigma_list[0] - mean_sigma_list[2]))) /
          mean_sigma_list[1];
        /*
        if (term_count == 0) { 
          fprintf( stderr, "\nmncn sort atts: %d ", n_att); 
        } else { 
          fprintf( stderr, "%d ", n_att); 
        }
        */
        term_count++; 
      } else {
        /* next model term is also MNcn */
        if (order_attributes_by_influence_p == TRUE) {
          sort_mncn_attributes( sort_list, sort_index, term_count, clsf,
                                clsf_class_number);
        }
        term_count = 1;
      }
      current_model_term = n_model_term;
    }
    else {
      /* do sort here if Mncn att group is embedded in list of all sorted atts
         and this att is not MNcn */
      if (term_count > 0)
        if (order_attributes_by_influence_p == TRUE) {
          sort_mncn_attributes( sort_list, sort_index, term_count, clsf,
                                clsf_class_number);
        }
      term_count = 0; current_model_term = 0;
    }
  }

  /* do sort here if Mncn att group is at end of list of all sorted atts */
  if ((term_count > 0) && (order_attributes_by_influence_p == TRUE)) {
    sort_mncn_attributes( sort_list, sort_index, term_count, clsf,
                          clsf_class_number);
  }
 
  for (i=0; i<min( num_atts_to_list, number_of_sorted_attributes); i++)
    line_cnt = format_attribute( clsf, clsf_class_number, sort_list[i].int_value,
                                line_cnt, discrete_atts_header_p, real_atts_header_p,
                                influence_report_fp, report_mode, comment_data_headers_p);  
  if (sort_list != NULL)
    free( sort_list);
}


/* PRINT_ATTRIBUTE_HEADER
   15feb95 wmt: new; combination of get_attribute_header & print_attribute_header
   09jun95 wmt: change real header to global comparisons
   14jun95 wmt: change discrete headers from Mean -> Prob
   18mar97 wmt: add report_mode 
   14may97 wmt: add comment_data_headers_p
   
  attribute header for text stream
  */
void print_attribute_header( int discrete_atts_header_p, int real_atts_header_p,
                             FILE *influence_report_fp, shortstr report_mode,
                             unsigned int comment_data_headers_p)
{
  if (discrete_atts_header_p) {
    fprintf( influence_report_fp,
            "%s\n%sDISCRETE ATTRIBUTE  (t = D)                                  "
             "   log(", (comment_data_headers_p == TRUE) ? "#" : "",
             (comment_data_headers_p == TRUE) ? "#" : "");
    fprintf( influence_report_fp,
            "\n%s numb  t mtt  description           I-jk   Value name/Index  "
            "   Prob-jkl/     Prob      Prob",
             (comment_data_headers_p == TRUE) ? "#" : "");
    fprintf( influence_report_fp, 
            "\n%s t   a                                                       "
            "   Prob-*kl)     -jkl      -*kl\n",
             (comment_data_headers_p == TRUE) ? "#" : "");
  }
  if (real_atts_header_p) {
         /* "\n%sINTEGER or REAL ATTRIBUTE  (t = I or R)                      " */
    fprintf( influence_report_fp,
            "%s\n%sREAL ATTRIBUTE  (t = R)                                      "
             "  |Mean-jk -", (comment_data_headers_p == TRUE) ? "#" : "",
             (comment_data_headers_p == TRUE) ? "#" : "");
    fprintf( influence_report_fp, 
            "\n%s numb  t mtt  description           I-jk      Mean     StDev "
            "   Mean-*k|/     Mean      StDev",
             (comment_data_headers_p == TRUE) ? "#" : "");
    fprintf( influence_report_fp, 
            "\n%s t   a                                        -jk      -jk   "
            "    StDev-jk     -*k       -*k\n%s\n",
             (comment_data_headers_p == TRUE) ? "#" : "",
             (comment_data_headers_p == TRUE) ? "#" : "");
  }
  else
    fprintf( influence_report_fp, "%s\n", (comment_data_headers_p == TRUE) ? "#" : "");
}


/* FORMAT_ATTRIBUTE
   15feb95 wmt: new
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   14jun97 wmt: pass clsf_class_number & clsf to format_real_attribute

   format the listing of the requested attribute
   */
int format_attribute( clsf_DS clsf, int clsf_class_number, int n_att, int line_cnt,
                      int discrete_atts_header_p, int real_atts_header_p,
                      FILE *influence_report_fp, shortstr report_mode,
                      unsigned int comment_data_headers_p)
{

  static char header[60], header_continued[60];
  int line_length = 20, descrp_length, n_model_term, i;
  shortstr temp = "", temp1 = "";
  i_discrete_DS discrete_influence_values;
  i_integer_DS integer_influence_values;
  i_real_DS real_influence_values;
  char *type, type_letter[2], *description, model_term_type_symbol[] = "          ";
  char *print_string;                                                             
  char dot = '.', caller[] = "format_attribute";
  model_DS model = clsf->classes[clsf_class_number]->model;

  if (eqstring (report_mode, "data") == TRUE)
    dot = ' ';
  type = report_att_type( clsf, clsf_class_number, n_att);
  if (eqstring( type, "discrete"))
    strcpy( type_letter, "D");
  else if (eqstring( type, "integer"))
    strcpy( type_letter, "I");
  else if (eqstring( type, "real"))
    strcpy( type_letter, "R");
  else {
    fprintf( stderr, "ERROR: type %s not handled\n", type);
    abort();
  }
  description = clsf->database->att_info[n_att]->dscrp;
  descrp_length = strlen( description);
  print_string = (char *) get( rpt_att_model_term_type( clsf, clsf_class_number, n_att),
                              "print_string");
  strcpy( model_term_type_symbol, (eqstring( print_string, "ignore"))
         ? "__" : print_string);
  n_model_term = attribute_model_term_number( n_att, model);

  for (i=0; i< (line_length - descrp_length - 1); i++)
    temp1[i] = dot;
  temp1[i] = '\0';
  strcat( strcat( strncat( temp, description, line_length),
                 (descrp_length < line_length) ? " " : ""), temp1);
  safe_sprintf( header, sizeof( header), caller, "%03d %03d %s %-4s  %20s",
               n_model_term, n_att, type_letter, model_term_type_symbol, temp);
  for (i=line_length; i< min( descrp_length, 2 * line_length); i++)
    header_continued[i - line_length] = description[i];
  header_continued[i - line_length] = '\0';
          
  if (eqstring( type, "discrete")) {
    discrete_influence_values =
      (i_discrete_DS) clsf->classes[clsf_class_number]->i_values[n_att];
    line_cnt =
      format_discrete_attribute( n_att, clsf->database, header, header_continued,
                                discrete_influence_values, line_length, description,
                                line_cnt, discrete_atts_header_p, real_atts_header_p,
                                influence_report_fp, report_mode,
                                 comment_data_headers_p);
  }
  else if (eqstring( type, "integer")) {
    integer_influence_values =
      (i_integer_DS) clsf->classes[clsf_class_number]->i_values[n_att];
    line_cnt =
      format_integer_attribute( header, header_continued, integer_influence_values,
                                line_length, description, model_term_type_symbol, line_cnt,
                                discrete_atts_header_p, real_atts_header_p,
                                influence_report_fp, report_mode,
                                comment_data_headers_p);
  }
  else if (eqstring( type, "real")) {
    real_influence_values =
      (i_real_DS) clsf->classes[clsf_class_number]->i_values[n_att];
    line_cnt =
      format_real_attribute( header, header_continued, real_influence_values, line_length,
                             n_att, description, model_term_type_symbol, line_cnt,
                             discrete_atts_header_p, real_atts_header_p,
                             influence_report_fp, report_mode, comment_data_headers_p,
                             clsf_class_number, clsf);
  }
  return (line_cnt);
}


/* FORMAT_DISCRETE_ATTRIBUTE
   17feb95 wmt: new
   14jun95 wmt: discrete attribute instance value significance computation
                changed from fabs( log( local_prob / global_prob)) to
                local_prob * log( local_prob / global_prob)
   26jun96 wmt: do not process attributes with warnings_and_errors
   04jul96 wmt: check for malloc/realloc returning NULL
   14oct96 wmt: correct bad test for warn_errs->single_valued_warning
                svw == NULL => eqstring( svw, "")
   18mar97 wmt: add report_mode
   14may97 wmt: add comment_data_headers_p
   20sep98 wmt: filter windows e+000 => e+00, etc with filter_e_format_exponents
   22jan02 wmt: prevent string overflow of discrete_string_name (strcpy => strncpy)
   17aug09 wmt: put header all on one line

   format discrete attribute for influence values report
   */
int format_discrete_attribute( int n_att, database_DS d_base, char *header,
                               char *header_continued, i_discrete_DS influence_values,
                               int line_length, char *description, int line_cnt,
                               int discrete_atts_header_p, int real_atts_header_p,
                               FILE *influence_report_fp, shortstr report_mode,
                               unsigned int comment_data_headers_p)
{
  int index, name_length, name_max = 20, line_cnt_max = G_line_cnt_max, i;
  int new_lines, list_index = 0;
  char blank = ' ', dot = '.';
  float *p_p_star_list;         /* triplets of term_index, local, & global probs */
  float local_prob, global_prob, att_value_influence;
  shortstr temp, discrete_string_name;
  formatted_p_p_star_DS formatted_p_p_star_list = NULL;
  int (* influ_influence_sort_func) () = float_p_p_star_compare_gtr;
  warn_err_DS warn_errs;
  /* |14|20|1|6|2|20|1|9|2|9|1|9| */
  /* force windows to keep column alignment */
#ifdef _WIN32
  char* format_string_1 = "%34s %6.3f  %-20s %+9.2e  %+9.2e %+9.2e\n";
  char* format_string_2 = "%-20s %+9.2e  %+9.2e %+9.2e\n";
#else
  /* char* format_string_1 = "%34s %6.3f  %-20s %9.2e  %9.2e %9.2e\n"; */
  char* format_string_1 = "%35s %6.3f  \n                                            %20s %9.2e  %9.2e %9.2e\n";
  char* format_string_2 = "%-20s %9.2e  %9.2e %9.2e\n";
#endif
  fxlstr e_format_string;

  static char header_prefix[60];

  warn_errs = d_base->att_info[n_att]->warnings_and_errors;
  if (eqstring (report_mode, "data") == TRUE)
    dot = ' ';
  if ((warn_errs->num_expander_warnings == 0) &&
      (warn_errs->num_expander_errors == 0) &&
      (eqstring( warn_errs->single_valued_warning, ""))) {
    for (index=0; index<influence_values->n_p_p_star_list; index += 3) {
      p_p_star_list = influence_values->p_p_star_list;
      strncpy( discrete_string_name,
               d_base->att_info[n_att]->translations[(int) p_p_star_list[index]],
               name_max + 1);
      /* if name_max + 1 exceeded, null terminate string */
      discrete_string_name[name_max] = '\0';
      name_length = strlen( discrete_string_name);
      local_prob = p_p_star_list[index+1];
      global_prob = p_p_star_list[index+2];
      att_value_influence = ((local_prob == 0.0) || (global_prob == 0.0)) ?
        0.0 : (float) safe_log( (double) (local_prob / global_prob));
      for (i=0; i< max( 0, (name_max - name_length - 1)); i++)
        temp[i] = dot;
      temp[i] = '\0';
      strcat( strcat( discrete_string_name, (name_length < name_max) ? " " : ""), temp);
      if (formatted_p_p_star_list == NULL) {
        formatted_p_p_star_list =
          (formatted_p_p_star_DS) malloc( sizeof( struct formatted_p_p_star));
        if (formatted_p_p_star_list == NULL) {
          fprintf( stderr,
                 "ERROR: format_discrete_attribute: out of memory, malloc returned NULL!\n");
          exit(1);
        }
      } else {
        formatted_p_p_star_list =
          (formatted_p_p_star_DS) realloc( formatted_p_p_star_list,
                                          (index + 1) * sizeof( struct formatted_p_p_star));
        if (formatted_p_p_star_list == NULL) {
          fprintf( stderr,
                 "ERROR: format_discrete_attribute: out of memory, realloc returned NULL!\n");
          exit(1);
        }
      }
      strcpy( formatted_p_p_star_list[list_index].discrete_string_name, discrete_string_name);
      formatted_p_p_star_list[list_index].abs_att_value_influence =
        (float) fabs( (double) att_value_influence);
      formatted_p_p_star_list[list_index].att_value_influence = att_value_influence;
      formatted_p_p_star_list[list_index].local_prob = local_prob;
      formatted_p_p_star_list[list_index].global_prob = global_prob;
      list_index++;
    }
    new_lines = influence_values->n_p_p_star_list / 3;
    /* sort by greatest influence value first */
    qsort( (char *) formatted_p_p_star_list, new_lines, sizeof( struct formatted_p_p_star),
          influ_influence_sort_func);
    line_cnt += new_lines;
    if ((eqstring (report_mode, "text") == TRUE) && (line_cnt > line_cnt_max)) {
      /* 2nd & subseq. pages */
      fprintf( influence_report_fp, "\f");
      line_cnt = real_atts_header_p * 4 + discrete_atts_header_p * 4 + new_lines;
      print_attribute_header( discrete_atts_header_p, real_atts_header_p,
                             influence_report_fp, report_mode, comment_data_headers_p);
    }

    /* put header all on one line */
    strncpy(header_prefix, header, 14);
    fprintf( influence_report_fp, "%s %s\n", header_prefix, description);
    strcpy(header, " "); 

    sprintf( e_format_string, format_string_1, header,
            influence_values->influence_value,
            formatted_p_p_star_list[0].discrete_string_name,
            formatted_p_p_star_list[0].att_value_influence,
            formatted_p_p_star_list[0].local_prob, formatted_p_p_star_list[0].global_prob);
    fprintf( influence_report_fp, "%s", filter_e_format_exponents( e_format_string)); 
    
    for (i=1; i<new_lines; i++) {
      if ((i == 1) && ((int) strlen( description) > line_length))
        fprintf( influence_report_fp, "%14c %-20s %6c  ", blank, header_continued, blank);
      else
        fprintf( influence_report_fp, "%44c", blank);
      sprintf( e_format_string, format_string_2,
              formatted_p_p_star_list[i].discrete_string_name,
              formatted_p_p_star_list[i].att_value_influence,
              formatted_p_p_star_list[i].local_prob, formatted_p_p_star_list[i].global_prob);
      fprintf( influence_report_fp, "%s", filter_e_format_exponents( e_format_string));
    }
    free( formatted_p_p_star_list);
  }
  return (line_cnt);
}


/* FORMAT_INTEGER_ATTRIBUTE
   17feb95 wmt: new
   14may97 wmt: add comment_data_headers_p
   */
int format_integer_attribute( char *header, char *header_continued,
                              i_integer_DS integer_influence_values,
                              int line_length, char *description,
                              char *model_term_type_symbol, int line_cnt,
                              int discrete_atts_header_p, int real_atts_header_p,
                              FILE *influence_report_fp, shortstr report_mode,
                              unsigned int comment_data_headers_p)
{
  fprintf( stderr, "Not supported yet\n");
  abort();
  return (line_cnt);
}


/* FORMAT_REAL_ATTRIBUTE 
   17feb95 wmt: new
   09jun95 wmt: do not output covariance matrix; use fixed decimal for
                correlation matrix; add (Mean-jk - Mean-*k) / StDev-jk 
                as a significance measure
   18mar97 wmt: add report_mode 
   14may97 wmt: add comment_data_headers_p
   14jun97 wmt: pass in clsf to use in printing multiple
                correlation matricies
   21nov97 wmt: correct correlation matrices print-out for non-
                contiguous attribute ranges, and print matrices
                once after all class attributes are listed.
   20mar98 wmt: outputing of MNcn correlation matrices after last class attribute
                instead of after each term is now done by a call to
                generate_mncn_correlation_matrices from
                autoclass_class_influence_values_report
   20sep98 wmt: filter windows e+000 => e+00, etc with filter_e_format_exponents
   17aug09 wmt: put header all on one line

   format real attribute for influence values report
   */
int format_real_attribute( char *header, char *header_continued,
                           i_real_DS influence_values, int line_length,
                           int n_att, char *description, char *model_term_type_symbol,
                           int line_cnt, int discrete_atts_header_p,
                           int real_atts_header_p, FILE *influence_report_fp,
                           shortstr report_mode, unsigned int comment_data_headers_p,
                           int clsf_class_number, clsf_DS clsf) 
{ 
  int line_cnt_max = G_line_cnt_max, new_lines = 1;
  float *mean_sigma_list = influence_values->mean_sigma_list;
  char blank = ' ';
  /* |14|20|1|6|1|(|9|2|9|)|1|9|1|(|9|1|9|)| */ 
  /* force windows to keep column alignment */
#ifdef _WIN32
  char* format_string_1 = "%34s %6.3f (%+9.2e %+9.2e) %+9.2e (%+9.2e %+9.2e)\n";  
  char* format_string_2 = "Prob-jk is known %+9.2e   Prob-*k   is  known %+9.2e\n";
#else
  /* char* format_string_1 = "%33s %6.3f (%9.2e %9.2e) %9.2e (%9.2e %9.2e)\n"; */
  char* format_string_1 = "%35s %6.3f (%9.2e %9.2e) %9.2e (%9.2e %9.2e)\n";
  char* format_string_2 = "Prob-jk is known %9.2e   Prob-*k   is  known %9.2e\n";
#endif
  fxlstr e_format_string;

  static char header_prefix[60];

  if (((int) strlen( description) > line_length) ||
      eqstring( model_term_type_symbol, "SNcm"))
    new_lines++;
  if ((eqstring (report_mode, "text") == TRUE) && (line_cnt > line_cnt_max)) {
    /* 2nd & subseq. pages */
    fprintf( influence_report_fp, "\f");
    line_cnt = real_atts_header_p * 4 + discrete_atts_header_p * 4 + new_lines;
    print_attribute_header( discrete_atts_header_p, real_atts_header_p,
                           influence_report_fp, report_mode, comment_data_headers_p);
  }
  else
    line_cnt += new_lines;

  /* put header all on one line */
  strncpy(header_prefix, header, 14);
  fprintf( influence_report_fp, "%s %s\n", header_prefix, description);
  strcpy(header, " "); 

  sprintf( e_format_string, format_string_1,
           header, influence_values->influence_value,
           mean_sigma_list[0], mean_sigma_list[1],
           ((float) fabs((double) (mean_sigma_list[0] - mean_sigma_list[2]))) /
           mean_sigma_list[1], mean_sigma_list[2], mean_sigma_list[3]);
  fprintf( influence_report_fp, "%s", filter_e_format_exponents( e_format_string));
  /*  if ((int) strlen( description) > line_length) */
  /*  fprintf( influence_report_fp, "%13c %-20s  %s", blank, header_continued, */
  /*  (eqstring( model_term_type_symbol, "SNcm")) ? "" : "\n"); */
  if (eqstring( model_term_type_symbol, "SNcm")) {
    if ((int) strlen( description) <= line_length)
      fprintf( influence_report_fp, "%37c", blank);
    sprintf( e_format_string, format_string_2,
            mean_sigma_list[4], mean_sigma_list[5]);
    fprintf( influence_report_fp, "%s", filter_e_format_exponents( e_format_string));
  }
  return (line_cnt);
} 


/* GENERATE_MNCN_CORRELATION_MATRICES
   20mar98 wmt: new

      outputing of MNcn correlation matrices after last class attribute
      instead of after each term is now done by a call to
      generate_mncn_correlation_matrices from
      autoclass_class_influence_values_report
*/
void generate_mncn_correlation_matrices ( clsf_DS clsf, int clsf_class_number,
                                          shortstr report_mode,
                                          unsigned int comment_data_headers_p,
                                         FILE *influence_report_fp )
{
  int i, j, first_row, n_att;
  int n_term_list = 0;
  float *term_list = NULL, *f_list;
  float **covar_matrix = NULL, **correl_matrix = NULL;
  i_real_DS real_influence_values = NULL;
  int n_atts = clsf->database->n_atts;
  char *att_type, *model_term_type;
  int correl_term_list[50] = {END_OF_INT_LIST};
  int num_correl_term_list, max_num_correl_term_list = 50;
  int *i_list, k;

  /* find one attr from each of the term attribute lists */
  for (i = (n_atts - 1); i >= 0; i--) {
    att_type = report_att_type( clsf, clsf_class_number, i);
    model_term_type = (char *) get( rpt_att_model_term_type( clsf,
                                                             clsf_class_number, i),
                                    "print_string");
    if ((eqstring( att_type, "real") == TRUE) &&
        (eqstring( model_term_type, "MNcn"))) {
      real_influence_values =  
        (i_real_DS) clsf->classes[clsf_class_number]->i_values[i];  
      n_term_list = real_influence_values->n_term_att_list;
      term_list = real_influence_values->term_att_list;
      /* fprintf( stderr, "i %d last %d\n", i, (int) term_list[n_term_list - 1]); */
      if (i == (int) term_list[n_term_list - 1]) { 
        push_int_list( correl_term_list, &num_correl_term_list, i,
                       max_num_correl_term_list);
      }
    }
  }
  /* 
    fprintf( stderr, "correl_term_list: ");   
    i_list = correl_term_list;   
    for ( ; *i_list != END_OF_INT_LIST; i_list++) {   
      fprintf( stderr, "%d ", *i_list);   
    }   
    fprintf( stderr, "\n");  
    */

  i_list = correl_term_list; 
  for ( ; *i_list != END_OF_INT_LIST; i_list++) {
    n_att = *i_list;
    /* fprintf( stderr, "n_att %d\n", *i_list); */
    real_influence_values =  
      (i_real_DS) clsf->classes[clsf_class_number]->i_values[n_att];  
    n_term_list = real_influence_values->n_term_att_list;
    term_list = real_influence_values->term_att_list;
    /*
    fprintf( stderr, "term_list: ");
    f_list = term_list;  
    for ( k=0; k < n_term_list; k++) {  
      fprintf( stderr, "%d ", (int) *f_list);
      f_list++;
    }  
    fprintf( stderr, "\n");  
    */
    if (eqstring (report_mode, "data") == TRUE)
      fprintf( influence_report_fp, "\nDATA_CORR_MATRIX");
    fprintf( influence_report_fp, "\n%s Correlation matrix (row & column indices are "
            "attribute numbers)\n   ",
             (comment_data_headers_p == TRUE) ? "#" : "");
    f_list = term_list;
    for ( k = 0; k < n_term_list; k++) {  
      fprintf( influence_report_fp, "%7d", (int) *f_list);
      f_list++;
    }
    fprintf( influence_report_fp, "\n");
    covar_matrix = real_influence_values->class_covar;
    correl_matrix = extract_rhos( covar_matrix, n_term_list);

    first_row = TRUE;
    for (i = 0; i < n_term_list; i++) {    
      if (first_row == TRUE)
        first_row = FALSE;
      else
        fprintf( influence_report_fp, "\n");

      for (j = 0; j < n_term_list; j++) {
        /* if ((j > 0) && ((j % NUM_TOKENS_IN_FXLSTR) == 0))
           fprintf( influence_report_fp, "\n%2c", blank); */
        if (j == 0) {
          if (eqstring (report_mode, "data") == TRUE)
            fprintf( influence_report_fp, "%02d", (int) term_list[i]);
          else
            fprintf( influence_report_fp, " %2d", (int) term_list[i]);
        }
        fprintf( influence_report_fp, " %6.3f", correl_matrix[i][j]);
      }
    }
    for (k=0; k<n_term_list; k++) 
      free( correl_matrix[k]);  
    free( correl_matrix); 
    fprintf( influence_report_fp, "\n\n");
  }
}


/* ATTRIBUTE_MODEL_TERM_NUMBER
   12jun95 wmt: new

   return model term number for n_att
*/
int attribute_model_term_number( int n_att, model_DS model)
{
  int i, j;

  for (i=0; i<model->n_terms; i++) {
    for (j=0; j<model->terms[i]->n_atts; j++) 
      if (model->terms[i]->att_list[j] == ((float) n_att))
        goto out;
  }
 out: return (i);
}


/* SORT_MNCN_ATTRIBUTES
   jun95 wmt: new
   04jul96 wmt: check for malloc/realloc returning NULL
   14oct96 wmt: type last_sorted_term_n_att as int, not float
   26feb98 wmt: move making last term_att be last sorted att to
                pre_format_attributes

   sort attributes within their model term by mean significance measure
   */
void sort_mncn_attributes( sort_cell_DS sort_list, int sort_index, int term_count,
                          clsf_DS clsf, int clsf_class_number)
{
  sort_cell_DS sort_list_temp;
  int temp_j, j;
  int (* mncn_sort_func) () = float_sort_cell_compare_gtr;
  i_real_DS real_iv_struct;

  /* fprintf( stderr, "sort_mncn_attributes: sort_index %d term_count %d\n",
          sort_index, term_count); */
  sort_list_temp = (sort_cell_DS) malloc( term_count * sizeof( struct sort_cell));
  if (sort_list_temp == NULL) {
    fprintf( stderr, "ERROR: sort_mncn_attributes: out of memory, malloc returned NULL!\n");
    exit(1);
  }
  /*
  fprintf( stderr, "\nmncn sorting: sort_index %d term_count %d\n", sort_index,
                        term_count);
  fprintf( stderr, "sort_mncn_attributes B: "); 
  for (j=(sort_index - term_count), temp_j=0; temp_j<term_count; j++, temp_j++) { 
    fprintf( stderr, "%d ", sort_list[j].int_value); 
  } 
  fprintf( stderr, "\n"); 
  */
  for (j=(sort_index - term_count), temp_j=0; temp_j<term_count; j++, temp_j++) {
    sort_list_temp[temp_j].int_value = sort_list[j].int_value;
    sort_list_temp[temp_j].float_value = sort_list[j].float_value;
  }

  qsort( (char *) sort_list_temp, term_count, 
         sizeof( struct sort_cell), mncn_sort_func);

  for (j=(sort_index - term_count), temp_j=0; temp_j<term_count; j++, temp_j++) {
    sort_list[j].int_value = sort_list_temp[temp_j].int_value;
    real_iv_struct =
      (i_real_DS) clsf->classes[clsf_class_number]->i_values[sort_list[j].int_value];
  }
  /*
  fprintf( stderr, "sort_mncn_attributes A: "); 
  for (j=(sort_index - term_count), temp_j=0; temp_j<term_count; j++, temp_j++) { 
    fprintf( stderr, "%d ", sort_list[j].int_value); 
  } 
  fprintf( stderr, "\n"); 
  */
  free( sort_list_temp);
}


/* FILTER_E_FORMAT_EXPONENTS
   20sep98 wmt: new

   for Windows filter e+000 => e+00 & e-000 => e-00
   */
char *filter_e_format_exponents ( fxlstr e_format_string )
{
  static char *filtered_numeric_string[STRLIMIT];

#ifdef _WIN32
  fxlstr suffix_string;
  int char_cnt = 0, i;
  char *match_addr_plus, *match_addr_minus, *match_addr;

  /* fprintf( stderr, "e_format_string %s\n", e_format_string); */
  strcpy( suffix_string, e_format_string);
  match_addr_plus = strstr( suffix_string, "e+0");
  match_addr_minus = strstr( suffix_string, "e-0");
  while ((match_addr_plus != NULL) || (match_addr_minus != NULL)) {
    if ((match_addr_plus != NULL) && (match_addr_minus != NULL)) {
      if (match_addr_plus < match_addr_minus)
	match_addr = match_addr_plus;
      else
	match_addr = match_addr_minus;
    } 
    else if (match_addr_plus != NULL)
      match_addr = match_addr_plus;
    else
      match_addr = match_addr_minus;

    char_cnt = char_cnt + match_addr - suffix_string + 2;

    /* strncpy does not append NULL to string2 after copying into string1 */
    for (i=0; i<STRLIMIT; i++)
      filtered_numeric_string[i] = '\0';
    suffix_string[0] = '\0';
    /* fprintf( stderr, "char_cnt %d e_format_string %s\n", char_cnt, e_format_string); */
    strncpy( (char *) filtered_numeric_string, e_format_string, char_cnt);
    /* fprintf( stderr, "filtered_numeric_string %s\n", filtered_numeric_string); */
    strcpy( suffix_string, e_format_string + char_cnt + 1);
    /* fprintf( stderr, "suffix_string %s\n", suffix_string); */
    strcat( (char *) filtered_numeric_string, suffix_string);

    match_addr_plus = strstr( suffix_string, "e+0");
    match_addr_minus = strstr( suffix_string , "e-0");
    strcpy( e_format_string, (char *) filtered_numeric_string);
  }
#else
  strcpy( (char *) filtered_numeric_string, e_format_string);
#endif

  return( (char *) filtered_numeric_string);
}
                     










