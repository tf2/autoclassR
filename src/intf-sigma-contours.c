#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#ifndef _WIN32
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

/* GENERATE_SIGMA_CONTOURS 

   call compute_sigma_contour_for_2_atts for permutations of
   sigma_att_list 

   24june97 wmt: new
   20sep98 wmt: filter windows e+000 => e+00, etc with filter_e_format_exponents
   */
   
void generate_sigma_contours ( clsf_DS clsf, int clsf_class_number,
                               int_list sigma_att_list,
                               FILE *influence_report_fp,
                               int comment_data_headers_p)
{
  float mean_x = 0.0, sigma_x = 0.0, mean_y = 0.0;
  float sigma_y = 0.0, rotation = 0.0;
  int att_x = -1, att_y = -1, i, j, error_p, sigma_contours_list_len = 0;
  int trans_att_x = -1, trans_att_y = -1, report_class_number;
  int *att_err_msg_p = NULL;
  att_DS att_info_x, att_info_y;
  int term_index_x = -1, term_index_y = -1;
  model_DS model = clsf->classes[clsf_class_number]->model;
  class_DS class = clsf->classes[clsf_class_number];
  term_DS *terms, term_x = NULL, term_y = NULL;
  char ignore_str[] = "ignore";
  /* force column alignment for windows */
#ifdef _WIN32
  char* format_string = "%06d %05d %+13e %+13e %+13e %+13e %+13e\n";
#else
  char* format_string = "%06d %05d %13e %13e %13e %13e %13e\n";
#endif
  fxlstr e_format_string;

  /* the first class to be output is report_class_number = 0 */
  report_class_number = map_class_num_clsf_to_report( clsf, clsf_class_number);
  /* allocate space for error msg flags */
  for (i=0; sigma_att_list[i] != END_OF_INT_LIST; i++) {
    sigma_contours_list_len++;
  }
  att_err_msg_p = (int *) malloc( (sigma_contours_list_len + 1) * sizeof( int));
  for (i = 0; i < sigma_contours_list_len; i++) {
    att_err_msg_p[i] = FALSE;
  }
  /* 6|6|14||14||14||14||14| */
  fprintf( influence_report_fp, "\nSIGMA CONTOURS \n"
           "%satt_x att_y        mean_x       sigma_x        mean_y       sigma_y"
           "  rotation-rad\n", (comment_data_headers_p == TRUE) ? "#" : "");

  for (i=0; sigma_att_list[i] != END_OF_INT_LIST; i++) {
    att_x = sigma_att_list[i];
    for (j=i; sigma_att_list[j] != END_OF_INT_LIST; j++) {
      att_y = sigma_att_list[j];
      if (att_x != att_y) {
        /* fprintf( stderr, "att_x %d att_y %d\n", att_x, att_y); */
        error_p = FALSE;
        att_info_x = clsf->database->att_info[att_x];
        att_info_y = clsf->database->att_info[att_y];
        /*
        fprintf( stderr, "att_x %d msg_p %d att_y %d msg_p %d\n", 
                 att_x, att_err_msg_p[i], 
                 att_y,  att_err_msg_p[j]); 
        */
        if (eqstring( att_info_x->type, "real") == FALSE) {
          if ((report_class_number == 0) && (att_err_msg_p[i] == FALSE)) {
            fprintf( stderr, "ADVISORY: compute sigma contour => att_n %d "
                     "(\"%s\") \n"
                     "          is not a type real attribute.\n",
                     att_x, att_info_x->dscrp);
            att_err_msg_p[i] = TRUE;
          }
          error_p = TRUE;
        }
        if (eqstring( att_info_y->type, "real") == FALSE) {
          if ((report_class_number == 0) && (att_err_msg_p[j] == FALSE)) {
            fprintf( stderr, "ADVISORY: compute sigma contour => att_n %d "
                     "(\"%s\") \n" 
                     "          is not a type real attribute.\n",
                     att_y, att_info_y->dscrp);
            att_err_msg_p[j] = TRUE;
          }
          error_p = TRUE;
        }
        /*
        fprintf( stderr, "generate_sigma_contours: att_index %d att_loc_string %s\n",  
                 att_x, model->att_locs[att_x]);  
        fprintf( stderr, "generate_sigma_contours: att_index %d att_loc_string %s\n",  
                 att_y, model->att_locs[att_y]);  
        fprintf( stderr, "error_p %d ignore %p report_class_number %d msg %d\n", 
                error_p, strstr( ignore_str, model->att_locs[att_x] ),  
                report_class_number, att_err_msg_p[i]); 
        fprintf( stderr, "error_p %d ignore %p report_class_number %d msg %d\n", 
                error_p, strstr( ignore_str, model->att_locs[att_y] ),  
                report_class_number, att_err_msg_p[j]);
        */
        if (error_p == FALSE) {
          if (strstr( ignore_str, model->att_locs[att_x] ) != NULL) {
            if ((report_class_number == 0) && (att_err_msg_p[i] == FALSE)) {
              fprintf( stderr, "ADVISORY: compute sigma contour => att_n %d "
                       "(\"%s\") \n"
                       "          has been declared ignore in the .model file.\n",
                       att_x, att_info_x->dscrp);
              att_err_msg_p[i] = TRUE;
            }
            error_p = TRUE;
          }
          if (strstr( ignore_str, model->att_locs[att_y] ) != NULL) {
            if ((report_class_number == 0) && (att_err_msg_p[j] == FALSE)) {
              fprintf( stderr, "ADVISORY: compute sigma contour => att_n %d "
                       "(\"%s\") \n"
                       "          has been declared ignore in the .model file.\n",
                       att_y, att_info_y->dscrp);
              att_err_msg_p[j] = TRUE;
            }
            error_p = TRUE;
          }
        }
        
        if (error_p == FALSE) {
          term_index_x = class_att_loc( class, att_x, &trans_att_x);
          term_index_y = class_att_loc( class, att_y, &trans_att_y);
          terms = model->terms;
          term_x = terms[term_index_x];
          term_y = terms[term_index_y];
          /*
          fprintf( stderr, "generate_sigma_contours: term_x->type %s\n", term_x->type); 
          fprintf( stderr, "generate_sigma_contours: term_y->type %s\n", term_y->type); 
          */
          if (strstr( term_x->type, "normal") == NULL) {
            if ((report_class_number == 0) && (att_err_msg_p[i] == FALSE)) {
              fprintf( stderr, "ADVISORY: compute sigma contour =>"
                       " term_type %s \n"
                       "          is not a `normal' term for att_n %d (\"%s\") \n",
                       term_x->type, att_x, att_info_x->dscrp);
              att_err_msg_p[i] = TRUE;
            }
            error_p = TRUE;
          }
          if (strstr( term_y->type, "normal") == NULL) {
            if ((report_class_number == 0) && (att_err_msg_p[j] == FALSE)) {
              fprintf( stderr, "ADVISORY: compute sigma contour =>"
                       " term_type %s \n"
                       "          is not a `normal' term for att_n %d (\"%s\") \n",
                       term_y->type, att_y, att_info_y->dscrp);
              att_err_msg_p[j] = TRUE;
            }
            error_p = TRUE;
          }
        }
        /*
          fprintf( stderr, "att_x %d trans_att_x %d term_index_x %d term_type_x %s\n",   
          att_x, trans_att_x, term_index_x, term_x->type);   
          fprintf( stderr, "att_y %d trans_att_y %d term_index_y %d term_type_y %s\n",   
          att_y, trans_att_y, term_index_y, term_y->type);
        */
        /*  fprintf( stderr, "att_x %d att_y %d\n", att_x, att_y); */
 
        if (error_p == FALSE) {
          compute_sigma_contour_for_2_atts ( clsf, clsf_class_number, 
                                             att_x, att_y,
                                             trans_att_x, trans_att_y,
                                             term_index_x, term_index_y,
                                             &mean_x, &sigma_x,
                                             &mean_y, &sigma_y,
                                             &rotation);
          sprintf( e_format_string, format_string,
                   trans_att_x, trans_att_y, mean_x, sigma_x, mean_y, sigma_y,
                   rotation);
          fprintf( influence_report_fp, "%s", filter_e_format_exponents( e_format_string));


        }
      }
    }
  }
  free( att_err_msg_p);
}



