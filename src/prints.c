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


/* 01jan95 wmt: new
 */
void sum_vector_f( float *v, int n, char *t)
{
  int i;
  float float_sum = 0.0;
  double double_sum = 0.0;

  if(v==NULL)
    {
      printf(" \npointer passed is null for %s\n",t);
      return;
    }
  printf("\nvector %s, n=%d\n", t, n);
  for (i=0; i<n; i++) {
    float_sum += v[i];
    double_sum += (double) v[i];
  }
  printf("float: %f, double %12.10f\n", float_sum, double_sum);
}


/* 09may95 wmt: add printf("\n");
 */
void print_vector_f(float *v, int n, char *t)
{
  int i;
  int vector_print_limit = 500;
  if(v==NULL)
    {
      printf(" \npointer passed is null for %s\n",t);
      return;
    }
  printf("\nvector %s, n=%d\n",t,n);
  if(n > vector_print_limit) {
    printf("\n\n****limiting n=%d to %d\n", n, vector_print_limit);
    n = vector_print_limit;
  }
  for(i=0;i<n;i++)printf(" %g ",v[i]);
  printf("\n");
}


/* 01jan95 wmt: add printf("\n");
 */
void print_matrix_f( float **v, int m, int n, char *t)
{
  int i,j;
  if(v==NULL)
    {
      printf("\npointer passed is null for %s\n",t);
      return;
    }
  printf("\nmatrix %s, m,n=%d %d\n",t,m,n);
  for(i=0;i<m;i++) {
    if(i==3 && i+200<m)
      printf(".\n.\n.\nskipping to %d\n",i+=200); /* for large ones*/
    printf("\n row %d\n",i);
    for(j=0;j<n;j++) {
      printf(" %g ",v[i][j]);
    }
  }
  printf("\n");
}


/* 01jan95 wmt: add printf("\n");
 */
void print_matrix_i( int **v, int m, int n, char *t)
{
  int i,j;
  if(v==NULL)
    {
      printf("\npointer passed is null for %s\n",t);
      return;
    }
  printf("\nmatrix %s, m,n=%d %d",t,m,n);
  for(i=0;i<m;i++)
    {
      printf("\n row %d\n",i);
      for(j=0;j<n;j++)printf(" %d ",v[i][j]);
    }
  printf("\n");
}


void print_mm_d_params(struct mm_d_param p, int n)
{
	int i, m = 0;

   /* these were ceclareted float *x*/
  	for(i=0;i<n;i++)
	{
		m=p.sizes[i]; 
		printf(" row %d,size=%d\n",i,m);
		print_vector_f(p.wts[i],m,"wts");            
		print_vector_f(p.probs[i],m,"probs");            
		print_vector_f(p.log_probs[i],m,"log_probs");            
	}
	print_vector_f(p.wts_vec,m,"wts_vec");            
	print_vector_f(p.probs_vec,m,"probs_vec");            
	print_vector_f(p.log_probs_vec,m,"log_probs_vec");            

}


void print_mm_s_params( struct mm_s_param p, int n)
{
	printf(" mm_s_params count,wt,prob %d %g %g %g\n",
		p.count,p.wt,p.prob,p.log_prob);
}

void print_mn_cn_params( struct mn_cn_param p, int n)
{ 
	printf(" ln_root %g log_ranges %g \n",p.ln_root,p.log_ranges);

	print_vector_f(p.emp_means,n,"emp means");  
	print_matrix_f(p.emp_covar,n,n,"emp_covar");

	print_vector_f(p.means,n,"means");
	print_matrix_f(p.covariance,n,n,"covariance");
	print_matrix_f(p.factor,n,n,"factor");

	print_vector_f(p.values,n,"values");
	print_vector_f(p.temp_v,n,"temp_v");
	print_matrix_f(p.temp_m,n,n,"temp_m");
}


void print_sm_params( struct sm_param p, int n)
{
	printf(" sm_param gamma_term,range,range_m1,inv_range,range_factor\n");
	printf(" %g %d %g %g %g \n",
		p.gamma_term,p.range,p.range_m1,p.inv_range,p.range_factor); 
	print_vector_f(p.val_wts,n,"val_wts");
	print_vector_f(p.val_probs,n,"val_probs");     
	print_vector_f(p.val_log_probs,n,"val_log_probs"); 
}


