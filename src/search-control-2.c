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


/*
static float above_cut_table[SIZEOF_ABOVE_CUT_TABLE][2] =
   {{0.0, 0.5}, {0.01, 0.496}, {0.1, 0.4602}, {0.2, 0.4207}, {0.3, 0.3821},
    {0.4, 0.3446}, {0.5, 0.3085}, {0.6, 0.2742}, {0.7, 0.242}, {0.8, 0.2119},
    {0.9, 0.1841}, {1.0, 0.1587}, {1.1, 0.1357}, {1.2, 0.1151}, {1.3, 0.0968},
    {1.4, 0.0808}, {1.5, 0.0668}, {1.6, 0.0548}, {1.7, 0.0446}, {1.8, 0.0359},
    {1.9, 0.0287}, {2.0, 0.0228}, {2.1, 0.0179}, {2.2, 0.0139}, {2.3, 0.0107},
    {2.4, 0.0082}, {2.5, 0.0062}, {2.6, 0.0047}, {2.7, 0.0035}, {2.8, 0.0026},
    {2.9, 0.0019}}; */

static float cut_where_above_table[SIZEOF_CUT_WHERE_ABOVE_TABLE][2] =
   {{0.5, 0.0}, {0.496, 0.01}, {0.4602, 0.1}, {0.4207, 0.2}, {0.3821, 0.3},
    {0.3446, 0.4}, {0.3085, 0.5}, {0.2742, 0.6}, {0.242, 0.7}, {0.2119, 0.8},
    {0.1841, 0.9}, {0.1587, 1.0}, {0.1357, 1.1}, {0.1151, 1.2}, {0.0968, 1.3},
    {0.0808, 1.4}, {0.0668, 1.5}, {0.0548, 1.6}, {0.0446, 1.7}, {0.0359, 1.8},
    {0.0287, 1.9}, {0.0228, 2.0}, {0.0179, 2.1}, {0.0139, 2.2}, {0.0107, 2.3},
    {0.0082, 2.4}, {0.0062, 2.5}, {0.0047, 2.6}, {0.0035, 2.7}, {0.0026, 2.8},
    {0.0019, 2.9}};


/* REMOVE_TOO_BIG
   02may95 wmt: discard values equal to limit, as well

   Remove items from the list that are greater then or equal to the given limit.
   */
int *remove_too_big( int limit, int  *list, int *num)
{
   int i, size = 0, *new;

   new = (int *) malloc(*num * sizeof(int));
   for (i=0; i<*num; i++)
      if (list[i] < limit) {
	 new[size] = list[i];
	 size++;
      }
   *num = size;
   return(new);
}


/* TOO_BIG
   10jan95 wmt: new

   Return TRUE if items from the list that are greater than the given limit.
   */
int too_big( int limit, int  *list, int num)
{
  int i, member_exceeds_limit = FALSE;

  for (i=0; i<num; i++)
    if (list[i] > limit) {
      member_exceeds_limit = TRUE;
      break;
    }
  return(member_exceeds_limit);
}


/* pulls x to within [min,max] */

double within( double min_val, double  x, double  max_val)
{
   return(max(min_val, min(max_val, x)));
}


/* SAFE_SUBSEQ_OF_TRIES
   23nov94 wmt: renamed variables for clarity,
        removed +1 from n_saved = n_to_save - begin +1

   won't bomb if out of range 
   orig says if (list and end element is null ) or (end>length) then 
   end=min(end,length) and then subseq from max(begin,0) to end

   returns the sub seq of seq from begin to end but not more than num
   and returns the count in *newnum
   */
search_try_DS *safe_subseq_of_tries( search_try_DS *seq,
			int begin, int n_to_save, int n_tries, int *n_saved)
{
   int i;
   search_try_DS *new_seq = NULL;

   if (begin < 0)
      begin = 0;        /* begin max(begin,0) */
   if( (*n_saved = n_to_save - begin) <= 0)
      return NULL;
   if (*n_saved > n_tries)
      *n_saved = n_tries;
   new_seq = (search_try_DS *) malloc(*n_saved * sizeof(search_try_DS));
   for (i=0; i<*n_saved; i++)
      new_seq[i] = seq[i+begin];
   return(new_seq);
}


/* PRINT_INITIAL_REPORT
   modified extensively to 26oct94 wmt: add standard capability

   tell the user about what will happen, what will be told during search
   */
