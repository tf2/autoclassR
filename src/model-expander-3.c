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
/* Run-time Model Expansion Functions */

 

model_DS conditional_expand_model_terms( model_DS model, int force,
                                        FILE *log_file_fp, FILE *stream)
{
   if ((force == TRUE) || (model->expanded_terms == FALSE))
      return (expand_model_terms( model, log_file_fp, stream));
   else return(NULL);
}


/* aju 980612: Prefixed enum member IGNORE with T so it would not clash 
   with predefined Win32 type.  */
enum MODEL_TYPES model_type (shortstr str) 
{
   return( 
      (eqstring(str, "multi_multinomial_d"))? MM_D  :(
      (eqstring(str, "multi_multinomial_s"))? MM_S  :(
      (eqstring(str, "multi_normal_cn"))    ? MN_CN :(

      (eqstring(str, "single_multinomial")) ? SM    :(
      (eqstring(str, "single_normal_cm"))   ? SN_CM :(
      (eqstring(str, "single_normal_cn"))   ? SN_CN :(
      (eqstring(str, "ignore"))             ? TIGNORE:(
      UNKNOWN))))))));
}

/* EXPAND_MODEL_TERMS
   03mar95 wmt: add call to set_ignore_att_info
   23may95 wmt: added G_prediction_p

   Builds the working <function>-fn's of a model from the attribute information
   in the terms field.  A model must be re-expanded if the terms field has been
   modified, or if any term defining function of a xxx-fn field or any
   expand-xxx-fn has been added or altered. */

model_DS expand_model_terms( model_DS model, FILE *log_file_fp, FILE *stream)
{
   int n_term;
   clsf_DS clsf_g;
   database_DS database;
   model_DS model_set[1];
   term_DS *terms, term;
   int initial_cycles_p = FALSE, delete_duplicates = FALSE, n_global_classes = 1,
       block_size = 0;

   model_set[0] = model;
   terms = model->terms;
   database = model->database;

   expand_model_reset( model);

   check_model_terms ( model, log_file_fp, stream);

   /* insure consistent model->att_ignore_ids */
   set_ignore_att_info ( model, database);

   /*  Loop over terms placing appropriate structures in the model parameter
       arrays and collecting function terms as lists in the function slots: */
   for (n_term=0; n_term<model->n_terms; n_term++) {
      term = terms[n_term];
      switch( model_type(term->type)) {
         case  MN_CN:
	    multi_normal_cn_model_term_builder(model, term, n_term);
	    break;
	 case  SM:
	    single_multinomial_model_term_builder(model, term, n_term);
	    break;
	 case  SN_CM:
	    single_normal_cm_model_term_builder(model, term, n_term);
	    break;
	 case  SN_CN:
	    single_normal_cn_model_term_builder(model, term, n_term);
	    break;
	 case MM_D:
            /****multi_multinomial_d_model_term_builder(model, term, n_term);
	    break; */
	 case MM_S:
             /****multi_multinomial_s_model_term_builder(model, term, n_term);
	    breaqk;***/
	 default:
            fprintf(stderr,"ERROR: unkown type in expand_model_terms: %s\n",term->type);
            abort();
      }
   }
   arrange_model_function_terms(model);
			              /* must proceed set-up-classification!! */
   model->expanded_terms = (int) get_universal_time();
   /*model_set = (model_DS *) malloc(sizeof(model_DS)); model-set experiment*/
   model_set[0] = model;
   if (G_prediction_p == FALSE) {
     clsf_g = set_up_clsf(n_global_classes, database, model_set, 1);
   
     block_set_clsf(clsf_g, n_global_classes, block_size, delete_duplicates, DISPLAY_WTS,
                    initial_cycles_p, log_file_fp, stream);
     model->global_clsf = clsf_g;
   }
   return(model);
}


/* Top level for Check-Term. */