void print_sn_cm_params( struct sn_cm_param p, int n)
{  
	printf(" known_wt, known_prob, known_log_prob, unknown_log_prob\n");
	printf(" %g %g %g %g\n"
		,p.known_wt, p.known_prob,p.known_log_prob, p.unknown_log_prob);  
   
	printf(" weighted_mean, weighted_var, mean %g %g %g\n"
		, p.weighted_mean, p.weighted_var, p.mean);
   
	printf(" sigma, log_sigma, variance, log_variance, inv_variance\n");
	printf(" %g %g %g %g %g\n"
		, p.sigma, p.log_sigma, p.variance, p.log_variance, p.inv_variance);
   
	printf(" ll_min_diff, skewness, kurtosis  %g %g %g\n"
		, p.ll_min_diff, p.skewness, p.kurtosis);
	printf(" prior_sigma_min_2, prior_mean_mean, prior_mean_sigma\n");
	printf(" %g %g %g\n",
		 p.prior_sigma_min_2, p.prior_mean_mean, p.prior_mean_sigma);
    
	printf(" prior_sigmas_term, prior_sigma_max_2, prior_mean_var\n");
	printf(" %g %g %g\n",
		 p.prior_sigmas_term, p.prior_sigma_max_2, p.prior_mean_var);
    
	printf(" prior_known_prior %g \n", p.prior_known_prior);
}


void print_sn_cn_params( struct sn_cn_param p, int n)
{
	printf(" weighted_mean, weighted_var, mean\n");
	printf(" %g %g %g\n"
		,p.weighted_mean,p.weighted_var,p.mean);
   
	printf("sigma, log_sigma, variance, log_variance, inv_variance\n");
	printf(" %g %g %g %g %g\n"
		,p.sigma, p.log_sigma, p.variance, p.log_variance, p.inv_variance);

	printf(" ll_min_diff, skewness, kurtosis  %g %g %g\n"
		,p.ll_min_diff,p.skewness,p.kurtosis);


}

void print_tparm_DS( tparm_DS p, char *t)
{

	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}

	printf("\n tparmDS %s;n_atts=%d type=%d\n",t ,p->n_atts, p->tppt);

	switch(p->tppt) {
	 case  SM:
	     print_sm_params( p->ptype.sm,p->n_atts);
	     break;
	  case  SN_CM:
	     print_sn_cm_params( p->ptype.sn_cm,p->n_atts);
	     break;
	  case  SN_CN:
	     print_sn_cn_params( p->ptype.sn_cn,p->n_atts);
	     break;
          case MM_D:
	     print_mm_d_params( p->ptype.mm_d,p->n_atts);
	     break;
          case MM_S:
	     print_mm_s_params( p->ptype.mm_s,p->n_atts);
	     break;
          case MN_CN:
             print_mn_cn_params( p->ptype.mn_cn,p->n_atts);
             break;
          default:
	     printf(" \n\n\n in print_tparms_DS UNKNOWN TYPE=%d\n",
			p->tppt);
        }

	printf(" collect %d\n",p->collect);

	printf(" n_term, n_att,  n_att_indices, n_datum, n_data\n");
	printf(" %d %d %d %d %d\n",
		p->n_term, p->n_att, p->n_att_indices, p->n_datum, p->n_data);
   
	printf(" w_j, ranges= %g %g\n",
		 p->w_j, p->ranges);

	printf(" class_wt,disc_scale\n");
	printf(" %g %g\n",
		p->class_wt, p->disc_scale);

	printf(" log_pi, log_att_delta, log_delta\n");
		/* , log_ranges, log_gamma_term ); */
	printf(" %g %g %g\n", p->log_pi, p->log_att_delta, p->log_delta);
		/*, p->log_ranges, p->log_gamma_term);*/
  /********* 
	printf(" percent_ratio, sigma_ratio %g %g \n", p->percent_ratio, p->sigma_ratio );
*********/
	printf("  wt_m, log_marginal %g %g\n", p->wt_m,p->log_marginal);
   

	print_vector_f( p->wts, p->n_datum, "wts");
	print_vector_f( p->datum, p->n_datum, "datum");
	print_vector_f( p->att_indices,  p->n_att_indices, "att_indices");
	print_matrix_f( p->data, p->n_data, p->n_att," data");   
}

void print_priors_DS( priors_DS p, char *t)
{
	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
	printf("\n\n priors %s\n",t);
	printf(" known_prior, sigma_min, sigma_max %g %g %g \n",
		p->known_prior,p->sigma_min,p->sigma_max);
   
	printf(" mean_mean, mean_sigma, mean_var   %g %g %g \n",
	       p->mean_mean,p->mean_sigma,p->mean_var);
   
	printf(" minus_log_log_sigmas_ratio, minus_log_mean_sigma %g %g\n",
		p->minus_log_log_sigmas_ratio,p->minus_log_mean_sigma);
}


