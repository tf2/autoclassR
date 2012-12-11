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


/* STORE_CLASS_DS
   09jan95 wmt: only store clsf_DS_max_n_classes(clsf) classes,
                free any excess

   Class storage Management:
   Given the size of a class, and the number that will be needed on an
   extensive search, it is usefull to directly manage the storage of
   temporarily un-needed classes.  Note however that this tends to counteract
   the compaction normally performed by the garbage collector.  These routines
   DO NOT attempt to update or maintain the clsf-DS-classes.
   */
void store_class_DS( class_DS cl, int max_n_classes)
{
  model_DS cl_model;
  clsf_DS null_clsf = NULL;

  cl_model = cl->model;

  if (cl_model->num_class_store >= max_n_classes)
    free_class_DS( cl, "clsf", null_clsf, 0);
  else if (find_class(cl, cl_model->class_store) == FALSE) { /* multiple reference check */
    cl->w_j = 0;
    cl->log_w_j = 0;
    cl->pi_j = 1.0;
    cl->log_pi_j = 0.0;
    cl->log_a_w_s_h_pi_theta = 0.0;
    cl->log_a_w_s_h_j = 0.0;
    cl->known_parms_p = FALSE;

    cl->i_values = NULL;
    cl->num_i_values = 0;
    cl->i_sum = 0.0;
    cl->max_i_value = 0.0;
    /* wts no longer zeroed here since may not even need and done in pop
       in case have to alloc.*/
    cl->next=cl_model->class_store;
    cl_model->class_store =  cl;
    cl_model->num_class_store += 1;
    if (G_clsf_storage_log_p == TRUE)
      fprintf(stdout, "\nstore_class_DS(%.2d [max=%d]): %p\n",
              cl_model->num_class_store, max_n_classes, (void *) cl);
  }
}


/* Get a class either from the model's class-store or by construction.

   10apr97 wmt: add database->n_data to get_class_DS, pop_class_DS, &
                build_class_DS args
 */

class_DS get_class_DS( model_DS model, int n_data, int want_wts_p, int check_model)
{
  FILE *log_file_fp = NULL;

   class_DS temp;
   if ( model == NULL ) {
      fprintf(stderr,"ERROR: get_class called with NULL model\n");
      abort();
   }

   if (check_model == TRUE)
      conditional_expand_model_terms(model, FALSE, log_file_fp, G_stream);

   if ((temp = pop_class_DS(model, n_data, want_wts_p)) != NULL)
         return(temp);

   return(build_class_DS(model, n_data, want_wts_p));
}


/* Pops a class off the model's class-store.  This may produce NULL.

   10apr97 wmt: add n_data arg, instead of using model->database->n_data
 */

class_DS pop_class_DS( model_DS model, int n_data, int want_wts_p)
{
   int i;
   class_DS cl;

   if ( (cl = model->class_store) == NULL)
	return(NULL); 

   model->num_class_store -= 1;
   model->class_store = cl->next;

   if (want_wts_p == TRUE) {
      if( cl->wts == NULL )
         cl->wts = (float *) malloc( n_data * sizeof(float));
      else
         if( cl->num_wts != n_data)
            cl->wts = (float *) realloc(cl->wts, n_data * sizeof(float));

      cl->num_wts = n_data;
      for (i=0; i<n_data; i++)
	 cl->wts[i] = 0.0;
   }
   else {
      if ( cl->wts != NULL)
         free(cl->wts);
      cl->wts = NULL;
      cl->num_wts = 0;
   }
   if (G_clsf_storage_log_p == TRUE)
     fprintf(stdout, "\npop_class_DS(%.2d): %p\n", model->num_class_store, (void *) cl);
   return(cl);
}


/* BUILD_CLASS_DS
   24oct94 wmt: init known_parms_p 
   10apr97 wmt: add n_data to args; use it rather than model->database->n_data

   Build a class instantiating a fully expanded model.
   */