void check_model_terms( model_DS model, FILE *log_file_fp, FILE *stream)
{
   int n_term;
   term_DS *terms;

   terms = model->terms;
   for (n_term=0; n_term<model->n_terms; n_term++)
      check_term(terms[n_term], model, n_term, log_file_fp, stream);
}


/* CHECK_TERM
   28jul95 wmt: since att_info can be realloc'ed in model-transforms.c,
                reset ptr for each time thru loop

   Check-Term checks the attributes indexed in att-list for appropriate
   type and sub-type.  It uses the att-trans-data property of the the
   term's type to identify the allowable types and determine if the
   subtype requires a transformation.
   */
void check_term( term_DS term, model_DS model, int n_term, FILE *log_file_fp, FILE *stream)
{
  void ***att_trans_data = NULL, ***good_subtypes_data = NULL;
  void ***conditions = NULL;
  shortstr term_type, att_type, str, att_sub_type;
  int i, att_index, *source_list, *temp, nal = 0;
  int n_source_list, n_att_trans_data;
  int n_good_subtypes_data, n_conditions;
  float *term_att_list, *new_att_list = NULL;
  att_DS att, *att_info;
  database_DS database;
  fxlstr long_str;
  char caller[] = "check_term";

  database = model->database;
  strcpy(term_type, term->type); /* The type of term set */
  term_att_list = term->att_list; /* List of att indices used by the term */
  /* Permitted att-type data (as above) */
  att_trans_data = (void ***) get(term_type, "att_trans_data");
  temp = (int *) get(term_type, "n_att_trans_data");
  if (temp == NULL)
    n_att_trans_data = 0;
  else
    n_att_trans_data = *temp;
  /* This will list the accepted and transformed attribute indices */
  for (i=0; i<term->n_atts; i++) { /* For each att-index of the Term: */
    att_index = (int) term_att_list[i];
    /* since att_info can be realloc'ed in model-transforms.c, reset ptr
       for each time thru loop */
    att_info = database->att_info;
    att = att_info[att_index];  /* Att descriptor for this index */
    strcpy(att_type, att->type); /* One of 'real, 'discrete & etc. */
    good_subtypes_data =
      (void ***) getf(att_trans_data, att_type, n_att_trans_data);
    sprintf(str, "n_%s", att_type); /* this statement added 3/2/JTP*/
    temp = (int *) getf(att_trans_data, str, n_att_trans_data);
    if (temp == NULL)
      n_good_subtypes_data = 0;
    else
      n_good_subtypes_data = *temp;
    strcpy(att_sub_type, att->sub_type);
    /* This attribute type is not handled by the term type. */
    if (good_subtypes_data == NULL) {
      safe_sprintf( long_str, sizeof( long_str), caller,
                   "ERROR[3]: model term type %s cannot handle\n"
                   "          type = %s, attribute #%d: \"%s\"\n",
                   term_type, att_type, att_index, att->dscrp);
      to_screen_and_log_file(long_str, log_file_fp, stream, TRUE);
      exit(1);
    }
    /* Try to find and substitute the source. */
    if (find_str_in_table(att_sub_type, G_transforms, NUM_TRANSFORMS) > -1) {
      source_list = get_source_list(att_index, att_info, NULL, 0,
                                    &n_source_list);
      if (n_source_list != 1)
        fprintf(stderr, "Multiple sources for attribute\n");
      att_index = source_list[0];
      strcpy(att_sub_type, att_info[att_index]->sub_type); 
    }
    conditions = getf(good_subtypes_data, att_sub_type, n_good_subtypes_data);
    sprintf(str, "n_%s", att_sub_type); 
    temp = getf(good_subtypes_data, str, n_good_subtypes_data);
    if (temp == NULL)
      n_conditions = 0;
    else n_conditions = *temp;
    /* Term is applicable to this subtype. */
    if (n_conditions == 0) {
      nal++;
      if (new_att_list == NULL)
        new_att_list = (float *) malloc(nal * sizeof(float));
      else new_att_list =
        (float *) realloc(new_att_list, nal * sizeof(float));
      new_att_list[nal-1] = att_index;
    }
    /* Term is applicable to a transform. */
    else if (getf(conditions, "transform", n_conditions) != NULL) {
      temp = (int *) malloc(sizeof(int));
      temp[0] = att_index;
      nal++;
      if (new_att_list == NULL)
        new_att_list = (float *) malloc(nal * sizeof(float));
      else new_att_list =
        (float *) realloc(new_att_list, nal * sizeof(float)); 
      new_att_list[nal-1] =
        find_transform(database,
                       getf(conditions, "transform", n_conditions), temp, 1,
                       log_file_fp,  stream);
    }
    /* <<Conditions other than transform shall be handled here.>> */
    else {
      safe_sprintf( long_str, sizeof( long_str), caller,
                   "ERROR[3]: %s model terms cannot handle subtype %s of type %s attributes\n",
                   term_type, att_sub_type, att_type);
      to_screen_and_log_file(long_str, log_file_fp, stream, TRUE);
      exit(1);
    }
  }
  term->n_atts = nal;
  term->att_list = new_att_list;
  update_location_info(model, term, term_att_list);
}


