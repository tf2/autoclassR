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


/* PUSH_CLSF
   09jan95 wmt: add G_clsf_storage_log_p printout
*/
void push_clsf( clsf_DS clsf)
{
  int n_global_clsfs = 0;
  clsf_DS stored_clsf;
  
  clsf->next = G_clsf_store;
  G_clsf_store = clsf;
  if (G_clsf_storage_log_p == TRUE) {
    if (G_clsf_store != NULL) {
      n_global_clsfs++;
      stored_clsf = G_clsf_store->next;
      while (stored_clsf != NULL) {
        n_global_clsfs++;
        stored_clsf = stored_clsf->next;
      }
    }
    fprintf(stdout, "\npush_clsf(%.2d): %p\n", n_global_clsfs, (void *) clsf);
  }
}


/* POP_CLSF
   09jan95 wmt: add G_clsf_storage_log_p printout
*/
clsf_DS pop_clsf(void)
{
  clsf_DS temp = NULL, stored_clsf;
  int n_global_clsfs = 0;

  if (G_clsf_store != NULL) {
    temp = G_clsf_store;
    G_clsf_store = G_clsf_store->next;
    if (G_clsf_storage_log_p == TRUE) {
      if (G_clsf_store != NULL) {
        n_global_clsfs++;
        stored_clsf = G_clsf_store->next;
        while (stored_clsf != NULL) {
          n_global_clsfs++;
          stored_clsf = stored_clsf->next;
        }
      }
      fprintf(stdout, "\npop_clsf(%.2d): %p\n", n_global_clsfs, (void *) temp);
    }
  }
  return(temp);
}


/* GET_CLSF_DS
   09jan95 wmt: pass clsf_DS_max_n_classes to store_class_DS
*/
clsf_DS get_clsf_DS( int n_classes)
{
   int i;
   clsf_DS clsf;

   clsf = pop_clsf();
   if (clsf != NULL) {
      if (clsf->n_classes < n_classes) {
	 if (clsf->classes == NULL)
	    clsf->classes = (class_DS *) malloc(n_classes * sizeof(class_DS));
	 else clsf->classes =
	    (class_DS *) realloc(clsf->classes, n_classes * sizeof(class_DS));
      }
      else
         for (i=n_classes; i<clsf->n_classes; i++) {
	    store_class_DS(clsf->classes[i], clsf_DS_max_n_classes(clsf));
	    clsf->classes[i] = NULL;
         }
	/* note that could do realloc here to shrink but didnt to save time */
   }
   else {
      clsf = create_clsf_DS();
      clsf->classes = (class_DS *) malloc(n_classes * sizeof(class_DS));
   }
   clsf->n_classes = n_classes;
   return(clsf);
}


/* ADJUST_CLSF_DS_CLASSES
   09jan95 wmt: pass clsf_DS_max_n_classes to store_class_DS
   10apr97 wmt: add database->n_data to get_class_DS call

   This adjusts the fill-pointer vector clsf-DS-classes, to be at least
   n-classes long and to contain exactly n-classes class structures.
   */
void adjust_clsf_DS_classes( clsf_DS clsf, int n_classes)
{
   model_DS *models;
   int j, n_models, old_n_classes;
   class_DS cl, *classes;

                                          /* Extend classes vector as needed. */
   models = clsf->models;
   n_models = clsf->num_models;
   old_n_classes = clsf->n_classes;

   if (old_n_classes < n_classes) {
      if (clsf->classes == NULL)
         clsf->classes = (class_DS *) malloc(n_classes * sizeof(class_DS));
      else 
         clsf->classes = (class_DS *) realloc(clsf->classes, n_classes * sizeof(class_DS));
   }
   classes = clsf->classes;
   if (old_n_classes < n_classes)            /* Add classes to classes vector */
      for (j=old_n_classes; j<n_classes; j++)
	 classes[j] = get_class_DS(models[j%n_models], clsf->database->n_data,
                                   TRUE, FALSE);
   else if (old_n_classes > n_classes)      /* Remove & store excess classes. */
      for (j=n_classes; j<old_n_classes; j++) {
	 cl = classes[j];
	 store_class_DS(cl, clsf_DS_max_n_classes(clsf));
	 classes[j] = NULL;
      }
	/* note that could do realloc here to shrink but didnt to save time */
   clsf->n_classes = n_classes;
}


