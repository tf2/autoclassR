#ifdef _MSC_VER
#pragma warning (disable: 4305 4244 4113)
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
/* #include <values.h> */
#include "getparams.h"
/* #include "params.h"  done below between typedefs and structs */

#if !defined(__SVR4) && defined(__sun) && defined(__GNUC__)
/* missing gcc SunOS 4.1.3 prototypes */
extern double drand48(void);
extern double erand48(unsigned short *);
extern long jrand48(unsigned short *);
extern void lcong48(unsigned short *);
extern long lrand48(void);
extern long mrand48(void);
extern long nrand48(unsigned short *);
extern unsigned short *seed48(unsigned short *);
extern void srand48(long);
#endif

/* NOTE: MULTI-LINE COMMENTS ARE NOT ALLOWED */

/* the following define symbols come from GNU gcc <values.h> */
/* MAXDOUBLE	1.797693134862315708e+308 */
/* MAXFLOAT	((float)3.40282346638528860e+38) */
/* MINDOUBLE	4.94065645841246544e-324 */
/* MINFLOAT	((float)1.40129846432481707e-45) */
/* LN_MAXFLOAT	8.8722839052068e+01 */
/* LN_MINFLOAT	-1.03278929903432e+02 */
/*  */
/* they are **not** consistent with */
/* "IEEE Standard for Binary Floating-Point Arithmetic," ANSI/IEEE */
/* Standard 754-1985, An American National Standard, August 12, 1985. */
/* as implemented in Genera 8.3 - Symbolics Lisp Machine Operating system */
/* the Genera 8.3 values are used here in the defines below. 04jan95 wmt */

#define square(x)( (x) * (x) )

/* aju 980612: min and max are already defined in MSVC
    Also, In win32, we use rand in place of lrand48, therefore
    change srand48 to srand.*/
#ifdef _MSC_VER
#define srand48 srand
#endif

#define TRUE                             1
#define FALSE                            0
/* #define MAXINT				 32767 */
/* #define DBG_LL                           0 */
#define LN_SINGLE_PI                     1.1447298858494002
#define ABSOLUTE_MIN_CLASS_WT            2.1
#define MIN_CLASS_WT_FACTOR              0.001
/* next line to globals.c to prevent CodeCenter warning 622 for */
/* max(SINGLE_FLOAT_EPSILON, LIKELIHOOD_TOLERANCE_RATIO)) */
/* #define LIKELIHOOD_TOLERANCE_RATIO       0.00001 */
/*  */
/* #define SINGLE_FLOAT_EPSILON             1.1920929e-7 */
/* #define DOUBLE_FLOAT_EPSILON             2.220446049250313e-16 */
/* #define LEAST_POSITIVE_SHORT_FLOAT       1.4012985e-45 */
/* #define LEAST_POSITIVE_SINGLE_FLOAT      1.1754944e-38 */
/* #define LEAST_NEGATIVE_SINGLE_FLOAT      -1.4012985e-45 */
/* #define MOST_POSITIVE_LONG_FLOAT         4.494232837155787e+307 */
/* #define MOST_POSITIVE_SINGLE_FLOAT       3.4028232e+38 */
/* #define MOST_NEGATIVE_SINGLE_FLOAT       -3.4028232e38 */
/* #define MOST_NEGATIVE_SINGLE_FLOAT_DIV_2 -1.7014116e38 */
/* #define MOST_NEGATIVE_LONG_FLOAT         -4.494232837155787e+307 */
/* #define INFINITY                         3.4028232e+38 */
/* replace above with defines from Symbolics Genera 8.3 04jan95 wmt */
#define SINGLE_FLOAT_EPSILON             5.960465e-8
#define DOUBLE_FLOAT_EPSILON             1.1102230246255157e-16
#define LEAST_POSITIVE_SHORT_FLOAT       1.1754944e-38
#define LEAST_POSITIVE_SINGLE_FLOAT      1.1754944e-38
#define LEAST_POSITIVE_LONG_FLOAT        2.2250738585072014e-308
#define LEAST_NEGATIVE_SINGLE_FLOAT      -1.1754944e-38
#define MOST_POSITIVE_LONG_FLOAT         1.7976931348623157e308
#define MOST_POSITIVE_SINGLE_FLOAT       3.4028235e38
#define MOST_NEGATIVE_SINGLE_FLOAT       -3.4028235e38
#define MOST_NEGATIVE_LONG_FLOAT         -1.7976931348623157e308

//#ifndef MACOSX
 // #define INFINITY                         3.4028235e38
//#endif

/* end replaces from Symbolics Genera 8.3 */
#define MOST_NEGATIVE_SINGLE_FLOAT_DIV_2 (MOST_NEGATIVE_SINGLE_FLOAT / 2.0)
#define LEAST_POSITIVE_SINGLE_LOG        (log( LEAST_POSITIVE_SINGLE_FLOAT) + 0.00001)
#define LEAST_POSITIVE_LONG_LOG          (log( LEAST_POSITIVE_LONG_FLOAT) + 0.00001)
#define MOST_POSITIVE_SINGLE_LOG ((log( MOST_POSITIVE_SINGLE_FLOAT / 2.0) + log ( 2.0)) - 0.00001)
#define MOST_POSITIVE_LONG_LOG ((log( MOST_POSITIVE_LONG_FLOAT / 2.0) + log ( 2.0)) - 0.00001)
#define LN_1_DIV_ROOT_2PI                -0.9189385
#define ARRAY_RANK_LIMIT                 65536
#define STRLIMIT                         160
#define SEARCH_LOG_FILE_TYPE             ".log"
#define REPORT_LOG_FILE_TYPE             ".rlog"
#define SEARCH_FILE_TYPE                 ".search"
#define RESULTS_FILE_TYPE                ".results"
#define DATA_FILE_TYPE                   ".db2"
#define HEADER_FILE_TYPE                 ".hd2"
#define MODEL_FILE_TYPE                  ".model"
#define FLOAT_UNKNOWN                    MOST_NEGATIVE_SINGLE_FLOAT 
#define INT_UNKNOWN                      -32767
#define DISPLAY_WTS                      FALSE   /* 5nov97 for testing jcs */
/* #define DISPLAY_WTS                      TRUE    5nov97 for testing jcs */
#define DISPLAY_PROBS                    FALSE   /* 19nov97 jcs - for testing */
/* #define DISPLAY_PROBS                    TRUE    19nov97 jcs - for testing */
#define DISPLAY_PARAMS                   FALSE   /* 19nov97 jcs - for testing */
/* #define DISPLAY_PARAMS                   TRUE    19nov97 jcs - for testing */
#define SN_CM_SIGMA_SAFETY_FACTOR        5.0
#define SN_CN_SIGMA_SAFETY_FACTOR        5.0

/* #define ATT_FLENGTH                   2              15dec94 wmt */
/* #define T_LENGTH			 0              15dec94 wmt */
#define NUM_ATT_TYPES 			5
#define SIZEOF_ABOVE_CUT_TABLE          31
#define SIZEOF_CUT_WHERE_ABOVE_TABLE    31

/* ADDED BY WMT */

#define SEARCH_PARAMS_FILE_TYPE         ".s-params"
#define REPORTS_PARAMS_FILE_TYPE        ".r-params"
#define CHECKPOINT_FILE_TYPE            ".chkpt"
#define TEMP_CHECKPOINT_FILE_TYPE       ".chkpt-tmp"
#define INFLU_VALS_FILE_TYPE            ".influ-text-"
#define XREF_CLASS_FILE_TYPE            ".class-text-"
#define XREF_CASE_FILE_TYPE             ".case-text-"
#define TEMP_SEARCH_FILE_TYPE           ".search-tmp"
#define TEMP_RESULTS_FILE_TYPE          ".results-tmp"
#define RESULTS_BINARY_FILE_TYPE        ".results-bin"
#define TEMP_RESULTS_BINARY_FILE_TYPE   ".results-tmp-bin"
#define CHECKPOINT_BINARY_FILE_TYPE     ".chkpt-bin"
#define TEMP_CHECKPOINT_BINARY_FILE_TYPE ".chkpt-tmp-bin"
#define PREDICT_FILE_TYPE               ".predict"
#define END_OF_INT_LIST                 -999
#define MAX_N_START_J_LIST              26
#define MAX_CLASS_REPORT_ATT_LIST       21
#define MAX_CLSF_N_LIST                 11
#define MAX_N_SIGMA_CONTOUR_LIST        30
/* ^^: including end of list token END_OF_INT_LIST */
#define ALL_ATTRIBUTES                  999
#define SHORT_STRING_LENGTH             41
/* wmt
#define VERY_LONG_STRING_LENGTH         20000 
#define VERY_LONG_TOKEN_LENGTH          500
*/
/* ^^: for get_line_tokens, e.g. 200 real valued attributes per datum */
#define VERY_LONG_STRING_LENGTH         2000000
#define VERY_LONG_TOKEN_LENGTH          50000

#define DATA_ALLOC_INCREMENT            1000
/* ^^: used in read_data and xref_get_data */
#define REL_ERROR                       0.01
/* ^^: used to test equality with percent_equal & find_duplicate */
/* data vector initialized with values UNINIT_DATA_VALUE (float_val) => -1.234560003215e-33   */
/* UNINIT_DATA_VALUE => -1.234560000000e-33 */
/* (float_val == UNINIT_DATA_VALUE) */
/* = 0 */
/* (percent_equal(float_val, UNINIT_DATA_VALUE, REL_ERROR)) */
/* = 1 */
#define NUM_TRANSFORMS                  2
/* ^^: used as length of G_transforms  */
#define NUM_TOKENS_IN_FXLSTR            ((int) floor( (double) STRLIMIT / 15.0))
/* ^^: used in write/read_vector/matrix_float/integer so that "lines" of vectors */
/*     and matricies do not exceed STRLIMIT -- 15.0 assumes format of "%.7e " */
#define WRITE_PERMISSIONS               0664
#define DATA_BINARY_FILE_TYPE           ".db2-bin"