/* COMPUTE_SIGMA_CONTOUR_FOR_2_ATTS
   21jun97 wmt: new
   06sep97 jcs: corrected initialization of *rotation

   compute sigma contour for two real valued attributes.
   ported from Draw-2-Attributes (intf-graphics.lisp)
   */
int compute_sigma_contour_for_2_atts ( clsf_DS clsf, int clsf_class_number,
                                       int att_x, int att_y,
                                       int trans_att_x, int trans_att_y,
                                       int term_index_x, int term_index_y,
                                       float *mean_x, float *sigma_x,
                                       float *mean_y, float *sigma_y,
                                       float *rotation) 
{
  int n_term_list;
  class_DS class = clsf->classes[clsf_class_number];
  model_DS model = clsf->classes[clsf_class_number]->model;
  term_DS *terms, term_x, term_y;
  i_real_DS i_real_struct = NULL;
  float **class_covar_x = NULL, **class_covar_y = NULL;
  float *term_list;
  float sigma_x_y, sum, diff, sigma_x_sq, sigma_y_sq, diff_sigma_x_y_term;
  float rotation_increment, arg;

  terms = model->terms;
  term_x = terms[term_index_x];
  term_y = terms[term_index_y];
  
  i_real_struct = (i_real_DS) class->i_values[trans_att_x];
  *mean_x = i_real_struct->mean_sigma_list[0];
  *sigma_x = i_real_struct->mean_sigma_list[1];
  class_covar_x = i_real_struct->class_covar;

  i_real_struct = (i_real_DS) class->i_values[trans_att_y];
  *mean_y = i_real_struct->mean_sigma_list[0];
  *sigma_y = i_real_struct->mean_sigma_list[1];
  class_covar_y = i_real_struct->class_covar;
  /*
  fprintf( stderr, "att_x %d, att_y %d, mean_x %e, sigma_x %e " 
           " mean_y %e sigma_y %e rotation %e\n", att_x, att_y, 
           *mean_x, *sigma_x, *mean_y, *sigma_y, *rotation); 
           */
  if (term_index_x == term_index_y) {
  /* We have a covariant pair _ so get rotation, and sigmas in rotated system */

    term_list = i_real_struct->term_att_list;
    n_term_list = i_real_struct->n_term_att_list;
    sigma_x_y = get_sigma_x_y( trans_att_x, trans_att_y, class, n_term_list,
                               term_list, class_covar_x);
    sigma_x_sq = *sigma_x * *sigma_x;
    sigma_y_sq = *sigma_y * *sigma_y;
    sum = sigma_x_sq + sigma_y_sq;
    diff = sigma_x_sq - sigma_y_sq;
    /*
    fprintf( stderr, "sigma_x_y %e sum %e diff %e \n", sigma_x_y, sum, diff);
    */
    if (*sigma_y > *sigma_x) {
      *rotation = 0.5 *  (float) M_PI;
    } else {
      *rotation = 0.0;
    }
    diff_sigma_x_y_term = sqrt( (diff * diff) +
                                          ( 4.0 * sigma_x_y * sigma_x_y));
    *sigma_x = sqrt( 0.5 * ( sum + diff_sigma_x_y_term));
    *sigma_y = sqrt( 0.5 * ( sum - diff_sigma_x_y_term));
    if (percent_equal( diff, 0.0, SINGLE_FLOAT_EPSILON) == TRUE) {
      rotation_increment = ( (float) M_PI) / 4.0;
    } else {
      arg = ( 2.0 * sigma_x_y ) / diff;
      rotation_increment = (float) (0.5 * atan( (double) arg));
    }
    *rotation = *rotation + rotation_increment;
  } else {
    /* Non-covariant pair - make certain *rotation is zero! 
       We were retaining the value computed for the last covariant pair.  */
    *rotation = 0.0;
  }
  return(1);
}