/* DISPLAY_STEP
   24jan95 wmt: free wts

   Displays the approximate marginal probability and class weights.
   */
void display_step( clsf_DS clsf, FILE *stream)
{
   int i;
   float *wts;
   wts = clsf_DS_w_j(clsf);
   fprintf(stream,"\ndisplaying weights for %d classes", clsf->n_classes);
   for (i=0; i<clsf->n_classes; i++)
      fprintf(stream, " %f", wts[i]);
   if (clsf->n_classes > 0)
      fprintf(stream, "\n");
   free( wts);
}


/* CREATE_CLSF_DS
   24oct94 wmt: added init for att_max_i_sum
   13nov94 wmt: added checkpoint struct
   24jan95 wmt: init temp->next
   02feb95 wmt: allocate reports struct
   */
clsf_DS create_clsf_DS( void)
{
   clsf_DS temp;
   chkpt_DS temp_chkpt;

   temp = (clsf_DS) malloc(sizeof(struct classification));
   if (G_clsf_storage_log_p == TRUE) {
     fprintf(stdout, "\ncreate_clsf_DS: %p\n", (void *) temp);
   }
   temp->log_p_x_h_pi_theta = 0.0;
   temp->log_a_x_h = 0.0;
   temp->database = NULL;
   temp->models = NULL;
   temp->num_models = 0;
   temp->n_classes = 0;
   temp->classes = NULL;
   temp->min_class_wt = 0.0;
   /* allocate checkpoint struct  */
   temp_chkpt = (chkpt_DS) malloc( sizeof( struct checkpoint));
   temp_chkpt->accumulated_try_time = 0;
   temp_chkpt->current_try_j_in = 0;
   temp_chkpt->current_cycle =0;
   temp->checkpoint = temp_chkpt;
   temp->reports = NULL;
   temp->next = NULL;
   return(temp); 
}


/*commented JTP not called and wouldnt work 
int clsf_DS_n_classes(clsf)
   clsf_DS clsf;
{
   return(sizeof(clsf) / sizeof(clsf_DS));
}*commented */


/* CLSF_DS_MAX_N_CLASSES
   11jan95 wmt: change computation -- put in floor
   */
int clsf_DS_max_n_classes( clsf_DS clsf)
{
   return (floor( (double) (((float) clsf->database->n_data) /
                            (1.1 * clsf->min_class_wt))));
}


/* COPY_CLSF_DS
   09dec94 wmt: realloc 1st arg cnew->num_models => cnew->models
   19jan95 wmt: copy num_wts, so it can be used to allocating storage
                in read_class_DS_s. revised by struct classification changes
   24jan95 wmt: change want_wts_p from local variable to parameter
   10apr97 wmt: add database->n_data to copy_class_DS call
   */  
clsf_DS copy_clsf_DS( clsf_DS cold, int want_wts_p)
{
   int i;
   clsf_DS cnew;

   cnew = get_clsf_DS(cold->n_classes);
   cnew->log_p_x_h_pi_theta  = cold->log_p_x_h_pi_theta;
   cnew->log_a_x_h           = cold->log_a_x_h;
   cnew->database            = cold->database;
   if (cnew->num_models != cold->num_models){
      if (cnew->models == NULL)
         cnew->models=(model_DS *) malloc(cold->num_models * sizeof(model_DS));
      else
         cnew->models=(model_DS *) realloc(cnew->models,
					cold->num_models * sizeof(model_DS));
   }
   cnew->num_models           = cold->num_models;
   for(i=0;i<cnew->num_models;i++)
      cnew->models[i] = cold->models[i];

   cnew->n_classes           = cold->n_classes;
   for(i=0;i<cold->n_classes;i++) {
      cnew->classes[i] = copy_class_DS( cold->classes[i], cold->database->n_data,
                                        want_wts_p);
      cnew->classes[i]->num_wts = cold->classes[i]->num_wts;
    }
   /* reports do not need to be copied */
   cnew->min_class_wt        = cold->min_class_wt;
   cnew->checkpoint->accumulated_try_time = cold->checkpoint->accumulated_try_time;
   cnew->checkpoint->current_try_j_in = cold->checkpoint->current_try_j_in;
   cnew->checkpoint->current_cycle = cold->checkpoint->current_cycle;

   return(cnew);
}