/* Solaris math.h under gcc does not define M_PI - 26apr95 */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif

/* On Macintosh this is not defined 12jun95 wmt */
/* MAXPATHLEN defines the longest permissable path length, */
/* including the terminating null, after expanding symbolic links. */
#ifndef MAXPATHLEN
#define	MAXPATHLEN	1024
#endif

/* RESULTS_DATA_TYPES 13mar95 wmt: new enumerated data types for binary i/o  */
enum results_data_types
{ INT_TYPE, CHAR_TYPE, FLOAT_TYPE, DOUBLE_TYPE,
    CLASS_TYPE, 
    TERM_TYPE, 
    WARN_ERR_TYPE,
    REAL_STATS_TYPE, 
    DISCRETE_STATS_TYPE,
    DUMMY_STATS_TYPE,
    ATT_TYPE, 
    DATABASE_TYPE, 
    MODEL_TYPE,
    CLASSIFICATION_TYPE, 
    CHECKPOINT_TYPE,
    TPARM_TYPE
} ;

/* END OF ADDED BY WMT */

typedef float *fptr;
/* typedef char string[STRLIMIT]; this has been replaced by fxlstr*/
typedef char fxlstr[STRLIMIT];
typedef struct priors *priors_DS;
typedef struct class *class_DS;
typedef struct term *term_DS;
typedef struct warn_err *warn_err_DS;
typedef struct real_stats *real_stats_DS;
typedef struct discrete_stats *discrete_stats_DS;
typedef struct att *att_DS;
typedef struct database *database_DS;
typedef struct model *model_DS;
typedef struct classification *clsf_DS;
typedef struct search_try *search_try_DS;
typedef struct search *search_DS;

/* ADDED BY WMT */

typedef char shortstr[SHORT_STRING_LENGTH];
typedef char very_long_str[VERY_LONG_STRING_LENGTH];
typedef struct checkpoint *chkpt_DS;
typedef struct reports *rpt_DS;
typedef struct sort_cell *sort_cell_DS;
typedef struct invalid_value_errors *invalid_value_errors_DS;
typedef struct incomplete_datum *incomplete_datum_DS;
typedef struct i_discrete *i_discrete_DS;
typedef struct i_integer *i_integer_DS;
typedef struct i_real *i_real_DS;
typedef struct xref_data *xref_data_DS;
typedef struct report_attribute_string *rpt_att_string_DS;
typedef struct ordered_influence_values *ordered_influ_vals_DS;
typedef struct formatted_p_p_star *formatted_p_p_star_DS;
typedef int *int_list;
/* int_list is terminated by END_OF_INT_LIST element */

/* END OF ADDED BY WMT */


#include "params.h"


/* IMPORTANT NOTE: IF ANY CHANGES ARE MADE TO STRUCT DEFINITIONS, AND YOU WANT THOSE */
/* SLOTS TO BE SAVED IN THE .RESULTS & .SEARCH FILES, AND READ IN FOR AC-SEARCH RESTARTS */
/* OR AC-REPORTS -- YOU MUST MAKE APPROPRIATE CHANGES TO IO-RESULTS.C (WRITE/READ-<.._DS) */
/* OR SEARCH-CONTROL.C (WRITE_SEARCH_DS & WRITE_SEARCH_TRY_DS)  18nov94 wmt */

struct priors { /* used only for sn-cm and sn-cn params*/
   float known_prior;                    /* Prior prob that values are known. */
   float sigma_min;                /* Min. bound on prior standard deviation. */
   float sigma_max;                /* Max. bound on prior standard deviation. */
   float mean_mean;         /* The mean of prior dist. of the attribute mean. */
   float mean_sigma;        /* Std Dev. of prior dist. of the attribute mean. */
   float mean_var;          /* Variance of prior dist. of the attribute mean. */
   float minus_log_log_sigmas_ratio;
   float minus_log_mean_sigma;
};


/* STRUCT CLASS
   29mar95 wmt: log_a_w_s_h_pi_theta & log_a_w_s_h_j: float => double
   */
struct class {
   float w_j;            /* Sum of weights in class. */
   float log_w_j;        /* Log of w_j */
   float pi_j;           /* Class probability parameter(~w_j/n-data). */
   float log_pi_j;       /* Log of class probability parameter. */
                         /* Log aprox-LH of Stats. WRT Hypo. & parameters. */
   double log_a_w_s_h_pi_theta;
   double log_a_w_s_h_j;  /* Log aprox-marginal-Lh of Stats. WRT class Hypo. */

   int known_parms_p;   /* Flag: Class parameters known & NOT TO BE UPDATED. */
			/* formerly known_params_p changed spelling when changed
					from char 'y' to int TRUE/FALSE*/
   int num_tparms;
   tparm_DS *tparms;

   int num_i_values;
   void **i_values;     /* N-attributes vector of influence value structures.
                         float * => void ** 06feb95 wmt*/
   float i_sum;         /* Sum of influence values over the attributes. */
   float max_i_value;   /* The maximum of the attribute influence values. */

   int num_wts;         /* Number of weights in the weight vector. */
   float *wts;          /* N-data vector of object membership probabilities. */

   model_DS model;       /* The class model. */
   class_DS next;	/* link to next class in class store for this model*/
};


struct term {
   shortstr type; /* One of the likelihood fn. terms in MODEL-TERM-TYPES. */
   int n_atts;                           /* Number of attributes in this set. */
   float *att_list; /* List of attributes (by number) in set. See ATT-GROUPS. */
   tparm_DS tparm;
};


struct warn_err {                                                 /* effects: */
   shortstr unspecified_dummy_warning;                  /* attribute definition */
   float *unused_translators_warning;   /* discrete translations - not used 18jan 95 wmt */
   shortstr single_valued_warning;                        /* model term type */
   int num_expander_warnings;
   fxlstr *model_expander_warnings;                        /* model term type */
   int num_expander_errors;
   fxlstr *model_expander_errors;                          /* model term type */
};


struct real_stats {                 /* See #'find-real-stats for constructor. */
   int count;          /* Number of values actually known for this attribute. */
   float mx;                     /* Maximum value of this attribute in data. */
   float mn;                     /* Minimum value of this attribute in data. */
   float mean;                       /* Mean value of this attribute in data. */
   float var;                          /* Variance of this attribute in data. */
};


struct discrete_stats {         /* See #'find-discrete-stats for constructor. */
   int range;       /* Values will run from 0 to range inclusive, 0==unknown. */
   int n_observed;
   int *observed; /* Accumulated number of instances of corresponding values. */
};


/* STRUCT ATT
   15dec94 wmt: replace fxlstr with shortstr to save space
   */

struct att {
   shortstr type;                                        /* One of (Att-types). */
   shortstr sub_type;                  /* One of corresponding (Att-sub-types). */
   shortstr dscrp;                                 /* Description of attribute. */
                       /* One of real-stats-DS or discrete-stats-DS structures. */
   real_stats_DS r_statistics;
   discrete_stats_DS d_statistics;
   int n_props;
   int range;
   float zero_point;            /* type changed from int 13dec94 wmt */
   int n_trans;
   char **translations;        /* ***translations => **translations 02dec94 wmt */
   float rel_error;
   void ***props;                           /* Plist of additional properties */
   warn_err_DS warnings_and_errors;                    /* warn_err_DS structure */
   float error;
   int missing;
};


struct invalid_value_errors {                   /* 29nov94 wmt: new */
  int n_datum;
  int n_att;
  shortstr value;
};

struct incomplete_datum {                      /* 29nov94 wmt: new */ 
  int n_datum;
  int datum_length;
};


struct database {    /* DJC - combination of database and compressed-database */
   fxlstr data_file;                                  /* The data file's name. */
   fxlstr header_file;                              /* The header file's name. */
   int n_data;             /* The number of data, used in compressed-database */
   int n_atts;       /* The number of attributes, used in compressed-database */
               /* Number of attributes used in the source to describe a data. */
   int input_n_atts;
   int allo_n_atts;
   int compressed_p;
	         /* Ordered N-atts vector of att-DS describing the attributes. */
   att_DS *att_info;
   float **data;     /* N-data vector of N-atts vectors, one for each object. */
   int *datum_length;     /* N-data vector, one for each object. 28nov94 wmt */
   /* int **map;               Bitmap structure for displaying data. 29nov94 */
   char separator_char;     /* additional data token separator (white space) */
   char comment_char;              /* additional data base comment character */
   char unknown_token;           /* additional data base unknown value token */
	                              /* add to MISSING-VALUE-REPRESENTATIONS */
   /* fxlstr data_syntax;       data base syntax: vector, list, or :line 29nov94 wmt */
   int num_tsp;         /* attribute's whose discrete translations were supplied */
   int *translations_supplied_p;
   int num_invalid_value_errors;        /* renamed from num_ive 29nov94 wmt */
   invalid_value_errors_DS *invalid_value_errors; /* type = real attributes only 29nov94 wmt */
   int num_incomplete_datum;        /* added 29nov94 wmt */
   incomplete_datum_DS *incomplete_datum; /* type = real attributes only 29nov94 wmt */
};


/* STRUCT MODEL
   23oct94 wmt: replace fxlstr with shortstr to save space
   18nov94 wmt: add data_file, header file, compressed, & n_data for compressed
                state when database is null
   */