/* This extends the att-locs and att-ignore-ids to account for any change
   in the att-list. */

void update_location_info( model_DS model, term_DS term, float *old_att_list)
{
   int i, n_term, new_length, old_i, new_i;
   float *new_att_list, mx;

   n_term = find_term(term, model->terms, model->n_terms);
   new_att_list = term->att_list;
   mx = (float) max_plus(new_att_list, term->n_atts);
                                          /* We have some new transformations */
   if (mx >= model->n_att_locs) {
                                              /* and must make room for them: */
      new_length = mx + 1.0;
      model->n_att_locs = new_length;
      model->n_att_ignore_ids = new_length;
      if (model->att_locs == NULL)
         model->att_locs = (shortstr *) malloc(new_length * sizeof(shortstr));
      else
	 model->att_locs = (shortstr *) realloc(model->att_locs, new_length * sizeof(shortstr));
      if (model->att_ignore_ids == NULL)
         model->att_ignore_ids = (shortstr *) malloc(new_length * sizeof(shortstr));
      else
	 model->att_ignore_ids = (shortstr *) realloc(model->att_ignore_ids,
                                                      new_length * sizeof(shortstr));
   }
   new_length = term->n_atts;
   for (i=0; i<new_length; i++) {
      old_i = old_att_list[i];
      new_i = new_att_list[i];
      if (old_i != new_i) {                /* Attribute has been transformed: */
	 sprintf(model->att_locs[old_i], "TRANSFORMED->%d", new_i);
	 sprintf(model->att_locs[new_i], "%d", n_term);
	 strcpy(model->att_ignore_ids[new_i], model->att_ignore_ids[old_i]);
      }
   }
}


/* EXPAND_MODEL_RESET
   22nov94 wmt: initialize priors
   */
void expand_model_reset(model_DS model)
{
  int i, n_terms = model->n_terms;
  class_DS cl;

  while( (cl=model->class_store) != NULL){
    model->class_store=cl->next;
    free(cl);
  }
  model->num_class_store = 0;
  model->expanded_terms = FALSE;

  if( model->priors != NULL) {
    for (i=0; i<model->num_priors; i++)
      if(model->priors[i] != NULL)
        free(model->priors[i]);
    free(model->priors);
    model->priors = NULL;
  }
  model->num_priors = n_terms;
  model->priors = (priors_DS *) malloc(n_terms * sizeof(priors_DS));
  for (i=0; i<model->num_priors; i++)
    model->priors[i] = NULL;
  if (model->global_clsf != NULL) {
    store_clsf_DS(model->global_clsf, NULL, 0);
    model->global_clsf = NULL;
  }
}