class_DS build_class_DS( model_DS model, int n_data, int want_wts_p)
{
  int i, num;
  class_DS temp;

  temp = (class_DS) malloc(sizeof(struct class));
  temp->w_j = temp->log_w_j = temp->log_pi_j = 0.0;
  temp->pi_j = 1.0;
  temp->log_a_w_s_h_pi_theta = temp->log_a_w_s_h_j = 0.0;
  temp->known_parms_p = FALSE;
  temp->model = model;
  temp->num_tparms =  num = model->n_terms;
  temp->tparms = (tparm_DS *) malloc(num * sizeof(tparm_DS));
  for (i=0; i<num; i++) 
    temp->tparms[i] = copy_tparm_DS(model->terms[i]->tparm);
       
  temp->num_i_values = 0;
  temp->i_values = NULL;
  temp->i_sum = temp->max_i_value = 0.0;
  if (want_wts_p == TRUE) {
    temp->num_wts = n_data;
    temp->wts = (float *) malloc(temp->num_wts * sizeof(float));
    for (i=0; i<n_data; i++)
      temp->wts[i] = 0.0;
  }
  else {
    temp->wts = NULL;
    temp->num_wts = 0;
  }
  temp->next = NULL;

  if (G_clsf_storage_log_p == TRUE) {
    fprintf( stdout, "\nbuild_class_DS: %p, num_wts %d, wts:%p, wts-len:%d\n",
            (void *) temp, temp->num_wts, (void *) temp->wts,
            temp->num_wts * (int) sizeof( float));
    if (G_n_freed_classes > 0)
      G_n_create_classes_after_free++;
  }
  return(temp);
}


class_DS build_compressed_class_DS( model_DS comp_model)
{
   class_DS temp;

   temp = (class_DS) malloc(sizeof(struct class));
   temp->w_j = temp->log_w_j = temp->log_pi_j = 0.0;
   temp->pi_j = 1.0;
   temp->log_a_w_s_h_pi_theta = temp->log_a_w_s_h_j = 0.0;
   temp->model = comp_model;
   temp->num_tparms = 0;
   temp->tparms = NULL;
   temp->num_i_values = 0;
   temp->i_values = NULL;
   temp->num_wts = 0;
   temp->wts = NULL;
   return(temp);
}


/* Copies a class-DS so as to ensure there is no sub-structure sharing.
   With wts==0, this produces a class-DS with compressed (nil) wts.
   See #'compress-clsf.  Normally the class-store is used to recycle classes.

   10apr97 wmt: add database->n_data to get_class_DS call; add n_data arg;
                add database->n_data to copy_to_class_DS call
   */

class_DS copy_class_DS(class_DS from_class, int n_data, int want_wts_p)
     /*JTP, classes_to_check) JTP*/
{
   class_DS to_class;
   model_DS model;

   model = from_class->model;
   to_class = get_class_DS(model, n_data, want_wts_p, TRUE);
   /* commented 
      if (classes_to_check != NULL && (to_class == from_class) ||
       (find_class(to_class, classes_to_check) != FALSE))
      fprintf(stderr, "Redundant class\n");
      *****commented JTP*/
   return(copy_to_class_DS( from_class, to_class, n_data, want_wts_p));
}


/* COPY_TO_CLASS_DS
   20jan95 wmt: copy num_wts, if want_wts_p is true
   08feb95 wmt: do not copy i_values; error if non NULL
   25feb95 wmt: if tparms already allocated, free them.
   10apr97 wmt: add n_data to args; use rather than model->database->n_data

   This completely replaces the structured contents of to-class with full depth
   copies of those of from-class, except that when wts==0 the wts are not
   copied.  Only the structural framework of to-class is retained, substructures
   being replaced. This would be better for some kind of recursive replace on
   params and i-values.
   */