void print_initial_report( FILE *stream, FILE *log_file_fp, int min_report_period,
                          time_t end_time, int max_n_tries, char *search_file_ptr,
                          char *results_file_ptr, char *log_file_ptr, int min_save_period,
                          int n_save)
{
  char caller[] = "print_initial_report", *str;
  int str_length = 2 * sizeof( fxlstr);

  str = (char *) malloc( str_length);
  sprintf(str, "\nWELCOME TO AUTOCLASS.\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "  1) Each time I have finished a new 'trial', or attempt to find a good\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     classification, I will print the number of classes that trial\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     started and ended with, such as 9->7.\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "  2) If that trial results in a duplicate of a previous run, I will print\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     'dup' first.\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "  3) If that trial results in a classification better than any previous, \n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     I will print 'best' first.\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  safe_sprintf( str, str_length, caller,
               "  4) If more than%s have passed since the last report, and a new\n",
               format_time_duration((time_t) min_report_period));
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     classification has been found which is better than any previous ones,\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     I will report on that classification and on the status of the search\n"
          "     so far.\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "  5) This report will include an estimate of the time it will take to find\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     another even better classification, and how much better that will be.\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     In addition, I will estimate a lower bound on how long it might take to\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  sprintf(str, "     find the very best classification, and how much better that might be.\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  to_screen_and_log_file( "  6) If you are warned about too much time in overhead, "
                         "you may want to\n", log_file_fp, stream, TRUE);
  to_screen_and_log_file( "     change the parameters n_save, min_save_period, "
                         "min_report_period, or\n", log_file_fp, stream, TRUE);
  to_screen_and_log_file( "     min_checkpoint_period.\n", log_file_fp, stream, TRUE);
  if (G_interactive_p == FALSE)
    sprintf( str, "  7) Since interactive_p = false, I will continue searching\n     ");
  else
    sprintf( str, "  7) To quit searching, type a 'q', hit <return>, and wait.  Otherwise I'll\n"
            "     go on ");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  if (end_time != 0) 
    safe_sprintf( str, str_length, caller, "until %s.\n", format_universal_time(end_time));
  else if (max_n_tries > 0)
    sprintf(str, "until I complete trial number (%d).\n", max_n_tries);
  else
    sprintf(str, "forever.\n");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);

  if (n_save > 0) {
    safe_sprintf( str, str_length, caller,
                 "  8) If needed, every%s I will save the best %d classifications\n",
                 format_time_duration((time_t) min_save_period), n_save);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
    safe_sprintf( str, str_length, caller,
                 "     so far to file: \n     %s%s\n",
                 (results_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                 results_file_ptr);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
    safe_sprintf( str, str_length, caller,
                 "     and a description of the search to file:\n     %s%s\n",
                 (search_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                 search_file_ptr);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  }
  if (log_file_fp != NULL) {
    safe_sprintf( str, str_length, caller,
                 "  9) A record of this search will be printed to file:\n     %s%s\n",
                 (log_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, log_file_ptr);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  }
  free( str);
}


/* PRINT_REPORT
   13oct94 wmt: modified
   26apr95 wmt: do not use NULL as value of delta_ln_p

   tell user about new best clsf, about search since last report, and about projections
   till next report */
void print_report( FILE *stream, FILE *log_file_fp, search_DS search, time_t last_save,
                  time_t last_report, int reconverge_p, char *n_classes_explain)
{
   int i, n_dups, n_dup_tries, time_so_far, n_peaks_seen;
   int n_not_reported, min_n_peak, min_best_time;
   time_t now, delta_time;
   float ln_p_avg, ln_p_sigma, ln_p, delta_ln_p, avg_best_delta_ln_p;
   float min_best_delta_ln_p, avg_better_ln_p, avg_better_time, time_overhead;
   clsf_DS clsf;
   search_try_DS try, *tries;
   fxlstr str;
   char caller[] = "print_report";

   now = get_universal_time();
   tries = search->tries;
   try = tries[0];
   clsf = try->clsf;
   upper_end_normal_fit(tries, search->n_tries, &ln_p_avg, &ln_p_sigma);
   n_dups = search->n_dups;
   n_dup_tries = search->n_dup_tries;
   time_so_far = search_duration(search, now, clsf, last_save, reconverge_p);
   ln_p = (float) try->ln_p;
   delta_time = now - last_report;
   if (search->last_try_reported != NULL)
     delta_ln_p = ln_p - (float) search->last_try_reported->ln_p;
   else
     delta_ln_p = 0.0;
   n_peaks_seen = search->n_tries;
   n_not_reported = try->n;
   for (i=0; i<n_peaks_seen; i++)
      if (search->last_try_reported == tries[i])
	 n_not_reported = i;
   n_not_reported -= 1;
   min_n_peak = min_n_peaks(n_dups, n_dup_tries);
   avg_best_delta_ln_p = min(0.0, (float) ln_avg_p( (double) ln_p_avg,
                                                   (double) ln_p_sigma)) - ln_p;
   min_best_delta_ln_p = max(0.0, (float) min_best_peak( min_n_peak, (double) ln_p_avg,
                                                        (double) ln_p_sigma) - ln_p);
   avg_better_ln_p = (float) avg_improve_delta_ln_p( n_peaks_seen, (double) ln_p_sigma);
   avg_better_time = (float) avg_time_till_improve(time_so_far, n_peaks_seen);
   min_best_time = min_time_till_best(time_so_far, min_n_peak, n_peaks_seen);
   if (total_try_time(tries, search->n_tries) == 0)
     time_overhead = 0.0;
   else
     time_overhead = 1.0 - min(1.0, ((float) total_try_time(tries, search->n_tries) /
                                     (float) time_so_far));
   sprintf(str,
           "\n\n----------------  NEW BEST CLASSIFICATION FOUND on try %d  -------------\n",
           try->n);
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   describe_clsf(clsf, stream, log_file_fp);

   if (0 < n_not_reported) {
     sprintf(str, "(Also found %d other better than last report.)\n", n_not_reported);
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   }
   sprintf(str, "\n-----------  SEARCH STATUS as of %s  -----------\n",
	   format_universal_time(get_universal_time()));
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   if (delta_ln_p == 0.0) {
     safe_sprintf(str, sizeof( str), caller, "It just took%s since beginning.\n",
                  format_time_duration(delta_time));
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   }
   else {
     safe_sprintf( str, sizeof( str), caller, "It just took%s to find a classification\n ",
                  format_time_duration(delta_time));
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
     print_log( (double) delta_ln_p, log_file_fp, stream, TRUE);
     sprintf(str, "times more probable.\n");
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   }
   safe_sprintf( str, sizeof( str), caller, "Estimate <%s to find a classification\n ",
           format_time_duration((time_t) avg_better_time));
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   print_log( (double) avg_better_ln_p, log_file_fp, stream, TRUE);
   sprintf(str, "times more probable.\n");
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   safe_sprintf( str, sizeof( str), caller,
                "Estimate >>%s to find the very best classification,\n which may be",
                format_time_duration((time_t) min_best_time));
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   print_log( (double) min_best_delta_ln_p, log_file_fp, stream, FALSE);
   sprintf(str, "to");
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   print_log( (double) avg_best_delta_ln_p, log_file_fp, stream, FALSE);
   sprintf(str, "times more probable.\n");
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   safe_sprintf( str, sizeof( str), caller,
                "Have seen %d of the estimated > %d possible classifications (based on %d\n"
                " duplicates do far).\n",
                n_peaks_seen, min_n_peak, n_dups);
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   safe_sprintf( str, sizeof( str), caller,
                "Log-Normal fit to classifications probabilities has M(ean) %.1f,\n"
                " S(igma) %.1f\n", ln_p_avg, ln_p_sigma);
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   safe_sprintf( str, sizeof( str), caller, "Choosing initial n-classes %s\n",
                n_classes_explain); 
   to_screen_and_log_file(str, log_file_fp, stream, TRUE); 
	                 
   if (time_overhead > 0.10) 
     safe_sprintf( str, sizeof( str), caller,
                  "WARNING: %.1f %% of time so far spend doing non-try overhead tasks - \n"
                  " should you save and/or report less?\n",
                  (time_overhead * 100.0));
   else
     sprintf(str, "Overhead time is %.1f %% of total search time\n", (time_overhead * 100.0));
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   if ((((float) n_peaks_seen / (float) min_n_peak) > 0.25) && 
       ((n_dups + n_peaks_seen) > 10)) { 
     safe_sprintf( str, sizeof( str), caller,
                  "WARNING: You may be running out of peaks to find.  Estimates are too \n"
                  " optimistic\n");
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   }    
   sprintf(str, "\n");
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);  
}


/* PRINT_FINAL_REPORT
   12oct94++ wmt: modified extensively
   18feb98 wmt: print list of trials whose num_cycles reached max_cycles

   sum up the best 'n_final_summary' trials of the search
   */
void print_final_report( FILE *stream, FILE *log_file_fp, search_DS search, time_t begin,
  time_t last_save, int n_save, char *stop_reason, unsigned int results_file_p,
  unsigned int search_file_p, int n_final_summary, char *log_file_ptr,
  char *search_params_file_ptr, char *results_file_ptr, clsf_DS clsf, 
  int reconverge_p, time_t last_report, time_t last_trial)
{
   int i, new_line_p = TRUE, str_length = 2 * sizeof( fxlstr);
   time_t now;
   char caller[] = "print_final_report", *str;
   search_try_DS search_try;

   str = (char *) malloc( str_length);
   now = get_universal_time();
   if ((last_report == begin) || (last_report != last_trial))
     to_screen_and_log_file("\n\n", log_file_fp, stream, TRUE);
   safe_sprintf( str, str_length, caller, "\nENDING SEARCH because %s at %s\n",
                stop_reason, format_universal_time(now));
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   safe_sprintf( str, str_length, caller, "  after a total of %d tries over%s\n",
                search->n,
                format_time_duration((time_t) search_duration( search, now, clsf, last_save,
                                                              reconverge_p)));
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   if (0 < search->time) {
     safe_sprintf( str, str_length, caller,
                  "This invocation of \"autoclass -search\" took%s\n",
                  format_time_duration(now - begin));
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   }
   safe_sprintf( str, str_length, caller, "A log of this search is in file:\n %s%s\n",
                (log_file_ptr[0] == G_slash) ? "" : G_absolute_pathname, log_file_ptr);
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   safe_sprintf( str, str_length, caller,
                "The search results are stored in file:\n %s%s\n",
                (results_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                results_file_ptr);
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   if ((search_file_p == TRUE) && (results_file_p == TRUE) && (n_save != 0)) {
     to_screen_and_log_file
       ( "This search can be restarted by having \"force_new_search_p = false\"",
        log_file_fp, stream, TRUE);
     safe_sprintf( str, str_length, caller, " in file:\n %s%s\n",
                  (search_params_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                  search_params_file_ptr);
     to_screen_and_log_file( str, log_file_fp, stream, TRUE);
     to_screen_and_log_file( " and reinvoking the \"autoclass -search ...\" form\n",
                            log_file_fp, stream, TRUE);
   }
   
   safe_sprintf( str, str_length, caller,
      "\n------------------  SUMMARY OF %d BEST RESULTS  ------------------\n",
           n_final_summary);
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   for (i=0; i<n_final_summary && i < search->n_tries; i++)
      print_search_try(stream, log_file_fp, search->tries[i], (i < n_save),
                       new_line_p, "", FALSE);

   safe_sprintf( str, str_length, caller,
      "\n------------------  SUMMARY OF TRY CONVERGENCE  ------------------\n");
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   for (i=0; i<n_final_summary && i < search->n_tries; i++) {
     search_try = search->tries[i];
     safe_sprintf( str, str_length, caller,
                   "try %4d    num_cycles %4d    max_cycles %4d", search_try->n,
                   search_try->num_cycles, search_try->max_cycles);
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
     if ((search_try->num_cycles == 0) && (search_try->max_cycles == 0)) {
       safe_sprintf( str, str_length, caller, "    ...\n");
     } else if (search_try->num_cycles == search_try->max_cycles) {
       safe_sprintf( str, str_length, caller,
                     "    **** NON-CONVERGENT *****\n");
     } else {
       safe_sprintf( str, str_length, caller,
                     "    convergent\n");
     }
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   }
   free( str);
}


/* PRINT_SEARCH_TRY
   14feb95 wmt: add new_line_p
   15jun95 wmt: add pad
   11apr97 wmt: remove ":"'s form output
   14may97 wmt: add comment_data_headers_p

   give a one line summary of the search try
   */
void print_search_try( FILE *stream, FILE *log_file_fp, search_try_DS try,
                      int saved_p, int new_line_p, char *pad,
                       unsigned int comment_data_headers_p)
{
  fxlstr str;
  char caller[] = "print_search_try";

   safe_sprintf( str, sizeof( str), caller,
                "%s%sPROBABILITY  exp(%.3f) N_CLASSES  %2d FOUND ON TRY  %3d",
                 (comment_data_headers_p == TRUE) ? "#" : "",
                pad, try->ln_p, try->j_out, try->n);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   if (0 < try->n_duplicates) {
     sprintf(str, " DUPS  %d", try->n_duplicates);
     to_screen_and_log_file(str, log_file_fp, stream, TRUE);
   }
  sprintf(str, "%s%s",(saved_p) ? " *SAVED*":"", (new_line_p) ? "\n" : "");
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
}


/* EMPTY_SEARCH_TRY

   take out the classification and store it */

void empty_search_try( search_try_DS try)
{
  if (try->clsf != NULL) {
    store_clsf_DS(try->clsf, NULL, 0);
    try->clsf = NULL;
  }
}


/* total time spent in all tries so far */

int total_try_time( search_try_DS *tries, int n_tries)
{
  int i, sum = 0;
  search_try_DS try;
  for (i=0; i<n_tries; i++) {
    try = tries[i];
    sum += try->time + ((try->duplicates != NULL) ?
			total_try_time(try->duplicates, try->n_duplicates)
			: 0.0);
  }
  return(sum);
}


/* TRY_VARIATION
   15nov94 wmt: use apply_search_start_fn & apply_search_try_fn
   27dec94 wmt: add rel_delta_range, max_cycles
   08jan95 wmt: for incomplete trials, return search_try_DS with j_out = 0
   30jan95 wmt: add checkpointing logic
   18feb98 wmt: add num_cycles and max_cycles

   This randomly initialize a clsf (unless reconverge_type is not ""), then
   passes it to try-fn for convergence.  It returns a search-try structure
   with a new compressed clsf and stats about the try.  Clsf is usually
   modified by the try-fn. 
*/
search_try_DS try_variation( clsf_DS clsf, int j_in, int trial_n, char *reconverge_type,
                            char *start_fn_type, char *try_fn_type,
                            unsigned int initial_cycles_p, time_t begin_try,
                            double halt_range, double halt_factor, double rel_delta_range,
                            int max_cycles, int n_average, double cs4_delta_range,
                            int sigma_beta_n_values, int converge_print_p,
                            FILE *log_file_fp, FILE *stream)
{
   clsf_DS new_clsf;
   search_try_DS temp;
   int complete_trial, want_wts_p = FALSE;
   shortstr str;
   time_t now;

   if (eqstring( G_checkpoint_file, "") != TRUE) {
     G_search_cycle_begin_time = begin_try;
     G_last_checkpoint_written = begin_try;
     if (eqstring( reconverge_type, "chkpt") == TRUE) {
       /* include try time from previous checkpoint sessions */
       sprintf( str, "[reconverge \"chkpt\" j_in=%d] ", j_in);
       to_screen_and_log_file( str, log_file_fp, stream, TRUE);
     }
     else {    /* init for start of checkpoint run */
       clsf->checkpoint->accumulated_try_time = 0;
       clsf->checkpoint->current_try_j_in = j_in;
       clsf->checkpoint->current_cycle = 0;
     }
   } 
   if (eqstring( reconverge_type, "results") == TRUE) {
     sprintf( str, "[reconverge \"results\" j_in=%d] ", j_in);
     to_screen_and_log_file( str, log_file_fp, stream, TRUE);
   }
   if (eqstring( reconverge_type, "") == TRUE) {
     sprintf( str, "[j_in=%d] ", j_in);
     to_screen_and_log_file( str, log_file_fp, stream, TRUE);
     apply_search_start_fn ( clsf, start_fn_type, initial_cycles_p, j_in, log_file_fp,
                            stream);
   }
   G_num_cycles = 0;
   complete_trial = apply_search_try_fn( clsf, try_fn_type, halt_range, halt_factor,
                                        rel_delta_range, max_cycles, n_average,
                                        cs4_delta_range, sigma_beta_n_values,
                                        converge_print_p, log_file_fp, stream);
   temp = (struct search_try *) malloc( sizeof( struct search_try));
   if (complete_trial == TRUE) {
     new_clsf = copy_clsf_DS( clsf, want_wts_p);
     compress_clsf(new_clsf, NULL, FALSE);
     temp->j_in = j_in;
     temp->clsf = new_clsf;
     temp->n = trial_n;
     temp->j_out = new_clsf->n_classes;
     now = get_universal_time();
     if (eqstring( G_checkpoint_file, "") != TRUE)
       temp->time = (int) ((now - G_search_cycle_begin_time) +
                           clsf->checkpoint->accumulated_try_time);
     else
       temp->time = (int) (now - begin_try);
     temp->ln_p = new_clsf->log_a_x_h;
     temp->n_duplicates = 0;
     temp->duplicates = NULL;
     temp->num_cycles = G_num_cycles;
     temp->max_cycles = max_cycles;
   }
   else
     temp->j_out = 0;
   return(temp);
}


/* SEARCH_DURATION
   17oct94 wmt: modified
   30jan95 wmt: add checkpointing time

   total time, including overhead, search has taken
   */
int search_duration( search_DS search, time_t now, clsf_DS clsf, time_t last_save,
                    int reconverge_p)
{
  int accumulated_try_time = 0;

  /* include try time from previous checkpoint sessions */
  if ((eqstring( G_checkpoint_file, "") != TRUE) && (reconverge_p == TRUE))
    accumulated_try_time = (int) clsf->checkpoint->accumulated_try_time;
  return( search->time + (int) (now - last_save) + accumulated_try_time);
}


/* CONVERGE
   27dec94 wmt: change function type to void. free ln_p_list
   18feb98 wmt: add G_num_cycles

   Robin's more compact version of converge-search - stoppoing criterion is
   the sum of the classes' log marginal probability.  Hence opposite, equal
   changes in two classes' values have no effect on this criterion.
   converge_search_3 is sensitive to each classes' probability value.
   */
int converge( clsf_DS clsf, int n_average, double halt_range, double halt_factor,
             double delta_factor, int display_wts, int min_cycles, int max_cycles,
             int converge_print_p, FILE *log_file_fp, FILE *stream)
{
  int n_no_change = 0, n_cycles = 0, complete_trial = TRUE;
  shortstr str;
  fxlstr long_str;
  int converge_cycle_p = TRUE;
  float ln_p, *ln_p_list;
  char caller[] = "converge";

  ln_p_list = (float *) malloc( max_cycles * sizeof( float));
  halt_range  = delta_factor * halt_range;
  halt_factor = delta_factor * halt_factor;
  if (eqstring( G_checkpoint_file, "") != TRUE)
    n_cycles = clsf->checkpoint->current_cycle;

  do {
    ln_p = (float) base_cycle(clsf, stream, display_wts, converge_cycle_p);
    ln_p_list[n_cycles] = ln_p;
    if ((n_cycles > 0) &&
        (max( (float) halt_range, (float) halt_factor * (- ln_p)) >
         (ln_p - ln_p_list[n_cycles-1])))
      n_no_change++;
    else 
      n_no_change = 0;
    n_cycles++;
    if (eqstring( G_checkpoint_file, "") != TRUE)
      clsf->checkpoint->current_cycle++;
    if (converge_print_p) {
      safe_sprintf( long_str, sizeof( long_str), caller,
              "\ncnt %d n_cls %d no_chng %d ln_p %.3f h_rng %.3f, h_fact*(-ln_p) %.3f",
              n_cycles, clsf->n_classes, n_no_change, ln_p, halt_range,
              halt_factor * (- ln_p));
      to_screen_and_log_file( long_str, log_file_fp, stream, TRUE);
    }
    if (char_input_test() == TRUE) {
      complete_trial = FALSE;
      break;
    }
  }
  while (( (n_cycles < min_cycles) || (n_average > n_no_change) ) &&
         (n_cycles   < max_cycles)) ;
  G_num_cycles = n_cycles;
  if (converge_print_p)
    to_screen_and_log_file( "\n", log_file_fp, stream, TRUE);
  sprintf(str, " [c: cycles %d]", n_cycles);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  free(ln_p_list);
  return( complete_trial);
}


/* CONVERGE_SEARCH_3
   27dec94 wmt: new function
   18feb98 wmt: add G_num_cycles 

   A class sensitive loop for driving clsf to a local minima.  The loop terminates
   when the difference between cycles of the (class->log_a_w_s_h_j) /
   class->w_j vector is less than rel_delta_range for each class.  
   */
int converge_search_3( clsf_DS clsf, double rel_delta_range, int display_wts,
                      int min_cycles, int max_cycles, int n_average,
                      int converge_print_p, FILE *log_file_fp, FILE *stream)
{
  int count = 0, n_class, converge_cycle_p = TRUE, i, n_no_change = 0;
  int complete_trial = TRUE;
  shortstr str;
  fxlstr long_str;
  char caller[] = "converge_search_3";
  double diff; 
  class_DS class;
  double *current_class_logs, *last_class_logs, *marginal_stack, *max_logs_diff;

  current_class_logs = (double *) malloc( clsf->n_classes * sizeof( double));
  last_class_logs = (double *) malloc( clsf->n_classes * sizeof( double));
  marginal_stack = (double *) malloc( max_cycles * sizeof( double));
  max_logs_diff = (double *) malloc( max_cycles * sizeof( double));
  if (eqstring( G_checkpoint_file, "") != TRUE)
    count = clsf->checkpoint->current_cycle;
  if (rel_delta_range < 0.0) {
    fprintf(stderr, "converge_search_3 called with rel_delta_range < 0.0\n");
    exit(1);
  }
  for (i=0; i<max_cycles; i++)
    max_logs_diff[i] = 0.0;
  for (n_class=0; n_class<clsf->n_classes; n_class++) {
    class = clsf->classes[n_class];
    current_class_logs[n_class] = class->log_a_w_s_h_j / (double) class->w_j;
  }
  while (TRUE) {
    count++;
    if (eqstring( G_checkpoint_file, "") != TRUE)
      clsf->checkpoint->current_cycle++;

    marginal_stack[count - 1] = base_cycle( clsf, stream, display_wts, converge_cycle_p);

    for (n_class=0; n_class<clsf->n_classes; n_class++)
      last_class_logs[n_class] = current_class_logs[n_class];
    for (n_class=0; n_class<clsf->n_classes; n_class++) {
      class = clsf->classes[n_class];
      current_class_logs[n_class] = class->log_a_w_s_h_j / (double) class->w_j;
    }
    for (n_class=0; n_class<clsf->n_classes; n_class++) {
      diff = fabs( (current_class_logs[n_class] - last_class_logs[n_class]));
      if (diff > max_logs_diff[count - 1])
        max_logs_diff[count - 1] = diff;
    }
    if ((count > 1) && (rel_delta_range > max_logs_diff[count - 1]))
      n_no_change++;
    else
      n_no_change = 0;
    if (converge_print_p) {
      safe_sprintf( long_str, sizeof( long_str), caller,
                   "\ncnt %d, n_class %d, range %.4f, diff %.4f, ln_p %.4f, "
                   "n_no_chng %d", count, clsf->n_classes, rel_delta_range,
                   max_logs_diff[count - 1], marginal_stack[count - 1], n_no_change);
      to_screen_and_log_file( long_str, log_file_fp, stream, TRUE);
    }
    if (char_input_test() == TRUE) {
      complete_trial = FALSE;
      break;
    }
    /* quit after max-cycles, or quit if no delta exceeds rel_delta_range for 3 cycles*/
    if ((count >= min_cycles) &&
        ((n_no_change >= n_average) ||
         (count >= max_cycles)))   
      break;
  }
  G_num_cycles = count;
  if (converge_print_p)
    to_screen_and_log_file( "\n", log_file_fp, stream, TRUE);
  sprintf(str, " [cs-3: cycles %d]", count);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  free( current_class_logs);
  free( last_class_logs);
  free( marginal_stack);
  free( max_logs_diff);
  return( complete_trial);
}
 

/* CONVERGE_SEARCH_3A

  18feb98 wmt: add G_num_cycles 
*/
int converge_search_3a( clsf_DS clsf, double rel_delta_range, int display_wts,
                      int min_cycles, int max_cycles, int n_average,
                      int converge_print_p, FILE *log_file_fp, FILE *stream)
{
  int count = 0, n_class, converge_cycle_p = TRUE, i, n_no_change = 0;
  int complete_trial = TRUE, halt_cnt = 0, n_max_class = 0;
  int num_halt_cycles = 0;      /* 10 for testing */
  shortstr str;
  fxlstr long_str;
  char caller[] = "converge_search_3a";
  double diff, range_delta_factor = rel_delta_range / (double) clsf->database->n_data;
  double range_delta = 0.0;
  class_DS class;
  double *current_class_logs, *last_class_logs, *marginal_stack, *max_logs_diff;

  current_class_logs = (double *) malloc( clsf->n_classes * sizeof( double));
  last_class_logs = (double *) malloc( clsf->n_classes * sizeof( double));
  marginal_stack = (double *) malloc( max_cycles * sizeof( double));
  max_logs_diff = (double *) malloc( max_cycles * sizeof( double));
  if (eqstring( G_checkpoint_file, "") != TRUE)
    count = clsf->checkpoint->current_cycle;
  if (rel_delta_range < 0.0) {
    fprintf(stderr, "converge_search_3a called with rel_delta_range < 0.0\n");
    exit(1);
  }
  for (i=0; i<max_cycles; i++)
    max_logs_diff[i] = 0.0;
  for (n_class=0; n_class<clsf->n_classes; n_class++) {
    class = clsf->classes[n_class];
    current_class_logs[n_class] = class->log_a_w_s_h_j / (double) class->w_j;
  }
  while (TRUE) {
    count++;
    if (eqstring( G_checkpoint_file, "") != TRUE)
      clsf->checkpoint->current_cycle++;

    marginal_stack[count - 1] = base_cycle( clsf, stream, display_wts, converge_cycle_p);

    for (n_class=0; n_class<clsf->n_classes; n_class++)
      last_class_logs[n_class] = current_class_logs[n_class];
    for (n_class=0; n_class<clsf->n_classes; n_class++) {
      class = clsf->classes[n_class];
      current_class_logs[n_class] = class->log_a_w_s_h_j / (double) class->w_j;
    }
    for (n_class=0; n_class<clsf->n_classes; n_class++) {
      diff = fabs( (current_class_logs[n_class] - last_class_logs[n_class]));
      if (diff > max_logs_diff[count - 1]) {
        max_logs_diff[count - 1] = diff;
        n_max_class = n_class;
      }
    }
    if ((count > 1) &&
        ((range_delta = range_delta_factor * (double) clsf->n_classes *
          fabs( current_class_logs[n_max_class])) >
         max_logs_diff[count - 1]))
      n_no_change++;
    else
      n_no_change = 0;
    if ((converge_print_p) && (count > 1)) {
      safe_sprintf( long_str, sizeof( long_str), caller,
                   "\ncnt %d, n_cls %d, range %.4f[%.4f], diff %.4f, ln_p %.4f, "
                   "n_no_chg %d, h %d", count, clsf->n_classes, rel_delta_range,
                   range_delta, max_logs_diff[count - 1], marginal_stack[count - 1],
                   n_no_change, halt_cnt);
      to_screen_and_log_file( long_str, log_file_fp, stream, TRUE);
    }
    if (char_input_test() == TRUE) {
      complete_trial = FALSE;
      break;
    }
    /* quit after max-cycles, or quit if no delta exceeds rel_delta_range for 3 cycles*/
    if ((count >= min_cycles) &&
        (((n_no_change >= n_average) && (++halt_cnt > num_halt_cycles)) ||
         (count >= max_cycles)))       
      break;
  }
  G_num_cycles = count;
  if (converge_print_p)
    to_screen_and_log_file( "\n", log_file_fp, stream, TRUE);
  sprintf(str, " [cs-3a: cycles %d]", count);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  free( current_class_logs);
  free( last_class_logs);
  free( marginal_stack);
  free( max_logs_diff);
  return( complete_trial);
}


/* CONVERGE_SEARCH_4
   27mar95 wmt: new function
   18feb98 wmt: add G_num_cycles 

   A class sensitive loop for driving clsf to a local minima.  The loop terminates
   when the beta (absolute slope of the best linear fit to the previous
   sigma_beta_n_values for each class) is less than cs4_delta_range
   */
int converge_search_4( clsf_DS clsf, int display_wts, int min_cycles, int max_cycles,
                      double cs4_delta_range, int sigma_beta_n_values,
                      int converge_print_p, FILE *log_file_fp, FILE *stream)
{
  int count = 0, n_class, converge_cycle_p = TRUE, i, sigma_beta_halt = FALSE;
  int complete_trial = TRUE, x_i, window_index = 0, index, n_classes_in_noise = 0;
  int in_noise_p, halt_cnt = 0;
  int num_halt_cycles = 0;      /* for testing 10 */
  shortstr str;
  fxlstr long_str;
  char caller[] = "converge_search_4";
  
  double sigma_squared = 0.0, beta_squared, x_bar, y_bar, sxy, syy, dummy, x_mean = 0.0;
  double sxx = 0.0;
  double sigma_factor = 10.0 / 11.0, beta;
  class_DS class;
  double ln_p, *ordered_class_sigma_betas, **class_sigma_beta_ratios;

  ordered_class_sigma_betas = (double *) malloc( sigma_beta_n_values * sizeof( double));
  class_sigma_beta_ratios = (double **) malloc( clsf->n_classes * sizeof( double *));
  if (eqstring( G_checkpoint_file, "") != TRUE)
    count = clsf->checkpoint->current_cycle;

  for (n_class=0; n_class<clsf->n_classes; n_class++)
    class_sigma_beta_ratios[n_class] = (double *) malloc( sigma_beta_n_values *
                                                        sizeof( double));
  for (i=0; i<sigma_beta_n_values; i++)
    x_mean += (double) i;
  x_mean /= (double) sigma_beta_n_values;
  for (i=0; i<sigma_beta_n_values; i++)
    sxx += square( (x_mean - (double) i));
  sxx /= (double) sigma_beta_n_values;
    
  while (TRUE) {
    count++;
    if (eqstring( G_checkpoint_file, "") != TRUE)
      clsf->checkpoint->current_cycle++;

    ln_p = base_cycle( clsf, stream, display_wts, converge_cycle_p);
   
    for (n_class=0; n_class<clsf->n_classes; n_class++) {
      class = clsf->classes[n_class];
      class_sigma_beta_ratios[n_class][window_index] = class->log_a_w_s_h_j /
        (double) class->w_j;
    }
    window_index++;
    if (window_index >= sigma_beta_n_values)
      window_index = 0;
    if (count >= sigma_beta_n_values) {
      x_bar = sigma_beta_n_values - x_mean;
      n_classes_in_noise = 0;
      sigma_beta_halt = TRUE;
      for (n_class=0; n_class<clsf->n_classes; n_class++) {
        /* put circular list into ordered list */
        for (i=0; i<sigma_beta_n_values; i++) {
          index = window_index + i;
          if (index >= sigma_beta_n_values)
            index -= sigma_beta_n_values;
          ordered_class_sigma_betas[i] = class_sigma_beta_ratios[n_class][index];
        }
        mean_and_variance( ordered_class_sigma_betas, sigma_beta_n_values, &y_bar, &dummy);
        sxy = syy = 0.0;
        for (x_i=0; x_i<sigma_beta_n_values; x_i++) {
          sxy += ((double) ( x_i - x_bar)) * ( ordered_class_sigma_betas[x_i] - y_bar);
          if (converge_print_p)
            syy += square( (ordered_class_sigma_betas[x_i] - y_bar));
        }
        sxy /= (double) sigma_beta_n_values;
        if (converge_print_p) {
          syy /= (double) sigma_beta_n_values;
          sigma_squared = sigma_factor * ( syy - ((square( sxy) / sxx)));
        }
        beta = sxy / sxx;
        beta_squared = square( beta);
        in_noise_p = FALSE;
        if (fabs( beta) < cs4_delta_range) {
          n_classes_in_noise++;
          in_noise_p = TRUE;
        }
        else {
          sigma_beta_halt = FALSE;
          if (converge_print_p == FALSE)
            break; 
        }
         if (converge_print_p) {
           safe_sprintf( long_str, sizeof( long_str), caller,
                        "\n   n_cls %d, s^2/b^2 %.4f, beta %.4f, range %.4f, in_n %d",
                        n_class, (beta_squared == 0.0) ? 0.0 : sigma_squared / beta_squared,
                        fabs( beta), cs4_delta_range, in_noise_p);
          to_screen_and_log_file( long_str, log_file_fp, stream, TRUE);
        }
      }
    }
    if ((converge_print_p) && (count >= sigma_beta_n_values)) {
      safe_sprintf( long_str, sizeof( long_str), caller,
                   "\ncnt %d, num_cls %d, ln_p %.4f, n_in_noise %d, range %.4f"
                   ", s_b_n_vals %d, h %d", count, clsf->n_classes, ln_p,
                   n_classes_in_noise, cs4_delta_range, sigma_beta_n_values, halt_cnt);
      to_screen_and_log_file( long_str, log_file_fp, stream, TRUE);
    }
    if (char_input_test() == TRUE) {
      complete_trial = FALSE;
      break;
    }
    if ((count >= min_cycles) &&
        (((sigma_beta_halt == TRUE) && (++halt_cnt > num_halt_cycles)) ||
         (count >= max_cycles)))       
      break;
  }
  G_num_cycles = count;
  if (converge_print_p)
    to_screen_and_log_file( "\n", log_file_fp, stream, TRUE);
  sprintf(str, " [cs-4: cycles %d]", count);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  for (n_class=0; n_class<clsf->n_classes; n_class++)
    free( class_sigma_beta_ratios[n_class]);
  free( class_sigma_beta_ratios);
  free( ordered_class_sigma_betas);
  return( complete_trial);
}
  

/* estimates the number of local maxima in the search space */

int min_n_peaks( int n_dups,int n_dup_tries)
{
   float val;

   val = (float) n_dup_tries / (float) (1.0 + n_dups);
   return(iround( (double) val));
}


/* this will remain the avg time till find better clsf */

double avg_time_till_improve( int time_so_far, int n_peaks_seen)
{
  return( (float) time_so_far *
         ((float) n_peaks_seen / (float) (n_peaks_seen + 1)));
}


/* a log normal distribution has a high average */

double ln_avg_p( double ln_p_avg, double ln_p_sigma)
{
   return( ln_p_avg + ( 0.5 * (ln_p_sigma * ln_p_sigma)));
}


/* estimate of the best peak, given an estimated number of peaks */

double min_best_peak( int min_n_peak, double ln_p_avg, double ln_p_sigma)
{
   return(typical_best(min_n_peak, ln_p_avg, ln_p_sigma));
}


/* ----------- WAYS TO CHOOSE N-CLASSES TO TRY  -------------------------------
   each new try needs to start with a certain number of seed classes
   here are two functions to do that. if ask each to "explain", will give string
   about how works
   here fit a log-normal to the n-classes-in (or j-in) of the 6 best tries so
   far then pick a new j-in for the next try randomly from that distribution. */


/* RANDOM_J_FROM_LN_NORMAL
   16oct94 wmt: modified

   fit log normal model to j (n-classes) of n-best tries so far, select a new j
   using that
   */
int random_j_from_ln_normal( int n_tries, search_try_DS *tries, int  max_j,
                            int explain_p, char *n_classes_explain)
{
   int i, n_best_track = 10, bnum;
   float ln_j_avg, ln_j_sigma, *flist_ptr, min_sigma = 1.5;
   search_try_DS *best;
   char caller[] = "random_j_from_ln_normal";

   best = safe_subseq_of_tries(tries, 0, n_best_track, n_tries, &bnum);
   flist_ptr = (float *) malloc(bnum * sizeof(float));
   for (i=0; i<bnum; i++)
      flist_ptr[i] = (float) safe_log((double) tries[i]->j_in);
   ln_j_avg = (float) average(flist_ptr, bnum);
   ln_j_sigma =
      (float) within(safe_log((double) (1.0 +
                                   (min_sigma * (float) safe_exp((double) (-1.0 * ln_j_avg))))),
                     sigma(flist_ptr, bnum, (double) ln_j_avg), 10.0);
   free(best);
   free(flist_ptr);
   if (explain_p == TRUE) {
     safe_sprintf( n_classes_explain, sizeof( fxlstr), caller,
                  "randomly from a log_normal [M-S, M, M+S] = \n"
                  " [%.1f, %.1f, %.1f]",
                  (float) safe_exp((double) (ln_j_avg - ln_j_sigma)),
                  (float) safe_exp((double) ln_j_avg),
                  (float) safe_exp((double) (ln_j_avg + ln_j_sigma)));
     return(0);
   }
   else
     return (ceil( safe_exp( random_from_normal( (double) ln_j_avg,
                                                (double) ln_j_sigma))));
}


/* RANDOM_FROM_NORMAL
   16oct94 wmt: modified
   17may95 wmt: Convert from srand/rand to srand48/lrand48 

   return random value picked off pseudo-normal distribution
   */
double random_from_normal( double mean, double sigma)
{
  int i, n_steps = 20;
  double diff = 0.0, offset = 1.0;
  double three_over_n_steps = 3.0 / (double) n_steps;
  double normalizer = G_rand_base_normalizer / 2.0;

  for (i=0; i<n_steps; i++)
    diff += (min( (double) lrand48( ), G_rand_base_normalizer) / normalizer) -
      offset;

  return (mean + (sigma * sqrt( three_over_n_steps) * diff));
}


/* about mean/mid of best will have when have n-samples */

double typical_best(int n_samples, double mean, double sigma)
{
   return(mean + (sigma * cut_where_above( 1.0 / (double) n_samples)));
}


/* how many standard devs up happens this % of time */

double cut_where_above (double percent)
{
   float val;

   val = (float) interpolate(cut_where_above_table, SIZEOF_CUT_WHERE_ABOVE_TABLE, percent);
   if (val == -1.0)
      val =  (float) sqrt( 2.0) * (float) inverse_erfc( 2.0 * percent);
   return (val);
}


/* ERFC_POLY
   09jan95 wmt: correct code from Lisp original: zs = 2.0 * ...
                1.0 + val1
   02mar95 wmt: do calculations in double precision

   polynomial in approx to error fn
   */
double erfc_poly( double z)
{
  double val1, val2, val3, zs = 2.0 * z * z;
  
  val1 = -1.0 * (1.0 / zs);
  val2 = 3.0 / (zs * zs);
  val3 = -1.0 * (15.0 / (zs * zs * zs));
  return (1.0 + val1 + val2 + val3);
}


double approx_inverse_erfc( double area, double z_try)
{
   double val;

   val = ((erfc_poly( z_try) / area) / sqrt( M_PI)) / z_try;
   return (sqrt( safe_log( val)));
}


/* INVERSE_ERFC
   02mar95 wmt: do calculations in double precision

   find inverse of erfc fn by converging to consistent solution */

double inverse_erfc (double area)
{
   double z_try = 3.0, accuracy = area * area, z_last = 0.0;

   while (accuracy < fabs(z_try - z_last)) {
      z_last = z_try;
      z_try = approx_inverse_erfc( area, z_try);
   }
   return( z_try);
}


/* table is set of pairs of floats */

double interpolate( float table[][2], int length, double key)
{
   int i = 0;

   while( key < table[i][0] && ++i < length);
   if( i >= length)
	return (-1.0);
   else if( key == table[i][0])
	return (table[i][1]);
   else if( i == 0 )
	return (-1.0);
   else
	return( table[i][1] - (table[i][1] - table[i-1][1]) * 
               ((table[i][0] - key ) / (table[i][0] - table[i-1][0])));
/****** commented old code
   float *high, high_key, *low, low_key;
   while (1) {
      low = high;
      low_key = high_key;
      high = table[i];
      high_key = high[0];
      if (key == high_key)
	 return(high[1]);
      if (++i >= length)
	 return(-1.0);
   }
*********commented */
}


/* UPPER_END_NORMAL_FIT
   25oct94 wmt: switched args in last call to sigma
   05jan95 wmt: use floor on (n_tries / 2); correct pair-up code

   fit a normal to the max of pairs of list, and infer mean/sigma implies for
   whole
   */
void upper_end_normal_fit( search_try_DS *tries, int n_tries, 
					float *ln_p_avg, float *ln_p_sigma)
{
  int i, i_free = 0, half_length;
  int ordered_p = TRUE;
  float *ln_p, avg, sig;

  ln_p = (float *) malloc(n_tries * sizeof(float));
  for (i=0; i<n_tries; i++)
    ln_p[i] = (float) tries[i]->ln_p;
  if (ordered_p == TRUE)
    randomize_list(ln_p, n_tries);
  if (n_tries < 6) {
    *ln_p_avg = (float) average(ln_p, n_tries);
    *ln_p_sigma = (float) sigma(ln_p, n_tries, (double) *ln_p_avg);
  }
  else {
    for (i=0; i+1<n_tries; i=i+2) {
      if (ln_p[i] > ln_p[i+1])
        ln_p[i_free] =  ln_p[i];
      else
        ln_p[i_free] = ln_p[i+1];
      i_free++;
    }
    half_length = (int) floor( (double) (n_tries / 2));
    avg = (float) average(ln_p, half_length);
    sig = (float) sigma(ln_p, half_length, (double) avg);
    *ln_p_avg = avg - (sig * (0.575 / 0.825)); /* get better numbers here! */
    *ln_p_sigma = sig / 0.825;
  }
  free(ln_p);
}


/* AVERAGE
   15dec94 wmt: add check for length == 0
   */
double average( float *list, int length)
{
  int i;
  float sum = 0.0;

  for (i=0; i<length; i++)
    sum += list[i];
  if (length == 0) {
    fprintf(stderr, "ERROR: average called for a zero length list!\n");
    abort ();
  }
  return (sum / (float) length);
}


/* square of standard deviation */

double variance( float *list, int length, double avg)
{
   int i;
   float sum = 0;

   if ((list != NULL) && (length > 1))
      for (i=0; i<length; i++)
	 sum += square( (double) list[i] - avg);
   return (sum / (float) (length - 1));
}


/* SIGMA
   14apr95 wmt: if variance is zero, return 0.0 rather than
                infinity
*/
double sigma( float *list, int num, double ln_p_avg)
{
  double val;

  val = variance( list, num, ln_p_avg);
  if (val != 0.0)
    return (sqrt( val));
  else
    return ( 0.0);
}


/* average amount by which the next best will be better
 */
double avg_improve_delta_ln_p( int n_peaks, double ln_p_sigma)
{
   return(next_best_delta( n_peaks, ln_p_sigma));
}


/* about mean/mid of difference between best have, next best will find
 */
double next_best_delta( int n_samples, double sigma)
{
   return(sigma * (typical_best(2 * n_samples, 0.0, 1.0) -
                   typical_best(n_samples, 0.0, 1.0)));
}


/* MIN_TIME_TILL_BEST
   09dec94 wmt: max of 0.0 and val to prevent negative return values

   estimates the number of local maxima in the search space
   */
int min_time_till_best( int time_so_far,int  min_n_peak,int n_peaks_seen) 
{
   float val;

   val = (float) (min_n_peak - n_peaks_seen) / (float) n_peaks_seen;
   return (time_so_far * max( 0.0, val));
}


/* SAVE_SEARCH
   17oct94 wmt: modified
   18feb95 wmt: write file as temporary and rename after completion
                (G_safe_file_writing_p should be true only for unix systems,
                since this does system calls for mv and rm shell commands)
   02may95 wmt: double size of str - prevent "Premature end of file
                reading symbol table" error.

   write the search out to a search-file, so can be read back in
   */ 
void save_search( search_DS search, char *search_file_ptr, time_t last_save,
                 clsf_DS clsf, int reconverge_p, int_list start_j_list,
                 int n_final_summary, int n_save)
{
  int i, n_tries, str_length = 2 * sizeof( fxlstr);
  time_t now;
  clsf_DS *try_clsfs;
  search_try_DS *tries;
  static fxlstr temp_search_file; 
  FILE *search_file_fp;
  char *str, caller[] = "save_search";

  str = (char *) malloc( str_length);
  temp_search_file[0] = '\0';
  now = get_universal_time();
  tries = search->tries;
  n_tries = search->n_tries;
  try_clsfs = (clsf_DS *) malloc(n_tries * sizeof(clsf_DS));
  for (i=0; i<n_tries; i++) 
    try_clsfs[i] = tries[i]->clsf;
  for (i=0; i<n_tries; i++) {   /* temporarily strip out the clsfs */
    tries[i]->clsf = NULL;
  }
  search->time = search_duration(search, now, clsf, last_save, reconverge_p);
  /* write out the search */ 
  if (G_safe_file_writing_p == TRUE) {
    make_and_validate_pathname( "search_tmp", search_file_ptr, &temp_search_file,
                               FALSE);
    search_file_fp = fopen( temp_search_file, "w");
  }
  else
    search_file_fp = fopen( search_file_ptr, "w");
  
  write_search_DS( search_file_fp, search, start_j_list, n_final_summary, n_save);

  fclose( search_file_fp);

  if (G_safe_file_writing_p == TRUE) {
    if ((search_file_fp = fopen( search_file_ptr, "r")) != NULL) {
      fclose( search_file_fp);
      safe_sprintf( str, str_length, caller, "rm %s", search_file_ptr);
      system( str);
    }
    safe_sprintf( str, str_length, caller, "mv %s %s", temp_search_file,
                 search_file_ptr);
    system( str);
  }
  /* restore clsfs */
  for (i=0; i<n_tries; i++)
    tries[i]->clsf = try_clsfs[i];
  free( try_clsfs);
  free( str);
}


/* WRITE_SEARCH_DS
   06nov94 wmt: new
   25jan95 wmt: add search->start_j_list
   14feb95 wmt: added n_final_summary, n_save

   code to write search DS to file using fprintf
   */
void write_search_DS( FILE *search_file_fp, search_DS search, int_list start_j_list,
                     int n_final_summary, int n_save)
{
  int i, n_last_try_reported;
  static shortstr id;
  char caller[] = "write_search_DS";

  safe_fprintf(search_file_fp, caller, "search_DS\n");
  safe_fprintf(search_file_fp, caller, "n, time, n_dups, n_dup_tries \n%d %d %d %d\n",
          search->n, search->time, search->n_dups, search->n_dup_tries);

  safe_fprintf(search_file_fp, caller, "last try reported\n");
  if (search->last_try_reported != NULL)
    n_last_try_reported = search->last_try_reported->n;
  else
    n_last_try_reported = 0;
  safe_fprintf(search_file_fp, caller, "%d\n", n_last_try_reported);

  safe_fprintf(search_file_fp, caller, "tries from best on down for n_tries \n%d\n",
          search->n_tries);
  for (i=0; i< search->n_tries; i++) {
    sprintf(id, "search_try_DS %d", i);
    write_search_try_DS( search->tries[i], id, i, search_file_fp);
  }
  safe_fprintf(search_file_fp, caller, "start_j_list\n");
  for (i=0; start_j_list[i] != END_OF_INT_LIST; i++)
    safe_fprintf(search_file_fp, caller, "%d\n", start_j_list[i]);
  safe_fprintf(search_file_fp, caller, "%d\n", END_OF_INT_LIST);
  
  safe_fprintf(search_file_fp, caller, "n_final_summary, n_save\n");
  safe_fprintf(search_file_fp, caller, "%d %d\n", n_final_summary, n_save);
}


/* WRITE_SEARCH_TRY_DS
 06nov94 wmt: new
 18feb98 wmt: add num_cycles and max_cycles

 use fprintf to write search tries
 */
void write_search_try_DS( search_try_DS try, shortstr id, int try_index, FILE *search_file_fp)
{
  int dup_index;
  shortstr dup_id;
  char caller[] = "write_search_try_DS";

  if (eqstring( id, "") != TRUE)
    safe_fprintf(search_file_fp, caller, "%s\n", id);
  safe_fprintf(search_file_fp, caller, "n, time, j_in, j_out, ln_p, num_cycles, max_cycles\n"
               "%d %d %d %d %.8e %d %d\n",
               try->n, try->time, try->j_in, try->j_out, try->ln_p,
               try->num_cycles, try->max_cycles);
  if (try_index >= 0) {          /* this is a not a dup try */
    safe_fprintf(search_file_fp, caller, "n_dups\n%d\n", try->n_duplicates);
    for (dup_index=0; dup_index<try->n_duplicates; dup_index++) {
      sprintf( dup_id, "search_try_DS %d dup_try_DS %d", try_index, dup_index);
      write_search_try_DS( try->duplicates[dup_index], dup_id, -1, search_file_fp);
    }
  }
}

 
/* GET_SEARCH_DS
   26oct94 wmt: add last_try_reported init
   11jan95 wmt: add time init
   25jan95 wmt: add start_j_list
   16may95 wmt: move malloc out of declaration
   */
search_DS get_search_DS( void)
{
  search_DS search;

  search  = (search_DS) malloc( sizeof( struct search));
  search->n = 0;
  search->time = 0;
  search->n_dups = 0;
  search->last_try_reported = NULL;
  search->n_dup_tries = 0;
  search->n_tries = 0;
  search->tries = NULL;
  search->start_j_list = NULL;
  search->n_final_summary = 0;         /* for search_summary (intf-reports.c) */
  search->n_save = 0;                  /* for search_summary (intf-reports.c) */

  return (search); 
}


/* search_DS copy_search_wo_tries( search_DS search) */
/* { */
/*    search_DS temp; */

/*    temp = get_search_DS(); */
/*    temp->n = search->n; */
/*    temp->time = search->time; */
/*    temp->n_dups = search->n_dups; */
/*    temp->n_dup_tries = search->n_dup_tries; */
/*    temp->last_try_reported = search->last_try_reported; */
/*    temp->n_tries = search->n_tries; */
/*    start_j_list; copy and allocate int_list */
/*    temp->n_final_summary = search->n_final_summary; */
/*    temp->n_save = search->n_save; */
/*    return(temp); */
/* } */


/* RECONSTRUCT_SEARCH
   11jan95 wmt: replace get_object_from_file with get_search_from_file
   31mar92 WMT - %= added because search file is written & loaded
   (with truncation) and results file is compiled (without truncation)
   01may95 wmt: change type of results_file: char * => fxlstr
   18may95 wmt: add results/search consistency check

   returns a search-state
      */
search_DS reconstruct_search( FILE *search_file_fp, char *search_file_ptr,
                             char *results_file_ptr)
{
  int i, n_best_clsfs, expand_p = TRUE, want_wts_p = TRUE, update_wts_p = FALSE;
  clsf_DS *best_clsfs, clsf;
  search_DS search = NULL;
  static int clsf_n_list[MAX_CLSF_N_LIST] = {END_OF_INT_LIST};
  static fxlstr results_file;
  int exit_if_error_p = TRUE, silent_p = FALSE, n_clsf;

  results_file[0] = '\0';
  
  validate_results_pathname( results_file_ptr, &results_file, "results",
                            exit_if_error_p, silent_p);

  best_clsfs = get_clsf_seq( results_file, expand_p, want_wts_p, update_wts_p,
                            "results", &n_best_clsfs, clsf_n_list);
  if (best_clsfs != NULL) {
    search = get_search_from_file( search_file_fp, search_file_ptr);
    if (search->n_tries < n_best_clsfs) {
      fprintf(stderr, "ERROR: number of search trials (%d) is less than "
              "the number of saved clsfs (%d)\n", search->n_tries, n_best_clsfs);
      exit(1);
    }
    if (search != NULL)
      for (i=0; i<n_best_clsfs; i++)
        search->tries[i]->clsf = best_clsfs[i];
  }
  for (n_clsf=0; n_clsf<n_best_clsfs; n_clsf++) {
    clsf = best_clsfs[n_clsf];
    if (clsf_search_validity_check( clsf, search) < 0) {
      fprintf( stderr,
              "ERROR: .results[-bin] file and .search file not from the same run\n");
      exit(1);
    }
  }
  return (search);
}
 

/* GET_SEARCH_FROM_FILE
   11jan95 wmt: new
   25jan95 wmt: add start_j_list
   14feb95 wmt: add n_summary & n_save

   read the .search file and allocate storage for search_DS and
   search_try_DS
   */
search_DS get_search_from_file( FILE *search_file_fp, char *search_file_ptr)
{
  search_DS search;
  fxlstr line;
  int invalid_error = FALSE, i, n_last_try_reported;
  int try_index, str_length = 2 * sizeof( fxlstr);
  shortstr token;
  char caller[] = "get_search_from_file", *str;

  str = (char *) malloc( str_length);
  search = get_search_DS();

  fgets( line, sizeof(line), search_file_fp);
  if (line == NULL)
    invalid_error = TRUE;
  sscanf( line, "%s", token);
  if (eqstring( token, "search_DS") != TRUE)
    invalid_error = TRUE;
  if (invalid_error == TRUE) {
    fprintf( stderr, "\nERROR: \"%s\" is not a valid search file\n", search_file_ptr);
    exit(1);
  }
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), search_file_fp);
  sscanf( line, "%d %d %d %d", &search->n, &search->time, &search->n_dups,
         &search->n_dup_tries);
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), search_file_fp);
  sscanf( line, "%d", &n_last_try_reported);
  for (i=0; i<2; i++)
    fgets( line, sizeof(line), search_file_fp);
  sscanf( line, "%d", &search->n_tries);

  for (try_index=0; try_index<search->n_tries; try_index++)
    get_search_try_from_file( search, NULL, try_index, search_file_fp, search_file_ptr);

  fgets( line, sizeof(line), search_file_fp);
  sscanf( line, "%s", token);
  if (eqstring( token, "start_j_list") != TRUE) {
    fprintf( stderr, "\nERROR: in \"%s\", expected \"start_j_list\", found \"%s\"\n",
            search_file_ptr, line);
    abort();
  }
  for (i=0; i<MAX_N_START_J_LIST; i++) {
    fgets( line, sizeof(line), search_file_fp);
    if (i == 0)
      search->start_j_list = (int_list) malloc( (i+1) * sizeof( int));
    else
      search->start_j_list = (int_list) realloc( search->start_j_list, (i+1) * sizeof( int));
    sscanf( line, "%d", &search->start_j_list[i]);
    if (search->start_j_list[i] == END_OF_INT_LIST)
      break;
  }

  for (i=0; i<2; i++)
    fgets( line, sizeof(line), search_file_fp);
  sscanf( line, "%d %d", &search->n_final_summary, &search->n_save);

  if ((i == (MAX_N_START_J_LIST - 1)) && (search->start_j_list[i] != END_OF_INT_LIST))
    search->start_j_list[i] = END_OF_INT_LIST;
  /* use n_last_try_reported to set ptr to trial */
  if (n_last_try_reported > 0) {
    for (i=0; i<search->n_tries; i++)
      if (search->tries[i]->n == n_last_try_reported)
        break;
    search->last_try_reported = search->tries[i];
  }
  else
    search->last_try_reported = NULL;

  safe_sprintf( str, str_length, caller,
               "ADVISORY: read %d search trials from \n          %s%s\n",
               search->n_tries,
               (search_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
               search_file_ptr);
  to_screen_and_log_file( str, G_log_file_fp, G_stream, TRUE);
  free( str);
  return(search);
}


