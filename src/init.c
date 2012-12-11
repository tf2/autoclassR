#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifdef _MSC_VER
#define getcwd _getcwd
#include <direct.h>
#else
#include <sys/param.h> /* for MAXPATHLEN */
#include <unistd.h> /* getcwd */
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


/* INIT
   15dec94 wmt: discarded G_att_fnames - put its values in G_transforms
   26apr95 wmt: use getcwd, rather than getwd for Solaris compatibility
   29apr95 wmt: set G_solaris
   */
void init( void)
{
  FILE *fp;
  double rand_range = 31.0, one = 1.0, two = 2.0;
#ifdef _WIN32
  char* slash = "\\";
#else
  char* slash = "/";
#endif

  strcpy(G_transforms[0] ,  "log_transform");
  strcpy( G_transforms[1] ,  "log_odds_transform");

  strcpy( G_att_type_data[0] ,  "dummy");
  strcpy( G_att_type_data[1] ,  "none");
  strcpy( G_att_type_data[2] ,  "discrete");
  strcpy( G_att_type_data[3] ,  "real");
  strcpy( G_att_type_data[4] ,  "real_and_error");

  strcpy( G_checkpoint_file, "");   

  /* library function getcwd (requires sys/param.h) */
  strcpy( G_absolute_pathname, "");
  if (getcwd( G_absolute_pathname, MAXPATHLEN - 2) == NULL) {
     strcat( G_absolute_pathname, "<current working directory>");
     strcat( G_absolute_pathname, slash);
     fprintf( stderr, "\nWARNING: calling getwd (current working directory) returned 0\n");
   }
   else 
     strcat( G_absolute_pathname, slash);

  /* set G_solaris, if applicable */
  fp = fopen( "/usr/ucb/hostname", "r");
  if (fp != NULL) {
    fclose( fp);
    G_solaris = TRUE;
  }
  G_rand_base_normalizer =  pow( two, rand_range) - one;

  init_properties();
}


