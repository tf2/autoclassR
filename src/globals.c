#include <time.h>
/* for MAXPATHLEN */
#ifndef _MSC_VER
#include <sys/param.h>
#endif
#include "autoclass.h"

/************************ GLOBALS globals *************************************
 22oct94 wmt: create
 */

void ***G_plist;
shortstr G_transforms[NUM_TRANSFORMS], G_att_type_data[NUM_ATT_TYPES];

int G_db_length = 0, G_m_id = 0, G_m_length = 0;
int G_plength = 0;
clsf_DS G_clsf_store = NULL;
fxlstr G_checkpoint_file; 
time_t G_search_cycle_begin_time, G_last_checkpoint_written, G_min_checkpoint_period;
database_DS G_input_data_base, *G_db_list = NULL;
model_DS *G_model_list = NULL;
int G_break_on_warnings = TRUE;
float G_likelihood_tolerance_ratio = 0.00001;
unsigned int G_save_compact_p = FALSE;
#ifdef _WIN32
shortstr G_ac_version = "3.3.6win";
#else
shortstr G_ac_version = "3.3.6unx";
#endif
FILE *G_log_file_fp = NULL, *G_stream = NULL;
char G_absolute_pathname[MAXPATHLEN];
int G_line_cnt_max = 65;                /* for reports */
/* only supported under unix, since it does system calls to mv and rm */
/* aju 980612: Implemented this rule for Win32. */
#ifdef _WIN32
int G_safe_file_writing_p = FALSE;
#else
int G_safe_file_writing_p = TRUE;
#endif
char G_data_file_format[10] = "";       /* "binary" or "ascii" */
int G_solaris = FALSE;                  /* used for open on Solaris 2.4 29apr95 wmt */
double G_rand_base_normalizer;
clsf_DS G_training_clsf = NULL;
int G_prediction_p = FALSE;
int G_interactive_p = TRUE;
int G_num_cycles = 0;
/* handle both Unix pathnames and Windows pathnames */
#ifdef _WIN32
char G_slash = '\\';
#else
char G_slash = '/';
#endif

/* for debugging */
int G_clsf_storage_log_p = FALSE;       /* TRUE enables print out of clsf storage activity */
int G_n_freed_classes = 0, G_n_create_classes_after_free = 0;