/* GET_SEARCH_TRY_FROM_FILE
   12jan95 wmt: new
   18feb98 wmt: get num_cycles and max_cycles if in search try

   read the .search file and allocate storage for search_try_DS
   
   */
void get_search_try_from_file( search_DS search, search_try_DS parent_try,
                              int try_index, FILE *search_file_fp,
                              char *search_file_ptr)
{
  fxlstr line;
  int i, file_try_index, dup_file_try_index, dup_index;
  shortstr token, dup_token;
  search_try_DS search_try;

  fgets( line, sizeof(line), search_file_fp);
  if (parent_try == NULL) {
    sscanf( line, "%s %d", token, &file_try_index);
    if ((eqstring( token, "search_try_DS") != TRUE) || (try_index != file_try_index)) {
      fprintf( stderr, "\nERROR: in \"%s\", search_try index = %d not found\n",
              search_file_ptr, try_index);
      abort();
    }
    if (search->tries == NULL) 
      search->tries = (search_try_DS *) malloc( sizeof(search_try_DS));
    else
      search->tries = (search_try_DS *) realloc( search->tries, (try_index + 1) *
                                                sizeof(search_try_DS));
    search_try = (struct search_try *) malloc( sizeof( struct search_try));
    search->tries[try_index] = search_try;
  }
  else {                        /* reading a dup try */
    sscanf( line, "%s %d %s %d", token, &file_try_index, dup_token, &dup_file_try_index);
    if ((eqstring( dup_token, "dup_try_DS") != TRUE) ||
        (try_index != dup_file_try_index)) {
      fprintf( stderr, "\nERROR: in \"%s\", dup_try_index %d not found for "
              "search_try index = %d\n", search_file_ptr, try_index, file_try_index);
      abort();
    }
    if (parent_try->duplicates == NULL) 
      parent_try->duplicates = (search_try_DS *) malloc( sizeof(search_try_DS));
    else
      parent_try->duplicates = (search_try_DS *) realloc( parent_try->duplicates,
                                                         (try_index + 1) *
                                                         sizeof(search_try_DS));
    search_try = (struct search_try *) malloc( sizeof( struct search_try));
    parent_try->duplicates[try_index] = search_try;
  }
    
  fgets( line, sizeof(line), search_file_fp);
  if (strstr( line, "num_cycles") == NULL) {
    /* old style search try */
    fgets( line, sizeof(line), search_file_fp);
    sscanf( line, "%d %d %d %d %le", &search_try->n, &search_try->time, &search_try->j_in,
            &search_try->j_out, &search_try->ln_p);
    search_try->num_cycles = 0;
    search_try->max_cycles = 0;
  } else {
    fgets( line, sizeof(line), search_file_fp);
    sscanf( line, "%d %d %d %d %le %d %d", &search_try->n, &search_try->time, 
            &search_try->j_in, &search_try->j_out, &search_try->ln_p,
            &search_try->num_cycles, &search_try->max_cycles);
  }
  search_try->clsf = NULL;
  search_try->duplicates = NULL;
  if (parent_try == NULL) {
    for (i=0; i<2; i++)
      fgets( line, sizeof(line), search_file_fp);
    sscanf( line, "%d", &search_try->n_duplicates);
    for (dup_index=0; dup_index<search_try->n_duplicates; dup_index++)
      get_search_try_from_file( search, search_try, dup_index, search_file_fp,
                               search_file_ptr);
  }
  else
    search_try->n_duplicates = 0;
}