class_DS copy_to_class_DS( class_DS from_class, class_DS to_class, int n_data,
                           int want_wts_p)
{
   int i, num, n_tparm;
   tparm_DS *tparms;
   model_DS model;

   to_class->model        = model = from_class->model;
   to_class->w_j                  = from_class->w_j;
   to_class->log_w_j              = from_class->w_j;
   to_class->pi_j                 = from_class->pi_j;
   to_class->log_pi_j             = from_class->log_pi_j;
   to_class->log_a_w_s_h_pi_theta = from_class->log_a_w_s_h_pi_theta;
   to_class->log_a_w_s_h_j        = from_class->log_a_w_s_h_j;
   to_class->known_parms_p       = from_class->known_parms_p;

   if (to_class->tparms != NULL) {
     for (n_tparm=0; n_tparm < to_class->num_tparms ; n_tparm++) {
       free_tparm_DS( to_class->tparms[n_tparm]);
     }
     free( to_class->tparms);
   }
   to_class->num_tparms     = num = from_class->num_tparms;
   tparms = from_class->tparms;
   to_class->tparms = (tparm_DS *) malloc(num * sizeof(tparm_DS));
   for (i=0; i<num; i++) {
      to_class->tparms[i] = copy_tparm_DS(tparms[i]);
   }
   to_class->num_i_values         = from_class->num_i_values;
   to_class->i_values             = from_class->i_values;
   to_class->i_sum                = from_class->i_sum;
   to_class->max_i_value          = from_class->max_i_value;
   if (from_class->i_values != NULL) {
     fprintf( stderr, "ERROR: from_class->i_values is non NULL\n");
     abort();
   }
   /* A compressed class has wts==0 */
   /* get class should have allocated wts and zeroed or cleaned up if compressed*/

   if ( want_wts_p == TRUE && from_class->num_wts == n_data ) {
      if( to_class->wts == NULL || from_class->wts == NULL ) {
         fprintf(stderr,"ERROR: copy class wt allocation error 1");
         abort();
       }
      for(i=0; i<n_data; i++)
         to_class->wts[i] = from_class->wts[i];
      to_class->num_wts = from_class->num_wts;
   }
   else
      if( to_class->wts != NULL || to_class->num_wts > 0 ) {
         fprintf(stderr,"ERROR: copy class wt allocation error 2");
         abort();
       }
   return(to_class);
}


/* CLASS_DS_TEST
   18oct94 wmt: modified
   20dec94 wmt: added call to class_equivalence_fn

   Determines if two classes have exactly the same parameterization.
   */
int class_DS_test( class_DS cl1, class_DS cl2, double rel_error)
{
  /* int i, cnt = 0;       debug */

  /* fprintf( stderr, "w_j %d log_a_w_s_h_pi_theta %d log_a_w_s_h_j %d "
          "class_equivalence_fn %d\n",
          (percent_equal( (double) cl1->w_j, (double) cl2->w_j,
                         rel_error) == TRUE),
          (percent_equal( cl1->log_a_w_s_h_pi_theta, cl2->log_a_w_s_h_pi_theta,
                         rel_error) == TRUE),
          (percent_equal( cl1->log_a_w_s_h_j, cl2->log_a_w_s_h_j,
                         rel_error) == TRUE),
          (class_equivalence_fn( cl1, cl2, rel_error, rel_error) == TRUE));
  fprintf( stderr, "clsf1=w_j %f log_a_w_s_h_pi_theta %f log_a_w_s_h_j %f\n",
          cl1->w_j, cl1->log_a_w_s_h_pi_theta, cl1->log_a_w_s_h_j);
  fprintf( stderr, "clsf2=w_j %f log_a_w_s_h_pi_theta %f log_a_w_s_h_j %f\n",
          cl2->w_j, cl2->log_a_w_s_h_pi_theta, cl2->log_a_w_s_h_j); */
  /* for this to work change update_wts_p to TRUE in reconstruct_search
     so that the wts vectors will be expanded */
  /* for (i=0; i<cl1->num_wts; i++) {
    if (percent_equal( (double) cl1->wts[i], (double) cl2->wts[i], (double) 0.1) == TRUE)
      cnt++; */
    /* else
      fprintf( stderr, "  %d:%.4f/%.4f", i, cl1->wts[i], cl2->wts[i]); */
  /* }
  fprintf( stderr, "num_wts %d, cnt %d\n", cl1->num_wts, cnt);  */
  /* centerline_stop(); */

   if ((model_DS_equal_p(cl1->model, cl2->model) == TRUE) &&
       (cl1->known_parms_p == cl2->known_parms_p) &&
       (percent_equal( (double) cl1->w_j, (double) cl2->w_j,
                      rel_error) == TRUE) &&
       (percent_equal( cl1->log_a_w_s_h_pi_theta, cl2->log_a_w_s_h_pi_theta,
                      rel_error) == TRUE) &&
       (percent_equal( cl1->log_a_w_s_h_j, cl2->log_a_w_s_h_j,
                      rel_error) == TRUE) &&
       (class_equivalence_fn( cl1, cl2, rel_error, rel_error) == TRUE))
      return(TRUE);
   else
     return(FALSE);
}


