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


/* FIND_TRANSFORM
   15dec94 wmt: redo error msg

   Attempts to find the index of the transformed attributes.
   */
int find_transform( database_DS d_base, shortstr transform, int *att_list, int length,
                   FILE *log_file_fp, FILE *stream)
{
   int *temp, i, return_value;
   shortstr short_str;
   fxlstr str;
   char caller[] = "find_transform";

   if (find_str_in_table(transform, G_transforms, NUM_TRANSFORMS) < 0) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[2]: Attempt to find unknown transform %s on attributes:\n"
                   "          ", transform);
      for (i=0; i<length; i++) {
        sprintf(short_str, "%d ", att_list[i]);
        strcat( str, short_str);
      }
      strcat( str, "\n");
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
   }
   temp = (int *) get(transform, "n_args");

   if ((length == 1) && (temp[0] == 1)) {
      return_value = find_singleton_transform(d_base, transform, att_list[0],
                                              log_file_fp, stream);
      return( return_value);
    }
   else {
      fprintf(stderr,
	      "ERROR: Currently unable to deal with multiple argument transforms: %s\n",
	      transform);
      abort();
      return -1;
   }
}


/* Returns the index of the transformed attribute, after generating it
   if necessary.
   */
int find_singleton_transform( database_DS d_base, shortstr transform, int att_index,
                             FILE *log_file_fp, FILE *stream)
{
  int *index = NULL, return_value;

  fprintf( stderr, "find_singleton_transform: att_index = %d, index = %d\n",
     att_index, (int) index); 
  index = (int *) getf(d_base->att_info[att_index]->props, transform,
                       d_base->att_info[att_index]->n_props);

  /* fprintf( stderr, "find_singleton_transform: att_index = %d, index = %d\n",
     att_index, (int) index); */

  if (index != NULL)
    return(*index);
  else {
    return_value = generate_singleton_transform( d_base, transform, att_index,
                                                log_file_fp, stream);
    return( return_value);
  }
}


/* GENERATE_SINGLETON_TRANSFORM
   15dec94 wmt: replaced G_att_fnames, ATT_FLENGTH
                with G_transforms, NUM_TRANSFORMS
   10may95 wmt: call output_real_att_statistics
   05jun95 wmt: Add G_prediction_p
   21jun95 wmt: when realloc'ing d_base->att_info, initialize elements to NULL;
                & (++d_base->n_atts) >=, instead of (d_base->n_atts ++) >
   28jul95 wmt: relloc att_info prior to storing new_att
   */
int generate_singleton_transform( database_DS d_base, shortstr transform, int att_index,
                                 FILE *log_file_fp, FILE *stream)
{
  int *temp = NULL, new_index, fnumber, i;
  att_DS att = d_base->att_info[att_index], new_att;
  shortstr att_type, att_subtype;
  fxlstr str;
  char caller[] = "generate_singleton_transform";

  strcpy(att_type, att->type);
  strcpy(att_subtype, att->sub_type);
  new_index = d_base->n_atts;
  safe_sprintf( str, sizeof( str), caller,
               "ADVISORY[2]: %s is being applied to attribute #%d:\n"
               "             \"%s\" and will be stored as attribute #%d.\n",
               transform, att_index, att->dscrp, new_index);
  to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  fnumber = find_str_in_table(transform, G_transforms, NUM_TRANSFORMS);
  switch(fnumber) {
  case  0:  new_att = log_transform(att_index, d_base);  break;
  case  1:  new_att = log_odds_transform_c(att_index, d_base);  break;
  default:  fprintf(stderr,"\nERROR: (generate_singleton_transform) "
                    "Undefined transform; %s\n", transform);
    exit(1);
  }
  temp = (int *) malloc(sizeof(int));
  temp[0] = att_index;
  add_to_plist(new_att, "source", temp, "int");
  add_to_plist(new_att, "source_sub_type", att->sub_type, "str");

  if ((d_base->n_atts) >= d_base->allo_n_atts) {
    d_base->allo_n_atts *= 1.5;
    d_base->att_info = (att_DS *) realloc( d_base->att_info,
                                          d_base->allo_n_atts * sizeof( att_DS));
    for (i=d_base->n_atts; i<d_base->allo_n_atts; i++) 
      d_base->att_info[i] = NULL;
  }
  d_base->att_info[d_base->n_atts] = new_att;
  d_base->n_atts++;

  if (eqstring( new_att->type, "real") == TRUE) {
    if ((G_prediction_p == TRUE) && (G_training_clsf != NULL)) {
      /* force the "test" database to use the same statistics
         as the "training" database. */
      new_att->r_statistics =
        G_training_clsf->database->att_info[new_index]->r_statistics;
      new_att->missing = G_training_clsf->database->att_info[new_index]->missing;
      new_att->range = G_training_clsf->database->att_info[new_index]->range;
    }
    else {
      find_real_stats( d_base, new_index, log_file_fp, stream);
      output_real_att_statistics( d_base, new_index, log_file_fp, stream);
    }
  }
  else if (eqstring( new_att->type, "discrete") == TRUE)
    find_discrete_stats( d_base, new_index);

  temp = (int *) malloc( sizeof( int));
  temp[0] = new_index;
  add_to_plist( att, transform, temp, "int");
  return (new_index);
}