struct model {             /* DJC - combination of model and compressed model */
                     /* 1 when model has been expanded by Expand-Model-Terms. */
   shortstr id;
   int expanded_terms;
   fxlstr model_file;                                /* The model source file. */
   int file_index;                           /* Index of model in model-file. */
   database_DS database;                    /* DB to which this model applies. */
   fxlstr data_file;            /* The data file's name - compressed model only  */
   fxlstr header_file;          /* The header file's name - compressed model only */
   int n_data;                  /* number of data in data_file - compressed model only */
   int compressed_p;              /* TRUE if compressed - compressed model only */
                          
   int n_terms;                /* Number of <active> independent terms in a model. */       
   term_DS *terms;                  /* Vector of term-DS, of length >= N-terms. */
   int n_att_locs;
   shortstr *att_locs;    /* N-atts vector of terms attribute location indices. */
   int n_att_ignore_ids;
                   /* N-atts vector of symbols denoting source of ignore term */
   shortstr *att_ignore_ids;

   int num_priors;
   priors_DS *priors;  /* Model priors coresponding to DB. for sn-cm and sn-cn */
   int num_class_store;
   class_DS class_store;  /* now this is a pointer to first class available for 
                        re-use or NULL if none; was  Fill-pointer vec of classes
                        stored for reuse. */
   clsf_DS global_clsf;	 /* A single class classification for this model. */
};


/* STRUCT CHECKPOINT
   13nov94 wmt: added this functionality

   to save search status between multiple runs to complete one trial
   */
struct checkpoint {
  int accumulated_try_time; 	/* stored by checkpoint_clsf  */
  int current_try_j_in; 	/* stored by try_variation */
  int current_cycle;            /* stored by search try_function */
};


/* STRUCT REPORTS
   20jan95 wmt: added this functionality from ac-x

   to save influence value calculations for reports
   */
struct reports {
  fxlstr current_results;       /* pathname of clsf results file (if it exists) */
  int   n_class_wt_ordering;
  int   *class_wt_ordering;     /* mapping between clsf class numbering & report */
                                /* class numbering: map_class_num/clsf_>report */
				/*                  map_class_num/report_>clsf */
  char  ***att_model_term_types; /* attribute model term types: array of length n_classes */
  				  /* whose elements are arrays of length n_attributes */
				  /* which contain a list of model term type & its mnemomic */
  float max_class_strength;     /* max strength value for all classes */
  float *class_strength;        /* strength value for each class */
  int   *datum_class_assignment; /* vector of most probable class for each datum _ n_data long */
  float *att_i_sums;            /* Sum over the classes of influence values for each attribute */
  float att_max_i_sum;          /* Maximum of att_i_sums */
  float *att_max_i_values;      /* Max I-value over all classes for each attribute */
  float max_i_value;            /* Maximum of att_max_i_values */
};


/* STRUCT CLASSIFICATION
   20jan95 wmt: eliminate n_duplicates and cycle_count
                move att_i_sums, att_max_i_sum, att_max_i_values and
                max_i_value into reports_DS
   29mar95 wmt: log_p_x_h_pi_theta & log_a_x_h: float ==> double
   */
struct classification { /* also commnly referred to as a clsf */
   double log_p_x_h_pi_theta;
   double log_a_x_h;
   database_DS database;
   int num_models;
   model_DS *models;
   int n_classes;
   class_DS *classes;
   float min_class_wt;
   rpt_DS reports;
   clsf_DS next;	/* for clsf_store linkage*/
   chkpt_DS checkpoint;
};


struct search_try {
  int n;           /* trial number.  this minus 1 trials have happened before
		       this one */
  int time;        /* how long this trial took internally, ignoring overhead
                      of saving, etc. */
  int j_in;        /* the number of classes this trial started with */
  int j_out;       /* the number of classes this trial ended with */
  double ln_p;      /* the probability of this classification and the data */
  int n_duplicates;
  /* a list of tries happened after this one came up with the same clsf */
  search_try_DS *duplicates;
  clsf_DS clsf;     /* the clsf this try came up with */
  /* added 18feb98 wmt */
  int num_cycles;   /* number of cycles needed to converge */
  int max_cycles;   /* .s-params value of max_cycles; if num_cycles ==
                        max_cycles, trial was terminated prior to convergence */
};


/*                       SEARCH STATES

   they can be saved and the reinvoked to continue the search.
   need to save at least one clsf in results when save a search file. */

struct search {
   int n;             /* the number of trials so far */
   int time;          /* the total time spent in previous, excluding this one */
   int n_dups;        /* number of times have found a duplicate clsf */
               /* number of times compared two clsfs to see if they were same */
   int n_dup_tries;
   search_try_DS last_try_reported;
   int n_tries;                        
   search_try_DS *tries;        /* an ordered list of search tries, from best on down */
   int_list start_j_list;       /* current state of start_j_list - for restarts */
   int n_final_summary;         /* for search_summary (intf-reports.c) */
   int n_save;                  /* for search_summary (intf-reports.c) */
};
  

struct sort_cell {                      /* 03feb95 wmt: new */ 
  float float_value;
  int int_value;
};


struct i_discrete {                /* 06feb95 wmt: new, sm modle */
  float influence_value;
  int n_p_p_star_list;             /* number of items in p_p_star_list */
  float *p_p_star_list;            /* triplets of term_index, local, & global
                                      probabilities */
};


struct i_integer {                /* 06feb95 wmt: new, not implemented yet */
  float influence_value;
  int n_mean_sigma_list;             /* number of items in mean_sigma_list */
  float *mean_sigma_list;            /* quadruplets of local_mean, local_sigma,
                                        global_mean, & global_sigma */
};


struct i_real {                    /* 06feb95 wmt: new, sn_cn, sn_sm, & mn_cn models */
  float influence_value;
  int n_mean_sigma_list;             /* number of items in  mean_sigma_list */
  float *mean_sigma_list;            /* for sn_cn (4 values): local_mean, local_sigma,
                                        global_mean, & global_sigma.
                                        for sn_cm (6 values): local_mean, local_sigma,
                                        local_known_prob, global_mean, global_sigma, &
                                        global_known_prob.
                                        for mn (4 values): */
  int n_term_att_list;
  fptr *class_covar;                  /* class covariance matrix  - MN only */
  fptr term_att_list;                 /* term attribute list  - MN only */
};


struct xref_data {              /* 07feb95 wmt: new, for class & case reports */
  int class_case_sort_key;      /* (n_class * num_data) + n_case  */
  int case_number;              /* one-based data case number for this datum */
  int n_attribute_data;         /* number of attribute_data items, specified by
                                   xref_class_report_att_list */
  shortstr *discrete_attribute_data;  /* list of strings - discrete input values */
  float *real_attribute_data;   /* list of floats - real input values */
  int n_collector;              /* number of wt_class_pairs */
  sort_cell_DS wt_class_pairs;  /* pairs of probability weights and classes for this datum */
};

                                   
struct report_attribute_string {   /* 09feb95 wmt: new, for class reports */
  int att_number;
  shortstr att_dscrp;
  int dscrp_length;             /* actually the max of the lengths of the attribute
                                   values */
};


struct ordered_influence_values {       /* 13feb95 wmt: new, for influence_values_header
                                                        for each attribute*/
 float att_i_sum;                       /* sort key */
 int n_att;
 char *att_dscrp_ptr;
 char *model_term_type_ptr;
 float norm_att_i_sum;                  /* normalized by  max_i_sum */
};


struct formatted_p_p_star {             /* 16feb95 wmt: for format_discrete_attribute */
  shortstr discrete_string_name;
  float abs_att_value_influence;        /* sort key -- absolute value */
  float att_value_influence;            
  float local_prob;
  float global_prob;
};


/************** FUNCTION PROTOTYPES ****************************************/

/*  system functions for which standard header location is not known */

#ifndef _WIN32
void srand48(long seedval);
double drand48();
#endif
long lrand48();

/* file init.c */

void init (void);

void init_properties(void);


/* file io-read-data.c */

void check_stop_processing( int total_error_cnt, int total_warning_cnt,
                           FILE *log_file_fp, FILE *stream);

void define_data_file_format( FILE *header_file_fp, FILE *log_file_fp, FILE *stream);

void process_data_header_model_files( FILE *log_file_fp, int regenerate_p, FILE *stream,
	database_DS db, model_DS *models, int num_models,
	int *total_error_cnt, int *total_warning_cnt);

void log_header( FILE *log_file_fp, FILE *stream, char *data_file_ptr, char *header_file_ptr,
                char *model_file_ptr, char *log_file_ptr);

database_DS read_database( FILE *header_file_fp, FILE *log_file_fp,
                          char *data_file_ptr, char *header_file_ptr, int max_data,
                          int reread_p, FILE *stream);

int check_for_non_empty( att_DS *atts, int n_atts);

void check_data_base( database_DS d_base, int n_data);

char *output_warning_msgs( int n_att, att_DS att, database_DS db, model_DS model);

char *output_error_msgs( int n_att, att_DS att);

void output_message_summary( 
   int unspecified_dummy_warning_cnt,
   int ignore_model_term_warning_cnt,
   int unused_translators_warning_cnt,
   int incomplete_datum_cnt,
   int single_valued_warnings_cnt,
   int invalid_value_errors_cnt,
   int model_expander_warning_cnt,
   int model_expander_error_cnt,
   int *total_error_cnt,
   int *total_warning_cnt,
   FILE *log_file, FILE *stream, int output_p);

void output_messages( database_DS db, model_DS *models, int num_models, FILE *log_file,
                     FILE *stream, int *total_error_cnt, int *total_warning_cnt,
                     char *output_msg_type_ptr);