/* COPY_TPARM_DS
   26jan95 wmt: for sn_cn, ensure all slots are copied

   routine was originally converted twice. 
   the other one was in model-multi-multinomial-d.c. It was deleted
   after comparing to this one and finding no difference except spacing. JTP
   */
tparm_DS copy_tparm_DS( tparm_DS old)
{
  int i, j, n_atts;
  struct mm_d_param *o1, *n1;
  struct mm_s_param *o2, *n2;
  struct mn_cn_param *o3, *n3;
  struct sm_param *o4, *n4;
  struct sn_cm_param *o5, *n5;
  struct sn_cn_param *o6, *n6;
  tparm_DS new;

  if(old == NULL) return (NULL); /*jtp added just for safety*/

  n_atts = old->n_atts;
  new = (tparm_DS) malloc(sizeof(struct new_term_params));
  *new = *old;                 /* note that there are vectors and other things that may
                                   need correcting after this mass copy*/
  if (new->tppt == MM_D) {
    o1 = &(old->ptype.mm_d);
    n1 =&( new->ptype.mm_d);
    n1->sizes = (int *) malloc(n_atts * sizeof(int));
    n1->wts = (fptr *) malloc(n_atts * sizeof(fptr));
    n1->probs = (fptr *) malloc(n_atts * sizeof(fptr));
    n1->log_probs = (fptr *) malloc(n_atts * sizeof(fptr));
    n1->wts_vec = (float *) malloc(n_atts * sizeof(float));
    n1->probs_vec = (float *) malloc(n_atts * sizeof(float));
    n1->log_probs_vec = (float *) malloc(n_atts * sizeof(float));
    for (i=0; i<n_atts; i++) {
      n1->sizes[i] = o1->sizes[i];
      n1->wts[i] = (float *) malloc(o1->sizes[i] * sizeof(float));
      n1->probs[i] = (float *) malloc(o1->sizes[i] * sizeof(float));
      n1->log_probs[i] = (float *) malloc(o1->sizes[i] * sizeof(float));
      n1->wts_vec[i] = o1->wts_vec[i];
      n1->probs_vec[i] = o1->wts_vec[i];
      n1->log_probs_vec[i] = o1->log_probs_vec[i];
      for (j=0; j<n1->sizes[i]; j++) {
        n1->wts[i][j] = o1->wts[i][j];
        n1->probs[i][j] = o1->probs[i][j];
        n1->log_probs[i][j] = o1->log_probs[i][j];
      }
    }
  }
  else if (new->tppt == MM_S ) {
    o2 = &(old->ptype.mm_s);
    n2 = &(new->ptype.mm_s);
    n2->wt = o2->wt;
    n2->prob = o2->prob;
    n2->log_prob = o2->log_prob;
  }
  else if (new->tppt == MN_CN ) {
    o3 = &(old->ptype.mn_cn);
    n3 =&( new->ptype.mn_cn);
    n3->emp_means = (fptr) malloc(n_atts * sizeof(float));
    n3->emp_covar = (fptr *) malloc(n_atts * sizeof(fptr));
    n3->means = (fptr) malloc(n_atts * sizeof(float));
    n3->covariance = (fptr *) malloc(n_atts * sizeof(fptr));
    n3->factor = (fptr *) malloc(n_atts * sizeof(fptr));
    n3->ln_root = o3->ln_root;
    n3->values = (fptr) malloc(n_atts * sizeof(float));
    n3->temp_v = (fptr) malloc(n_atts * sizeof(float));
    n3->temp_m = (fptr *) malloc(n_atts * sizeof(fptr));
    n3->min_sigma_2s = (float *)malloc(n_atts * sizeof(float));

    for (i=0; i<n_atts; i++) {
      n3->emp_means[i] = o3->emp_means[i];
      n3->means[i] = o3->means[i];
      n3->values[i] = o3->values[i];
      n3->temp_v[i] = o3->temp_v[i];
      n3->min_sigma_2s[i] = o3->min_sigma_2s[i];

      n3->emp_covar[i] = (fptr) malloc(n_atts * sizeof(float));
      n3->covariance[i] = (fptr) malloc(n_atts * sizeof(float));
      n3->factor[i] = (fptr) malloc(n_atts * sizeof(float));
      n3->temp_m[i] = (fptr) malloc(n_atts * sizeof(float));
      for (j=0; j<n_atts; j++) {
        n3->emp_covar[i][j] = o3->emp_covar[i][j];
        n3->covariance[i][j] = o3->covariance[i][j];
        n3->factor[i][j] = o3->factor[i][j];
        n3->temp_m[i][j] = o3->temp_m[i][j];
      }
    }
  }
  else if (new->tppt == SM ) {

    o4 = &(old->ptype.sm);
    n4 = &(new->ptype.sm);
    *n4=*o4;                    /*get all scalars then replace pointers below*/
    n4->val_wts = (fptr) malloc(n_atts * sizeof(float));
    n4->val_probs = (fptr) malloc(n_atts * sizeof(float));
    n4->val_log_probs = (fptr) malloc(n_atts * sizeof(float));
    for (i=0; i<n_atts; i++) {
      n4->val_wts[i] = o4->val_wts[i];
      n4->val_probs[i] = o4->val_probs[i];
      n4->val_log_probs[i] = o4->val_log_probs[i];
    }
  }
  else if (new->tppt == SN_CM ) {

    o5 = &(old->ptype.sn_cm);
    n5 = &(new->ptype.sn_cm);
    *n5=*o5;                    /*get all scalars then replace pointers below*/
    /* no vectors or arrays to copy */
  }
  else if (new->tppt == SN_CN ) {

    o6 = &(old->ptype.sn_cn);
    n6 = &(new->ptype.sn_cn);
    *n6 = *o6;                  /*get all scalars then replace pointers below*/
    /* no vectors or arrays to copy */
  }
  else {
    fprintf(stderr, "ERROR: unknown enum MODEL_TYPES in copy_tparm=%d",
            new->tppt);
    abort();
  }
  return(new);
}