void print_class_DS( class_DS p , char *t)
{
	int i;
	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
	printf(" \n\nclass %s\n",t);

	printf(" w_j, pi_j %g %g\n", p->w_j, p->pi_j);

	printf(" log_pi_j, log_a_w_s_h_pi_theta, log_a_w_s_h_j %g %g %g\n",
		 p->log_pi_j, p->log_a_w_s_h_pi_theta, p->log_a_w_s_h_j);

	printf(" known_parms_p, num_tparms %d %d\n",
			p->known_parms_p, p->num_tparms);
   
	for (i=0;i<p->num_tparms;i++) 
			print_tparm_DS(p->tparms[i]," from class");
   
	/* print_vector_f(p->i_values,p->num_i_values," i_values"); */
	printf(" void **i_values; N-attributes vector of influence value structures.\n");
   
	printf(" i_sum,max_i_value %g %g \n",p->i_sum, p->max_i_value);
   
	print_vector_f(p->wts,p->num_wts,"wt vector");
	/* *wts; N-data vector of object membership probabilities. */
   
	printf("skipping call to print_model that is in print_class\n");
        /*commented	print_model_DS(p->model, "model from class"); *****/
		
	printf(" next pointer is%sNULL\n",
		(NULL==p->next)?" ":" NOT");
}


void print_term_DS ( term_DS p, char *t)
{
	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
     /* One of the likelihood fn. terms in MODEL-TERM-TYPES. */

     printf("\n\n term %s, n_atts,type=%d %s\n",t,p->n_atts,p->type);
     print_vector_f(p->att_list,p->n_atts,"att_list from term");
   /*float *att_list; List of attributes (by number) in set. See ATT-GROUPS. */
	print_tparm_DS(p->tparm,"from term");
}


void print_real_stats_DS( real_stats_DS p, char *t)
{
	printf(" real stats from %s\n",t);
	printf(" count,max,min,mean,var %d %5g %g %g %g\n",
		p->count,p->mx,p->mn,p->mean,p->var);

}
void print_discrete_stats_DS( discrete_stats_DS p, char *t)
{
	int i;
	printf(" discrete stats from %s\n",t);
	printf(" range,n_observed %d %d \n",
		p->range,p->n_observed);
	for(i=0;i<=p->range;i++)printf(" %d %d \n",i,p->observed[i]);

}

void print_att_DS( att_DS p, char *t)
{
	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
	printf(" att %s, \n type,subtype,descrp=%s %s \"%s\" \n",
		t,p->type, p->sub_type, p->dscrp);

  	if(eqstring(p->type,"real"))
		print_real_stats_DS(p->r_statistics," rstats from att");
	else
		print_discrete_stats_DS(p->d_statistics," dstats from att");
	
	printf(" n_props,range,zero_point,n_trans %d %d %f %d\n",
		p->n_props, p->range, p->zero_point, p->n_trans);
   
	printf(" translations triple pointer is%sNULL\n",
		(NULL==p->translations)?" ":" NOT ");
	printf(" props triple pointer is%sNULL\n",
		(NULL==p->props)?" ":" NOT ");
  
	printf(" not printing warings and errors\n");

	printf(" rel_error, error, missing %g %g %d\n",
		p->rel_error, p->error, p->missing);

}


void print_database_DS( database_DS p, char *t)
{
	int i;
	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
	printf(" not prinitng file pointers for data and header in db %s\n",t);   
	printf(" n_data,n_atts,input_n_atts,compressed_p %d %d %d %d \n",
		p->n_data, p->n_atts, p->input_n_atts, p->compressed_p);
		 /* Ordered N-atts vector of att_DS describing the attributes. */
	for(i=0;i<p->n_atts;i++)
	{
		printf("\n%d th ",i);
		print_att_DS(p->att_info[i]," info from database");
	}
	print_matrix_f(p->data,p->n_data,p->n_atts," data");
   
	printf(" separator_char,comment_char,unknown_token %c %c %c\n",
		p->separator_char,p->comment_char,p->unknown_token);
   
	printf("num_tsp %d, translations_supplied_p is %sNULL\n",
		p->num_tsp,(NULL==p->translations_supplied_p)?" ":"NOT ");

	printf("num_invalid_value_errors %d, invalid_value_errors is %sNULL\n",
		p->num_invalid_value_errors,(NULL==p->invalid_value_errors)?" ":"NOT ");

	printf("num_incomplete_datum %d, incomplete_datum is %sNULL\n",
		p->num_incomplete_datum, (NULL==p->incomplete_datum)?" ":"NOT ");

}