/* CLSF_DS_TEST
   18oct94 wmt: modified
   18may95 wmt: Remove "db_ds_same_source_p" and use "db_same_source_p", instead.

   Here we use search to eliminate any need for a canonical ordering.
   The cost in search time is On^2 on the classes.
   */
int clsf_DS_test( clsf_DS clsf1, clsf_DS clsf2, double rel_error)
{
  int i, found = 1;

   /* fprintf( stderr, "\nnew_clsf %.4f old_clsf %.4f\n", clsf1->log_a_x_h,
          clsf2->log_a_x_h); */

  if ((db_same_source_p( clsf1->database, clsf2->database) == TRUE) &&
      (clsf1->n_classes == clsf2->n_classes) &&
      (percent_equal( clsf1->log_a_x_h, clsf2->log_a_x_h,
                     rel_error) == TRUE)) {
    /* For each class in clsf-2, the following seeks a %= class in clsf-1: */
    for (i=0; i<clsf2->n_classes; i++) {
      /* fprintf( stderr, "\nnew_clsf class %d\n", i); */
      if (find_class_test2( clsf2->classes[i], clsf1, rel_error) == FALSE) {
        found = 0;
        break;       /* do not break for debug */
      }
    }
    if (found == 1)
      return(TRUE);
    else
      return(FALSE);
  }
  else {
    /* debug 
    for (i=0; i<clsf2->n_classes; i++) {        
      fprintf( stderr, "\nnew_clsf class %d\n", i); 
      find_class_test2( clsf2->classes[i], clsf1, rel_error);
    } */
    return(FALSE);
  }
}


/* STORE_CLSF_DS_CLASSES
   09jan95 wmt: pass clsf_DS_max_n_classes to store_class_DS

   Classification storage management:
   Push all classes in 'clsf onto the appropriate class-store for recycling,
   and reset the 'clsf's counters.  Discards class pointers duplicated in
   check-classes.  The duplicate checking is not normally necessary, but there
   have been pathlogical cases where two classifications have pointers to the
   same class after breaks.
   */
void store_clsf_DS_classes( clsf_DS clsf, class_DS *check_classes, int length)
{
   int i, n_classes;
   class_DS *classes;

   n_classes = clsf->n_classes;
   classes = clsf->classes;
				                      /* Clean up everything! */
   for (i=0; i<n_classes; i++) {
      /* store calls find so comment here if (find_class(classes[i], check_classes)== 0)*/
	 store_class_DS(classes[i], clsf_DS_max_n_classes(clsf));
      /* commented JTP dont free this since is in class store free(classes[i]);
      classes[i] = NULL; no need since free below commented JTP */
   }
   free(clsf->classes);
   clsf->classes = NULL;
   clsf->n_classes = 0;
}


/* STORE_CLSF_DS
   09jan95 wmt: check for clsf == temp; NULL database after calling
                store_clsf_DS_classes
   20jan95 wmt: revised by struct classification changes

   This stores the classes and classification for recycling.
   */
void store_clsf_DS( clsf_DS clsf, class_DS *check_classes, int length)
{
   int found;
   clsf_DS temp;

   clsf->log_p_x_h_pi_theta = 0.0;
   clsf->log_a_x_h = 0.0;

   store_clsf_DS_classes(clsf, check_classes, length);

   /* no dont free this, its the real database JTP free(clsf->database); */
   clsf->database = NULL;

    /* dont free since will probably need same number next time*/
   /* commented if(clsf->models != NULL) free(clsf->models);
   		clsf->models = NULL;
   		clsf->num_models = 0; ***** commented */

   clsf->min_class_wt = 0.0;

   found = 0;
   temp = G_clsf_store;
   if (clsf == temp)
     found = 1;
   while (found == 0 && temp != NULL) {
      if (clsf == temp->next)
	 found = 1;
      temp = temp->next;
   }
   if (found == 0)
      push_clsf(clsf);
}


float *clsf_DS_w_j( clsf_DS clsf)
{
   int i, n_classes = clsf->n_classes;
   float *wts;
   class_DS *classes = clsf->classes;

   wts = (float *) malloc(n_classes * sizeof(float));
   for (i=0; i<n_classes; i++)
      wts[i] = classes[i]->w_j;
   return(wts);
}