/* FIND_DUPLICATE
   15oct94 wmt: modified 
   
   go down tries seeking a duplicate clsf.  stops when tries are empty,
   or when past n-store on a restart
   */
int find_duplicate( search_try_DS try, search_try_DS *tries, int n_store,
                   int *n_dup_tries_ptr, double rel_error, int n_tries, int restart_p)
{
  int i, n, count = 0;
  clsf_DS clsf;
  search_try_DS old_try;

  *n_dup_tries_ptr = 0;
  if (tries == NULL) 
    return(FALSE);
  clsf = try->clsf;

  for (i=0; i<n_tries; i++) { 
    old_try = tries[i];
    count++;
    if (old_try->clsf != NULL) {
      (*n_dup_tries_ptr)++;
      if (clsf_DS_test(clsf, old_try->clsf, rel_error)) {
        n = old_try->n_duplicates ++;
        if (old_try->duplicates == NULL)
          old_try->duplicates =
            (search_try_DS *) malloc(old_try->n_duplicates *
                                     sizeof(search_try_DS));
        else
          old_try->duplicates =
            (search_try_DS *) realloc(old_try->duplicates,
                                      old_try->n_duplicates * sizeof(search_try_DS));
        old_try->duplicates[n] = try;
        empty_search_try(try);
        return(TRUE);
      }
    }
    else
      if ((restart_p == FALSE) || (count > n_store))
        return(FALSE);
  }
  return(FALSE);
}