void output_db_error_messages( database_DS db, FILE *log_file, FILE *stream, int output_p);

void read_data( database_DS d_base, FILE *data_file_fp, int max_data,
               char *data_file_ptr, FILE *log_file_fp, FILE *stream);

void define_attribute_definitions( FILE *header_file_fp, char *header_file_ptr,
                                  FILE *log_file_fp, FILE *stream);

void process_attribute_definitions( database_DS d_base, FILE *header_file_fp,
                                   char *header_file_ptr, FILE *log_file_fp,
                                   FILE *stream);

att_DS process_attribute_def( int att_num, int *input_error, char **tokens,
                             int num_tokens, FILE *log_file_fp, FILE *stream);

att_DS create_att_DS( int att_num, int *input_error_ptr, int range, double rel_error,
                     double error, double zero_point, char *type_ptr, char *sub_type_ptr,
                     char *dscrp_ptr, FILE *log_file_fp, FILE *stream);

warn_err_DS create_warn_err_DS(void);

/* 
void define_discrete_translations( char ***discrete_translations, int num, 
                                  database_DS data_base);
*/

char ***expand_att_list( char ***att_list, int num, int *nlength);

int find_str_in_list( char *str, char **translations, int num);

/*
void process_discrete_translations( database_DS d_base, char ***value_translations, 
					       int vlength);
*/

void process_translation_msgs( int *translations_not_provided,  int num,
		char *default_translation, att_DS *att_info, FILE *stream);

char **process_translation( database_DS d_base, int n_att, att_DS att_dscrp,
				    int nat,   char **att_translation);

char **read_data_doit( database_DS d_base, FILE *data_file, int first_read,
                      int *instance_length_ptr, int n_comment_chars, char *comment_chars,
                      int binary_instance_length, float **binary_instance);

float *translate_instance( database_DS d_base, char **instance, int instance_length,
                          int n_datum, FILE *log_file_fp, FILE *stream);

double translate_real( database_DS d_base, int n_datum, int n_att, char *value);

int translate_discrete( database_DS d_base, int n_att, att_DS attribute, 
				   char *value, FILE *log_file_fp, FILE *stream);

char **get_line_tokens( FILE *stream, int separator_char, int n_comment_chars,
                       char *comment_chars, int first_read, int *instance_length_ptr);

int read_from_string( char *s1, char *s2, int string_limit, int separator_char,
                       int n_comment_chars, char *comment_chars, int position);

int read_line( char *s, int string_limit, FILE *stream);

/* void read_dscrp(FILE *stream, fxlstr dscrp); does not exist 15dec94 wmt */

void find_att_statistics( database_DS d_base, FILE *log_file_fp, FILE *stream);

void find_real_stats( database_DS d_base, int n_att, FILE *log_file_fp, FILE *stream);

void store_real_stats( real_stats_DS statistics, att_DS att,
   int count, double mean, double variance, int missing, double  mx, double mn);

void find_discrete_stats( database_DS d_base, int n_att);

void output_att_statistics( database_DS d_base, FILE *log_file_fp, FILE *stream);

void output_real_att_statistics( database_DS d_base, int n_att, FILE *log_file_fp,
                                FILE *stream);

void output_created_translations( database_DS d_base, FILE *log_file_fp, FILE *stream);

void check_errors_and_warnings( database_DS database, model_DS *models, int num_models);


/* file io-read-model.c */


model_DS *read_model_file( FILE *model_file_fp, FILE *log_file_fp, database_DS d_base,
		   int regenerate_p, int expand_p, FILE *stream, int *newlength,
                   char *model_file_ptr);

char ***read_model_doit( FILE *model_file_fp, int **sizes, int *num, int model_index,
                        int first_read, FILE *log_file_fp, FILE *stream);

char ***read_lists( FILE *stream, int **sizes, int *num);

char **read_list( FILE *stream, int *num);

model_DS *define_models(char ****model_groups, database_DS d_base, char *source, 
	     FILE *stream, int expand_p, int regenerate_p, int num_model_groups,
             int *newnum, int *num_groups, int **sizes, FILE *log_file_fp);

void generate_attribute_info( model_DS model, char ***model_group, int i_model,
		int num_groups, int *sizes, database_DS d_base, FILE *log_file_fp,
                FILE *stream);

void extend_terms_single( char *model_type, char **list, int size, model_DS model,
                  int model_index, FILE *log_file_fp, database_DS d_base, FILE *stream);

void extend_terms_multi( char *model_type, char **list, int size, model_DS model,
                  int model_index, FILE *log_file_fp, database_DS d_base, FILE *stream);

void extend_default_terms( char *model_type, int i_model, model_DS model, 
				database_DS d_base, FILE *log_file_fp, FILE *stream);

void read_model_reset( model_DS model);

void set_ignore_att_info( model_DS model, database_DS d_base);

int *get_sources_list( int *att_index_list, int num,  att_DS *att_info,
			   int *traced, int n_traced);

int int_cmp(int x,int  y);

int *get_source_list( int att_index, att_DS *att_info, int *traced,int n_traced,
                     int *n_source);

int exist_intersection( int *fl1,int *fl2,int l1,int l2);

char ***canonicalize_model_group( char ***model_group);

void print_att_locs_and_ignore_ids( model_DS model, int model_index);


/* file io-results.c */


void compress_clsf( clsf_DS clsf, model_DS dbmodel, int want_wts_p);

clsf_DS expand_clsf( clsf_DS clsf, int want_wts_p, int updatewts);

void expand_clsf_models( clsf_DS clsf);

void expand_clsf_wts( clsf_DS clsf, float **wts_vector, int num_wts);

void save_clsf_seq( clsf_DS *clsf_seq, int num, char *save_file_ptr,
                   unsigned int save_compact_p, char *results_or_chkpt);

void write_clsf_seq( clsf_DS *clsf_seq, int num, FILE *stream);

void write_clsf_DS( clsf_DS clsf, FILE *stream, int clsf_num);

void write_database_DS( database_DS database, FILE *stream);

void write_att_DS( att_DS att_info, int n_att, FILE *stream);

void write_model_DS( model_DS model, int model_num, database_DS database, 
                    FILE *stream);

void write_term_DS( term_DS term, int n_term, FILE *stream);

void write_tparm_DS( tparm_DS term_param, int parm_num, FILE *stream);

void write_mm_d_params(struct mm_d_param *param, int n_atts, FILE *stream);

void write_mm_s_params( struct mm_s_param *param, int n_atts, FILE *stream);

void write_mn_cn_params( struct mn_cn_param *param, int n_atts, FILE *stream);

void write_sm_params( struct sm_param *param, int n_atts, FILE *stream);

void write_sn_cm_params( struct sn_cm_param *param, FILE *stream);

void write_sn_cn_params( struct sn_cn_param *param, FILE *stream);

void write_priors_DS( priors_DS priors, int n_priors, FILE *stream);

void write_class_DS_s( class_DS *classes, int n_classes, FILE *stream);

int make_and_validate_pathname ( char *type, char *file_arg, fxlstr *file_ptr,
                                int validate_p);       

int validate_results_pathname( char *file_pathname, fxlstr *found_file_ptr,
                               char *type, int exit_if_error_p, int silent_p);

int validate_data_pathname( char *file_pathname, fxlstr *found_file_ptr,
                               int exit_if_error_p, int silent_p);

clsf_DS *get_clsf_seq( char *results_file_ptr, int expand_p, int want_wts_p, int update_wts_p,
                      char *file_type, int *n_best_clsfs_ptr, int_list expand_list);

clsf_DS *read_clsf_seq( FILE *results_file_fp, char *results_file_ptr, int expand_p,
                       int want_wts_p, int update_wts_p, int *n_best_clsfs_ptr,
                       int_list expand_list);

clsf_DS read_clsf( FILE *results_file_fp, int expand_p, int want_wts_p, int update_wts_p,
                  int clsf_index, clsf_DS first_clsf, int file_ac_version,
                  int_list expand_list);

database_DS read_database_DS( clsf_DS clsf, FILE *results_file_fp, int file_ac_version);

model_DS read_model_DS( clsf_DS clsf, int model_index, FILE *results_file_fp,
                       int file_ac_version);

void read_class_DS_s( clsf_DS clsf, int n_classes, FILE *results_file_fp,
                     clsf_DS first_clsf, int file_ac_version);

void read_att_DS( database_DS d_base, int n_att, FILE *results_file_fp,
                 int file_ac_version);

void read_tparm_DS( tparm_DS tparm, int n_parm, FILE *results_file_fp,
                   int file_ac_version);

void read_mm_d_params(struct mm_d_param *param, int n_atts, FILE *results_file_fp,
                      int file_ac_version);

void read_mm_s_params( struct mm_s_param *param, int n_atts, FILE *results_file_fp,
                      int file_ac_version);

void read_mn_cn_params( struct mn_cn_param *param, int n_atts,
                       FILE *results_file_fp, int file_ac_version);

void read_sm_params( struct sm_param *param, int n_atts, FILE *results_file_fp,
                    int file_ac_version);

void read_sn_cm_params( struct sn_cm_param *param, FILE *results_file_fp,
                       int file_ac_version);

void read_sn_cn_params( struct sn_cn_param *param, FILE *results_file_fp,
                       int file_ac_version);


/* file io-results-bin.c */

void safe_write( FILE *results_fp, char *data, int data_length, 
                int data_type, char *caller);

void check_load_header( int header_type, int expected_type, char *caller);

void dump_clsf_seq( clsf_DS *clsf_seq, int num, FILE *results_fp);