/* LIST_CLSF_STORAGE
   06jan95 wmt: new
   18feb95 wmt: add list_global_clsf_p
   05feb97 wmt: change ptr types from int to void *

   list the storage pointers to active clsf structures from
   current clsf and search structures and from global clsf store
   return list of clsf pointers terminated by END_OF_INT_LIST
   */
void **list_clsf_storage ( clsf_DS clsf, search_DS search, int print_p,
                        int list_global_clsf_p)
{
  void **clsf_list_ptr = NULL;
  int num_clsf_ptrs = 0, n_try, n_model;
  int n_global_clsfs = 0;
  clsf_DS stored_clsf, global_clsf;

  if (clsf == NULL) {
    fprintf(stderr, "\n NULL clsf passed to list_clsf_storage\n");
    abort();
  }
  if (print_p == TRUE)
    printf("clsf: %p\n", (void *) clsf);
  num_clsf_ptrs++;
  clsf_list_ptr = (void **) malloc( num_clsf_ptrs * sizeof( void *));
  clsf_list_ptr[num_clsf_ptrs - 1] = (void *) clsf;

  if (list_global_clsf_p == TRUE) {
    for (n_model=0; n_model<clsf->num_models; n_model++) {
      if ((global_clsf = clsf->models[n_model]->global_clsf) != NULL) {
        if (print_p == TRUE)
          printf("model global clsf: %p\n", (void *) global_clsf);
        num_clsf_ptrs++;
        clsf_list_ptr = (void **) realloc( clsf_list_ptr, num_clsf_ptrs * sizeof( void *));
        clsf_list_ptr[num_clsf_ptrs - 1] = (void *) global_clsf;
      }
    }
  }
  if (search != NULL) {
    for (n_try=0; n_try<search->n_tries; n_try++) {
      if (search->tries[n_try]->clsf != NULL) {
        if (print_p == TRUE) 
          printf("search_try_clsf %d: %p\n", n_try + 1, (void *) search->tries[n_try]->clsf);
        num_clsf_ptrs++;
        clsf_list_ptr = (void **) realloc( clsf_list_ptr, num_clsf_ptrs * sizeof( void *));
        clsf_list_ptr[num_clsf_ptrs - 1] = (void *) search->tries[n_try]->clsf;
      }
    }
  }
  if (G_clsf_store != NULL) {
    n_global_clsfs++;
    if (print_p == TRUE)
      printf("G_clsf_store %d: %p\n", n_global_clsfs, (void *) G_clsf_store);
    num_clsf_ptrs++;
    clsf_list_ptr = (void **) realloc( clsf_list_ptr, num_clsf_ptrs * sizeof( void *));
    clsf_list_ptr[num_clsf_ptrs - 1] = (void *) G_clsf_store;
    stored_clsf = G_clsf_store->next;
    while (stored_clsf != NULL) {
      n_global_clsfs++;
      if (print_p == TRUE)
        printf("G_clsf_store %d: %p\n", n_global_clsfs, (void *) stored_clsf);
      num_clsf_ptrs++;
      clsf_list_ptr = (void **) realloc( clsf_list_ptr, num_clsf_ptrs * sizeof( void *));
      clsf_list_ptr[num_clsf_ptrs - 1] = (void *) stored_clsf;
      stored_clsf = stored_clsf->next;
    }
  }
  num_clsf_ptrs++;
  clsf_list_ptr = (void **) realloc( clsf_list_ptr, num_clsf_ptrs * sizeof( void *));
  clsf_list_ptr[num_clsf_ptrs - 1] = NULL;
  return( clsf_list_ptr);
}


/* FREE_CLSF_DS
   06jan95 wmt: new
   30jun00 wmt: add pointer checks before freeing

   free storage for clsf and its classes and class parameters
   */
