#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
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


/* PUTPARAMS
   11nov94 wmt: modified from putparam -- add paramtypes TDOUBLE & TINT_LIST
   11jan95 wmt: add paramptr_overridden

   to print parameter table
   when overridden-p = TRUE, only print params which have been overridden
 */
void putparams( FILE *fp, PARAMP pp, int only_overridden_p)
{
  int first_param = TRUE, params_cnt = 0, *int_list_paramptr;
  void *paramptr_output;

  while (pp->paramptr !=NULL) {
    if ((only_overridden_p == FALSE) ||
        ((only_overridden_p == TRUE) && pp->overridden_p)) {
      if (first_param == TRUE) {
        fprintf(fp,"%s=", pp->paramname);
        first_param = FALSE;
      }
      else
        fprintf(fp,"; %s=", pp->paramname);

      if ((only_overridden_p == FALSE) && (pp->overridden_p == TRUE))
        paramptr_output = pp->paramptr_overridden;
      else
        paramptr_output = pp->paramptr;

      switch (pp->paramtype)
        {
        case TBOOL: fprintf(fp,"%s", (*(BOOLEAN *)(paramptr_output)) ? "true":"false");
          break;
        case TSTRING:  fprintf(fp,"\"%s\"", (char *) paramptr_output);
          break;
        case TINT:     fprintf(fp,"%d", *((int *)(paramptr_output)));
          break;
        case TDOUBLE:  fprintf(fp,"%15.12e", *((double *)(paramptr_output)));
          break;
        case TFLOAT:  fprintf(fp,"%e", *((float *)(paramptr_output)));
          break;
        case TINT_LIST: int_list_paramptr = (int *) paramptr_output;
          fprintf( fp, "(");
          output_int_list( int_list_paramptr, fp, NULL);
          fprintf( fp, ")");
          break;
        default: fprintf(fp," of unknown paramtype=%d",pp->paramtype);
        }
      params_cnt++;
    }
    pp++;
  }
  if ((params_cnt == 0) && (only_overridden_p == TRUE))
    fprintf(fp,"None.");
  fprintf(fp,"\n\n");
  return;
} /*end putparams*/


/* GETPARAMS
   11nov94 wmt: modified from getparm -- add paramtypes TDOUBLE & TINT_LIST
   07dec94 wmt: add error checking of inputs & appropriate messages
   11jan95 wmt: add paramptr_overridden
   26apr95 wmt: test eqstring to FALSE, not NULL
   15mar97 wmt: remove " " from strpbrk not-to-occur list in two places; for TINT_LIST
                allows "= 84, 92 " to be read as 84 & 92, rather than 84 & 84

   to read all of file on fp and set values of parameters defined in
   parameter table pp. (initialized by defparams)
   any parameters not set will remain unchanged
   any lines not recognized will just be output
   */