void dump_clsf_DS( clsf_DS clsf, FILE *results_fp, int clsf_num);

void dump_database_DS( database_DS database, FILE *results_fp);

void dump_att_DS( att_DS att_info, int n_att, FILE *results_fp);

void dump_model_DS( model_DS model, int model_num, database_DS database,
                  FILE *results_fp);

void dump_term_DS( term_DS term, int n_term, FILE *results_fp);

void dump_tparm_DS( tparm_DS term_param, int parm_num, FILE *results_fp);

void dump_mm_d_params(struct mm_d_param *param, int n_atts, FILE *results_fp);

void dump_mm_s_params( struct mm_s_param *param, int n_atts, FILE *results_fp);

void dump_mn_cn_params( struct mn_cn_param *param, int n_atts, FILE *results_fp);

void dump_sm_params( struct sm_param *param, int n_atts, FILE *results_fp);

/* void dump_sn_cm_params( struct sn_cm_param *param, FILE *results_fp); */

/* void dump_sn_cn_params( struct sn_cn_param *param, FILE *results_fp); */

void dump_class_DS_s( class_DS *classes, int n_classes, FILE *results_fp);

clsf_DS *load_clsf_seq( FILE *results_file_fp, char *results_file_ptr, int expand_p,
                       int want_wts_p, int update_wts_p, int *n_best_clsfs_ptr,
                       int_list expand_list);

clsf_DS load_clsf( FILE *results_file_fp, int expand_p, int want_wts_p, int update_wts_p,
                  int clsf_index, clsf_DS first_clsf, int file_ac_version,
                  int_list expand_list);

database_DS load_database_DS( clsf_DS clsf, FILE *results_file_fp, int file_ac_version);

void load_att_DS( database_DS d_base, int n_att, FILE *results_file_fp, int file_ac_version);

model_DS load_model_DS( clsf_DS clsf, int model_index, FILE *results_file_fp,
                       int file_ac_version);

void load_class_DS_s( clsf_DS clsf, int n_classes, FILE *results_file_fp,
                     clsf_DS first_clsf, int file_ac_version);

void load_tparm_DS( tparm_DS tparm, int n_parm, FILE *results_file_fp, int file_ac_version);

void load_mm_d_params( struct mm_d_param *param, int n_atts, FILE *results_file_fp,
                      int file_ac_version);

void load_mm_s_params( struct mm_s_param *param, int n_atts, FILE *results_file_fp,
                      int file_ac_version);

void load_mn_cn_params( struct mn_cn_param *param, int n_atts, FILE *results_file_fp,
                       int file_ac_version);

void load_sm_params( struct sm_param *param, int n_atts, FILE *results_file_fp,
                    int file_ac_version);


/* file matrix-utilities.c */

float *setf_v_v( float *v1, float *v2, int num);
float *incf_v_v(float *v1, float *v2, int num);
float *decf_v_v(float *v1, float *v2, int num);
float *incf_v_vs(float *v1, float *v2, double scale, int num);
float *setf_v_vs(float *v1, float *v2, double scale, int num);
fptr *incf_m_vvs( fptr *m1, float *v1, float *v2, double scale, int num);
double diagonal_product( fptr *m1, int num);
fptr *extract_diagonal_matrix( fptr *m1, fptr *m_diagonal, int num);
void update_means_and_covariance( float **data,int n_data, float *att_indices,
     float *wts, float *est_means, float *means, fptr *covar, float *values, int num);
fptr *n_sm( double scale, fptr *m1, int num);
float *vector_root_diagonal_matrix( fptr *m1, int num);
double dot_vv( float *row, float *col, int num);
double dot_mm( fptr *m1, fptr *m2, int num);
float *collect_indexed_values( float *acc_v, float *index_list, float *values_v, int num);
fptr *copy_to_matrix( fptr *from, fptr *to, int num);
float *n_sv( double scale, float *vec, int num);
fptr *setf_m_ms( fptr *m1, fptr *m2, double scale, int num);
fptr *incf_m_ms( fptr *m1, fptr *m2, double scale, int num);
fptr *limit_min_diagonal_values( fptr *m, float *mins_vec, int num);
fptr *invert_factored_square_matrix( fptr *f1, fptr *m_invert, int num);
double determinent_f(fptr *fs, int num);
double star_vmv( fptr *m, fptr v, int num);
double trace_star_mm( fptr *m1, fptr *m2, int num);
fptr *extract_rhos( fptr *m, int num);
fptr *invert_diagonal_matrix( fptr *m, int num);
fptr *root_diagonal_matrix( fptr *m, int num);
fptr *star_mm( fptr *m1, fptr *m2, int num);
fptr *make_matrix( int num_rows, int num_cols);


/* file model-expander-3.c */

model_DS conditional_expand_model_terms( model_DS model, int force, 
                                        FILE *log_file_fp, FILE *stream);
enum MODEL_TYPES model_type (shortstr str);
model_DS expand_model_terms( model_DS model, FILE *log_file_fp, FILE *stream);
void check_model_terms( model_DS model, FILE *log_file_fp, FILE *stream);
void check_term( term_DS term, model_DS model, int n_term, FILE *log_file_fp, FILE *stream);
void update_location_info( model_DS model, term_DS term, float *old_att_list);
void expand_model_reset(model_DS model);
void update_params_fn( class_DS class, int n_classes, database_DS data_base, int collect);
void arrange_model_function_terms( model_DS model);
double log_likelihood_fn( float *datum, class_DS class, double limit);
double update_l_approx_fn( class_DS class);
double update_m_approx_fn( class_DS class);
int class_equivalence_fn( class_DS class_1, class_DS class_2,
				    double percent_ratio, double sigma_ratio);
tparm_DS *model_global_tparms( model_DS model);


/* model-multi-multinomial-d.c */

/* model-multi-multinomial-s.c */

/* file model-multi-normal-cn.c */

void mn_cn_params_influence_fn( model_DS model, tparm_DS tparm, int term_index, int n_att,
                               float *v_ptr, float *class_mean_ptr, float *class_sigma_ptr,
                               float *global_mean_ptr, float *global_sigma_ptr,
                               float **term_att_list_ptr, int *n_term_att_list_ptr,
                               float ***class_covar_ptr);
tparm_DS make_mn_cn_param( int n_atts);
void multi_normal_cn_model_term_builder( model_DS model, term_DS term, int n_term);
double multi_normal_cn_log_likelihood( tparm_DS tparm);
double multi_normal_cn_update_l_approx( tparm_DS tparm);
double multi_normal_cn_update_m_approx( tparm_DS tparm);
void multi_normal_cn_update_params( tparm_DS tparm, int known_params_p);
int multi_normal_cn_class_equivalence( tparm_DS tparm1, tparm_DS tparm2, double sigma_ratio);
void multi_normal_cn_class_merged_marginal( tparm_DS tparm0, tparm_DS tparm1, tparm_DS tparm,
                                            float wt_0, float wt_1, float wt_m);

						      
/* file model-single-normal-cm.c */

void sn_cm_params_influence_fn( model_DS model, tparm_DS tparm, int term_index,int  n_att,
	float *v, float *class_mean, float *class_sigma, float *class_known_prob,
	float *global_mean, float *global_sigma, float *global_known_prob);
void single_normal_cm_model_term_builder( model_DS model, term_DS term, int n_term);
double single_normal_cm_log_likelihood( tparm_DS tparm);
double single_normal_cm_update_l_approx( tparm_DS tparm);
double single_normal_cm_update_m_approx( tparm_DS tparm);
void single_normal_cm_update_params( tparm_DS tparm, int known_parms_p);
int single_normal_cm_class_equivalence( tparm_DS tparm1,tparm_DS tparm2, double sigma_ratio);
void single_normal_cm_class_merged_marginal( tparm_DS tparm0,tparm_DS tparm1,tparm_DS tparmm);



/* file model-single-normal-cn.c */

void sn_cn_params_influence_fn( model_DS model, tparm_DS tparm, int term_index, int n_att,
   float *v, float *class_mean,float *class_sigma, float *global_mean, float *global_sigma);

void single_normal_cn_model_term_builder( model_DS model, term_DS term, int n_term);
double single_normal_cn_log_likelihood( tparm_DS tparm);
double single_normal_cn_update_l_approx( tparm_DS tparm);
double single_normal_cn_update_m_approx( tparm_DS tparm);
void single_normal_cn_update_params( tparm_DS tparm, int known_parms_p);
int single_normal_cn_class_equivalence( tparm_DS tparm1,tparm_DS tparm2, double sigma_ratio);
void single_normal_cn_class_merged_marginal( tparm_DS tparm0,tparm_DS tparm1, tparm_DS tparmm);



/* file model-single-multinomial.c */

void sm_params_influence_fn( model_DS model, tparm_DS term_params, int term_index,
	 int n_att, float *influence_value, float **class_div_global_att_prob_list_ptr,
         int *length);
void single_multinomial_model_term_builder( model_DS model, term_DS term, int n_term);
double single_multinomial_log_likelihood( tparm_DS tparm);
double single_multinomial_update_l_approx( tparm_DS tparm);
double single_multinomial_update_m_approx( tparm_DS tparm);
void single_multinomial_update_params( tparm_DS tparm, int known_parms_p);
int single_multinomial_class_equivalence( tparm_DS tparm1, tparm_DS tparm2,
                                           double percent_ratio);
void single_multinomial_class_merged_marginal( tparm_DS tparm1, tparm_DS tparm2,
                                               tparm_DS tparmm);


/* file model-transforms.c */

int find_transform( database_DS d_base, shortstr transform, int *att_list, int length,
                   FILE *log_file_fp, FILE *stream);