/* INSERT_NEW_TRIAL
   27oct94 wmt: do not use  tries[--i]
   06jan95 wmt: n_tries = number of try; delete the least significant clsf,
                after ordering by ln_p

   add new try to list of old tries, store N_store clsfs
   but store tries till cows come home */

search_try_DS *insert_new_trial( search_try_DS try, search_try_DS *tries, 
			int n_tries, int n_stored_clsf, int max_n_store)
{
  int i, exceeded_trial_index;

  if (tries == NULL) 
    tries = (search_try_DS *) malloc(sizeof(search_try_DS));
  else
    tries = (search_try_DS *)realloc(tries, n_tries * sizeof(search_try_DS));

  i = n_tries - 1;
  /*    while( i >0 && try->ln_p > tries[i-1]->ln_p) */
  /* 	tries[i] = tries[--i]; */
  while ( i >0 && try->ln_p > tries[i-1]->ln_p) {
    tries[i] = tries[i-1];
    i--;
  }
  tries[i] = try;
      
  /*    if (n_stored_clsf >= max_n_store) JPT */
  /*       empty_search_try(tries[max_n_store]); */
  if (n_tries > n_stored_clsf) {
    exceeded_trial_index = max( min( max_n_store, n_tries - 1), i);
    if (tries[exceeded_trial_index] != NULL)
      empty_search_try(tries[exceeded_trial_index]);
  }
  return(tries);
}


