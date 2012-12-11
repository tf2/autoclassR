#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
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


/* AUTOCLASS_SEARCH
   06oct94 -> ?? wmt: numerous modifications
   07apr95 wmt: other functions moved to search-control-2.c, so that they
      can be compiled with -O flag.  This function with -O flag has 2 errors:
      segmentation fault after returning from print_final-report; &
      trashed autoclass_search parameter model_file_ptr; under gcc 2.6.3
      (see autoclass-c/prog/autoclass.make)
   26apr95 wmt: Check for non-NULL "best_clsfs" prior to writing ".results[-bin]"
      file.  Change NULL to 0 for 6th arg of defparam
   17may95 wmt: Convert from srand/rand to srand48/lrand48
   19dec97 wmt: if force_new_search_p is false, exit if there is no 
                <...>.results[-bin] file.  Make t the default for force_new_search_p 
   18feb98 wmt: add num_cycles and max_cycles to search try

   autoclass search main program 
   This function is the one users should see.  It has a bizillion keywords, but
   the simplest use is to just pass in a clsf [not any more :-) wmt]

   This function manages a search for good classifications, coordinating all
   the pieces without doing much itself.  See the argument list, the
   documentation around that list in the code, and the initial text printed out.
   Note that there is no explicit testing of arguments for correct type or
   mutual compatability.
   */