void free_clsf_DS( clsf_DS clsf)
{
  int i_class, i_att, n_classes = 0, n_atts;
  class_DS class;

  if (G_clsf_storage_log_p == TRUE)
    fprintf(stdout, "free_clsf: %p\n", (void *) clsf);

  if (clsf->classes != NULL) {
    n_classes = clsf->n_classes;
    for (i_class=0; i_class < clsf->n_classes; i_class++) {
      class = clsf->classes[i_class];
      free_class_DS( class, "clsf", clsf, i_class);
    }
    free( clsf->classes);
  }

  if (clsf->reports != NULL) {
    n_atts = clsf->database->n_atts;
    if (clsf->reports->class_strength != NULL) 
      free( clsf->reports->class_strength);
    if (clsf->reports->class_wt_ordering != NULL) 
      free( clsf->reports->class_wt_ordering);
    if (clsf->reports->att_model_term_types != NULL) {
      for (i_class=0; i_class<n_classes; i_class++) {
        for (i_att=0; i_att<n_atts; i_att++) 
          free( clsf->reports->att_model_term_types[i_class][i_att]);
        free( clsf->reports->att_model_term_types[i_class]);
      }
      free( clsf->reports->att_model_term_types);
    }
    free( clsf->reports->att_i_sums);
    free( clsf->reports->att_max_i_values);
    free( clsf->reports);
  }
  free( clsf->checkpoint);
  free( clsf);
}


/* CLSF_ATT_TYPE
   06feb95 wmt: new

   data type, one of G_att_type_data, of attribute n_att: 
   this is input data type,  model terms can change this to "ignore"
   */

char *clsf_att_type( clsf_DS clsf, int n_att)
{
  return (clsf->database->att_info[n_att]->type);
}
  
  
/* FREE_CLSF_CLASS_SEARCH_STORAGE
   18feb95 wmt: new
   05feb97 wmt: type of ptr variables changed from int * to void **
   04may01 jcw: Now clears G_model_list and G_m_length after free.

   free storage allocated for classification, class, search, and
   search try structures. used for both search and report functions
   */
void free_clsf_class_search_storage( clsf_DS clsf, search_DS search, int list_global_clsf_p)
{
  void **clsf_storage_ptrs = NULL, **class_storage_ptrs = NULL;
  int i, j;
  void **clsf_storage_ptrs_start, **class_storage_ptrs_start;
  clsf_DS null_clsf = NULL;
  search_try_DS try;

  if (G_clsf_storage_log_p == TRUE)
    printf("\nn_freed_classes = %.2d, n_create_classes_after_free = %.2d\n",
           G_n_freed_classes, G_n_create_classes_after_free);
  clsf_storage_ptrs = list_clsf_storage( clsf, search, G_clsf_storage_log_p,
                                        list_global_clsf_p);
  clsf_storage_ptrs_start = clsf_storage_ptrs;
  while (*clsf_storage_ptrs != NULL) {
    free_clsf_DS( (clsf_DS) *clsf_storage_ptrs);
    clsf_storage_ptrs++;
  }
  if (clsf_storage_ptrs != NULL)
    free( clsf_storage_ptrs_start);

  if ((list_global_clsf_p == TRUE) && (G_model_list != NULL)) {
    class_storage_ptrs = list_class_storage( G_clsf_storage_log_p);
    class_storage_ptrs_start = class_storage_ptrs;
    while (*class_storage_ptrs != NULL) {
      free_class_DS( (class_DS) *class_storage_ptrs, "model", null_clsf, 0);
      class_storage_ptrs++;
    }
    if (class_storage_ptrs != NULL)
      free( class_storage_ptrs_start);

    for (i=0; i<G_m_length; i++)
      free_model_DS( G_model_list[i], i);
    free( G_model_list);
    G_model_list = NULL;
    G_m_length = 0;
  }

  if (search != NULL) {
    for (i=0; i < search->n_tries; i++) {
      try = search->tries[i];
      if ((try->duplicates != NULL) && (try->n_duplicates != 0)) {
        for (j=0; j < try->n_duplicates; j++) {
          free( try->duplicates[j]);
          if (G_clsf_storage_log_p == TRUE)
            printf( "free search_try_dup: %d of %d\n", j, i);
        }
        free( try->duplicates);
      }
      free( try);
      if (G_clsf_storage_log_p == TRUE)
        printf( "free search_try: %d\n", i);
    }
    free( search->tries);  
    free( search);
    if (G_clsf_storage_log_p == TRUE)
      printf( "free search\n");
  }
}