/* DESCRIBE_CLSF
   14oct94 wmt: changes
   26oct94 wmt: extra sizeof in malloc

   output class weights and marginal probability of clsf */

void describe_clsf( clsf_DS clsf, FILE *stream, FILE *log_file_fp)
{
   int i, *temp_num_ptr;
   float *wts;
   fxlstr str;
   int (* comp_func) () = int_compare_greater;
   char caller[] = "describe_clsf";

   temp_num_ptr = (int *) malloc(clsf->n_classes * sizeof(int));

   sprintf( str, "It has %d CLASSES with WEIGHTS", clsf->n_classes);
   to_screen_and_log_file( str, log_file_fp, stream, TRUE);
   wts = clsf_DS_w_j( clsf);
   for (i=0; i<clsf->n_classes; i++) 
     temp_num_ptr[i] = iround( (double) wts[i]);
   qsort( temp_num_ptr, clsf->n_classes, sizeof(int), comp_func);
   for (i=0; i<clsf->n_classes; i++) {
     sprintf( str, " %d", *(temp_num_ptr + i));
     to_screen_and_log_file( str, log_file_fp, stream, TRUE);
   }
   safe_sprintf( str, sizeof( str), caller,
                "\nPROBABILITY of both the data and the classification = exp(%.3f)\n",
                clsf->log_a_x_h);
   to_screen_and_log_file(str, log_file_fp, stream, TRUE);

   free( wts);
   free( temp_num_ptr);
}