/* CLASS_ATT_LOC
   return transformed index for attribute att_index 

   sprintf(model->att_locs[old_i], "TRANSFORMED->%d", new_i);
   sprintf(model->att_locs[new_i], "%d", n_term);

   23jun97 wmt: new
   26feb98 wmt: check for real attributes defined in .hd2, but
                are ignored in .model; generate error to user
   */
int class_att_loc( class_DS class, int att_index, int *trans_att_index)
{
  model_DS model;
  shortstr att_loc_string;
  int term_index = -1;
  char greater_than_char = '>';
  char *str_index;

  model = class->model;
  
  strcpy( att_loc_string, model->att_locs[att_index]);
  /*
    fprintf( stderr, "att_index %d att_loc_string %s\n",
     att_index, att_loc_string);
  */
  str_index = strchr( att_loc_string, greater_than_char);
  if (str_index != NULL) {
    /* this is a transformed attribute -- what we expected
       att type => real scalar */ 
    strcpy( att_loc_string, ++str_index);
    sscanf( att_loc_string, "%d", trans_att_index);
    strcpy( att_loc_string, model->att_locs[*trans_att_index]);
  } else {
    /* att type => real location -- no transformation done */
    *trans_att_index = att_index;
  }
  sscanf( att_loc_string, "%d", &term_index);
  /*
  fprintf( stderr, "att_index %d trans_att_index %d term_index %d\n",   
           att_index, *trans_att_index, term_index);  
  */
  return( term_index);
}


/* GET_SIGMA_X_Y
   return the covariant element for att_x and att_y

   23jun97 wmt: new
   */
float get_sigma_x_y (int att_x, int att_y, class_DS class, int n_term_list,
                     float *term_list, float **covariance)
{
  int covar_index_x = -1, covar_index_y = -1, i;

  for (i = 0; i < n_term_list; i++) {
    if (((int) floor( (double) term_list[i])) == att_x) {
      covar_index_x = i;
      break;
    }
  }
  for (i = 0; i < n_term_list; i++) {
    if (((int) floor( (double) term_list[i])) == att_y) {
      covar_index_y = i;
      break;
    }
  }
  /*
  fprintf( stderr, "covar_index_x %d covar_index_y %d \n",
           covar_index_x, covar_index_y);
           */
  return covariance[covar_index_x][covar_index_y];
}