/* UPDATE_PARAMS_FN
   20dec94 wmt: return type to void


   Updates the Likelihood parameters for class.  Each element of the model-DS
   field update-params-terms should update the corresponding attribute set's
   class parameters to the MAXIMUM POSTERIOR VALUES when evaluated in the
   environment produced by the initial let* statement.
   */
void update_params_fn( class_DS class, int n_classes, database_DS data_base, int collect)
{
  int i, num;                   /*, j, num, n_data, fnumber;*/
  /*float **data, *wts, class_wt, class_wt_1, disc_scale;*/
  /*float (*function)();*/
  tparm_DS tparm;

  /*data = data_base->data; */
  /*n_data = data_base->n_data;*/
  /*params = class->params; */
  /*wts = class->wts;*/
  /*class_wt = class->w_j;*/
  /*class_wt_1 = class_wt + 1;*/
  /*disc_scale = 1.0 / (class_wt + 1.0);*/

  class->pi_j = (class->w_j + (1.0 / n_classes)) / (data_base->n_data + 1.0);
  class->log_pi_j = (float) safe_log((double) class->pi_j);
  num = class->model->n_terms;
  for (i=0; i<num; i++) {
    tparm=class->tparms[i];

    tparm->collect = collect;
    tparm->data = data_base->data;
    tparm->n_data = data_base->n_data;
    tparm->wts = class->wts;
    tparm->class_wt = class->w_j;
    tparm->disc_scale = 1.0 / (class->w_j + 1.0);

    switch(tparm->tppt) {
    case  MN_CN:
      multi_normal_cn_update_params( tparm, class->known_parms_p);
      break;
    case  SM:
      single_multinomial_update_params(tparm,class->known_parms_p);
      break;
    case  SN_CM:
      single_normal_cm_update_params(tparm,class->known_parms_p);
      break;
    case  SN_CN:
      single_normal_cn_update_params(tparm,class->known_parms_p);
      break;

    case MM_D:
    case MM_S:
    default:
      fprintf(stderr, "ERROR: unknown type in update_params;i,t=%d %d",
              i, tparm->tppt);
      abort();
    }
  }
  /*****old
    for (i=0; i<num; i++) {
    terms[i]->parms->class = class;
    terms[i]->parms->collect = collect;
    terms[i]->parms->data = data;
    terms[i]->parms->n_data = n_data;
    terms[i]->parms->params = params;
    terms[i]->parms->wts = wts;
    terms[i]->parms->class_wt = class_wt;
    terms[i]->parms->disc_scale = disc_scale;
    fnumber = find_string(terms[i]->type, FLOAT_FNAMES, FLOAT_FLENGTH);
    call_model_function(fnumber, terms[i]->parms);
    }
    *********/
    
  /* return(1); */
}


/* Reverses accumulated model terms to get canonical order. */

void arrange_model_function_terms( model_DS model)
{
   /*model->log_likelihood_terms    = reverse(model->log_likelihood_terms);
   model->update_l_approx_terms   = reverse(model->update_l_approx_terms);
   model->update_m_approx_terms   = reverse(model->update_m_approx_terms);
   model->update_params_terms     = reverse(model->update_params_terms);
   model->class_equivalence_terms = reverse(model->class_equivalence_terms);
   model->class_merged_marginal_terms =
      reverse(model->class_merged_marginal_terms); */
}


/* LOG_LIKELIHOOD_FN
   20dec94 wmt: return type to double
   
   Computes the log-likelihood that datum belongs to class as
   pi_j*p_X_i_C_j_theta_j, using the individual likelihood terms
   given in model-DS-log-likelihood-terms.
   */