int find_singleton_transform( database_DS d_base, shortstr transform, int att_index,
                             FILE *log_file_fp, FILE *stream);
int generate_singleton_transform( database_DS d_base, shortstr transform, int att_index,
                                 FILE *log_file_fp, FILE *stream);
att_DS log_transform( int att_index, database_DS d_base);
att_DS log_odds_transform_c( int att_index, database_DS d_base);


/* file model-update.c */

void update_approximations( clsf_DS clsf);
void update_parameters( clsf_DS clsf);
int delete_null_classes(clsf_DS clsf);
void update_wts( clsf_DS training_clsf, clsf_DS test_clsf);
int most_probable_class_for_datum_i( int i,class_DS *classes,  int n_classes);
void update_ln_p_x_pi_theta( clsf_DS clsf, int no_change);


/* file search-basic.c */

clsf_DS generate_clsf( int n_classes, FILE *header_file_fp,
                       FILE *model_file_fp, FILE *log_file_fp, FILE *stream, int reread_p,
                       int regenerate_p, char *data_file_ptr, char *header_file_ptr,
                       char *model_file_ptr, char *log_file_ptr, int restart_p,
                       char *start_fn_type, unsigned int initial_cycles_p, int n_data,
                       int start_j_list_from_s_params);
int random_set_clsf( clsf_DS clsf, int n_classes, int delete_duplicates, int  display_wts,
                    unsigned int initial_cycles_p, FILE *log_file_fp, FILE *stream);
clsf_DS set_up_clsf( int n_classes, database_DS database, model_DS *model_set,
                    int n_models);
void block_set_clsf( clsf_DS clsf, int n_classes, int block_size,
                    int delete_duplicates, int display_wts, unsigned int initial_cycles_p,
                    FILE *log_file_fp, FILE *stream);
int initialize_parameters( clsf_DS clsf, int display_wts, int  delete_duplicates,
                          unsigned int initial_cycles_p, FILE *log_file_fp, FILE *stream);
class_DS *delete_class_duplicates( int *num, class_DS *classes);


/* file search-control.c */

int autoclass_search( char *data_file, char *header_file, char *model_file,
                     char *search_params_file, char *search_file, char *results_file,
                     char *log_file);
int *remove_too_big( int limit, int  *list, int *num);
int too_big( int limit, int  *list, int num);
double within( double min_val, double  x, double  max_val);
search_try_DS *safe_subseq_of_tries( search_try_DS *seq,
		 int begin, int end, int num, int *newnum);
void print_initial_report( FILE *stream, FILE *log_file_fp, int min_report_period,
                          time_t end_time, int max_n_tries, char *search_file_ptr,
                          char *results_file_ptr, char *log_file_ptr, int min_save_period,
                          int n_save);
void print_report( FILE *stream, FILE *log_file_fp, search_DS search, time_t last_save,
                  time_t last_report, int reconverge_p, char *n_classes_explain);
void print_final_report( FILE *stream, FILE *log_file_fp, search_DS search, time_t begin,
   time_t last_save, int n_save, char *stop_reason, unsigned int results_file_p,
   unsigned int search_file_p, int n_final_summary, char *log_file_ptr,
   char *search_params_file_ptr, char *results_file_ptr, clsf_DS clsf, 
   int reconverge_p, time_t last_report, time_t last_trial);

void print_search_try( FILE *stream, FILE *log_file_fp, search_try_DS try, int saved_p,
                      int new_line_p, char *pad, unsigned int comment_data_headers_p);
void empty_search_try( search_try_DS try);
int total_try_time( search_try_DS *tries, int n_tries);
search_try_DS try_variation( clsf_DS clsf, int j_in, int trial_n, char *reconverge_type,
        char *start_fn_type, char *try_fn_type, unsigned int initial_cycles_p,
        time_t begin_try, double halt_range, double halt_factor, double rel_delta_range,
        int max_cycles, int n_average, double cs4_delta_range, int sigma_beta_n_values,
        int converge_print_p, FILE *log_file_fp, FILE *stream);
int search_duration( search_DS search, time_t now, clsf_DS clsf, time_t last_save,
                    int reconverge_p);
int converge( clsf_DS clsf, int n_average, double halt_range, double halt_factor,
	      double delta_factor, int display_wts, int min_cycles, int max_cycles,
              int converge_print_p, FILE *log_file_fp, FILE *stream);
int converge_search_3( clsf_DS clsf, double rel_delta_range, int display_wts,
                       int min_cycles, int max_cycles, int n_average,
                       int converge_print_p, FILE *log_file_fp, FILE *stream);
int converge_search_3a( clsf_DS clsf, double rel_delta_range, int display_wts,
                       int min_cycles, int max_cycles, int n_average,
                       int converge_print_p, FILE *log_file_fp, FILE *stream);
int converge_search_4( clsf_DS clsf, int display_wts, int min_cycles, int max_cycles,
                      double cs4_delta_range, int sigma_beta_n_values,
                      int converge_print_p, FILE *log_file_fp, FILE *stream);
int min_n_peaks( int n_dups, int n_dup_tries);
double avg_time_till_improve( int time_so_far, int n_peaks_seen);
double ln_avg_p( double ln_p_avg, double ln_p_sigma);
double min_best_peak( int min_n_peak, double ln_p_avg, double ln_p_sigma);
int random_j_from_ln_normal( int n_tries, search_try_DS *tries, int  max_j, int explain_p,
                            char *n_classes_explain);
double random_from_normal( double mean, double sigma);
double typical_best(int n_samples, double mean, double sigma);
double cut_where_above( double percent);
double erfc_poly( double z);
double approx_inverse_erfc( double area, double z_try);
double inverse_erfc( double area);
double interpolate( float table[][2], int length, double key);
void upper_end_normal_fit( search_try_DS *tries, int n_tries,
				float *ln_p_avg, float  *ln_p_sigma);
double average( float *list, int length);
double variance( float *list, int length, double avg);
double sigma( float *list, int num, double ln_p_avg);
double avg_improve_delta_ln_p( int n_peaks, double ln_p_sigma);
double  next_best_delta( int n_samples, double sigma);
int min_time_till_best( int time_so_far,int  min_n_peak,int n_peaks_seen);
void save_search( search_DS search, char *search_file_ptr, time_t last_save, clsf_DS clsf,
                 int reconverge_p, int_list start_j_list, int n_final_summary,
                 int n_save);
void write_search_DS( FILE *search_file_fp, search_DS search, int_list start_j_list,
                     int n_final_summary, int n_save);
void write_search_try_DS( search_try_DS try, shortstr id, int try_num, FILE *search_file_fp );
search_DS get_search_DS(void);
search_DS copy_search_wo_tries( search_DS search);
search_DS reconstruct_search( FILE *search_file_fp, char *search_file_ptr,
                             char *results_file_ptr);
search_DS get_search_from_file( FILE *search_file_fp, char *search_file_ptr);
void get_search_try_from_file( search_DS search, search_try_DS parent_try,
                              int try_index, FILE *search_file_fp, char *search_file_ptr);
int find_duplicate( search_try_DS try, search_try_DS *tries, int n_store, int *n_dup_tries_ptr,
                   double rel_error, int n_tries, int restart_p);
search_try_DS *insert_new_trial( search_try_DS try, search_try_DS *tries,
			     int n_tries, int n_store, int max_n_store);
void describe_clsf( clsf_DS clsf, FILE *stream, FILE *log_file_fp);
void print_log (double log_number, FILE *log_file_fp, FILE *stream, int verbose_p);
void apply_search_start_fn (clsf_DS clsf, char *start_fn_type,
                            unsigned int initial_cycles_p, int j_in, FILE *log_file_fp,
                            FILE *stream);
int apply_search_try_fn (clsf_DS clsf, char *try_fn_type, double halt_range,
                         double halt_factor, double rel_delta_range, int max_cycles,
                         int n_average, double cs4_delta_range, int sigma_beta_n_values,
                         int converge_print_p, FILE *log_file_fp, FILE *stream);
int apply_n_classes_fn ( char *n_classes_fn_type, int n_tries, search_try_DS *tries,
                         int  max_j, int explain_p, char *n_classes_explain);
int validate_search_start_fn (char *start_fn_type);
int validate_search_try_fn (char *try_fn_type);
int validate_n_classes_fn (char *n_classes_fn_type);
void describe_search( search_DS search);


/* file search-converge.c */

double base_cycle( clsf_DS clsf, FILE *stream, int display_wts, int converge_cycle_p);


/* file statistics.c */

void central_measures_x( float **data, int n_data, int  n_att, float *wts, double est_mean,
		       float *unknown, float *known, float *mean, float *variance,
		       float *skewness, float *kurtosis);


/* file struct-class.c */

void store_class_DS( class_DS cl, int max_n_classes);
class_DS get_class_DS( model_DS model, int n_data, int  want_wts_p, int check_model);
class_DS pop_class_DS( model_DS model, int n_data, int want_wts_p);
class_DS build_class_DS( model_DS model, int n_data, int want_wts_p);
class_DS build_compressed_class_DS( model_DS comp_model);
class_DS copy_class_DS(class_DS from_class, int n_data, int want_wts_p);
/*JTP, classes_to_check) JTP*/
class_DS copy_to_class_DS(class_DS from_class, class_DS to_class, int n_data, int want_wts_p);
int class_DS_test( class_DS cl1, class_DS cl2, double rel_error);
tparm_DS copy_tparm_DS(tparm_DS old);
void free_class_DS( class_DS class, char *type, clsf_DS clsf, int i_class);
void free_tparm_DS( tparm_DS tparm);
void **list_class_storage ( int print_p);
double class_strength_measure( class_DS class);

