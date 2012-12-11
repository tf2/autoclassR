

/* typedef struct term_params *term_params_DS;  not used -- commented 21nov94 wmt */
typedef struct new_term_params *tparm_DS;
/* tparm is new structure that does away with old calling mechanism
		and uses union for each model */
/* aju 980612: Prefixed enum member IGNORE with T so it would not clash 
    with predefined Win32 type.  This affects model-expander-3.c and 
    struct-class.c.*/

enum MODEL_TYPES {UNKNOWN, TIGNORE, MM_D, MM_S, MN_CN, SM, SN_CM, SN_CN} ;


struct mm_d_param {
   int *sizes;
              /* Accumulated weights for the combination of attribute values. */
   fptr *wts;
   fptr *probs;     /* Probabilities for the combination of attribute values. */
	        /* Log-probabilities for the combination of attribute values. */
   fptr *log_probs;
   float *wts_vec;           /* Displaced to wts for rapid non-indexed access */
   float *probs_vec;                                    /* Displaced to probs */
   float *log_probs_vec;                            /* Displaced to log-probs */
};


struct mm_s_param {
   int count;
   float wt;   /* Accumulated weight for the combination of attribute values. */
   float prob;        /* Probability for the combination of attribute values. */
		  /* Log-probability for the combination of attribute values. */
   float log_prob;
};

struct mn_cn_param {                    /* Multi-Normal-cn model parameters. */
   float ln_root;                          /* literally -ln<root<det<covar>>> */
   float log_ranges;
   float *emp_means;                              /* Empherical means vector. */
   fptr *emp_covar;                          /* Empherical covariance matrix. */
   float *means;                                             /* Means vector. */
   fptr *covariance;                                    /* Covariance matrix. */
   fptr *factor;                      /* L/U factorization of the Covariance. */
   float *values;                         /* Used as a temporary accumulator. */
   float *temp_v;                         /* Used as a temporary accumulator. */
   fptr *temp_m;                          /* Used as a temporary accumulator. */
   float *min_sigma_2s;

};


struct sm_param {
   float gamma_term;
   int range;
   float range_m1, inv_range, range_factor; 
   fptr val_wts;              /* Accumulated weight for each attribute value. */
   fptr val_probs;                   /* Probability for each attribute value. */
   fptr val_log_probs;                     /* Log-probs for attribute values. */
};


struct sn_cm_param {                   /* Single-Normal-CM model parameters. */
   float known_wt;                    /* Accumulated weight for known values. */
   float known_prob;                         /* Probability for known values. */
   float known_log_prob;                        /* Log-prob for known values. */
   float unknown_log_prob;                    /* Log-prob for unknown values. */
   float weighted_mean;
   float weighted_var;
   float mean;
   float sigma;
   float log_sigma;
   float variance;
   float log_variance;
   float inv_variance;
                  /* Minimum usable difference betweeen the mean and a value. */
   float ll_min_diff;
   float skewness;
   float kurtosis;

   float prior_sigma_min_2, prior_mean_mean, prior_mean_sigma;
   float prior_sigmas_term, prior_sigma_max_2, prior_mean_var;
   float prior_known_prior;
};


struct sn_cn_param {
   float weighted_mean;
   float weighted_var;
   float mean;
   float sigma;
   float log_sigma;
   float variance;
   float log_variance;
   float inv_variance;
                  /* Minimum usable difference betweeen the mean and a value. */
   float ll_min_diff;
   float skewness;
   float kurtosis;

   float prior_sigma_min_2, prior_mean_mean, prior_mean_sigma;
   float prior_sigmas_term, prior_sigma_max_2, prior_mean_var;
};


/* tparm is new structure that does away with old calling mechanism
		and uses union for each type of term
	it is a combination of the oldr params and term_params */

struct new_term_params {/* pointer to this is a tparm_DS*/
   int n_atts;
   enum MODEL_TYPES tppt;
   union {
      struct mm_d_param mm_d;
      struct mm_s_param mm_s;
      struct mn_cn_param mn_cn;
      struct sm_param sm;
      struct sn_cm_param sn_cm;
      struct sn_cn_param sn_cn;
   } ptype;


   int collect;
   int n_term, n_att, /*n_atts,*/
		 n_att_indices, n_datum, n_data;/*, range; */
   float w_j, ranges;/* , inv_range, range_factor; */
   float class_wt;
   float disc_scale;
/*  moved to sm 
   float gamma_term;
   float range_m1;
*/
/* moved to sn_cm
   float prior_sigma_min_2, prior_mean_mean, prior_mean_sigma;
   float prior_sigmas_term, prior_sigma_max_2, prior_mean_var;
   float prior_known_prior;
*/
   float log_pi;
   float log_att_delta;
   float log_delta;

/* not used sncm
   float log_ranges;  moved to mn_cn
   float log_gamma_term;
   float percent_ratio;
   float sigma_ratio; */

   /* these are just pointers to other structs or allocated memory 27jan95 wmt */
   float *wts;
   float *datum;
   float *att_indices;   
   float **data;
   
   float wt_m;
   float log_marginal;
};