/* LOG_TRANSFORM
   22oct94 wmt: moved new_att malloc to near top
   27nov94 wmt: use percent_equal for float tests
   18dec94 wmt: increment datum_length array for transformed attribute
   23dec94 wmt: set unknown values to FLOAT_UNKNOWN, rather than INT_UNKNOWN
   28jul95 wmt: use safe_log to transform values.

   Generates a log-transform for `real' attributes, which have a constant error
   or relative error.  The constant error case can only have an approximate
   transformed error.  Such should really be transformed to a real&error type
   attribute where the new relative error is sensitive to the instance value. 
*/
att_DS log_transform( int att_index, database_DS d_base)
{
  void ***props;
  int i, j, n, n_props;
  float value, **data = d_base->data, zero, error, new_error;
  float rel_error, new_rel_error = 0.0, mean, mn;
  att_DS att, new_att;
  char caller[] = "log_transform";

  att = d_base->att_info[att_index];
  n_props = att->n_props;       
  props = (void ***) malloc(att->n_props * sizeof(void **));
  for (i=0; i<att->n_props; i++)
    props[i] = att->props[i];

  /****commented 
    temp = getf(props, "zero_point", att->n_props);
    if (temp == NULL)
    zero = 0.0;
    else zero = (float) temp[0];
    **********/

  zero = att->zero_point;
  error = att->error;
  rel_error = att->rel_error;
  mean = att->r_statistics->mean;
  mn = att->r_statistics->mn;

  /* ----  Input checks:  ---- */
  if (eqstring(att->type, "real") == FALSE) {
    fprintf(stderr, "ERROR: Attempt to apply log_transform to non-numerical "
            "attribute %d of type %s\n",
            att_index, att->type);
    exit(1);
  }
  if (zero > mn) { 
    if( (n = att->warnings_and_errors->num_expander_warnings ++) == 0)
      att->warnings_and_errors->model_expander_warnings =
        (fxlstr *) malloc(sizeof(fxlstr));
    else 
      att->warnings_and_errors->model_expander_warnings =
        (fxlstr *) realloc(att->warnings_and_errors->model_expander_warnings,
                           (n+1) * sizeof(fxlstr));
    safe_sprintf( att->warnings_and_errors->model_expander_warnings[n],
                 sizeof( att->warnings_and_errors->model_expander_warnings[n]),
                 caller,
                 "log transform of attribute# %d using mn %f rather than %f for"
                 " zero_point.\n Suggest decreasing attribute's rel_error.\n",
                  att_index, mn, zero);
    zero = mn;
    /****** temp[0] = zero;
      add_to_plist(att, "zero_point", zero, "flt"); ***commented */
    att->zero_point = mn;
  } 
  /* ----  Generate the transformed properties:  ---- */

  /* Remove "zero_point" from props */
  for (i=0; i<n_props; i++)
    if (eqstring(props[i][0], "zero_point") == TRUE) {
      for (j=i; j<(n_props-1); j++)
        props[j] = props[j+1];
      n_props--;
      break;
    }
  if (rel_error != 0.0) {
    new_error = (float) safe_log((double) (1.0 + rel_error));
    new_rel_error = 0.0;
  }
  /* Approximates a rel_error from error about the mean */
  else if (error != 0.0)
    new_error = (float) safe_log((double) (1.0 + (error / (mean - zero))));
  else {
    fprintf(stderr, "ERROR: Attribute %d has no error property\n",
            att_index);
    exit(1);
  }
  /* Shift zero by the source error so that integral cannot = -infinity */
  if (zero == mn) {
    if (error != 0.0)
      zero -= error;
    else
      zero -= rel_error * (mean - zero);
  }

  /* ----  Generate the new attribute values:  ---- */
  for (i=0; i<d_base->n_data; i++) {
    value = data[i][att_index];
    d_base->data[i] = (float *) realloc(d_base->data[i],
                                        (d_base->n_atts + 1) * sizeof( float));
    if (percent_equal( (double) value, FLOAT_UNKNOWN, REL_ERROR))
      d_base->data[i][d_base->n_atts] = (float) FLOAT_UNKNOWN;
    else
      d_base->data[i][d_base->n_atts] = (float) safe_log( (double) (value - zero));
    /* increment datum_length array for transformed attribute 18dec94 wmt */
    d_base->datum_length[i] += 1; 
  }

  new_att = (att_DS) malloc( sizeof( struct att));
  strcpy(new_att->type, att->type);
  strcpy(new_att->sub_type, "log_transform");
  if (eqstring(att->type, "real")) {
    new_att->r_statistics = (real_stats_DS) malloc(sizeof(struct real_stats));
    new_att->d_statistics = NULL;
  }
  else {
    new_att->d_statistics =
      (discrete_stats_DS) malloc(sizeof(struct discrete_stats));
    new_att->r_statistics = NULL;
  }
  safe_sprintf( new_att->dscrp, sizeof( new_att->dscrp), caller,
               "Log %s", att->dscrp);
  new_att->props = props;
  new_att->n_props = n_props;
  new_att->n_trans = att->n_trans;
  new_att->translations = att->translations;
  new_att->range = att->range;
  new_att->zero_point = att->zero_point;
  new_att->error = new_error;
  new_att->rel_error = new_rel_error;
  /* missing is set in store_real_stats */
  new_att->warnings_and_errors =  create_warn_err_DS();
   
  return(new_att);
}