/* PRINT_LOG
   14oct94 wmt: modified
   
   print log value and its non-log value
   */
void print_log (double log_number, FILE *log_file_fp, FILE *stream, int verbose_p)
{
  fxlstr str;

  sprintf(str, " exp(%.1f) ", log_number);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  if (verbose_p == TRUE) {
    sprintf(str, "[= %.1e] ", safe_exp( log_number));
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  }
}


/* APPLY_SEARCH_START_FN
   15nov94 wmt: new function
   */
void apply_search_start_fn (clsf_DS clsf, char *start_fn_type,
                            unsigned int initial_cycles_p, int j_in,
                            FILE *log_file_fp, FILE *stream)
{
  int block_size = 0, delete_duplicates = FALSE;
 
  if (eqstring(start_fn_type, "random"))
    random_set_clsf(clsf, j_in, delete_duplicates, DISPLAY_WTS,
                    initial_cycles_p, log_file_fp, stream);
  else if (eqstring(start_fn_type, "block"))
    block_set_clsf(clsf, j_in, block_size, delete_duplicates, DISPLAY_WTS,
                   initial_cycles_p, log_file_fp, stream);
  else {
    fprintf(stderr, "\nERROR: start function type \"%s\" not handled!\n",
            start_fn_type);
    exit(1);
  }
}