/* file struct-clsf.c */

void push_clsf( clsf_DS clsf);
clsf_DS pop_clsf(void);
clsf_DS get_clsf_DS( int n_classes);
void adjust_clsf_DS_classes( clsf_DS clsf, int n_classes);
void display_step( clsf_DS clsf, FILE *stream);
clsf_DS create_clsf_DS( void);
int clsf_DS_max_n_classes( clsf_DS clsf);
clsf_DS copy_clsf_DS( clsf_DS cold, int want_wts_p);
int clsf_DS_test( clsf_DS clsf1, clsf_DS clsf2, double rel_error);
void store_clsf_DS_classes( clsf_DS clsf, class_DS *check_classes, int length);
void store_clsf_DS( clsf_DS clsf, class_DS *check_classes, int length);
float *clsf_DS_w_j( clsf_DS clsf);
void **list_clsf_storage ( clsf_DS clsf, search_DS search, int print_p, int list_global_clsf_p);
void free_clsf_DS( clsf_DS clsf);
char *clsf_att_type( clsf_DS clsf, int n_att);
void free_clsf_class_search_storage( clsf_DS clsf, search_DS search, int list_global_clsf_p);

/* struct-data.c */

database_DS find_database( char *data_file_ptr, char *header_file_ptr, int n_data);
int db_DS_same_source_p( database_DS db1, database_DS db2);
int every_db_DS_same_source_p( database_DS db1, model_DS *models);
database_DS compress_database( database_DS db);
int db_DS_equal_p( database_DS db1, database_DS db2);
int att_DS_equal_p( att_DS att1, att_DS att2);
database_DS create_database( void);
database_DS expand_database( database_DS comp_database);
int extend_database( database_DS db, database_DS comp_db);
int db_same_source_p( database_DS db, database_DS comp_db);
int att_info_equal( database_DS db, database_DS comp_db);
int att_props_equivalent_p( att_DS att_1, att_DS att_2);
int att_stats_equivalent_p( att_DS att_1, att_DS att_2);


/* file struct-matrix.c */

fptr *compute_factor( fptr *factor, int num);
float *solve( fptr *fs, float *b, int num);


/* file struct-model.c */

model_DS find_similar_model( char *model_file, int file_index, database_DS database);
int model_DS_equal_p( model_DS m1, model_DS m2);
model_DS expand_model( model_DS comp_model);
model_DS find_model( char *model_file_ptr, int file_index, database_DS database);
void free_model_DS( model_DS model, int i_model);


/* utils.c */

void to_screen_and_log_file( fxlstr msg, FILE *log_file_fp, FILE *stream, int output_p);
time_t get_universal_time(void);
char *format_universal_time(time_t universal_time);
char *format_time_duration (time_t delta_universal_time);   
int iround( double number);  
/* long int round_osx( double number);  */
int int_compare_less (int *i_ptr, int *j_ptr);
int int_compare_greater (int *i_ptr, int *j_ptr);
int eqstring( char *str1, char *str2);
float *fill( float *wts, double info, int num, int  end);
void checkpoint_clsf( clsf_DS clsf);
int *delete_duplicates( int *list, int num);
double max_plus( float *fl, int num);
int class_duplicatesp( int n_classes, class_DS *classes);
int find_term( term_DS term,term_DS  *terms, int n_terms);
int find_class( class_DS class, class_DS class_store);
int find_class_test2( class_DS class, clsf_DS  clsf, double rel_error);
int find_database_p( database_DS data, database_DS *databases, int n_data);
int find_model_p( model_DS model, model_DS *models, int n_models);
int member_int( int val, int *list, int num);
int find_str_in_table( char *str, shortstr table[], int num);
int new_random( int n_data, int *used_list, int num);
float *randomize_list( float *y, int n);
int y_or_n_p(fxlstr str);
float *reverse( float *flist, int n);
double sigma_sq( int n, double sum, double sum_sq, double min_variance);
int char_input_test( void);
int percent_equal( double n1, double n2, double rel_error);
int prefix(char *str, char *substr);
void *getf( void ***list, char *property, int num);
void *get( char *target,char *property);
void add_property( shortstr target, shortstr pname, void *value);
void add_to_plist ( att_DS att, char *target, void *value, char *type);
void write_vector_float(float *vector, int n, FILE *stream);
void write_matrix_float( float **vector, int m, int n, FILE *stream);
void write_matrix_integer( int **vector, int m, int n, FILE *stream);
void read_vector_float(float *vector, int n, FILE *stream);
void read_matrix_float( float **vector, int m, int n, FILE *stream);
void read_matrix_integer( int **vector, int m, int n, FILE *stream);
int discard_comment_lines (FILE *stream);
void flush_line (FILE *stream);
int read_char_from_single_quotes (char *param_name, FILE *stream);
int strcontains( char *str, int c);
int output_int_list( int_list list, FILE *log_file_fp, FILE *stream);
int pop_int_list( int *list, int *n_list, int *value);
void push_int_list( int *list, int *n_list, int value, int max_n_list);
int member_int_list( int val, int_list list);
int float_sort_cell_compare_gtr( sort_cell_DS i_cell, sort_cell_DS j_cell);
int class_case_sort_compare_lsr( xref_data_DS i_xref, xref_data_DS j_xref);
int att_i_sum_sort_compare_gtr( ordered_influ_vals_DS i_influ_val,
                               ordered_influ_vals_DS j_influ_val);
int float_p_p_star_compare_gtr( formatted_p_p_star_DS i_formatted_p_p_star,
                               formatted_p_p_star_DS j_formatted_p_p_star);
void safe_fprintf( FILE *stream, char *caller, char *format, ...);
void safe_sprintf( char *str, int str_length, char *caller, char *format, ...);


/* utils-math.c */
/* put in separate file 06nov94 wmt */

double log_gamma( double x, int low_precision);
int atoi_p (char *string_num, int *integer_p_ptr);
double atof_p (char *string_num, int *float_p_ptr);
double safe_exp( double x);
void mean_and_variance( double *vector, int cnt, double *mean_ptr, double *variance_ptr);
double safe_log( double x);

/* getparams.c */

void putparams( FILE *fp, PARAMP pp, int only_overridden_p);
int getparams( FILE *fp, PARAMP params);
void defparam( PARAMP params, int nparams, char *name, PARAMTYPE type, void *ptr,
              int max_length);

/* prints.c */

void print_vector_f(float *v, int n, char *t);
void sum_vector_f( float *v, int n, char *t);
void print_matrix_f( float **v, int m, int n, char *t);
void print_matrix_i( int **v, int m, int n, char *t);
void print_mm_d_params(struct mm_d_param p, int n);
void print_mm_s_params( struct mm_s_param p, int n);
void print_mn_cn_params( struct mn_cn_param p, int n);
void print_sm_params( struct sm_param p, int n);
void print_sn_cm_params( struct sn_cm_param p, int n);
void print_sn_cn_params( struct sn_cn_param p, int n);
void print_tparm_DS( tparm_DS p, char *t);
void print_priors_DS( priors_DS p, char *t);
void print_class_DS( class_DS p , char *t);
void print_term_DS ( term_DS p, char *t);
void print_real_stats_DS( real_stats_DS p, char *t);
void print_discrete_stats_DS( discrete_stats_DS p, char *t);
void print_att_DS( att_DS p, char *t);
void print_database_DS( database_DS p, char *t);
void print_model_DS( model_DS p, char *t);
void print_clsf_DS( clsf_DS p, char *t);
void print_search_try_DS( search_try_DS p, char *t);
void print_search_DS( search_DS p, char *t);


/* autoclass.c */

void autoclass_args (void);


/* intf-reports.c */

int autoclass_reports( char *results_file_ptr, char *search_file_ptr,
                      char *reports_params_file_ptr, char *influ_vals_file_ptr,
                      char *xref_class_file_ptr, char *xref_case_file_ptr,
                      char *test_data_file, char *log_file_ptr);

int clsf_search_validity_check( clsf_DS clsf, search_DS search);

void influence_values_report_streams( clsf_DS clsf, search_DS search, int num_atts_to_list,
                                      shortstr report_mode, char *influ_vals_file_ptr,
                                      char *results_file_ptr,
                                      int clsf_num, clsf_DS test_clsf,
                                      unsigned int order_attributes_by_influence_p,
                                      unsigned int comment_data_headers_p,
                                      int_list sigma_contours_att_list);

xref_data_DS case_class_data_sharing( clsf_DS clsf, shortstr report_mode,
                                      shortstr report_type, char *xref_class_file_ptr,
                                      char *xref_case_file_ptr, char *results_file_ptr,
                                      int_list xref_class_report_att_list, int clsf_num,
                                      clsf_DS test_clsf, int last_classification_p,
                                      int prediction_p,
                                      unsigned int comment_data_headers_p,
                                      int max_num_xref_class_probs);

xref_data_DS case_report_streams( clsf_DS clsf, shortstr report_mode,
                                  char *xref_case_file_ptr,
                                  char *results_file_ptr, xref_data_DS xref_data,
                                  int clsf_num, clsf_DS test_clsf,
                                  int last_classification_p,
                                  unsigned int comment_data_headers_p,
                                  int max_num_xref_class_probs);

xref_data_DS class_report_streams( clsf_DS clsf, shortstr report_mode,
                                   char *x_class_file_ptr, char *results_file_ptr, 
                                   int_list xref_class_report_att_list,
                                   xref_data_DS xref_data, int clsf_num,
                                   clsf_DS test_clsf, int last_classification_p,
                                   unsigned int comment_data_headers_p,
                                   int max_num_xref_class_probs);