double log_likelihood_fn( float *datum, class_DS class, double limit)
{
  int i = 0, num;
  float sum = 0.0;
  tparm_DS tparm;

  num = class->num_tparms;
  do { 
    tparm=class->tparms[i];
    tparm->datum=datum;

    switch(tparm->tppt) {
    case  MN_CN:
      sum += (float) multi_normal_cn_log_likelihood(tparm);
      break;
    case  SM:
      sum += (float) single_multinomial_log_likelihood(tparm);
      break;
    case  SN_CM:
      sum += (float) single_normal_cm_log_likelihood(tparm);
      break;
    case  SN_CN:
      sum += (float) single_normal_cn_log_likelihood(tparm);
      break;

    case MM_D:
    case MM_S:
    default:
      fprintf(stderr, "ERROR: unknown type in log_likelihood; parm=%d, type= %d",
              i, tparm->tppt);
      abort();
    }

  } while ((++i < num) && (sum >= (float) limit));
  /******
    while ((i == 0) || (sum >= limit)) { 
    terms[i]->parms->datum = datum;
    terms[i]->parms->class = class;
    terms[i]->parms->params = params;
    fnumber = find_string(terms[i]->type, FLOAT_FNAMES, FLOAT_FLENGTH);
    sum += call_model_function(fnumber, terms[i]->parms);
    i++;
    if (i == num)
    break;
    }
    ******/
  return (class->log_pi_j + sum);
}


/* UPDATE_L_APPROX_FN
   20dec94 wmt: return type to double
   29mar95 wmt: calculation in double
   
   Updates the APPROXIMATE LIKELIHOOD class-DS-log-a<w.S/H.pi.theta> using the
   update-L-approx-terms field with the current statistics and parameters.
   Returns class-DS-log-a<w.S/H.pi.theta>.
   */
double update_l_approx_fn( class_DS class)
{
  int i, num;                   /* j, num, fnumber;*/
  float w_j;
  double sum = 0.0;
  /*float (*function)();*/
  tparm_DS tparm;

  w_j = class->w_j;
  num = class->model->n_terms;
  for (i=0; i<num; i++) {
    tparm=class->tparms[i];
    tparm->w_j = w_j;

    switch(tparm->tppt) {
    case  MN_CN:
      sum += multi_normal_cn_update_l_approx(tparm);
      break;
    case  SM:
      sum += single_multinomial_update_l_approx(tparm);
      break;
    case  SN_CM:
      sum += single_normal_cm_update_l_approx(tparm);
      break;
    case  SN_CN:
      sum += single_normal_cn_update_l_approx(tparm);
      break;

    case MM_D:
    case MM_S:
    default:
      fprintf(stderr, "ERROR: unknown type in update_l_approx; parm=%d, type=%d",
              i, tparm->tppt);
      abort();
    }
  }
  /****old
    terms[i]->parms->w_j = w_j;
    terms[i]->parms->params = params;
    fnumber = find_string(terms[i]->type, FLOAT_FNAMES, FLOAT_FLENGTH);
    sum += call_model_function(fnumber, terms[i]->parms);
    }
    ******/

  /*commented printf ("in update_l_approx w_j,class->log_pi_j, sum=%f %f %f\n",
    w_j,class->log_pi_j,sum); dbg*/ 

  class->log_a_w_s_h_pi_theta = ((double) (w_j * class->log_pi_j)) + sum;
  return (class->log_a_w_s_h_pi_theta);
}


/* UPDATE_M_APPROX_FN
   20dec94 wmt: return type to double
   29mar95 wmt: calculation in double

   Updates the APPROXIMATE MARGINAL LIKELIHOOD log-a<w.S/H_j> field using the
   update-M-approx-terms field and the current statistics and parameters.
   Note that when known-params-p, it is assumed that UPDATE-L-APPROX-FN-x was
   previously called.  Returns the log-a<w.S/H_j> of the class structure.
   */