int autoclass_search( char *data_file_ptr, char *header_file_ptr, char *model_file_ptr,
                     char *search_params_file_ptr, char *search_file_ptr,
                     char *results_file_ptr, char *log_file_ptr)
{
  /* passed to clsf-DS-%= when deciding if a new clsf is duplicate of old
     REL_ERROR is 0.01 -- was 0.05 */
  float rel_error = REL_ERROR; 
  /* initially try these numbers of classes, so not to narrow the search
     too quickly.  the state of this list is saved in the <..>.search file
     and used on restarts, unless an override specification of start_j_list
     is made in this file for the restart run. */
  static int start_j_list[MAX_N_START_J_LIST] = {2, 3, 5, 7, 10, 15, 25, END_OF_INT_LIST};
  /* unless 0, overrides above 2 args, and always uses this j-in */
  int fixed_j = 0;
  /* wait at least this time since last report till report verbosely again */
  int min_report_period = 30;
  /* the search will end this time from now if it hasn't already */
  int max_duration = 0;
  /* if >0, search will end  after this many clsf tries have been done */
  int max_n_tries = 0;
  /* save this many clsfs to disk in results-file
     if 0, don't save anything (no .search & .results files) */
  int n_save = 2;
  /* if FALSE, do not write a log file */
  unsigned int log_file_p = TRUE;
  /* if FALSE, do not write a search file */
  unsigned int search_file_p = TRUE;
  /* if FALSE, do not write a results file */
  unsigned int results_file_p = TRUE;
  /* to protect against possible crash, will save to disk this often - seconds */
  int min_save_period = 1800;
  /* don't store any more than this many clsfs internally */
  int max_n_store = 10;         /* was 100 06jan94 wmt */
  /* print out descriptions of this many of the searches at the end */
  int n_final_summary = 10;
  /* clsf start function: "block" or "random" */
  shortstr start_fn_type = "random";
  /* clsf try function: "converge_search_3", "converge_search_4" or "converge" */
  shortstr try_fn_type = "converge_search_3";
  /* will call this function to decide how many classes to start next try
     with.  based on best clsfs so far. only "random_ln_normal" so far */
  shortstr n_classes_fn_type = "random_ln_normal";
  /* if TRUE, perform base_cycle in initialize_parameters */
  unsigned int initial_cycles_p = TRUE;
  /* FALSE saves as ascii text, TRUE saves as machine dependent binary. */
  unsigned int save_compact_p = TRUE;
  /* FALSE reads as ascii text, TRUE reads as machine dependent binary. */
  unsigned int read_compact_p = TRUE;
  /* FALSE uses 1 as the seed for srand48, the pseudo-random number function
     TRUE uses universal time clock as the seed */
  unsigned int randomize_random_p = TRUE;
  /* if > 0, will only read this many datum from .db2, rather than the whole file */
  int n_data = 0;
  /* passed to try_fn_type "converge"
     one of two candidate tests for log_marginal (clsf->log_a_x_h) delta between 
     successive convergence cycles. the largest of halt_range and (halt_factor * 
     current_log_marginal) is used. */
  float halt_range = 0.5;
  /* passed to try_fn_type "converge"
     one of two candidate tests for log_marginal (clsf->log_a_x_h) delta between 
     successive convergence cycles. the largest of halt_range and (halt_factor * 
     current_log_marginal) is used. */
  float halt_factor = 0.0001;
  /* passed to try_fn_type "converge_search_3"
     delta for each class of log aprox-marginal-likelihood of class statistics
     with-respect-to the class hypothesis (class->log_a_w_s_h_j) divided by the 
     class weight (class->w_j) between successive convergence cycles.
     increasing this value loosens the convergence and reduces the number of
     cycles. decreasing this value tightens the convergence and increases the
     number of cycles */
  float rel_delta_range = 0.0025;
  /* passed to try functions "converge_search_3" and "converge"
     number of cycles for which the convergence criterion must be satisfied for 
     the trial to terminate. */
  int n_average = 3;
  /* passed to try_fn_type "converge_search_4"
     delta for each class of log aprox-marginal-likelihood of class statistics 
     with-respect-to the class hypothesis (class->log_a_w_s_h_j) divided by the 
     class weight (class->w_j) over sigma_beta_n_values convergence cycles.  
     increasing this value loosens the convergence and reduces the number of 
     cycles. decreasing this value tightens the convergence and increases the 
     number of cycles */
  float cs4_delta_range = 0.0025;
  /* passed to try_fn_type "converge_search_4"
     number of past values to use in computing sigma^2 and beta^2. */
  int sigma_beta_n_values = 6;
  /* passed to all try functions.  They will end a trial if this many cycles
     have been done and the convergence criteria has not been satisfied. */
  int max_cycles = 200;
  /* if TRUE, the selected try function will print to the screen values useful in
     specifying non-default values for halt_range, halt_factor, rel_delta_range,
     n_average, sigma_beta_n_values, and  cs4_delta_range. */
  unsigned int converge_print_p = FALSE;
  /* if TRUE, will ignore any previous search results, discarding the existing 
     .search & .results[-bin] files after confirmation by the user.
     if FALSE, will continue the search using the existing .search &
     .results[-bin] files. */
  unsigned int force_new_search_p = TRUE;
  /* if TRUE, checkpoints of the current classification will be output every
     min_checkpoint_period seconds.  file extension is .chkpt -- useful for very 
     large classifications */
  unsigned int checkpoint_p = FALSE;
  /* if checkpoint_p = TRUE, the checkpointed classification will be written
     this often - in seconds (= 3 hours) */
  int min_checkpoint_period = 10800;
  /* can be either "chkpt" or "results"
     if "chkpt", continue convergence of the classification contained in 
     <...>.chkpt[-bin] -- checkpoint_p must be true.
     if "results", continue convergence of the best classification
     contained in <...>.results[-bin] -- checkpoint_p must be false. */
  shortstr reconverge_type = "";
  /* if false, no output is directed to the screen.  Assuming log_file_p = true,
     output will be directed to the log file only. */
  unsigned int screen_output_p = TRUE;
  /* if false, standard input is not queried each cycle for the character q.
     Thus either parameter max_n_tries or max_duration must be specified, or
     AutoClass will run forever. */
  unsigned int interactive_p = TRUE;
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

  /* -------------------------------------------------- */

  static FILE *log_file_fp = NULL, *search_file_fp = NULL;
  static FILE *header_file_fp = NULL, *model_file_fp = NULL;
  static FILE *stream, *search_params_file_fp = NULL;
  search_DS restart_search = NULL, search = NULL;
  clsf_DS clsf = NULL;
  int restart_p = FALSE, n_dup_tries, s_parms_error_cnt = 0;
  time_t begin, now, last_search_save, last_report, end_time = 0, last_results_save;
  time_t begin_try;
  int i, j_in, n_stored_clsf, dup_p, max_j, n_start_j_list, *new_start_j_list;
  int bclength = n_save, last_bclength = -1, list_global_clsf_p = TRUE;
  search_try_DS latest_try, best, *ss;
  static clsf_DS  *best_clsfs = NULL, *last_saved_clsfs = NULL;
  shortstr stop_reason;
  char temp_str[5], caller[] = "autoclass_search";
  PARAM params[MAXPARAMS];
  int n_params = 0, want_wts_p = TRUE, start_j_list_from_s_params = FALSE;
  int expand_p = TRUE, update_wts_p = FALSE, checkpoint_clsf_cnt = 0;
  int results_file_found = FALSE;
  static int clsf_n_list[MAX_CLSF_N_LIST] = {END_OF_INT_LIST};
  static fxlstr checkpoint_file, maybe_checkpoint_file, results_file, n_classes_explain;
  fxlstr str;
  int double_str_length = 2 * sizeof( fxlstr);
  int reread_p = FALSE, regenerate_p = FALSE, silent_p;
  char *double_str;
  unsigned int compact_p;
 
  /* -------------------------------------------------- */
  stream = stdout; /* aju 980612: Cannot use as initialzer; not a constant per MSVC.*/
  double_str = (char *) malloc( double_str_length);
  G_stream = stdout;
  checkpoint_file[0] = maybe_checkpoint_file[0] = results_file[0] = '\0';
  params[0].paramptr = NULL;
  defparam( params, n_params++, "rel_error", TFLOAT, &rel_error, 0);
  defparam( params, n_params++, "start_j_list", TINT_LIST, start_j_list,
           MAX_N_START_J_LIST); 
  defparam( params, n_params++, "n_classes_fn_type", TSTRING, n_classes_fn_type,
           SHORT_STRING_LENGTH);
  defparam( params, n_params++, "fixed_j", TINT, &fixed_j, 0);
  defparam( params, n_params++, "min_report_period", TINT, &min_report_period, 0);
  defparam( params, n_params++, "max_duration", TINT, &max_duration, 0);
  defparam( params, n_params++, "max_n_tries", TINT, &max_n_tries, 0);
  defparam( params, n_params++, "n_save", TINT, &n_save, 0);
  defparam( params, n_params++, "log_file_p", TBOOL, &log_file_p, 0);
  defparam( params, n_params++, "search_file_p", TBOOL, &search_file_p, 0);
  defparam( params, n_params++, "results_file_p", TBOOL, &results_file_p, 0);
  defparam( params, n_params++, "min_save_period", TINT, &min_save_period, 0);
  defparam( params, n_params++, "max_n_store", TINT, &max_n_store, 0);
  defparam( params, n_params++, "n_final_summary", TINT, &n_final_summary, 0);
  defparam( params, n_params++, "start_fn_type", TSTRING, start_fn_type,
           SHORT_STRING_LENGTH);
  defparam( params, n_params++, "try_fn_type", TSTRING, try_fn_type,
           SHORT_STRING_LENGTH);
  defparam( params, n_params++, "initial_cycles_p", TBOOL, &initial_cycles_p, 0);
  defparam( params, n_params++, "save_compact_p", TBOOL, &save_compact_p, 0);
  defparam( params, n_params++, "read_compact_p", TBOOL, &read_compact_p, 0);
  defparam( params, n_params++, "randomize_random_p", TBOOL, &randomize_random_p, 0);
  defparam( params, n_params++, "n_data", TINT, &n_data, 0);
  defparam( params, n_params++, "halt_range", TFLOAT, &halt_range, 0);
  defparam( params, n_params++, "halt_factor", TFLOAT, &halt_factor, 0);
  defparam( params, n_params++, "rel_delta_range", TFLOAT, &rel_delta_range, 0);
  defparam( params, n_params++, "n_average", TINT, &n_average, 0);
  defparam( params, n_params++, "cs4_delta_range", TFLOAT, &cs4_delta_range, 0);
  defparam( params, n_params++, "sigma_beta_n_values", TINT, &sigma_beta_n_values, 0);
  defparam( params, n_params++, "max_cycles", TINT, &max_cycles, 0);
  defparam( params, n_params++, "converge_print_p", TBOOL, &converge_print_p, 0);
  defparam( params, n_params++, "force_new_search_p", TBOOL, &force_new_search_p, 0);
  defparam( params, n_params++, "checkpoint_p", TBOOL, &checkpoint_p, 0);
  defparam( params, n_params++, "min_checkpoint_period", TINT, &min_checkpoint_period, 0);
  defparam( params, n_params++, "reconverge_type", TSTRING, &reconverge_type,
           SHORT_STRING_LENGTH);
  defparam( params, n_params++, "screen_output_p", TBOOL, &screen_output_p, 0);
  defparam( params, n_params++, "interactive_p", TBOOL, &interactive_p, 0);
  defparam( params, n_params++, "break_on_warnings_p", TBOOL, &break_on_warnings_p, 0);
  defparam( params, n_params++, "free_storage_p", TBOOL, &free_storage_p, 0);

  /* -------------------------------------------------- */

  /* read search params file */
  fprintf(stdout, "\n\n\n### Starting Check of %s%s\n",
          (search_params_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
          search_params_file_ptr);
  search_params_file_fp = fopen(search_params_file_ptr, "r");   
  s_parms_error_cnt = getparams(search_params_file_fp, params) +
    validate_search_start_fn( start_fn_type) + validate_search_try_fn( try_fn_type) +
      validate_n_classes_fn( n_classes_fn_type);
  fclose(search_params_file_fp);
  for (i=0; start_j_list[i] != END_OF_INT_LIST; i++);
  n_start_j_list = i;
  for (i=0; i<n_params; i++) 
    if (eqstring( params[i].paramname, "start_j_list")) {
      if (params[i].overridden_p == TRUE)
        start_j_list_from_s_params = TRUE;
      break;
    }
  if ((eqstring( reconverge_type, "") != TRUE) &&
      (eqstring( reconverge_type, "chkpt") != TRUE) &&
      (eqstring( reconverge_type, "results") != TRUE)) {
    fprintf( stderr, "ERROR: reconverge_type must be  either \"chkpt\", or "
            "\"results\".\n");
    s_parms_error_cnt++;
  }    
  if (randomize_random_p == TRUE)
    srand48( (long) (get_universal_time()));
  G_save_compact_p = save_compact_p;
  max_n_store = max( n_save, max_n_store);
  G_interactive_p = interactive_p;
  G_break_on_warnings = break_on_warnings_p;
  if (interactive_p == FALSE)
    fprintf( stdout, "\nADVISORY: interactive_p = false, the quit character q/Q will "
            "not be recognized.\n          I hope you have specified max_n_tries or "
            "max_duration!\n\n");
  if (screen_output_p == FALSE) {
    stream = G_stream = NULL;
    if (interactive_p == TRUE) 
      fprintf( stdout, "\nADVISORY: screen_output_p = false, no more screen output for "
              "this search.\n          I hope you have specified max_n_tries or "
              "max_duration !\n");
  }
  else
    fprintf( stream, "### Ending Check of   %s%s\n",
            (search_params_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
            search_params_file_ptr);
  if (s_parms_error_cnt > 0)
    exit(1);

  /* aju 980612: Added conditionality on interactive_p */
  /* 01jul98 wmt: removed conditionality -- important that user is warned */
  /* 22apr00 wmt: bring back conditionality, to prevent batch runs from
     going into a loop */
  if ((randomize_random_p == FALSE) || (eqstring( start_fn_type, "block") == TRUE)) {
    if (interactive_p == TRUE) {
      fprintf( stderr,
               "\nWARNING: either start_fn_type = \"block\", or randomize_random_p\n"
               "         = false, or both.  These parameter settings are for testing\n"
               "         *only* -- they should not be utilized for normal AutoClass\n"
               "         runs.\n\n");
      sprintf( str, "Do you want to continue {y/n} ");
      if (y_or_n_p( str))
        fprintf( stderr, "Test run continues ...\n\n");
      else
        exit(1);
    } else {
      fprintf( stderr,
               "\nERROR: either start_fn_type = \"block\", or randomize_random_p\n"
               "       = false, or both.  These parameter settings are for testing\n"
               "       *only* -- they should not be utilized for normal AutoClass\n"
               "       runs.\n\n");
      exit(1);
    }
  }
  /* end of search params processing */

  /* data file is not opened because it can be either binary or ascii -
     G_data_file_format */
  if (eqstring( header_file_ptr, "") != TRUE)
    header_file_fp = fopen( header_file_ptr, "r");
  if (eqstring( model_file_ptr, "") != TRUE)
    model_file_fp = fopen( model_file_ptr, "r");
  if (log_file_p == TRUE) {
    log_file_fp = fopen( log_file_ptr, "a");
    G_log_file_fp = log_file_fp;
  }

  safe_sprintf( str, sizeof( str), caller,
               "\n\nAUTOCLASS C (version %s) STARTING at %s \n\n", G_ac_version,
               format_universal_time(get_universal_time()));
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  if (log_file_p == TRUE) {
    fprintf(log_file_fp, "AUTOCLASS -SEARCH default parameters:\n");
    putparams(log_file_fp, params, FALSE);
  }
  if (log_file_p == TRUE) {
    fprintf(log_file_fp, "USER supplied parameters which override the defaults:\n");
    putparams(log_file_fp, params, TRUE);
  }

  if ((eqstring( reconverge_type, "chkpt") == TRUE) && (checkpoint_p == FALSE)) {
    fprintf( stderr, "ERROR: if reconverge_type is \"chkpt\", checkpoint_p must "
            "be true, as well\n");
    exit(1);
  }
  if ((eqstring( reconverge_type, "results") == TRUE) && (checkpoint_p == TRUE)) {
    fprintf( stderr, "ERROR: if reconverge_type is \"results\", checkpoint_p must "
            "be false, as well\n");
    exit(1);
  }
  if (((eqstring( reconverge_type, "results") == TRUE) ||
       (eqstring( reconverge_type, "chkpt") == TRUE)) &&
      ( force_new_search_p == TRUE)) {
    fprintf( stderr, "ERROR: if reconverge_type is \"results\" or \"chkpt\", "
             "force_new_search_p must be false, as well\n");
    exit(1);
  }
  /* set value of G_checkpoint_file */
  if (checkpoint_p == TRUE) {
    G_checkpoint_file[0] = '\0';
    G_min_checkpoint_period = min_checkpoint_period;
    make_and_validate_pathname( (save_compact_p) ? "checkpoint_bin" : "checkpoint",
                               search_params_file_ptr, &G_checkpoint_file, FALSE);
  }
  if (force_new_search_p == TRUE) {
    compact_p = save_compact_p;
    silent_p = TRUE;
  } else {
    compact_p = read_compact_p;
    silent_p = FALSE;
  }
  /* set proper extension of results_file_ptr for validate_results_pathname */
  results_file_ptr[0] = results_file[0] = '\0';
  make_and_validate_pathname( (compact_p) ? "results_bin" : "results",
                               search_params_file_ptr, &results_file, FALSE);
  strcpy( results_file_ptr, results_file);
  results_file[0] = '\0';               /* needed for validate_results_pathname */
  results_file_found = validate_results_pathname( results_file_ptr, &results_file,
                                                  "results", FALSE, silent_p);
  /* in case validate_results_pathname finds a different file than that input */
  results_file_ptr[0] = '\0';
  strcpy( results_file_ptr, results_file);

  /* aju 980612: Added conditionality on interactive_p */
  /* 01jul98 wmt: removed conditionality -- important that user is warned */
  /* 22apr00 wmt: bring back conditionality, to prevent batch runs from
     going into a loop */
  if ((force_new_search_p == TRUE) && ( results_file_found == TRUE)) {
    if (interactive_p == TRUE) {
      fprintf( stderr, "\nWARNING: force_new_search_p is true and continuing will"
               " discard the \n         search trials in:\n         %s%s\n",
               G_absolute_pathname, results_file_ptr);
      sprintf( str, "Do you want to continue {y/n} ");
      if (! y_or_n_p( str))
        exit(1);
    } else {
      fprintf( stderr, "ERROR: force_new_search_p is true and continuing will"
               " discard the \n       search trials in:\n       %s%s\n",
               G_absolute_pathname, results_file_ptr);
      exit(1);
    }
  }    
  if ((force_new_search_p == FALSE) && ( results_file_found == FALSE)) {
    fprintf( stderr, "\nERROR: if force_new_search_p is false, there must "
             "be a <...>.results%s file\n", (read_compact_p) ? "-bin" : "");
    exit(1);
  } 
  if (checkpoint_p == TRUE) 
    to_screen_and_log_file("\nWARNING: \"autoclass -search\" running in checkpointing "
                           "mode\n\n",
                           log_file_fp, stream, TRUE);
  /* for testing
  fprintf( stderr, "run continues  ...\n"); 
  exit(1);
  */

  if (force_new_search_p == FALSE) {
    search_file_fp = fopen( search_file_ptr, "r");
    restart_search = reconstruct_search( search_file_fp, search_file_ptr, results_file_ptr);
    fclose( search_file_fp);

    if (restart_search != NULL) {
      search = restart_search;
      /* if start_j_list has not been specified in the .s-params file
         use it's state from the .search file */
      if (start_j_list_from_s_params == TRUE) {
        to_screen_and_log_file( "ADVISORY: start_j_list=(", log_file_fp, stream, TRUE);
        output_int_list( search->start_j_list, log_file_fp, stream);
        to_screen_and_log_file( ") has been overridden by (", log_file_fp, stream, TRUE);
        output_int_list( start_j_list, log_file_fp, stream);
        safe_sprintf( double_str, double_str_length, caller, ")\n          from %s%s\n",
                     (search_params_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                     search_params_file_ptr);
        to_screen_and_log_file( double_str, log_file_fp, stream, TRUE);
      }
      else {
        to_screen_and_log_file( "\nADVISORY: start_j_list=(", log_file_fp, stream, TRUE);
        output_int_list( start_j_list, log_file_fp, stream);
        to_screen_and_log_file( ") has been overridden by (", log_file_fp, stream, TRUE);
        output_int_list( search->start_j_list, log_file_fp, stream);
        safe_sprintf( double_str, double_str_length, caller, ")\n          from %s%s\n\n",
                     (search_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                     search_file_ptr);
        to_screen_and_log_file( double_str, log_file_fp, stream, TRUE);    
        for (i=0; i<MAX_N_START_J_LIST; i++) {
          start_j_list[i] = search->start_j_list[i];
          if (start_j_list[i] == END_OF_INT_LIST)
            break;
        }
        n_start_j_list = i;
      }
    }
    else
      search = get_search_DS();
  }
  else
    search = get_search_DS();

  if (0 < search->n)            /* restart-p flags continuation of a search */
    restart_p = TRUE;
  else
    restart_p = FALSE;

  if (eqstring( reconverge_type, "chkpt") == TRUE) {
    make_and_validate_pathname( (read_compact_p) ? "checkpoint_bin" : "checkpoint",
                               search_params_file_ptr, &maybe_checkpoint_file, FALSE);

    validate_results_pathname( maybe_checkpoint_file, &checkpoint_file, "checkpoint",
                              TRUE, FALSE);
    clsf = copy_clsf_DS( (get_clsf_seq( checkpoint_file, expand_p, want_wts_p,
                                       update_wts_p, "checkpoint",
                                       &checkpoint_clsf_cnt, clsf_n_list))[0],
                        want_wts_p);
  }
  else if (search->tries != NULL)       /* reconverge_type = "results", as well */
    clsf = copy_clsf_DS( search->tries[0]->clsf, want_wts_p);
  else if (eqstring( data_file_ptr, "") != TRUE)
    /* or if given data-file, get clsf from that */
    clsf = generate_clsf( 1, header_file_fp, model_file_fp, log_file_fp,
                         stream, reread_p, regenerate_p, data_file_ptr, header_file_ptr,
                         model_file_ptr, log_file_ptr, restart_p, start_fn_type,
                         initial_cycles_p, n_data, start_j_list_from_s_params);
  else {
    fprintf(stderr,
            "\nERROR: Haven't been given enough info to find a classification\n");
    /*       fprintf(stderr, "autoclass_search needs one of these as input:\n"); */
    /*       fprintf(stderr, "  1) a classification\n"); */
    /*       fprintf(stderr, "  2) a data-file and header-file\n"); */
    /*       fprintf(stderr, */
    /* 	 "  3) a search-file and results-file from a previous run\n"); */
    /*       fprintf(stderr, "  4) a search object from a previous run\n"); */
    exit(1);
  }

  /* set proper extension of results_file_ptr for print functions (save files) */
  results_file_ptr[0] = results_file[0] = '\0';
  make_and_validate_pathname( (save_compact_p) ? "results_bin" : "results",
                             search_params_file_ptr, &results_file, FALSE);
  strcpy( results_file_ptr, results_file);

  expand_clsf( clsf, want_wts_p, update_wts_p); /* just in case was compressed */

  if ((max_n_tries != 0) && (restart_p == TRUE))
    max_n_tries = search->n + max_n_tries;
  max_j = clsf_DS_max_n_classes(clsf); /* n-classes must be < limit */
  if (too_big( max_j, start_j_list, n_start_j_list) == TRUE) {
    new_start_j_list = remove_too_big(max_j, start_j_list, &n_start_j_list);
    for (i=0; i<n_start_j_list; i++)
      start_j_list[i] = new_start_j_list[i];
    start_j_list[i] = max_j;    /* add max_j to list */
    start_j_list[i+1] = END_OF_INT_LIST;
    n_start_j_list++;
    free(new_start_j_list);
    to_screen_and_log_file( "\nADVISORY: start_j_list has been modified to: (",
                           log_file_fp, stream, TRUE);
    output_int_list( start_j_list, log_file_fp, stream);
    to_screen_and_log_file(")\n", log_file_fp, stream, TRUE);  
  }
  if ((n_start_j_list == 0) && (search->tries == NULL) && (fixed_j == 0)) {
    to_screen_and_log_file
      ("ERROR: A new search must have at least one item in start_j_list\n",
       log_file_fp, stream, TRUE);
    exit(1);
  }
  if (eqstring( header_file_ptr, "") != TRUE)
    fclose( header_file_fp);
  if (eqstring( model_file_ptr, "") != TRUE)
    fclose( model_file_fp);

  begin = get_universal_time();         /* start the clock */
  now = begin;
  last_search_save = begin - 1;        /* must not be equal to now */
  last_results_save = begin;
  last_report = begin;
  if (max_duration != 0)
    end_time = begin + (time_t) max_duration;
  /* tell the user about what will happen */
  print_initial_report( stream, log_file_fp, min_report_period, end_time, max_n_tries,
                       search_file_ptr, results_file_ptr, log_file_ptr, min_save_period,
                       n_save);

  /* 	printf("\n###commented calls to print_model and print_clsf\n"); dbg JTP*/
  /*** print_model_DS(clsf->models[0]," first model of clsf"); dbg JTP*/
  /***	print_clsf_DS(clsf," after init"); dbg JTP*/

  if (restart_p == TRUE)
    safe_sprintf( str, sizeof( str), caller, "\nRESTARTING SEARCH at %s\n\n",
                 format_universal_time(begin));
  else
    safe_sprintf( str, sizeof( str), caller, "\nBEGINNING SEARCH at %s\n\n",
                 format_universal_time(begin));
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  if (log_file_p == TRUE)
    fclose( log_file_fp);

  /*commentedprintf(" \nquitting before BIG LOOOP\n"); exit(1);  dbg JTP*/
  /* top of THE BIG LOOP */
  while (TRUE) {
    search->n ++;
    begin_try = get_universal_time();
    if (eqstring( reconverge_type, "chkpt") == TRUE) {
      pop_int_list( start_j_list, &n_start_j_list, &j_in);
      j_in = clsf->checkpoint->current_try_j_in;
    }
    else if (eqstring( reconverge_type, "results") == TRUE)
     j_in = clsf->n_classes;
    else if (fixed_j != 0)
      j_in = fixed_j;
    else if (pop_int_list( start_j_list, &n_start_j_list, &j_in) > 0)
      ;
    else 
      j_in = (int) within( 1.0,
                          (double) apply_n_classes_fn( n_classes_fn_type, search->n_tries,
                                                      search->tries, max_j, FALSE,
                                                      n_classes_explain),
                          (double) max_j);

    /* DO A NEW TRIAL */
    if (log_file_p == TRUE) {
      log_file_fp = fopen( log_file_ptr, "a"); 
      G_log_file_fp = log_file_fp;
    }
    latest_try = try_variation(clsf, j_in, search->n, reconverge_type, start_fn_type,
                               try_fn_type, initial_cycles_p, begin_try,
                               (double) halt_range, (double) halt_factor,
                               (double) rel_delta_range, max_cycles,
                               n_average, (double) cs4_delta_range,
                               sigma_beta_n_values, converge_print_p, log_file_fp,
                               stream);

    if (latest_try->j_out != 0) { /* complete trial */
      n_stored_clsf = min(max_n_store, (search->n - search->n_dups));
      dup_p = find_duplicate(latest_try, search->tries, n_stored_clsf, &n_dup_tries,
                             (double) rel_error, search->n_tries, restart_p);
      search->n_dup_tries += n_dup_tries;
      if (dup_p == TRUE)        /* update counts of duplicates */
        search->n_dups += 1;
      /* unless latest try is a duplicate, add latest try to list of tries */
      else {
        search->n_tries ++;     /* assuming store tries forever but only store
                                   clsf on tries up to n_store_clsf*/
        search->tries = insert_new_trial(latest_try, search->tries,
                                         search->n_tries, n_stored_clsf, max_n_store);
        if (G_clsf_storage_log_p == TRUE)
          describe_search( search);
      }

      best = search->tries[0];
      now = get_universal_time();
      /* search->time += (int) latest_try->time; 12oct94 wmt: */
      /* after each try, print a short report describing n-classes tried */
      if (latest_try == best)
        sprintf(str, " %s%d->%d(%d) ", "best", latest_try->j_in, latest_try->j_out, search->n);
      else if (dup_p == TRUE)
        sprintf(str, " %s%d->%d(%d) ", "dup", latest_try->j_in, latest_try->j_out, search->n);
      else
        sprintf(str, " %d->%d(%d) ", latest_try->j_in, latest_try->j_out, search->n);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);

      /* fprintf( stderr, "\nautoclass_search: G_num_cycles %d max_cycles %d\n",
         G_num_cycles, max_cycles); */
      if (G_num_cycles >= max_cycles) {
        safe_sprintf( str, sizeof( str ), caller,
                      "\nWARNING: trial %d terminated prior to convergence"
                      " since number of \n"
                      "         cycles reached \"max_cycles\" (%d).\n",
                      latest_try->n, max_cycles);
        to_screen_and_log_file( str, G_log_file_fp, G_stream, TRUE);
      }
      /* when long enough since last long report and have found a new best clsf */
      if ((search->n_tries > 2) && (best != NULL) &&
          (now > ((time_t) min_report_period + last_report)) &&
          ((search->last_try_reported == NULL) ||
           (best->ln_p > search->last_try_reported->ln_p))) {
        if (fixed_j != 0) 
          sprintf(n_classes_explain, "as fixed at %d", fixed_j);
        else if (n_start_j_list > 0) {
          sprintf(n_classes_explain, "off of list: (");
          for (i=0; i < n_start_j_list; i++) {
            sprintf(temp_str, " %d", start_j_list[i]);
            strcat(n_classes_explain, temp_str);
          }
          strcat(n_classes_explain, " )");
        } 
        else 
          apply_n_classes_fn( n_classes_fn_type, search->n_tries, search->tries,
                             max_j, TRUE, n_classes_explain);
        print_report(stream, log_file_fp, search, last_search_save, last_report,
                     eqstring( reconverge_type, "chkpt"), n_classes_explain);

        last_report = now;
        search->last_try_reported = best;
      }

      if (n_save >0) {
        ss = safe_subseq_of_tries(search->tries, 0, n_save, 
                                  search->n_tries, &bclength);
        while (bclength > 1 && ss[bclength - 1]->clsf == NULL)
          bclength--;
        if (best_clsfs != NULL)
          free(best_clsfs);
        best_clsfs = (clsf_DS *) malloc( bclength * sizeof(clsf_DS));

        for (i=0; i<bclength; i++)
          best_clsfs[i] = ss[i]->clsf;
        free(ss);
        if (bclength == last_bclength)
          for (i=0;i<last_bclength;i++)
            if (best_clsfs[i] != last_saved_clsfs[i])
              last_bclength=-1;
        /* if enough time has passed and they are different from before */
        /* save them in a results file */
        if (now > ((time_t) min_save_period + max( last_results_save,
                                                  last_search_save))) {
          if ((results_file_p == TRUE) && (last_bclength != bclength)) {

            save_clsf_seq( best_clsfs, bclength, results_file_ptr, save_compact_p,
                          "results");
            safe_sprintf( str, sizeof( str), caller, " [saved %s/%s at %s]\n",
                         (save_compact_p) ? RESULTS_BINARY_FILE_TYPE : RESULTS_FILE_TYPE,
                         SEARCH_PARAMS_FILE_TYPE, format_universal_time (now));
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            if (last_saved_clsfs != NULL) 
              free(last_saved_clsfs);
            last_results_save = now;
            last_saved_clsfs = (clsf_DS *) malloc( bclength * sizeof(clsf_DS));

            for (i=0; i<bclength; i++)
              last_saved_clsfs[i] = best_clsfs[i];
            last_bclength = bclength;
          }
          /* and save a search-file, if not nil (keep .search up to date even if
             better results have not been found) */
          if (search_file_p == TRUE) {
           save_search( search, search_file_ptr, last_search_save, clsf,
                        eqstring( reconverge_type, "chkpt"), start_j_list,
                        n_final_summary, n_save);
            last_search_save = get_universal_time();
            /* search->time has been updated */
            if (now != last_results_save) {
              safe_sprintf( str, sizeof( str), caller, " [saved .search at %s]\n",
                           format_universal_time (now));
              to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            }
          }
        }
      }
      /* reset reconverge_type, so that subsequent trials will use the 
         start_j_list */
      strcpy( reconverge_type, "");
    } /* end of complete trial */
    else
      search->n --;

    if (char_input_test())      /* stop if the user asks */
      strcpy(stop_reason, "you asked me to");
    else if ((end_time != 0) && (now > end_time)) /* or time up, */
      strcpy(stop_reason, "max duration has expired");
    else if ((max_n_tries != 0) && (latest_try->n == max_n_tries)) /* or # tries up */
      strcpy(stop_reason, "max number of tries reached");
    else
      strcpy(stop_reason, "");
    if (eqstring(stop_reason, "") == FALSE) /* quit the try loop */
      break;

    strcpy( reconverge_type, "");
    if (log_file_p == TRUE) 
      fclose( log_file_fp);
  }             /* end of big loop */

  print_final_report( stream, log_file_fp, search, begin, last_search_save, n_save,
                     stop_reason, results_file_p, search_file_p, n_final_summary,
                     log_file_ptr, search_params_file_ptr, results_file_ptr, clsf,
                     eqstring( reconverge_type, "chkpt"), last_report, now); 

  if ((now != last_search_save) &&     /* have more trials occurred since last save */
      (best_clsfs != NULL)) {
    /* save a results file, if results changed */
    if (((last_saved_clsfs == NULL) || (last_saved_clsfs != best_clsfs)) &&
        (n_save != 0) && (results_file_p == TRUE)) 
      save_clsf_seq( best_clsfs, bclength, results_file_ptr, save_compact_p, "results");
      
    /* save a search-file, if supposed to */
    if ((n_save != 0) && (search_file_p == TRUE))
      save_search( search, search_file_ptr, last_search_save, clsf,
                  eqstring( reconverge_type, "chkpt"), start_j_list,
                  n_final_summary, n_save);
  }
  if (free_storage_p == TRUE) {
    /* database storage and model storage, including global-clsf, are not freed */
    free_clsf_class_search_storage( clsf, search, list_global_clsf_p);
  }
  safe_sprintf( str, sizeof( str), caller,
               "\nAUTOCLASS C (version %s) STOPPING at %s \n", G_ac_version,
               format_universal_time(get_universal_time()));
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  if (log_file_p == TRUE)
    fclose(log_file_fp);
  return(0);
}
 