/* FREE_CLASS_DS
   07jan95 wmt: new

   free storage for class and its parameters
   */
void free_class_DS( class_DS class, char *type, clsf_DS clsf, int n_class)
{
  int n_tparm, n_att;
  tparm_DS tparm;
  char *att_type;
  i_discrete_DS i_discrete_struct;
  i_real_DS i_real_struct;

  if (G_clsf_storage_log_p == TRUE)
    fprintf(stdout, "free_class(%s): %p\n", type, (void *) class);
  G_n_freed_classes++;

  for (n_tparm=0; n_tparm < class->num_tparms ; n_tparm++) {
    tparm = class->tparms[n_tparm];
    free_tparm_DS( tparm);
  }
  free( class->tparms);

  if (class->i_values != NULL) {
    for (n_att=0; n_att<clsf->database->n_atts; n_att++) {
      att_type = report_att_type( clsf, n_class, n_att);
      if (eqstring( att_type, "discrete") == TRUE) {
        i_discrete_struct = (i_discrete_DS) class->i_values[n_att];
        free( i_discrete_struct->p_p_star_list);
        free( i_discrete_struct);
      }
      if (eqstring( att_type, "integer") == TRUE) {
        fprintf( stderr, "att_type integer not supported\n");
        abort();
      }
      if (eqstring( att_type, "real") == TRUE) {
        i_real_struct = (i_real_DS) class->i_values[n_att];
        free( i_real_struct->mean_sigma_list);
        free( i_real_struct);
        /* term_att_list & class_covar are pointers to other storage, not malloc's */
      }
    }   
    free( class->i_values);
  }
  /* wts are freed by compress_clsf called by try_variation,
     but for situations where they are not */
  if (class->wts != NULL) 
    free( class->wts);
  
  free( class);
}


/* FREE_TPARM_DS
   06jan94 wmt: new -- adapted from copy_tparm_DS
   24mar97 wmt: allow tparm->tppt to be UNKNOWN, TIGNORE
                print advisory msg, not error msg & abort

   aju 980612: Prefixed enum member IGNORE with T so it would not clash 
               with predefined Win32 type.  

   free components of tparm_DS
   */