double update_m_approx_fn( class_DS class)
{
  int i,num;                    /* j, num, fnumber;*/
  float w_j = class->w_j;
  double sum = 0.0;
  /*   float (*function)();*/
  tparm_DS tparm; 

  if (class->known_parms_p == TRUE)
    class->log_a_w_s_h_j = class->log_a_w_s_h_pi_theta;
  else if (w_j <= 1.0) 
    fprintf(stderr,
            "update_m_approx-fn called with w_j = %f, log_a_w_s_h_j not updated.\n",
            w_j);
  else {
    num = class->model->n_terms;
    for (i=0; i<num; i++) {
      tparm=class->tparms[i];
      tparm->w_j = w_j;

      switch(tparm->tppt) {
      case  MN_CN:
        sum += multi_normal_cn_update_m_approx(tparm);
        break;
      case  SM:
        sum += single_multinomial_update_m_approx(tparm);
        break;
      case  SN_CM:
        sum += single_normal_cm_update_m_approx(tparm);
        break;
      case  SN_CN:
        sum += single_normal_cn_update_m_approx(tparm);
        break;

      case MM_D:
      case MM_S:
      default:
        fprintf(stderr, "ERROR: unknown type in update_m_approx; parm=%d, type=%d",
                i, tparm->tppt);
        abort();
      }
    }
    /**** old
      for (i=0; i<num; i++) {
      terms[i]->parms->w_j = w_j;
      terms[i]->parms->params = params;
      fnumber = find_string(terms[i]->type, FLOAT_FNAMES, FLOAT_FLENGTH);
      sum += call_model_function(fnumber, terms[i]->parms);
      }
      ******/

    /*commented printf(" in update_m_approx sum=%f\n",sum); dbg*/ 

    class->log_a_w_s_h_j = sum;
  }
  return (class->log_a_w_s_h_j);
}

/* CLASS_EQUIVALENCE_FN
   20dec94 wmt: reply type changed from float to int

   */
int class_equivalence_fn( class_DS class_1, class_DS class_2, 
				  double percent_ratio, double sigma_ratio)
{
   int i, num, ans = FALSE, reply;
   float w_j1 = class_1->w_j, w_j2 = class_2->w_j;
   		/*float (*function)(); */
   tparm_DS tparm1,tparm2;

   if ( model_DS_equal_p(class_1->model,class_2->model) == TRUE ) {
      num = class_1->model->n_terms;
      for (i=0; i<num; i++) {
         tparm1=class_1->tparms[i];
	 tparm1->w_j=w_j1;
         tparm2=class_2->tparms[i];
	 tparm2->w_j=w_j2;

         if(tparm1->tppt != tparm2->tppt ){
            fprintf(stderr, "ERROR: unequal type in class_equiv;i,s=%d %d != %d",
				i, tparm1->tppt, tparm2->tppt);
            abort();
         }
      switch(tparm1->tppt) {
         case  MN_CN:
            reply = multi_normal_cn_class_equivalence(tparm1,tparm2,sigma_ratio);
	    break;
	 case  SM:
            reply = single_multinomial_class_equivalence(tparm1,tparm2,percent_ratio);
	    break;
	 case  SN_CM:
            reply = single_normal_cm_class_equivalence(tparm1,tparm2,sigma_ratio);
	    break;
	 case  SN_CN:
            reply = single_normal_cn_class_equivalence(tparm1,tparm2,sigma_ratio);
	    break;

	 case MM_D:
	 case MM_S:
	 default:
	    fprintf(stderr, "ERROR: unknown type in class_equivalence;i,s=%d %d",
                    i, tparm1->tppt);
            abort();
      }

/*****old
         terms[i]->parms->w_j = w_j1;
         terms[i]->parms->w_j2 = w_j2;
         terms[i]->parms->params = params1;
         terms[i]->parms->params_2 = params2; 
         terms[i]->parms->percent_ratio = percent_ratio;
         terms[i]->parms->sigma_ratio = sigma_ratio;
         fnumber = find_string(terms[i]->type, FLOAT_FNAMES, FLOAT_FLENGTH);
         reply = call_model_function(fnumber, terms[i]->parms);
********/
         if (reply == FALSE) {
            ans = FALSE; 
            break;
         }
         else ans = TRUE;
      }  
   }
   else
     ans = FALSE; 
   return ans;
}
/* CLASS_MERGED_MARGINAL_FN
   22oct94 wmt: pass wt_0, wt_1, wt_m to multi_normal_cn_class_merged_marginal
   20dec94 wmt: return type to double
   09jan95 wmt: pass clsf_DS_max_n_classes to store_class_DS
   10apr97 wmt: add database->n_data to get_class_DS call


   this function is not called currently in this implementation  - it is called
   under the "search merge" search strategies which are not implemented
   */