int getparams( FILE *fp, PARAMP params)
{
  char buff[LINLIM], *bp;
  PARAMP pp;
  int error_cnt = 0, i = 0, *int_list_paramptr, integer_p;
  int float_p, *int_list_paramptr_overridden;
  char *string_char_paramptr;
  shortstr input_string;

  while ( fgets( buff, LINLIM, fp) != NULL) {
    /* skip comment lines and blank lines */
    if ((buff[0] == '#') || (buff[0] == '!') || (buff[0] == ';') || 
             (buff[0] == '\n') || (buff[0] == ' '))                
      continue;
    else {
      bp = strtok(buff, "= \t\n");
      pp = params;

      while ((pp->paramptr != NULL) &&
             (eqstring( bp, pp->paramname) == FALSE))
        pp++;

      if (pp->paramptr == NULL) {
        fprintf( stderr, "ERROR: undefined parameter: %s\n", buff);
        error_cnt++;
      }
      else {
        if (pp->paramtype != TINT_LIST)
          bp = strtok(NULL,"\n");
        if (bp == NULL) {
          fprintf( stderr, "ERROR: no value given for: %s\n", buff);
          error_cnt++;
          continue;
        }
        /* bp still points to string with "= ", "=", " ", etc */
        /* while (strpbrk(bp, "= \t") != NULL)
           take off leading = & \t, but not " ", since trailing blanks
           cause bp to be incremented to nothing. */
        while (strpbrk(bp, "=\t") != NULL)
          bp++;
        switch (pp->paramtype)
          {
          case TBOOL:
            sscanf(bp, "%s", input_string);
            if ((eqstring(input_string, "true") == FALSE) &&
                (eqstring(input_string, "false") == FALSE)) {
              fprintf( stderr, "ERROR: for parameter %s, \n"
                      "       neither true or false was read.\n", pp->paramname);
              error_cnt++;
              break;
            }
            pp->paramptr_overridden = (void *) malloc(sizeof(BOOLEAN));
            *(BOOLEAN *)(pp->paramptr_overridden) = *(BOOLEAN *)(pp->paramptr);
            *(BOOLEAN *)(pp->paramptr) = (eqstring(input_string, "true") == TRUE)
              ?TRUE:FALSE;
            break;
          case TSTRING:
            string_char_paramptr = (char *) pp->paramptr;
            pp->paramptr_overridden = (void *) malloc( pp->max_length * sizeof( char));
            strcpy( (char *) pp->paramptr_overridden, string_char_paramptr); 
            i = 0;
            /* skip leading blanks */
            for (; (*bp == ' '); bp++);
            if (*bp != '\"') {  /* " - for highlit-19 */
              fprintf( stderr, "ERROR: for parameter %s, first character of value "
                      "is not a '\"'\n", pp->paramname);
              error_cnt++;
            }              
            for (bp++;  /* skip initial double quote */
                 ((*bp != '\0') && (*bp != '\"'));      /* " - for highlit-19 */
                 bp++) {
              i++;
              if (i >= pp->max_length) {
                fprintf( stderr, "ERROR: for parameter %s, more than %d characters.\n"
                        "       were input\n", pp->paramname, pp->max_length - 1);
                error_cnt++;
                break;
              }
              sscanf(bp, "%c", string_char_paramptr);
              string_char_paramptr++;
            }
            *string_char_paramptr = '\0';
            break;
          case TINT:
            /* fprintf( stderr, "getparams: INT bp %s\n", bp); */
            pp->paramptr_overridden = (void *) malloc( sizeof( int));
            *(int *) pp->paramptr_overridden =  *(int *) pp->paramptr;
            sscanf(bp, "%s", input_string);
            *(int *) pp->paramptr = atoi_p(input_string, &integer_p);
            if (integer_p != TRUE) {
              fprintf( stderr, "ERROR: for parameter %s, number read, %s, \n"
                      "       was not an integer\n", pp->paramname, input_string);
              error_cnt++;
            }
            break;
          case TFLOAT:
            pp->paramptr_overridden = (void *) malloc( sizeof( float));
            *(float *) pp->paramptr_overridden =  *(float *) pp->paramptr;
            sscanf(bp, "%s", input_string);
            *(float *) pp->paramptr = (float) atof_p(input_string, &float_p);
            if (float_p != TRUE) {
              fprintf( stderr, "ERROR: for parameter %s, number read, %s, \n"
                      "       was not a float\n", pp->paramname, input_string);
              error_cnt++;
            }
            break;
          case TDOUBLE:
            pp->paramptr_overridden = (void *) malloc( sizeof( double));
            *(double *) pp->paramptr_overridden =  *(double *) pp->paramptr;
            sscanf(bp, "%s", input_string);
            *(double *) pp->paramptr = atof_p(input_string, &float_p);
            if (float_p != TRUE) {
              fprintf( stderr, "ERROR: for parameter %s, number read, %s, \n"
                      "       was not a double\n", pp->paramname, input_string);
              error_cnt++;
            }
            break;
          case TINT_LIST:
            /* fprintf( stderr, "getparams: bp %s\n", bp); */
            pp->paramptr_overridden = (void *) malloc( pp->max_length * sizeof( int));
            int_list_paramptr = (int *) pp->paramptr;
            int_list_paramptr_overridden = (int *) pp->paramptr_overridden;
            for( ; *int_list_paramptr != END_OF_INT_LIST; int_list_paramptr++) {
              *int_list_paramptr_overridden =  *int_list_paramptr;
              int_list_paramptr_overridden++;
            }
            *int_list_paramptr_overridden = END_OF_INT_LIST;
            i = 0;
            int_list_paramptr = (int *) pp->paramptr;
            bp = strtok(NULL,",\n");
            for ( ;  bp != NULL; bp = strtok(NULL,",\n")) {
              /* fprintf( stderr, "getparams: i %d bp %s\n", i, bp); */
              i++;
              if (i >= pp->max_length) {
                fprintf( stderr, "\nERROR: more than %d values input for %s",
                        pp->max_length - 1, pp->paramname);
                error_cnt++;
                break;
              }
              /* while (strpbrk(bp, "= :\t") != NULL) */
              /* take off leading = & \t, but not " ", since trailing blanks
                 cause bp to be incremented to nothing. */
              while (strpbrk(bp, "=:\t") != NULL)
                bp++;
              /* 
              fprintf( stderr, "getparams: while: length bp %d bp `%s'\n", 
                       strlen(bp), bp);
              */
              if ((strlen(bp) == 0) || (strlen(bp) == strspn(bp, " "))) {
                /* no values -- a null list */
                break;
              }
              sscanf(bp, "%s", input_string);
              /* fprintf( stderr, "getparams: input_string %s\n", input_string); */
              *int_list_paramptr = atoi_p(input_string, &integer_p);
              if (integer_p != TRUE) {
                fprintf( stderr, "ERROR: for parameter %s, number read, %s, \n"
                        "       was not an integer\n", pp->paramname, input_string);
                error_cnt++;
                break;
              }
              int_list_paramptr++;
            }
            *int_list_paramptr = END_OF_INT_LIST;
            break;
          default:
            fprintf(stderr, "ERROR: bad paramtype= %d for %s; parameter not set\n",
                   pp->paramtype,buff);
            error_cnt++;
          }
        pp->overridden_p = TRUE;
      }
    }
  }
  return (error_cnt);
} /* end getparam */


/* DEFPARAM
   11nov94 wmt: modified from defparm -- add overridden_p & max_length
   11jan95 wmt: add paramptr_overridden

   define parameter table entries
 */
void defparam( PARAMP params, int nparams, char *name, PARAMTYPE type, void *ptr,
              int max_length)
{
	if (nparams>=MAXPARAMS)
	{
		printf(" too many params; max = %d",MAXPARAMS);
		abort();
	}
	if ((int) strlen(name) >= PARAMNAMLEN)
	{
		printf(" param name too long. limit is %d",PARAMNAMLEN);
		abort();
	}
	strcpy( params[nparams].paramname, name);
	params[nparams].paramtype = type;
	params[nparams].paramptr = ptr;
	params[nparams].paramptr_overridden = NULL;
        params[nparams].overridden_p = FALSE;
        params[nparams].max_length = max_length;        /* for STRING, TINT_LIST */

	params[nparams+1].paramptr = NULL;
}/* end defparam */