void print_model_DS( model_DS p, char *t)
{   
	int i;
	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
	printf("\n\n model %s; id =%s, expanded_terms univ time=%d\n",
		t, p->id, p->expanded_terms);
	printf(" model file pointer not printed; file index =%d\n",
		p->file_index);
	print_database_DS(p->database, "database in model");

	printf("\n\nthis model contains %d terms\n",p->n_terms);

	for(i=0;i<p->n_terms;i++)
		print_term_DS(p->terms[i]," ith term in model");

	printf(" n_att_locs=%d\n", p->n_att_locs);
	for(i=0;i<p->n_att_locs;i++)
		printf("%d %s\n",i,p->att_locs[i]);

	printf(" n_att_ignore_ids=%d\n", p->n_att_ignore_ids);
	for(i=0;i<p->n_att_ignore_ids;i++)
		printf("%d %s\n",i,p->att_ignore_ids[i]);
   
	printf(" num_priors=%d\n", p->num_priors);
	for(i=0;i< p->num_priors;i++)
		print_priors_DS(p->priors[i],"priors");
	
	printf(" num_class_store=%d; class_store is%sNULL\n",
		p->num_class_store,(NULL==p->class_store)?" ":" NOT ");
   
	printf(" not printing global clsf from model\n");
/*commented	print_clsf_DS(p->global_clsf," global clsf from model"); *****/
}


void print_clsf_DS( clsf_DS p, char *t)
{
	int i;

	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
	printf(" clsf %s\n",t);


	printf("log_p_x_h_pi_theta, log_a_x_h %g %g\n",
	   p->log_p_x_h_pi_theta, p->log_a_x_h);

	printf("  database pointer is%sNULL\n",
		(NULL==p->next)?" ":" NOT ");
   
   
	printf(" num_models=%d\n", p->num_models);
	printf(" skipping 1 call for each to print_model in clsf\n");
	/****** commented  for(i=0;i< p->num_models;i++)
		print_model_DS(p->models[i]," ith clsf model"); ****/
   
	printf(" n_classes=%d\n", p->n_classes);
	printf("  class pointer is%sNULL\n",
		(NULL==p->classes)?" ":" NOT ");
	for(i=0;i< p->n_classes;i++)
		print_class_DS(p->classes[i],"ith clsf class");
   
	printf("min_class_wt %g\n", p->min_class_wt );
   
	printf("  clsf reports pointer is%sNULL\n",
		(NULL==p->reports)?" ":" NOT ");
   
	printf("  clsf_store next pointer is%sNULL\n",
		(NULL==p->next)?" ":" NOT ");
}


void print_search_try_DS( search_try_DS p, char *t)
{
	int i;

	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
	printf(" search try %s",t);

	printf(" n,time,j_in,j_out,ln_p, %d %d %d %d %g\n",
		p->n,p->time,p->j_in,p->j_out,p->ln_p);
   
	printf(" number of duplicates=%d\n",p->n_duplicates);
	for(i=0;i<p->n_duplicates;i++)
		print_search_try_DS(p->duplicates[i],"duplicate tries");

	/* a list of tries happened after this one came up with the same clsf */
   
	print_clsf_DS( p->clsf,"clsf from try");     
	/* the clsf this try came up with */
}

void print_search_DS( search_DS p, char *t)
{
	int i;

	if(p==NULL)
	{
		printf("\npointer passed is null for %s\n",t);
			return;
	}
	printf(" search struct %s\n",t);
	printf(" n, time, n_dups, n_dup_tries %d %d %d %d\n",
		 p->n, p->time, p->n_dups, p->n_dup_tries);
   
	print_search_try_DS(p->last_try_reported," last try reported");

	printf(" tries from best on down for n_tries =%d\n",p->n_tries);
	for (i=0;i< p->n_tries;i++)
		print_search_try_DS(p->tries[i],"best to worst try");
        printf(" start_j_list: ");
        for (i=0; p->start_j_list[i] != END_OF_INT_LIST; i++)
          printf("%d, ", p->start_j_list[i]);
        printf("\n n_final_summary, n_save %d %d\n", p->n_final_summary, p->n_save);
}