double class_merged_marginal_fn( clsf_DS clsf, class_DS class_0, class_DS class_1)
{
  model_DS model;
  int check_model = FALSE, want_wts_p = TRUE;
  class_DS class_m;  
  float wt_0, wt_1, wt_m;
  double log_marginal; 
  int i, num;
  tparm_DS tparm0,tparm1,tparmm;

  if ( (model_DS_equal_p(class_0->model,class_1->model) == FALSE) ||
      (class_0->known_parms_p == TRUE) ||
      (class_1->known_parms_p == TRUE) ) 
    return (0.0);
  else {
    model = class_0->model;
    class_m = get_class_DS(model, clsf->database->n_data, want_wts_p, check_model);  
    wt_0 = class_0->w_j;
    wt_1 = class_1->w_j;
    wt_m = wt_0 + wt_1;
    if ( (wt_0 == 0.0) || (wt_1 == 0.0) ) 
      return (0.0);
    else {
      class_m->w_j = wt_m;
      class_m->log_a_w_s_h_pi_theta = 0.0;
      class_m->log_a_w_s_h_j = 0.0;
      num = class_0->model->n_terms;
      for (i=0; i<num; i++) {
        tparm0=class_0->tparms[i];
        tparm1=class_1->tparms[i];
        tparmm=class_m->tparms[i];

        if(tparm0->tppt != tparm1->tppt){ 
          fprintf(stderr, "ERROR: unequal type in class_merge;i,s=%d %d != %d",
                  i,tparm0->tppt,tparm1->tppt);
          abort();
        }
        switch(tparm1->tppt) {
        case  MN_CN:
          multi_normal_cn_class_merged_marginal(tparm0, tparm1, tparmm, wt_0, wt_1, wt_m);
          break;
        case  SM:
          single_multinomial_class_merged_marginal(tparm0,tparm1,tparmm);
          break;
        case  SN_CM:
          single_normal_cm_class_merged_marginal(tparm0,tparm1,tparmm);
          break;
        case  SN_CN:
          single_normal_cn_class_merged_marginal(tparm0,tparm1,tparmm);
          break;

        case MM_D:
        case MM_S:
        default:
          fprintf(stderr, "ERROR: unknown type in class_merged_marginal;i,s=%d %d",
                  i,tparm1->tppt);
          abort();
        }

            
        /*****old
          terms[i]->parms->params = params_0;
          terms[i]->parms->params_2 = params_1;
          terms[i]->parms->params_m = params_m;
          terms[i]->parms->w_j = wt_0;
          terms[i]->parms->w_j2 = wt_1;
          terms[i]->parms->wt_m = wt_m;
          fnumber = find_string(terms[i]->type, FLOAT_FNAMES, FLOAT_FLENGTH);
          call_model_function(fnumber, terms[i]->parms);
          *******/
      }
      update_params_fn(class_m, (clsf->n_classes)-1, clsf->database, FALSE); /*no collect*/
      (void) update_l_approx_fn(class_m);
      log_marginal = update_m_approx_fn(class_m);
      store_class_DS(class_m, clsf_DS_max_n_classes(clsf));
      return (log_marginal);
    }
  }
}


tparm_DS *model_global_tparms( model_DS model)
{
   return(model->global_clsf->classes[0]->tparms);
}