xref_data_DS xref_get_data( clsf_DS clsf, char *type, int_list report_attributes,
                           xref_data_DS xref_data, int last_classification_p,
                           int prediction_p, int max_num_xref_class_probs);

int map_class_num_clsf_to_report( clsf_DS clsf, int clsf_n_class);

int map_class_num_report_to_clsf( clsf_DS clsf, int report_n_class);

void autoclass_xref_by_case_report( clsf_DS clsf, FILE *xref_case_report_fp,
                                    shortstr report_mode, xref_data_DS xref_data,
                                    char *results_file_ptr,
                                    clsf_DS test_clsf, int last_classification_p,
                                    unsigned int comment_data_headers_p,
                                    int max_num_xref_class_probs);

void classification_header( clsf_DS clsf, char *results_file_ptr, FILE *xref_case_report_fp,
                           shortstr report_mode, clsf_DS test_clsf,
                            unsigned int comment_data_headers_p);

void xref_paginate_by_case( xref_data_DS xref_data, int n_data, FILE *xref_case_report_fp,
                           shortstr report_mode, int initial_line_cnt_max,
                            unsigned int comment_data_headers_p);

void xref_output_page_headers( char *type, int page_1_p, int num_report_attribute_strings,
                              rpt_att_string_DS *report_attribute_strings,
                              FILE *xref_case_report_fp, shortstr report_mode,
                               unsigned int comment_data_headers_p);

void autoclass_xref_by_class_report( clsf_DS clsf, FILE *xref_class_report_fp,
                                     shortstr report_mode, xref_data_DS xref_data,
                                     int_list report_attributes, char *results_file_ptr,
                                     clsf_DS test_clsf, int last_classification_p,
                                     unsigned int comment_data_headers_p,
                                     int max_num_xref_class_probs);

void xref_paginate_by_class( clsf_DS clsf, xref_data_DS xref_data,
                            int_list report_attributes, FILE *xref_class_report_fp,
                            shortstr report_mode, int initial_line_cnt,
                             unsigned int comment_data_headers_p);

rpt_att_string_DS *xref_class_report_attributes( clsf_DS clsf,
                                                int_list report_attribute_numbers,
                                                shortstr **attribute_formats_ptr,
                                                int *prob_tab_ptr);

void xref_paginate_by_class_hdrs( FILE *xref_class_report_fp, shortstr report_mode,
                                 int *cnt_ptr, int line_cnt,
                                 sort_cell_DS wt_class_pairs, int init,
                                 int num_report_attribute_strings,
                                 rpt_att_string_DS *report_attribute_strings,
                                  unsigned int comment_data_headers_p);

void xref_output_line_by_class( clsf_DS clsf, FILE *xref_class_report_fp,
                               shortstr report_mode, shortstr **attribute_formats_ptr,
                               xref_data_DS xref_datum_ptr,
                               sort_cell_DS wt_class_pairs, int prob_tab,
                               int_list report_attribute_numbers,
                                unsigned int comment_data_headers_p);

void autoclass_influence_values_report( clsf_DS clsf, search_DS search, int num_atts_to_list,
                                        char *results_file_ptr, int header_information_p,
                                        FILE *influence_report_fp, shortstr report_mode, 
                                        clsf_DS test_clsf,
                                        unsigned int order_attributes_by_influence_p,
                                        unsigned int comment_data_headers_p,
                                        int_list sigma_contours_att_list);

void influence_values_header( clsf_DS clsf, search_DS search, char *results_file_ptr,
                              int header_information_p, FILE *influence_report_fp,
                              shortstr report_mode, clsf_DS test_clsf,
                              unsigned int comment_data_headers_p);

void autoclass_class_influence_values_report( clsf_DS clsf, search_DS search,
                                              char *class_number_type,
                                              int report_class_number,
                                              int num_atts_to_list,
                                              int header_information_p,
                                              char *results_file_ptr, int single_class_p,
                                              FILE *influence_report_fp, shortstr report_mode, 
                                              clsf_DS test_clsf,
                                              unsigned int order_attributes_by_influence_p,
                                              unsigned int comment_data_headers_p,
                                              int_list sigma_contours_att_list);

int populated_class_p( int clsf_class_number, char *class_number_type, clsf_DS clsf);

ordered_influ_vals_DS ordered_normalized_influence_values( clsf_DS clsf);

void influence_values_explanation( FILE *influence_report_fp);

void search_summary( search_DS search, FILE *influence_report_fp, shortstr report_mode,
                     unsigned int comment_data_headers_p);

void class_weights_and_strengths( clsf_DS clsf, FILE *influence_report_fp,
                                 shortstr report_mode,
                                  unsigned int comment_data_headers_p);

void class_divergences( clsf_DS clsf, FILE *influence_report_fp, shortstr report_mode,
                        unsigned int comment_data_headers_p);

void text_stream_header( int single_class_p, FILE *influence_report_fp,
                        shortstr report_mode, 
                        int header_information_p, clsf_DS clsf, search_DS search,
                        char *results_file_ptr, char *title_line_1, char *title_line_2,
                        clsf_DS test_clsf, unsigned int order_attributes_by_influence_p,
                         unsigned int comment_data_headers_p);

void pre_format_attributes( clsf_DS clsf, int clsf_class_number, int num_atts_to_list,
                           int line_cnt, int discrete_atts_header_p,
                           int real_atts_header_p, FILE *influence__fp,
                           shortstr report_mode, 
                           unsigned int order_attributes_by_influence_p,
                            unsigned int comment_data_headers_p);

void print_attribute_header( int discrete_atts_header_p, int real_atts_header_p,
                            FILE *influence_report_fp, shortstr report_mode,
                             unsigned int comment_data_headers_p);

int format_attribute( clsf_DS clsf, int clsf_class_number, int n_att, int line_cnt,
                     int discrete_atts_header_p, int real_atts_header_p,
                     FILE *influence_report_fp, shortstr report_mode,
                      unsigned int comment_data_headers_p);

int format_discrete_attribute( int n_att, database_DS d_base, char *header,
                              char *header_continued, i_discrete_DS influence_values,
                              int line_length, char *description, int line_cnt,
                              int discrete_atts_header_p, int real_atts_header_p,
                              FILE *influence_report_fp, shortstr report_mode,
                               unsigned int comment_data_headers_p);

int format_integer_attribute( char *header, char *header_continued,
                             i_integer_DS influence_values, int line_length,
                             char *description, char *model_term_type_symbol, int line_cnt,
                             int discrete_atts_header_p, int real_atts_header_p,
                             FILE *influence_report_fp, shortstr report_mode,
                              unsigned int comment_data_headers_p);

int format_real_attribute( char *header, char *header_continued,
                           i_real_DS influence_values, int line_length,
                           int n_att, char *description, char *model_term_type_symbol,
                           int line_cnt, int discrete_atts_header_p, int real_atts_header_p,
                           FILE *influence_report_fp, shortstr report_mode,
                           unsigned int comment_data_headers_p, int clsf_class_number,
                           clsf_DS clsf);

void generate_mncn_correlation_matrices ( clsf_DS clsf, int clsf_class_number,
                                          shortstr report_mode,
                                          unsigned int comment_data_headers_p,
                                          FILE *influence_report_fp);

int attribute_model_term_number( int n_att, model_DS model);

void sort_mncn_attributes( sort_cell_DS sort_list, int sort_index, int term_count,
                          clsf_DS clsf, int clsf_class_number);

char *filter_e_format_exponents ( fxlstr e_format_string);

/* intf-extensions.c */

clsf_DS *initialize_reports_from_results_pathname( char *results_file_ptr,
                                                  int_list clsf_n_list,
                                                  int *num_clsfs_found_ptr,
                                                  int prediction_p);
clsf_DS init_clsf_for_reports( clsf_DS clsf, int prediction_p);

int *get_class_weight_ordering( clsf_DS clsf);

char ***get_attribute_model_term_types( clsf_DS clsf);

char *report_att_type( clsf_DS clsf, int n_class, int n_att);

char *rpt_att_model_term_type( clsf_DS clsf, int n_class, int n_att);

void get_models_source_info( model_DS *models, int num_models,
                             FILE *xref_case_text_fp,
                             unsigned int comment_data_headers_p);

void get_class_model_source_info( class_DS class, char *class_model_source,
                                  unsigned int comment_data_headers_p);


/* intf-influence-values.c */

void compute_influence_values( clsf_DS clsf);

double influence_value( int n_class, int n_att, clsf_DS clsf, char *att_type,
                       void **influence_struct_DS_ptr);

int find_attribute_modeling_class( clsf_DS clsf, int n_class, int n_att,
                                  class_DS *class_ptr);


/* predictions.c */

clsf_DS autoclass_predict( char *data_file_ptr, clsf_DS training_clsf,
                           clsf_DS test_clsf, FILE *log_file_fp, char *log_file_ptr);

int same_model_and_attributes( clsf_DS clsf1, clsf_DS clsf2);


/* intf-sigma-contours.c */

void generate_sigma_contours ( clsf_DS clsf, int clsf_class_number,
                               int_list att_list,
                               FILE *influence_report_fp,
                               int comment_data_headers_p);

int compute_sigma_contour_for_2_atts ( clsf_DS clsf, int clsf_class_number,
                                       int att_x, int att_y,
                                       int trans_att_x, int trans_att_y,
                                       int term_index_x, int term_index_y,
                                       float *mean_x, float *sigma_x,
                                       float *mean_y, float *sigma_y,
                                       float *rotation);

int class_att_loc( class_DS class, int att_index, int *trans_att_index);

float get_sigma_x_y (int att_x, int att_y, class_DS class, int n_term_list,
                     float *term_list, float **covariance);