void free_tparm_DS( tparm_DS tparm)
{
  int i, n_atts;
  struct mm_d_param *tparm1;
  struct mm_s_param *tparm2;
  struct mn_cn_param *tparm3;
  struct sm_param *tparm4;
  struct sn_cm_param *tparm5;
  struct sn_cn_param *tparm6;

  if(tparm == NULL) return;

  n_atts = tparm->n_atts;
  if (tparm->tppt == MM_D) {
    tparm1 =&( tparm->ptype.mm_d);
    free( tparm1->sizes);
    free( tparm1->wts);
    free( tparm1->probs);
    free( tparm1->log_probs);
    free( tparm1->wts_vec);
    free( tparm1->probs_vec);
    free( tparm1->log_probs_vec);
    for (i=0; i<n_atts; i++) {
      free( tparm1->wts[i]);
      free( tparm1->probs[i]);
      free( tparm1->log_probs[i]);
     }
    }
  else if (tparm->tppt == MM_S ) {
    tparm2 = &(tparm->ptype.mm_s);
  }
  else if (tparm->tppt == MN_CN ) {
    tparm3 =&( tparm->ptype.mn_cn);
    free( tparm3->emp_means);
    free( tparm3->means);
    free( tparm3->values);
    free( tparm3->temp_v);
    free( tparm3->min_sigma_2s);
    for (i=0; i<n_atts; i++) {
      free( tparm3->emp_covar[i]);
      free( tparm3->covariance[i]);
      free( tparm3->factor[i]);
      free( tparm3->temp_m[i]);
    }
    free( tparm3->emp_covar);
    free( tparm3->covariance);
    free( tparm3->factor);
    free( tparm3->temp_m);
  }
  else if (tparm->tppt == SM ) {
    tparm4 = &(tparm->ptype.sm);
    free( tparm4->val_wts);
    free( tparm4->val_probs);
    free( tparm4->val_log_probs);
  }
  else if (tparm->tppt == SN_CM ) {
    tparm5 = &(tparm->ptype.sn_cm);
  }
  else if (tparm->tppt == SN_CN ) {
    tparm6 = &(tparm->ptype.sn_cn);
  }
  else if ((tparm->tppt == UNKNOWN ) || (tparm->tppt == TIGNORE )) {
    /* nothing to free */
  }
  else {
    fprintf(stderr, "ADVISORY: unknown enum MODEL_TYPES in free_tparm_DS = %d\n",
            tparm->tppt);
    /* abort(); */
  }
  /* fprintf(stderr, "ADVISORY: enum MODEL_TYPES in free_tparm_DS = %d\n",
          tparm->tppt); */
  free( tparm);
}


/* LIST_CLASS_STORAGE
   07jan95 wmt: new
   05feb97 wmt: use list of void *, rather than list of int

   list the storage pointers to class structures which are stored
   in models
   return list of clsf pointers terminated by END_OF_INT_LIST
   */
void **list_class_storage ( int print_p)
{
  void **class_list_ptr = NULL;
  int num_class_ptrs = 0, n_model, n_class;
  class_DS class;
  model_DS model;

  if (G_model_list == NULL) {
    fprintf(stderr, "\nG_model_list is NULL\n");
    abort();
  }
  for (n_model=0; n_model<G_m_length; n_model++) {
    model = G_model_list[n_model];
    class = model->class_store;
    for (n_class=0; n_class < model->num_class_store; n_class++) {
      num_class_ptrs++;
      if (print_p == TRUE)
        printf("model-%d class_store %d: %p\n", n_model, num_class_ptrs, (void *) class);
      if (class_list_ptr == NULL)
        class_list_ptr = (void **) malloc( num_class_ptrs * sizeof(void *));
      else
        class_list_ptr = (void **) realloc( class_list_ptr, num_class_ptrs * sizeof(void *));
      class_list_ptr[num_class_ptrs - 1] = (void *) class;
      class = class->next;
    }
  }
  num_class_ptrs++;
  if (class_list_ptr == NULL)
    class_list_ptr = (void **) malloc( num_class_ptrs * sizeof(void *));
  else
    class_list_ptr = (void **) realloc( class_list_ptr, num_class_ptrs * sizeof(void *));
  class_list_ptr[num_class_ptrs - 1] = NULL;

  return( class_list_ptr);
}


/* CLASS_STRENGTH_MEASURE
   03feb95 wmt: new

   hueristic measure of class strength: class information density
   */
double class_strength_measure( class_DS class)
{

  return ( class->log_a_w_s_h_pi_theta / ((double) class->w_j));
}
