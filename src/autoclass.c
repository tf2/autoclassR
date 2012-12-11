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

/* SUPRESS CODECENTER WARNING MESSSAGES */
      
/* empty body for 'while' statement */
/*SUPPRESS 570*/
/* formal parameter '<---->' was not used */
/*SUPPRESS 761*/
/* automatic variable '<---->' was not used */
/*SUPPRESS 762*/
/* automatic variable '<---->' was set but not used */
/*SUPPRESS 765*/


/* MAIN
   06oct94 wmt: initial revisions
   24jan95 wmt: revised arguments
   18may95 wmt: added "-predict" mode
   02dec98 wmt: initialize log file for "-predict" mode

   main entry for module autoclass search, reports, and prediction
   # show args
        % autoclass
   # autoclass search
        % autoclass -search <..>.db2[-bin] <..>.hd2 <..>.model <..>.s-params 
   # autoclass reports
        % autoclass -reports <..>.results[-bin] <..>.search <..>.r-params 
   # autoclass prediction
        % autoclass -predict <test..>.db2 <training..>.results[-bin] 
                <training..>.search <training..>.r-params
   */
int main( int argc, char *argv[])
{
  char *data_file_arg_ptr, *header_file_arg_ptr, *model_file_arg_ptr,
        *search_params_file_arg_ptr, *reports_params_file_arg_ptr, *ac_option,
        *results_file_arg_ptr, *search_file_arg_ptr;
  static fxlstr data_file, header_file, model_file, search_params_file,
        search_file, results_file, log_file, reports_params_file;
  static fxlstr influ_vals_file, xref_class_file, xref_case_file,
         test_data_file;
  int valid_file_p = TRUE, num_search_args = 6, num_reports_args = 5;
  int exit_if_error_p = FALSE, silent_p = FALSE, num_predict_args = 6;

  data_file[0] = header_file[0] = model_file[0] = search_params_file[0] = '\0';
  search_file[0] = results_file[0] = log_file[0] = '\0';
  reports_params_file[0] = influ_vals_file[0] = xref_class_file[0] = '\0';
  xref_case_file[0] = test_data_file[0] = '\0';

  init();  

  switch (argc) {

  case 1 :
    fprintf( stdout, "\n\nAUTOCLASS C (version %s)\n", G_ac_version);
    autoclass_args();    /* show arg options  */
    exit(0);
    break;

  case 5 :  
  case 6 :     
    ac_option = argv[1];
    if ((eqstring( ac_option, "-search") != TRUE) &&
        (eqstring( ac_option, "-reports") != TRUE) &&
        (eqstring( ac_option, "-predict") != TRUE)) {
      fprintf( stderr, "ERROR: the second argument must be \"-search\","
              " \"-reports\", or \"-predict\"\n");
      exit(1);
    }
    break;

  default : 
    fprintf (stderr,
             "\nERROR: invalid number of arguments for \"autoclass\" \n");
             //fprintf (stderr,argc);
    autoclass_args();
    exit(1);
    break;
  }

  if (eqstring( ac_option, "-search") == TRUE) {
    if (argc != num_search_args) {
      fprintf (stderr,
               "\nERROR: invalid number of arguments for \"autoclass -search\" \n");
      autoclass_args();
      exit(1);
    }
    data_file_arg_ptr = argv[2];
    header_file_arg_ptr = argv[3];
    model_file_arg_ptr = argv[4];
    search_params_file_arg_ptr = argv[5];
    if (validate_data_pathname( data_file_arg_ptr, &data_file, exit_if_error_p,
                               silent_p) != TRUE) 
      valid_file_p = FALSE;
    if (make_and_validate_pathname("header", header_file_arg_ptr, &header_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (make_and_validate_pathname("model", model_file_arg_ptr, &model_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (make_and_validate_pathname("search params", search_params_file_arg_ptr,
                                   &search_params_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (valid_file_p == FALSE) {
      autoclass_args();
      exit(1);
    }
    /*     make .log, .search & .results pathnames, using search_params_file_arg_ptr for path */
    make_and_validate_pathname( "log", search_params_file_arg_ptr, &log_file, FALSE);
    make_and_validate_pathname( "search", search_params_file_arg_ptr, &search_file, FALSE);
    make_and_validate_pathname( "results", search_params_file_arg_ptr, &results_file, FALSE);

    autoclass_search( data_file, header_file, model_file, search_params_file,
                     search_file, results_file, log_file);
  } else if (eqstring( ac_option, "-reports") == TRUE) { 
    if (argc != num_reports_args) {
      fprintf (stderr,
               "\nERROR: invalid number of arguments for \"autoclass -reports\" \n");
      autoclass_args();
      exit(1);
    }
    results_file_arg_ptr = argv[2];
    search_file_arg_ptr = argv[3];
    reports_params_file_arg_ptr = argv[4];
    if (validate_results_pathname( results_file_arg_ptr, &results_file, "results",
                                  exit_if_error_p, silent_p)
        != TRUE) 
      valid_file_p = FALSE;
    if (make_and_validate_pathname( "search", search_file_arg_ptr, &search_file, TRUE)
        != TRUE)
      valid_file_p = FALSE;
    if (make_and_validate_pathname( "reports params", reports_params_file_arg_ptr,
                                   &reports_params_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (valid_file_p == FALSE) {
      autoclass_args();
      exit(1);
    }
    /* make .influ-textn, .class-text-n & .case-text-n pathnames, using
       reports_params_file_arg_ptr for path */
    make_and_validate_pathname("influ_vals", reports_params_file_arg_ptr, &influ_vals_file,
                               FALSE);
    make_and_validate_pathname("xref_class", reports_params_file_arg_ptr, &xref_class_file,
                               FALSE);
    make_and_validate_pathname("xref_case", reports_params_file_arg_ptr, &xref_case_file,
                               FALSE);
    make_and_validate_pathname("rlog", reports_params_file_arg_ptr, &log_file, FALSE);

    autoclass_reports( results_file, search_file, reports_params_file, influ_vals_file,
                      xref_class_file, xref_case_file, test_data_file, log_file);

  } else {                        /* -predict */
    if (argc != num_predict_args) {
      fprintf (stderr,
               "\nERROR: invalid number of arguments for \"autoclass -predict\" \n");
      autoclass_args();
      exit(1);
    }
    data_file_arg_ptr = argv[2];
    results_file_arg_ptr = argv[3];
    search_file_arg_ptr = argv[4];
    reports_params_file_arg_ptr = argv[5];
    if (validate_data_pathname( data_file_arg_ptr, &test_data_file, exit_if_error_p,
                               silent_p) != TRUE) 
      valid_file_p = FALSE;
    if (validate_results_pathname( results_file_arg_ptr, &results_file, "results",
                                  exit_if_error_p, silent_p) != TRUE) 
      valid_file_p = FALSE;
    if (make_and_validate_pathname( "search", search_file_arg_ptr, &search_file, TRUE)
        != TRUE)
      valid_file_p = FALSE;
    if (make_and_validate_pathname( "reports params", reports_params_file_arg_ptr,
                                   &reports_params_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (valid_file_p == FALSE) {
      autoclass_args();
      exit(1);
    }
    /* make .class-text-n & .case-text-n pathnames, using data_file_arg_ptr
       for path */
    make_and_validate_pathname("xref_class", data_file_arg_ptr, &xref_class_file,
                               FALSE);
    make_and_validate_pathname("xref_case", data_file_arg_ptr, &xref_case_file,
                               FALSE);
    make_and_validate_pathname("log", reports_params_file_arg_ptr, &log_file, FALSE);

    autoclass_reports( results_file, search_file, reports_params_file, influ_vals_file,
                      xref_class_file, xref_case_file, test_data_file, log_file);
  }
  return(0);
}


int autoSearch(char **f1, char **f2, char **f3, char **f4)
{
  char *data_file_arg_ptr, *header_file_arg_ptr, *model_file_arg_ptr,
        *search_params_file_arg_ptr, *reports_params_file_arg_ptr, *ac_option,
        *results_file_arg_ptr, *search_file_arg_ptr;
  static fxlstr data_file, header_file, model_file, search_params_file,
        search_file, results_file, log_file, reports_params_file;
  static fxlstr influ_vals_file, xref_class_file, xref_case_file,
         test_data_file;
  int valid_file_p = TRUE, num_search_args = 6, num_reports_args = 5;
  int exit_if_error_p = FALSE, silent_p = FALSE, num_predict_args = 6;

  data_file[0] = header_file[0] = model_file[0] = search_params_file[0] = '\0';
  search_file[0] = results_file[0] = log_file[0] = '\0';
  reports_params_file[0] = influ_vals_file[0] = xref_class_file[0] = '\0';
  xref_case_file[0] = test_data_file[0] = '\0';

  init();  

  data_file_arg_ptr = *f1;
  header_file_arg_ptr = *f2;
  model_file_arg_ptr = *f3; 
  search_params_file_arg_ptr = *f4;

    if (validate_data_pathname( data_file_arg_ptr, &data_file, exit_if_error_p,
                               silent_p) != TRUE) 
      valid_file_p = FALSE;
    if (make_and_validate_pathname("header", header_file_arg_ptr, &header_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (make_and_validate_pathname("model", model_file_arg_ptr, &model_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (make_and_validate_pathname("search params", search_params_file_arg_ptr,
                                   &search_params_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (valid_file_p == FALSE) {
      autoclass_args();
      exit(1);
    }
    ///    make .log, .search & .results pathnames, using search_params_file_arg_ptr for path /
    make_and_validate_pathname( "log", search_params_file_arg_ptr, &log_file, FALSE);
    make_and_validate_pathname( "search", search_params_file_arg_ptr, &search_file, FALSE);
    make_and_validate_pathname( "results", search_params_file_arg_ptr, &results_file, FALSE);

    autoclass_search( data_file, header_file, model_file, search_params_file,
                     search_file, results_file, log_file);
  
  return(0);
}


int autoReport(char **f11, char **f22, char **f33)
{
  char *data_file_arg_ptr, *header_file_arg_ptr, *model_file_arg_ptr,
        *search_params_file_arg_ptr, *reports_params_file_arg_ptr, *ac_option,
        *results_file_arg_ptr, *search_file_arg_ptr;
  static fxlstr data_file, header_file, model_file, search_params_file,
        search_file, results_file, log_file, reports_params_file;
  static fxlstr influ_vals_file, xref_class_file, xref_case_file,
         test_data_file;
  int valid_file_p = TRUE, num_search_args = 6, num_reports_args = 5;
  int exit_if_error_p = FALSE, silent_p = FALSE, num_predict_args = 6;

	fprintf(stderr, "huehcuehcueh");
	
  data_file[0] = header_file[0] = model_file[0] = search_params_file[0] = '\0';
  search_file[0] = results_file[0] = log_file[0] = '\0';
  reports_params_file[0] = influ_vals_file[0] = xref_class_file[0] = '\0';
  xref_case_file[0] = test_data_file[0] = '\0';

  init();  

    results_file_arg_ptr = *f11;
    search_file_arg_ptr = *f22;
    reports_params_file_arg_ptr = *f33;
	
    if (validate_results_pathname( results_file_arg_ptr, &results_file, "results",
                                  exit_if_error_p, silent_p)
        != TRUE) 
      valid_file_p = FALSE;
    if (make_and_validate_pathname( "search", search_file_arg_ptr, &search_file, TRUE)
        != TRUE)
      valid_file_p = FALSE;
    if (make_and_validate_pathname( "reports params", reports_params_file_arg_ptr,
                                   &reports_params_file, TRUE) != TRUE)
      valid_file_p = FALSE;
    if (valid_file_p == FALSE) {
      autoclass_args();
      exit(1);
    }
    /* make .influ-textn, .class-text-n & .case-text-n pathnames, using
       reports_params_file_arg_ptr for path */
    make_and_validate_pathname("influ_vals", reports_params_file_arg_ptr, &influ_vals_file,
                               FALSE);
    make_and_validate_pathname("xref_class", reports_params_file_arg_ptr, &xref_class_file,
                               FALSE);
    make_and_validate_pathname("xref_case", reports_params_file_arg_ptr, &xref_case_file,
                               FALSE);
    make_and_validate_pathname("rlog", reports_params_file_arg_ptr, &log_file, FALSE);
	
    autoclass_reports( results_file, search_file, reports_params_file, influ_vals_file,
                      xref_class_file, xref_case_file, test_data_file, log_file);
	
  return(0);
}

int autoPredict(char **f111, char **f222, char **f333, char **f444)
{
	char *data_file_arg_ptr, *header_file_arg_ptr, *model_file_arg_ptr,
	*search_params_file_arg_ptr, *reports_params_file_arg_ptr, *ac_option,
	*results_file_arg_ptr, *search_file_arg_ptr;
	static fxlstr data_file, header_file, model_file, search_params_file,
	search_file, results_file, log_file, reports_params_file;
	static fxlstr influ_vals_file, xref_class_file, xref_case_file,
	test_data_file;
	int valid_file_p = TRUE, num_search_args = 6, num_reports_args = 5;
	int exit_if_error_p = FALSE, silent_p = FALSE, num_predict_args = 6;
	
	fprintf(stderr, "huehcuehcueh");
	
	data_file[0] = header_file[0] = model_file[0] = search_params_file[0] = '\0';
	search_file[0] = results_file[0] = log_file[0] = '\0';
	reports_params_file[0] = influ_vals_file[0] = xref_class_file[0] = '\0';
	xref_case_file[0] = test_data_file[0] = '\0';
	
	init();  
	
	data_file_arg_ptr = *f111;
	results_file_arg_ptr = *f222;
    results_file_arg_ptr = *f222;
    search_file_arg_ptr = *f333;
    reports_params_file_arg_ptr = *f444;
	
    if (validate_results_pathname( results_file_arg_ptr, &results_file, "results",
                                  exit_if_error_p, silent_p)
        != TRUE) 
		valid_file_p = FALSE;
    if (make_and_validate_pathname( "search", search_file_arg_ptr, &search_file, TRUE)
        != TRUE)
		valid_file_p = FALSE;
    if (make_and_validate_pathname( "reports params", reports_params_file_arg_ptr,
                                   &reports_params_file, TRUE) != TRUE)
		valid_file_p = FALSE;
    if (valid_file_p == FALSE) {
		autoclass_args();
		exit(1);
    }
    /* make .influ-textn, .class-text-n & .case-text-n pathnames, using
	 reports_params_file_arg_ptr for path */
    make_and_validate_pathname("influ_vals", reports_params_file_arg_ptr, &influ_vals_file,
                               FALSE);
    make_and_validate_pathname("xref_class", reports_params_file_arg_ptr, &xref_class_file,
                               FALSE);
    make_and_validate_pathname("xref_case", reports_params_file_arg_ptr, &xref_case_file,
                               FALSE);
    make_and_validate_pathname("rlog", reports_params_file_arg_ptr, &log_file, FALSE);
	
    autoclass_reports( results_file, search_file, reports_params_file, influ_vals_file,
                      xref_class_file, xref_case_file, test_data_file, log_file);
		
	return(0);
}




/* AUTOCLASS_ARGS
   10oct94 wmt: new
   24jan95 wmt: name changed from autoclass_search_args
   18may95 wmt: added "-predict" mode

   output argument options to autoclass
   */
void autoclass_args ()
{
#ifdef _WIN32
  char operate[] = "> Autoclass.exe";
#else
  char operate[] = "> autoclass";
#endif
  fprintf (stderr,
           "\n AutoClass Search: "
           "\n      %s -search <.db2[-bin] file path> <.hd2 file path>"
           "\n             <.model file path> <.s-params file path> \n",
           operate);
  fprintf (stderr,
           "\n AutoClass Reports: "
           "\n      %s -reports <.results[-bin] file path> "
                "<.search file path> "
           "\n             <.r-params file path> \n", operate);
  fprintf (stderr,
           "\n AutoClass Prediction: "
           "\n      %s -predict <test.. .db2 file path>"
           "\n             <training.. .results[-bin] file path>"
           "\n             <training.. .search file path> "
           "<training.. .r-params file path> \n\n", operate);
}