/* APPLY_SEARCH_TRY_FN
   15nov94 wmt: new function
   10dec94 wmt: change converge halt_range from 2.5 to 0.5 to get better
                convergence and to agree with AutoClass X (Lisp version)
   27dec94 wmt: add converge_search_3 and rel_delta_range, max_cycles
   27mar95 wmt: add converge_search_4
   */
int apply_search_try_fn (clsf_DS clsf, char *try_fn_type, double halt_range,
                         double halt_factor, double rel_delta_range,
                         int max_cycles, int n_average, double cs4_delta_range,
                         int sigma_beta_n_values, int converge_print_p,
                         FILE *log_file_fp, FILE *stream)
{
  int min_cycles = 3, complete_trial;
  double delta_factor = 1.0;
  
  if (eqstring(try_fn_type, "converge"))
    complete_trial = converge( clsf, n_average, halt_range, halt_factor,
                              delta_factor, DISPLAY_WTS, min_cycles, max_cycles,
                              converge_print_p, log_file_fp, stream);
  else if (eqstring(try_fn_type, "converge_search_3"))
    complete_trial = converge_search_3( clsf, rel_delta_range, DISPLAY_WTS,
                                       min_cycles, max_cycles, n_average,
                                       converge_print_p, log_file_fp, stream);
  else if (eqstring(try_fn_type, "converge_search_4"))
    /* complete_trial = converge_search_3a( clsf, rel_delta_range, DISPLAY_WTS,
                                       min_cycles, max_cycles, n_average,
                                       converge_print_p, log_file_fp, stream); */
    complete_trial = converge_search_4( clsf, DISPLAY_WTS, min_cycles, max_cycles,
                                       cs4_delta_range, sigma_beta_n_values,
                                       converge_print_p, log_file_fp, stream);
  else {
    fprintf(stderr, "\nERROR: try function type \"%s\" not handled!\n",
            try_fn_type);
    exit(1);
  }
  return( complete_trial);
}


/* APPLY_N_CLASSES_FN 
   14dec94 wmt: new function
   */
int apply_n_classes_fn ( char *n_classes_fn_type, int n_tries, search_try_DS *tries,
                         int  max_j, int explain_p, char *n_classes_explain)
{
  int return_value;
  
  if (eqstring(n_classes_fn_type, "random_ln_normal"))
    return_value = random_j_from_ln_normal( n_tries, tries, max_j, explain_p,
                                    n_classes_explain);
  else {
    fprintf(stderr, "\nERROR: number of classes function type \"%s\" not handled!\n",
            n_classes_fn_type);
    exit(1);
  }
  return return_value;
}


/* VALIDATE_SEARCH_START_FN
   10dec94 wmt: new function
   */
int validate_search_start_fn (char *start_fn_type)
{
  int err_cnt = 0;

  if (eqstring(start_fn_type, "random"))
    ;
  else if (eqstring(start_fn_type, "block"))
    ;
  else {
    fprintf( stderr, "ERROR: start function type \"%s\" not handled!\n"
            "       allowable types are \"random\" & \"block\"\n",
            start_fn_type);
    err_cnt = 1;
  }
  return( err_cnt);
}


/* VALIDATE_SEARCH_TRY_FN
   10dec94 wmt: new function
   27dec94 wmt: add converge_search_3
   27mar95 wmt: add converge_search_4
   */
int validate_search_try_fn (char *try_fn_type)
{
  int err_cnt = 0;
  
  if (eqstring(try_fn_type, "converge"))
    ;
  else if (eqstring(try_fn_type, "converge_search_3"))
    ;
  else if (eqstring(try_fn_type, "converge_search_4"))
    ;
  else {
    fprintf( stderr, "ERROR: try function type \"%s\" not handled!\n"
            "       allowable types are \"converge_search_3\", "
            "\"converge_search_4\", and \"converge\", \n",
            try_fn_type);
    err_cnt = 1;
  }
  return( err_cnt);
}

  
/* VALIDATE_N-CLASSES_FN
   14dec94 wmt: new function
   */
int validate_n_classes_fn (char *n_classes_fn_type)
{
  int err_cnt = 0;
  
  if (eqstring(n_classes_fn_type, "random_ln_normal"))
    ;
  else {
    fprintf( stderr, "ERROR: number of classes function type \"%s\" not handled!\n"
            "       allowable types are \"random_ln_normal\" only\n",
            n_classes_fn_type);
    err_cnt = 1;
  }
  return( err_cnt);
}

  
/* DESCRIBE_SEARCH
   09jan95 wmt: new

   print out search trials and clsf->log_a_x_h
   */
void describe_search( search_DS search)
{
  search_try_DS try;
  int try_index;

  fprintf(stdout, "\n");
  for (try_index=0; try_index < search->n_tries; try_index++) {
    try = search->tries[try_index];
    fprintf(stdout, "try %.2d: n %.2d j_in %.2d j_out %.2d clsf_ln_p %.3f clsf %s\n",
            try_index + 1, try->n, try->j_in, try->j_out, try->ln_p,
            (try->clsf != NULL) ? "yes" : "no");
  }
}