void init_properties(void)
{
   void ***t1, ***t1temp, ***t1temptemp;
   int *i2, *val1;
   char **t2, ***types;

   add_property("multi_multinomial_d", "modulus", "multiple");
   add_property("multi_multinomial_d", "type", "multinomial");
   add_property("multi_multinomial_d", "error", NULL);
   add_property("multi_multinomial_d", "missing", "allowed");
   add_property("multi_multinomial_d", "print_string", "MM_D");
   add_property("multi_multinomial_d", "params", "mm_d_params_DS");
	                                         /* See check_attribute_types */
   t1 = (void ***) malloc(2 * sizeof(void **));
   t1[0] = (void **) malloc(2 * sizeof(void *));
   t1[1] = (void **) malloc(2 * sizeof(void *));
   t1[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[0][0], "discrete");
   t1[0][1] = (void ***) malloc(3 * sizeof(void **));
   t1temp = t1[0][1];
   t1temp[0] = (void **) malloc(2 * sizeof(void *));
   t1temp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[0][0], "nominal");
   t1temp[0][1] = NULL;
   t1temp[1] = (void **) malloc(2 * sizeof(void *));
   t1temp[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[1][0], "ordered");
   t1temp[1][1] = NULL;
   t1temp[2] = (void **) malloc(2 * sizeof(void *));
   t1temp[2][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[2][0], "circular");
   t1temp[2][1] = NULL;
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t1[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[1][0], "n_discrete");
   t1[1][1] = i2;
   val1 = (int *) malloc(sizeof(int));
   val1[0] = 2;
   add_property("multi_multinomial_d", "att_trans_data", t1);
   add_property("multi_multinomial_d", "n_att_trans_data", val1);
   add_property("multi_multinomial_d", "single_equivalent",
		"single_multinomial");
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t2 = (char **) malloc(i2[0] * sizeof(char *));
   t2[0] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[0], "multi_multinomial_d");
   t2[1] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[1], "multi_multinomial_s");
   t2[2] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[2], "multi_multinomial_choose");
   add_property("multi_multinomial_d", "multiple_equivalent", t2);
   add_property("multi_multinomial_d", "n_multiple_equivalent", i2);


   add_property("multi_multinomial_s", "modulus", "multiple");
   add_property("multi_multinomial_s", "type", "multinomial");
   add_property("multi_multinomial_s", "error", NULL);
   add_property("multi_multinomial_s", "missing", "allowed");
   add_property("multi_multinomial_s", "print_string", "MM_S");
   add_property("multi_multinomial_s", "params", "mm_d_params_DS");
	                                         /* See check_attribute_types */
   t1 = (void ***) malloc(2 * sizeof(void **));
   t1[0] = (void **) malloc(2 * sizeof(void *));
   t1[1] = (void **) malloc(2 * sizeof(void *));
   t1[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[0][0], "discrete");
   t1[0][1] = (void ***) malloc(3 * sizeof(void **));   /* 22oct94 wmt: 2=>3 */
   t1temp = t1[0][1];
   t1temp[0] = (void **) malloc(2 * sizeof(void *));
   t1temp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[0][0], "nominal");
   t1temp[0][1] = NULL;
   t1temp[1] = (void **) malloc(2 * sizeof(void *));
   t1temp[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[1][0], "ordered");
   t1temp[1][1] = NULL;
   t1temp[2] = (void **) malloc(2 * sizeof(void *));
   t1temp[2][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[2][0], "circular");
   t1temp[2][1] = NULL;
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t1[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[1][0], "n_discrete");
   t1[1][1] = i2;
   val1 = (int *) malloc(sizeof(int));
   val1[0] = 2;
   add_property("multi_multinomial_s", "att_trans_data", t1);
   add_property("multi_multinomial_s", "n_att_trans_data", val1);
   add_property("multi_multinomial_s", "single_equivalent",
		"single_multinomial");
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t2 = (char **) malloc(i2[0] * sizeof(char *));
   t2[0] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[0], "multi_multinomial_s");
   t2[1] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[1], "multi_multinomial_d");
   t2[2] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[2], "multi_multinomial_choose");
   add_property("multi_multinomial_s", "multiple_equivalent", t2);
   add_property("multi_multinomial_s", "n_multiple_equivalent", i2);


   add_property("multi_normal_cn", "modulus", "multiple");
   add_property("multi_normal_cn", "type", "normal");
   add_property("multi_normal_cn", "error", "constant");
   add_property("multi_normal_cn", "missing", "no");
   add_property("multi_normal_cn", "print_string", "MNcn");
   add_property("multi_normal_cn", "params", "mn_cn_params_DS");
	                                         /* See check_attribute_types */
   t1 = (void ***) malloc(2 * sizeof(void **));
   t1[0] = (void **) malloc(2 * sizeof(void *));
   t1[1] = (void **) malloc(2 * sizeof(void *));
   t1[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[0][0], "real");
   t1[0][1] = (void ***) malloc(3 * sizeof(void **));
   t1temp = t1[0][1];
   t1temp[0] = (void **) malloc(2 * sizeof(void *));
   t1temp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[0][0], "location");
   t1temp[0][1] = NULL;
   t1temp[1] = (void **) malloc(2 * sizeof(void *));
   t1temp[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[1][0], "scalar");
   t1temp[1][1] = (void ***) malloc(sizeof(void **));
   t1temptemp = t1temp[1][1];
   t1temptemp[0] = (void **) malloc(2 * sizeof(void  *));
   t1temptemp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temptemp[0][0], "transform");
   t1temptemp[0][1] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temptemp[0][1], "log_transform");
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t1[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[1][0], "n_real");
   t1[1][1] = i2;
   t1temp[2] = (void **) malloc(2 * sizeof(void *));
   t1temp[2][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[2][0], "n_scalar");
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 1;
   t1temp[2][1] = i2;
   val1 = (int *) malloc(sizeof(int));
   val1[0] = 2;
   add_property("multi_normal_cn", "att_trans_data", t1);
   add_property("multi_normal_cn", "n_att_trans_data", val1);
   add_property("multi_normal_cn", "single_equivalent", "single_normal_cn");
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 1;
   t2 = (char **) malloc(i2[0] * sizeof(char *));
   t2[0] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[0], "multi_normal_cn");
   add_property("multi_normal_cn", "multiple_equivalent", t2);
   add_property("multi_normal_cn", "n_multiple_equivalent", i2);


   add_property("single_multinomial", "modulus", "single");
   add_property("single_multinomial", "type", "multinomial");
   add_property("single_multinomial", "error", NULL);
   add_property("single_multinomial", "missing", "allowed");
   add_property("single_multinomial", "print_string", "SM");
   add_property("single_multinomial", "params", "sm_params_DS");
	                                         /* See check_attribute_types */
   t1 = (void ***) malloc(2 * sizeof(void **));
   t1[0] = (void **) malloc(3 * sizeof(void *));
   t1[1] = (void **) malloc(2 * sizeof(void *));
   t1[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[0][0], "discrete");
   t1[0][1] = (void ***) malloc(3 * sizeof(void **));
   t1temp = t1[0][1];
   t1temp[0] = (void **) malloc(2 * sizeof(void *));
   t1temp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[0][0], "nominal");
   t1temp[0][1] = NULL;
   t1temp[1] = (void **) malloc(2 * sizeof(void *));
   t1temp[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[1][0], "ordered");
   t1temp[1][1] = NULL;
   t1temp[2] = (void **) malloc(2 * sizeof(void *));
   t1temp[2][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[2][0], "circular");
   t1temp[2][1] = NULL;
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t1[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[1][0], "n_discrete");
   t1[1][1] = i2;
   val1 = (int *) malloc(sizeof(int));
   val1[0] = 2;
   add_property("single_multinomial", "att_trans_data", t1);
   add_property("single_multinomial", "n_att_trans_data", val1);
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t2 = (char **) malloc(i2[0] * sizeof(char *));
   t2[0] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[0], "multi_multinomial_d");
   t2[1] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[1], "multi_multinomial_s");
   t2[2] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[2], "multi_multinomial_choose");
   add_property("single_multinomial", "multiple_equivalent", t2);
   add_property("single_multinomial", "n_multiple_equivalent", i2);


   add_property("single_normal_cn", "modulus", "single");
   add_property("single_normal_cn", "type", "normal");
   add_property("single_normal_cn", "error", "constant");
   add_property("single_normal_cn", "missing", "no");
   add_property("single_normal_cn", "print_string", "SNcn");
   add_property("single_normal_cn", "params", "sn_cn_params_DS");
	                                         /* See check_attribute_types */
   t1 = (void ***) malloc(2 * sizeof(void **));
   t1[0] = (void **) malloc(2 * sizeof(void *));
   t1[1] = (void **) malloc(2 * sizeof(void *));
   t1[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[0][0], "real");
   t1[0][1] = (void ***) malloc(3 * sizeof(void **));
   t1temp = t1[0][1];
   t1temp[0] = (void **) malloc(2 * sizeof(void *));
   t1temp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[0][0], "location");
   t1temp[0][1] = NULL;
   t1temp[1] = (void **) malloc(2 * sizeof(void *));
   t1temp[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[1][0], "scalar");
   t1temp[1][1] = (void ***) malloc(sizeof(void **));
   t1temptemp = t1temp[1][1];
   t1temptemp[0] = (void **) malloc(2 * sizeof(void  *));
   t1temptemp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temptemp[0][0], "transform");
   t1temptemp[0][1] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temptemp[0][1], "log_transform");
   t1temp[2] = (void **) malloc(2 * sizeof(void *));
   t1temp[2][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[2][0], "n_scalar");
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 1;
   t1temp[2][1] = i2;
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t1[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[1][0], "n_real");
   t1[1][1] = i2;
   val1 = (int *) malloc(sizeof(int));
   val1[0] = 2;
   add_property("single_normal_cn", "att_trans_data", t1);
   add_property("single_normal_cn", "n_att_trans_data", val1);
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 1;
   t2 = (char **) malloc(i2[0] * sizeof(char *));
   t2[0] = (char *) malloc(STRLIMIT * sizeof(char));
   strcpy(t2[0], "multi_normal_cn");
   add_property("single_normal_cn", "multiple_equivalent", t2);
   add_property("single_normal_cn", "n_multiple_equivalent", i2);


   add_property("single_normal_cm", "modulus", "single");
   add_property("single_normal_cm", "type", "normal");
   add_property("single_normal_cm", "error", "constant");
   add_property("single_normal_cm", "missing", "yes");
   add_property("single_normal_cm", "print_string", "SNcm");
   add_property("single_normal_cm", "params", "sn_cm_params_DS");
	                                         /* See check_attribute_types */
   t1 = (void ***) malloc(2 * sizeof(void **));
   t1[0] = (void **) malloc(2 * sizeof(void *));
   t1[1] = (void **) malloc(2 * sizeof(void *));
   t1[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[0][0], "real");
   t1[0][1] = (void ***) malloc(3 * sizeof(void **));
   t1temp = t1[0][1];
   t1temp[0] = (void **) malloc(2 * sizeof(void *));
   t1temp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[0][0], "location");
   t1temp[0][1] = NULL;
   t1temp[1] = (void **) malloc(2 * sizeof(void *));
   t1temp[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[1][0], "scalar");
   t1temp[1][1] = (void ***) malloc(sizeof(void **));
   t1temptemp = t1temp[1][1];
   t1temptemp[0] = (void **) malloc(2 * sizeof(void  *));
   t1temptemp[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temptemp[0][0], "transform");
   t1temptemp[0][1] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temptemp[0][1], "log_transform");
   t1temp[2] = (void **) malloc(2 * sizeof(void *));
   t1temp[2][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1temp[2][0], "n_scalar");
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 1;
   t1temp[2][1] = i2;
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 3;
   t1[1][0] = (char *) malloc(20 * sizeof(char));
   strcpy(t1[1][0], "n_real");
   t1[1][1] = i2;
   val1 = (int *) malloc(sizeof(int));
   val1[0] = 2;
   add_property("single_normal_cm", "att_trans_data", t1);
   add_property("single_normal_cm", "n_att_trans_data", val1);
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 0;
   add_property("single_normal_cm", "multiple_equivalent", NULL);
   add_property("single_normal_cm", "n_multiple_equivalent", i2);


   i2 = (int *) malloc(sizeof(int));
   i2[0] = 1;
   add_property("log_transform", "n_args", i2);
   types = (char ***) malloc(sizeof(char **));
   types[0] = (char **) malloc(3 * sizeof(char *));
   types[0][0] = (char *) malloc(20 * sizeof(char));
   strcpy(types[0][0], "real");
   types[0][1] = (char *) malloc(20 * sizeof(char));
   strcpy(types[0][1], "location");
   types[0][2] = (char *) malloc(20 * sizeof(char));
   strcpy(types[0][2], "scalar");
   i2 = (int *) malloc(sizeof(int));
   i2[0] = 1;
   add_property("log_transform", "types", types);
   add_property("log_transform", "n_types", i2);
}