/* LOG_ODDS_TRANSFORM_C
   20ct94 wmt: moved newe_att malloc to near top;  add n_props = att->n_props;
   27nov94 wmt: use percent_equal for float tests
   23dec94 wmt: set unknown values to FLOAT_UNKNOWN, rather than INT_UNKNOWN

   Generates a log-odds-transform for `real' attributes, which have a constant
   error or relative error.  The constant error case can only have an
   approximate transformed error.  Such should really be transformed to a
   real&error type attribute where the new relative error is sensitive to the
   instance value. */

att_DS log_odds_transform_c( int att_index, database_DS d_base)
{
  void ***props;
  int i, *temp, n_props;
  float value, **data = d_base->data;
  float mx, mn;
  att_DS att, new_att;
  char caller[] = "log_odds_transform_c";

  att = d_base->att_info[att_index];
  props = (void ***) malloc(att->n_props * sizeof(void **));
  new_att = (att_DS) malloc(sizeof(struct att));
  for (i=0; i<att->n_props; i++) /*******************************/
    new_att->props[i] = att->props[i];
  temp = getf(props, "min", att->n_props);
  if (temp == NULL)
    mn = 0.0;
  else mn = (float) temp[0];
  temp = getf(props, "max", att->n_props);
  if (temp == NULL)
    mx = 1.0;
  else mx = (float) temp[0];

  n_props = att->n_props; 

  /* ----  Generate the new attribute values:  ---- */
  for (i=0; i<d_base->n_data; i++) {
    value = data[i][att_index];
    d_base->data[i] = (float *) realloc(d_base->data[i],
                                        (d_base->n_atts + 1) * sizeof(float));
    if (percent_equal( (double) value, (double) FLOAT_UNKNOWN, (double) REL_ERROR))
      d_base->data[i][d_base->n_atts] = (float) FLOAT_UNKNOWN;
    else
      d_base->data[i][d_base->n_atts] =
        (float) ( safe_log((double) (value - mn)) - safe_log((double) (mx - value)));
  }
  strcpy(new_att->type, att->type);
  strcpy(new_att->sub_type, "log_odds_transform_c");
  if (eqstring(att->type, "real")) {
    new_att->r_statistics = (real_stats_DS) malloc(sizeof(struct real_stats));
    new_att->d_statistics = NULL;
  }
  else {
    new_att->d_statistics =
      (discrete_stats_DS) malloc(sizeof(struct discrete_stats));
    new_att->r_statistics = NULL;
  }
  safe_sprintf( new_att->dscrp, sizeof( new_att->dscrp), caller, "log %s",
               att->dscrp);
  new_att->props = props;
  new_att->n_props = n_props;
  new_att->n_trans = att->n_trans;
  new_att->translations = att->translations;
  new_att->range = att->range;
  new_att->zero_point = att->zero_point;
  new_att->error = att->error;
  new_att->rel_error = att->rel_error;
  /* missing is set in store_real_stats */
  new_att->warnings_and_errors = create_warn_err_DS();
  return(new_att);
}

