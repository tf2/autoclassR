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

/* READ_MODEL_FILE
   02dec94 wmt: pass in model_file_ptr
   04dec94 wmt: read model file in line format, rather than list format

   This reads a list of model groups from a .model file, and hands it on to
   Define-Models for processing (with the model-file as its source).
   :Expand_P t means to call #'Expand-Model-Terms, creating a useable model.
   :Regenerate_P t means to regenerate any previous versions of these
   models that are found in *Model-List*.
   */
model_DS *read_model_file( FILE *model_file_fp, FILE *log_file_fp, database_DS d_base,
                          int regenerate_p, int expand_p, FILE *stream,
                          int *newlength, char *model_file_ptr)
{
   char ****model_groups = NULL, ***model_group;
   int length = 0, *size, **sizes = NULL, *num_groups = NULL, num, i, j, k;
   model_DS *models;
   int first_read = TRUE, str_length = 2 * sizeof( fxlstr);
   char caller[] = "read_model_file", *str;

   str = (char *) malloc( str_length);
   length = 0;
   while ((model_group = read_model_doit(model_file_fp, &size, &num, length,
                                         first_read, log_file_fp, stream)) != NULL) {
      length++;
      if (model_groups == NULL) {
         model_groups = (char ****) malloc(length * sizeof(char ***));
	 sizes = (int **) malloc(length * sizeof(int *));
	 num_groups = (int *) malloc(length * sizeof(int));
      }
      else {
	 model_groups =
	    (char ****) realloc(model_groups, length * sizeof(char ***));
	 sizes = (int **) realloc(sizes, length * sizeof(int *));
	 num_groups = (int *) realloc(num_groups, length * sizeof(int));
      }
      model_groups[length - 1] = model_group;
      sizes[length - 1] = size;
      num_groups[length - 1] = num;
   }
   models = define_models(model_groups, d_base, model_file_ptr, stream, expand_p,
                          regenerate_p, length, newlength, num_groups, sizes,
                          log_file_fp);
   safe_sprintf( str, str_length, caller,
                "ADVISORY[3]: read %d model def%s from \n             %s%s\n",
                length, (length > 1) ? "s" : "",
                (model_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                model_file_ptr);
   to_screen_and_log_file( str, log_file_fp, stream, TRUE);

   for (i=0; i<length; i++) {
     for (j=0; j<num_groups[i]; j++) {
       for (k=0; k<sizes[i][j]; k++)
         free( model_groups[i][j][k]);
       free( model_groups[i][j]);
     }
     free( model_groups[i]);
     free( sizes[i]);
   }
   free( model_groups);
   free( sizes);
   free( num_groups);
   
   /* fprintf( stderr, "read_model_file\n");
   for (model_index=0; model_index<length; model_index++) 
     print_att_locs_and_ignore_ids( models[model_index], model_index); */

   if (models == NULL) {
      safe_sprintf( str, str_length, caller, "ERROR[3]: No models read from\n"
                   "          \"%s%s\"\n",
                   (model_file_ptr[0] == G_slash) ? "" : G_absolute_pathname,
                   model_file_ptr);
      to_screen_and_log_file( str, log_file_fp, stream, TRUE);
      exit(1);
   }
   free( str);
   return(models);
}


/* READ_MODEL_DOIT
   04dec94 wmt: new - built from read_lists
   
   read model file in line format
   */
char ***read_model_doit( FILE *model_file_fp, int **sizes, int *num, int model_index,
                        int first_read, FILE *log_file_fp, FILE *stream)
{
  int size, n_comment_chars = 5, num_index_tokens = 3, i;
  char **list = NULL, ***big_list = NULL, caller[] = "read_model_doit";
  char separator_char = ' ', comment_chars[6];
  int num_model_def_lines, integer_p, model_index_read, model_line;
  fxlstr str;

  *num = 0;
  /* allow general comment lines anywhere in model file */
  comment_chars[0] = '!'; comment_chars[1] = '#'; comment_chars[2] = ';';
  comment_chars[3] = ' '; comment_chars[4] = '\n'; comment_chars[5] = '\0';
  
  if (( list = get_line_tokens(model_file_fp, (int) separator_char, n_comment_chars,
                               comment_chars, first_read, &size)) != NULL) {
    if (size != num_index_tokens) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[3]: expected 3 items: model_index <index_num> "
                   "<num_model_def_lines>, \n          read %d: %s %s %s %s\n",
                   size, list[0], (size >= 2) ? list[1] : "", (size >= 3) ? list[2] : "",
                   (size >= 4) ? "<...>" : "");
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
    }
    if (eqstring(list[0], "model_index") != TRUE) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[3]: expected model_index, read %s\n", list[0]);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
    }
    model_index_read = atoi_p(list[1], &integer_p);
    if (integer_p != TRUE) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[3]: model index read, %s, was not an integer\n", list[1]);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
    }
    if (model_index_read != model_index) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[3]: expected model index %d, read %d\n", model_index,
                   model_index_read);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
    }
    num_model_def_lines = atoi_p(list[2], &integer_p);
    if (integer_p != TRUE) {
       safe_sprintf( str, sizeof( str), caller,
                   "ERROR[3]: num model definition lines read, %s, was not an integer\n",
                   list[2]);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
    }
    for (i=0; i<size; i++)
      free( list[i]);
    free( list);

    for (model_line=0; model_line <num_model_def_lines; model_line++) {
      list = get_line_tokens(model_file_fp, separator_char, n_comment_chars,
                             comment_chars, first_read, &size);
      if (size > 0) {
        *num += 1;
        if (big_list == NULL) {
          big_list = (char ***) malloc(*num * sizeof(char **));
          *sizes = (int *) malloc(*num * sizeof(int));
        }
        else {
          big_list = (char ***) realloc(big_list, *num * sizeof(char **));
          *sizes = (int *) realloc(*sizes, *num * sizeof(int));
        }
        big_list[*num - 1] = list;
        (*sizes)[*num - 1] = size - 1;
      }
    }
    if (*num < num_model_def_lines) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[3]: expected %d model definition lines, only read %d\n",
                   num_model_def_lines, *num);
      to_screen_and_log_file( str, log_file_fp, stream, TRUE);
      exit(1);      
    }
  }
  return(big_list);
}


char ***read_lists( FILE *stream, int **sizes, int *num)
{
   int c, size;
   char **list, ***big_list = NULL;

   *num = 0;
   while ( ((c = fgetc(stream)) != EOF) && (c !='(' ));

   if (c == EOF)
      return(NULL);

   while (( list = read_list(stream, &size) ) != NULL) {
      *num += 1;
      if (big_list == NULL) {
         big_list = (char ***) malloc(*num * sizeof(char **));
	 *sizes = (int *) malloc(*num * sizeof(int));
      }
      else {
	 big_list = (char ***) realloc(big_list, *num * sizeof(char **));
	 *sizes = (int *) realloc(*sizes, *num * sizeof(int));
      }
      big_list[*num - 1] = list;
      (*sizes)[*num - 1] = size - 1;
   }

   while ( ((c = fgetc(stream)) != EOF) && ( c !=')' ) );

   return(big_list);
}


char **read_list( FILE *stream, int *num)
/* modified to store left and right parens in list if reads a list containing list
	so typically returnx xxxmodel n1 n2 n3
			or
			     xxxmodel ( n1 n2 ... ) ( m1 m2 m3...)
*/
{
   int i, c;
   char temp[255], /* arbitrarilyh chose 255 but no check done*/
       **list = NULL;
   int needright=1;

   *num = 0;
   /* read down to start of this list or return NULL if hit EOF first */
   while ( (c=fgetc(stream)) != EOF && c != '(' );
   if (c == EOF)
      return(NULL);
   
   list = (char **) malloc(sizeof(char *));

   do {
      fscanf(stream, "%s", temp);

      if(temp[0] == '(' ){/* list contains list*/
         needright++;
         list = (char **) realloc(list, ++(*num) * sizeof(char *));
         list[*num - 1] = (char *) malloc(2 * sizeof(char));
	 strcpy(list[*num - 1], "(");
	 if ((int) strlen(temp) > 1) /* has number too*/
	    for (i=0;temp[i] != '\0';i++)temp[i]=temp[i+1];/*squeeze the ( */
	 else
	     fscanf(stream, "%s", temp); /* all we had was paren so get next */
      }

      for(i=0;temp[i] != '\0'; i++)
      if(temp[i] ==  ')' ){
         temp[i]='\0'; /* overlay )*/
         if(needright-- > 1 ){ /* need to output this right paren */
            if(i != 0){ /*  but came in with no whitespace by number*/

	       list = (char **) realloc(list, ++(*num) * sizeof(char *));
	       list[*num - 1] = (char *) malloc((strlen(temp)+1) * sizeof(char));
	       strcpy(list[*num - 1], temp);
            }
	    strcpy(temp, ")" );
	 }
	 break;
       }

      if(temp[0] != '\0'){
         list = (char **) realloc(list, ++(*num )* sizeof(char *));
         list[*num - 1] = (char *) malloc((strlen(temp)+1) * sizeof(char));
	 strcpy(list[*num - 1], temp);

      }
   } while (needright);

   while( (c=fgetc(stream)) != EOF  && c != '\n' && c != ')' );

   return(list);
}


/* DEFINE_MODELS
   22oct94 wmt: use n_atts, init att_locs to "", init model->terms,
        model->priors, model->global_clsf
   17nov94 wmt: FILE *source => char *source
   22nov94 wmt: init att_ignore_ids to ""


   This generates a set of models from a model-group list.  A model-group is a
   compact representation of an AutoClass likelihood model which specifies what
   model terms are to be applied to which attributes.  If there is a model in
   *model-list* which corresponds to a model group, that will be used and
   optionally regenerated from the model group. Otherwise a new model is
   created.  The d-base must be a database-DS.  The :source must be some unique
   identifier.  When called from Read-Model-File the model file pathname is
   used.  :Expand_P t means to call #'Expand-Model-Terms, creating a useable
   model.  :Regenerate_P t means to regenerate an existing model using the new
   model-group. */

model_DS *define_models( char ****model_groups, database_DS d_base, char *source,
                        FILE *stream, int expand_p, int regenerate_p, int num_model_groups,
                        int *newnum, int *num_groups, int **sizes, FILE *log_file_fp)
{
   int i_model, i_att, index = 0, length = 0, n_atts = d_base->n_atts;
   char ***model_group, caller[] = "define_models";
   model_DS model, *models;
   fxlstr str;

   models = NULL;
   for (i_model=0; i_model<num_model_groups; i_model++) {
      model_group = model_groups[i_model];
      model_group = canonicalize_model_group(model_group);
      model = find_similar_model(source, index, d_base);
      if (model != NULL) {
	 if (regenerate_p == TRUE) {
	    read_model_reset(model);
	    generate_attribute_info(model, model_group, i_model,
				    num_groups[i_model], sizes[i_model], d_base,
                                    log_file_fp, stream);
	 }
      }
      else {
	 model = (model_DS) malloc(sizeof(struct model));
	 safe_sprintf( model->id, sizeof( model->id), caller, "MODEL-%d", G_m_id++);
	 model->expanded_terms = FALSE;   /* Ensures GENERATE-ATTRIBUTE-INFO call */
	 model->n_terms = model->num_priors = 0;
         model->terms = NULL;
         model->priors = NULL;
	 strcpy(model->model_file, source);
	 model->file_index = index;
	 model->n_att_locs = n_atts; 
         model->att_locs = (shortstr *) malloc(n_atts * sizeof(shortstr)); 
         for (i_att=0; i_att<n_atts; i_att++)
           model->att_locs[i_att][0] = '\0';
	 model->n_att_ignore_ids = n_atts;
         model->att_ignore_ids = (shortstr *) malloc(n_atts * sizeof(shortstr));
         for (i_att=0; i_att<n_atts; i_att++)
           model->att_ignore_ids[i_att][0] = '\0';
	 model->database = d_base;
	 /* JTP model->class_store = (class_DS *) malloc(20 * sizeof(class_DS));*/
	 model->class_store = NULL; /* added for new class_store scheme*/
	 model->num_class_store = 0;
         model->global_clsf = NULL;
         /* compressed model params */
         strcpy( model->data_file, "");
         strcpy( model->header_file, "");
         model->n_data = 0;
         model->compressed_p = FALSE;
         
	 generate_attribute_info(model, model_group, i_model,
				 num_groups[i_model], sizes[i_model], d_base,
                                 log_file_fp, stream);
      }
      if (expand_p == TRUE)
	 conditional_expand_model_terms(model, regenerate_p,  log_file_fp, stream);

      if (find_model_p( model, G_model_list, G_m_length) == FALSE) {
	 if (G_model_list == NULL)
	    G_model_list = (model_DS *) malloc(++G_m_length * sizeof(model_DS));
	 else
           G_model_list =
             (model_DS *) realloc(G_model_list, ++G_m_length * sizeof(model_DS));
	 G_model_list[G_m_length - 1] = model;
      }
      if (models == NULL)
         models = (model_DS *) malloc(++length * sizeof(model_DS));
      else models = (model_DS *) realloc(models, ++length * sizeof(model_DS));
      models[length - 1] = model;
      index++;
   }
   if (models == NULL) {
      safe_sprintf( str, sizeof( str), caller,
                   "ERROR[3]: No models read from model source: %s\n", source);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
      exit(1);
   }
   *newnum = length;
   return(models);
}


/* GENERATE_ATTRIBUTE_INFO
   05nov94 wmt: last ignore attribute not processed:
                for (j=0; j<sizes[i]; j++) => for (j=1; j<=sizes[i]; j++)
                and add check for out of bounds attribute number
   06dec94 wmt: pass model index to extend_terms

   Set up N-terms, terms, att-locs, att-ignore, att-indepn, and att-groups
   of model.
   */
void generate_attribute_info( model_DS model, char ***model_group, int i_model,
			int num_groups, int *sizes, database_DS d_base,
                        FILE *log_file_fp, FILE *stream)
{
  shortstr default_set_type;
  int i_group, j, n_att, integer_p;
  shortstr set_type;
  fxlstr str;
  char caller[] = "generate_attribute_info";

  default_set_type[0] = '\0';
  /* Make space for non-ignored groups. */
  for (i_group=0; i_group < num_groups; i_group++) {
    strcpy(set_type, model_group[i_group][0]);
    if (eqstring(model_group[i_group][1], "default") == FALSE) {
      if (model_type(set_type) == UNKNOWN){
        n_att = atoi_p(model_group[i_group][1], &integer_p);
        safe_sprintf( str, sizeof( str), caller,
                     "ERROR[3]: for model index = %d, model term type = %s, \n"
                     "          is not handled\n", i_model, set_type);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        exit(1);
      }
      else if (prefix(set_type, "ignore") == TRUE) {
        for (j=1; j<=sizes[i_group]; j++) {
          n_att = atoi_p(model_group[i_group][j], &integer_p);
          if (integer_p != TRUE) {
            safe_sprintf( str, sizeof( str), caller,
                         "ERROR[3]: for model index = %d, model term type = ignore, "
                         "attribute \n          number read, %s, was not an integer\n",
                         i_model, model_group[i_group][j]);
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            exit(1);
          }
          else if (n_att >= model->n_att_locs) {
            safe_sprintf( str, sizeof( str), caller,
                         "ERROR[3]: for model index = %d, %d is an invalid model term type = "
                         "ignore \n          attribute number, must be less than %d\n",
                         i_model, n_att, model->n_att_locs);
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            exit(1);
          }
          else if (eqstring(model->att_locs[n_att], "") == FALSE) {
            safe_sprintf( str, sizeof( str), caller,
                         "ERROR[3]: for model index = %d, model term type = ignore, \n"
                         "          attempt to re-use attribute %d\n", i_model, n_att);
            to_screen_and_log_file(str, log_file_fp, stream, TRUE);
            exit(1);
          }
          else {
            strcpy(model->att_locs[n_att], "ignore");
            strcpy(model->att_ignore_ids[n_att], "ignore_model");
          }
        }
      }
      else if (prefix(set_type, "single") == TRUE) {
        extend_terms_single(set_type, model_group[i_group], sizes[i_group], model, i_model,
                            log_file_fp, d_base, stream);
      }
      else if (prefix(set_type, "multi") == TRUE) {
        extend_terms_multi(set_type, model_group[i_group], sizes[i_group], model, i_model,
                     log_file_fp, d_base, stream);
      }
      else {
        fprintf(stderr,
                "ERROR: No method for generating attribute sets for set_type %s\n",
                set_type);
        exit(1);
      } 
    }
    /* Default: check for previous default and otherwise set up default. */
    else {
      if (eqstring(default_set_type, "") == FALSE) {
        safe_sprintf( str, sizeof( str), caller,
                     "ERROR[3]: for model index = %d, default model term type, %s, \n"
                     "          specified twice.\n", i_model, set_type);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        exit(1);
      }
      strcpy(default_set_type, set_type);
    }
  }
  if (default_set_type[0] != '\0')
    extend_default_terms(default_set_type, i_model, model, d_base, log_file_fp, stream);
  for (n_att=0; n_att<model->n_att_locs; n_att++)
    if (eqstring(model->att_locs[n_att], "") == TRUE) {
      set_ignore_att_info(model, d_base);
      break;
    } 
}


/* EXTEND_TERMS_SINGLE
   04nov94 wmt: added test for out of bounds attribute value read from
                model file.
   06dec94 wmt: pass model index to extend_terms
   23dec94 wmt: renamed from extend_terms
   28feb95 wmt: do not allocate tparm - it is done by appropriate model term builder

   For each set in set-list, build a term and add it to model->terms.
   a set is a single attribute number 
   */
void extend_terms_single( char *model_type, char **list, int size, model_DS model,
                         int model_index, FILE *log_file_fp, database_DS d_base,
                         FILE *stream)
{
  shortstr *model_att_locs = model->att_locs;
  int i, n_att, num, integer_p;
  int begi = 0, endi = size, j, doinglist = FALSE;
  fxlstr str;
  att_DS *att_info = d_base->att_info;
  char caller[] = "extend_terms_single";

  /* Check and store set indices in att-loc array */
  for (i=0; i<size; i++) {
    if(list[i+1][0] == '(') {
      doinglist=TRUE;
      begi = ++i;
    }
    else if( !doinglist)
      endi=1+(begi=i);
    else if(list[i+1][0] == ')') {
      doinglist=FALSE;
      endi = i++;
    } /* if doinglist is FALSE then begi,endi are FROM up TO but not incl*/

    if( !doinglist){
      for(j=begi;j<endi;j++){
        n_att = atoi_p(list[j+1], &integer_p);
        if (integer_p != TRUE) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[3]: for model index = %d, model term type = %s, \n"
                       "          attribute number read, %s, was not an integer\n",
                       model_index, model_type, list[j+1]);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          exit(1);
        }
        else if (n_att >= model->n_att_locs) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[3]: for model index = %d, model term type = %s, \n"
                       "          %d is an invalid attribute number, must be less than %d\n",
                       model_index, model_type, n_att, model->n_att_locs);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          exit(1);
        }
        else if (eqstring(model_att_locs[n_att], "") == FALSE) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[3]: for model index = %d, model term type = %s, \n"
                       "          attempt to re-use attribute %d\n",
                       model_index, model_type, n_att);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          exit(1);
        }
        else if (eqstring(att_info[n_att]->type, "dummy") == FALSE)
          safe_sprintf( model_att_locs[n_att], sizeof( model_att_locs[n_att]),
                       caller, "%d", model->n_terms);
      }

      num = model->n_terms ++;
      if (model->terms == NULL)
        model->terms = (term_DS *) malloc(model->n_terms * sizeof(term_DS));
      else 
        model->terms = (term_DS *) realloc(model->terms, model->n_terms * sizeof(term_DS));

      model->terms[num] = (term_DS) malloc(sizeof(struct term));
      strcpy(model->terms[num]->type, model_type);
      /* model->terms[num]->tparm = (tparm_DS) malloc(sizeof(struct new_term_params)); */
      model->terms[num]->n_atts = endi-begi;
      model->terms[num]->att_list =
        (float *) malloc((model->terms[num]->n_atts) * sizeof(float));
      j = 0;
      while (begi++ < endi) {
        /* these token have already been read by atoi_p, so atof_p is not needed
           07dec94 wmt */
        model->terms[num]->att_list[j++] = (float) atof(list[begi]);
      }
    } /*endif for not doing a list*/
  }
}


/* EXTEND_TERMS_MULTI
   23dec94 wmt: derived from extend_terms to handle multi models
   28feb95 wmt: do not allocate tparm - it is done by appropriate model term builder

   For set-list, build a term and add it to model->terms.
   a set-list is two or more attribute numbers 
   */
void extend_terms_multi( char *model_type, char **list, int size, model_DS model,
                        int model_index, FILE *log_file_fp, database_DS d_base,
                        FILE *stream)
{
  shortstr *model_att_locs = model->att_locs;
  int n_att, num, integer_p;
  int begi, endi, j;
  fxlstr str;
  att_DS *att_info = d_base->att_info;
  char caller[] = "extend_terms_multi";

  /* Check and store set indices in att-loc array */
/*   for (i=0; i<size; i++) { */
/*     if(list[i+1][0] == '(') { */
/*       doinglist=TRUE; */
/*       begi = ++i; */
/*     } */
/*     else if( !doinglist) */
/*       endi=1+(begi=i); */
/*     else if(list[i+1][0] == ')') { */
/*       doinglist=FALSE; */
/*       endi = i++; */
/*     } */
    /* if doinglist is FALSE then begi,endi are FROM up TO but not incl*/

    begi = 0;
    endi = size;
      for(j=begi; j<endi; j++) {
        n_att = atoi_p(list[j+1], &integer_p);
        if (integer_p != TRUE) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[3]: for model index = %d, model term type = %s, \n"
                       "          attribute number read, %s, was not an integer\n",
                       model_index, model_type, list[j+1]);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          exit(1);
        }
        else if (n_att >= model->n_att_locs) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[3]: for model index = %d, model term type = %s, \n"
                       "          %d is an invalid attribute number, must be less than %d\n",
                       model_index, model_type, n_att, model->n_att_locs);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          exit(1);
        }
        else if (eqstring(model_att_locs[n_att], "") == FALSE) {
          safe_sprintf( str, sizeof( str), caller,
                       "ERROR[3]: for model index = %d, model term type = %s, \n"
                       "          attempt to re-use attribute %d\n",
                       model_index, model_type, n_att);
          to_screen_and_log_file(str, log_file_fp, stream, TRUE);
          exit(1);
        }
        else if (eqstring(att_info[n_att]->type, "dummy") == FALSE)
          safe_sprintf( model_att_locs[n_att], sizeof( model_att_locs[n_att]),
                       caller, "%d", model->n_terms);
      }

  num = model->n_terms ++;
  if (model->terms == NULL)
    model->terms = (term_DS *) malloc(model->n_terms * sizeof(term_DS));
  else 
    model->terms = (term_DS *) realloc(model->terms, model->n_terms * sizeof(term_DS));
 
  model->terms[num] = (term_DS) malloc(sizeof(struct term));
  strcpy(model->terms[num]->type, model_type);
  /* model->terms[num]->tparm = (tparm_DS) malloc(sizeof(struct new_term_params)); */
  model->terms[num]->n_atts = endi - begi;
  model->terms[num]->att_list =
    (float *) malloc((model->terms[num]->n_atts) * sizeof(float));
  j = 0;
  while (begi++ < endi) {
    /* these token have already been read by atoi_p, so atof_p is not needed
       07dec94 wmt */
    model->terms[num]->att_list[j++] = (float) atof(list[begi]);
  }
}


/* EXTEND_DEFAULT_TERMS
   05nov94 wmt: added test for invalid attribute number read from model file
   06dec94 wmt: pass model index to default_extend_terms
   28feb95 wmt: do not allocate tparm - it is done by appropriate model term builder

   For each non 'dummy attribute not already set , build an term-DS
   and add it to (model-DS-terms model) for model-type /= 'ignore. 
   */
void extend_default_terms( char *model_type, int i_model, model_DS model,
			   database_DS d_base, FILE *log_file_fp, FILE *stream)
{
  shortstr *model_att_locs = model->att_locs;
  int i, j, n_att, nad = 0, nsl = 0, *atts_defaulted = NULL, **set_list = NULL;
  int num;
  att_DS *att_info = d_base->att_info;
  fxlstr str;
  char caller[] = "extend_default_terms";

  if (eqstring(model_type, "ignore") == TRUE) {
    safe_sprintf( str, sizeof( str), caller,
                 "ERROR[3]: for model index = %d, ignore is not a valid default "
                 "model term type\n", i_model);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
    exit(1);
  }
  for (n_att=0; n_att<model->n_att_locs; n_att++) {
    if ((eqstring(model_att_locs[n_att], "") == TRUE) && 
        (eqstring(att_info[n_att]->type, "dummy") == FALSE)) { 
      nad++;
      if (atts_defaulted == NULL)
        atts_defaulted = (int *) malloc(nad * sizeof(int));
      else atts_defaulted =
        (int *) realloc(atts_defaulted, nad * sizeof(int));
      atts_defaulted[nad-1] = n_att;
      nsl++;
      if (set_list == NULL)
        set_list = (int **) malloc(nsl * sizeof(int *));
      else set_list = (int **) realloc(set_list, nsl * sizeof(int *));
      set_list[nsl-1] = (int *) malloc(sizeof(int));
      set_list[nsl-1][0] = n_att;
    }
  }
  for (i=nsl-1; i>=0; i--) {
  /* for (i=0; i<nsl; i++) { -- this causes att_info to be out of order */
    for (j=0; j<1; j++) {       /* Check and store set indices in att-loc array */
      n_att = set_list[i][j];
      if (n_att >= model->n_att_locs) {
        safe_sprintf( str, sizeof( str), caller,
                     "ERROR[3]: for model index = %d, model term type = %s, %d "
                     "is an invalid \n          attribute number, must be less than %d\n",
                     i_model, model_type, n_att, model->n_att_locs);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        exit(1);
      }
      else if (eqstring(model_att_locs[n_att], "") == FALSE) {
        safe_sprintf( str, sizeof( str), caller,
                     "ERROR[3]: for model index = %d, model term type = %s, "
                     "attempt to \n          re-use attribute %d\n",
                     i_model, model_type, n_att);
        to_screen_and_log_file(str, log_file_fp, stream, TRUE);
        exit(1);
      }
      else
        safe_sprintf( model_att_locs[n_att], sizeof( model_att_locs[n_att]),
                     caller, "%d", model->n_terms);
    }

    model->n_terms += 1;
    num = model->n_terms - 1;
    if (model->terms == NULL)
      model->terms = (term_DS *) malloc(model->n_terms * sizeof(term_DS));
    else model->terms =
      (term_DS *) realloc(model->terms, model->n_terms * sizeof(term_DS));
    model->terms[num] = (term_DS) malloc(sizeof(struct term));
    strcpy(model->terms[num]->type, model_type);
    model->terms[num]->n_atts = 1;
    model->terms[num]->att_list = (float *) malloc(sizeof(float));
    model->terms[num]->att_list[0] = set_list[i][0];
    /* model->terms[num]->tparm =
      (tparm_DS) malloc(sizeof(struct new_term_params)); */
  }
  if (atts_defaulted != NULL) {
    safe_sprintf( str, sizeof( str), caller,
                 "ADVISORY[3]: the default model term type, %s, \n"
                 "             will be used for these attributes:\n", model_type);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
    for (i=0; i<nad; i++) {
      safe_sprintf( str, sizeof( str), caller, "             #%d: \"%s\"\n",
                   atts_defaulted[i], att_info[atts_defaulted[i]]->dscrp);
      to_screen_and_log_file(str, log_file_fp, stream, TRUE);
    }
  }
}


/* This resets the model just sufficiently that Generate-Attribute-Info will
   work. */


void read_model_reset( model_DS model)
{
   int n_atts;

   model->expanded_terms = FALSE;
   n_atts = model->database->input_n_atts;
   model->att_locs = NULL;
   model->n_att_locs = 0;
   model->att_ignore_ids = NULL;
   model->n_att_ignore_ids = 0;
   /* commented JTP free(model->class_store); */
   model->class_store = NULL; /* this means old ones lost but this routine not called*/
   model->num_class_store = 0;
   store_clsf_DS(model->global_clsf, NULL, 0);
   /* dont free since is in store JTP free(model->global_clsf); */
   model->global_clsf = NULL;
}


/* SET_IGNORE_ATT_INFO
   04nov94 wmt: test for "", not NULL on unspecified_dummy_warning
   02mar95 wmt: find_str_in_table test is >=, rather than >

   replaces nil in model-DS-att-locs with "ignore".
   Fills att_ignore_ids with "transformed-attribute-ignored",
   "att_type_not_specified", "att_type_is_dummy",
   or "model_term_not_specified" */

void set_ignore_att_info( model_DS model, database_DS d_base)
{
  int i;
  shortstr *locs = model->att_locs, *att_ignore_ids = model->att_ignore_ids;

  for (i=0; i<model->n_att_locs; i++)
    if (eqstring(locs[i], "") == TRUE) {
      strcpy(locs[i], "ignore");
      if (find_str_in_table(d_base->att_info[i]->sub_type, G_transforms,
                            NUM_TRANSFORMS) >= 0)
        strcpy(att_ignore_ids[i], "transformed-attribute-ignored");
      else if (eqstring( d_base->att_info[i]->warnings_and_errors->unspecified_dummy_warning,
                        "") == FALSE)
        strcpy(att_ignore_ids[i], "att_type_not_specified");
      else if (eqstring(d_base->att_info[i]->type, "dummy") == TRUE)
        strcpy(att_ignore_ids[i], "att_type_is_dummy");
      else
        strcpy(att_ignore_ids[i], "model_term_not_specified");
    }
  /* fprintf( stderr, "set_ignore_att_info\n");
  print_att_locs_and_ignore_ids( model, model->file_index); */
}


/* this back traces the effect of translations, finding the source attribute
   indices.  Returns the list of ultimate source attribute indices corresponding
   to those in 'att-index-list.
   JCS 6/90 */

int *get_sources_list( int *att_index_list, int num,  att_DS *att_info,
					 int *traced, int n_traced)
{
  int *list = NULL;
/**** only call to this was recursive one so commented since
 couldnt really figure out what was being altered, and what was geing 
 done in place.  in LISP is called by 
 (defun Model-Group-List (model &key (use-sources t))

   int i, j, *temp, *list, n_temp, n_list;
   int (*cmp)() = int_cmp;

   for (i=0; i<num; i++) {
      temp = get_source_list(i, att_info, traced, n_traced, &n_temp);
      if (list == NULL) {
	 list = temp;
	 n_list = n_temp;
      }
      else {
	 list = (int *) realloc(list, (n_list + n_temp) * sizeof(int));
	 for (j=0; j<n_temp; j++)
	    list[j+n_list] = temp[j];
	 n_list += n_temp;
      }
   }
   
   qsort(delete_duplicates(list, n_list), n_list, sizeof(int), int_cmp);
     note that is supposed to return pointer and note that
     qsort does in place which really cant be one to delte_dup

     *****************************/
  fprintf(stderr,"## all code commented in get_sources_list\n");
  abort();

  return (list);
}


/* 09dec94 wmt: replaced by int_compare_greater & int_compare_less
        args should be of type int *

int int_cmp(int x,int  y)
{
   if (x < y)
      return(-1);
   else if (x == y)
      return(0);
   else return(1);
}
*/


/* This returns a list containing the source attrubute indices of the indexed
   attribute.
   JCS 6/90 */

int *get_source_list( int att_index, att_DS *att_info, int *traced,int n_traced, 
			  int *n_source)
{
   int *source, *temp;

   source = (int *) getf(att_info[att_index]->props, "source",
		         att_info[att_index]->n_props);
   temp = (int *) getf(att_info[att_index]->props, "n_source",
			att_info[att_index]->n_props);
   *n_source = *temp;
   if (source == NULL) {
      source = (int *) malloc(sizeof(int));
      source[0] = att_index;
      return(source);
   }
   else if (*n_source == 1) {
      if (member_int(source[0], traced, n_traced) == TRUE) {
	 fprintf(stderr, "ERROR: get_source_list found circular reference\n");
	 exit(1);
      }
      else return(source);
   }
   else {
      if (exist_intersection(traced, source, n_traced, *n_source)) {
	 fprintf(stderr,
	  "ERROR: get_source_list found circularity of attribute source references\n");
	 exit(1);
      }
      else 
	 get_sources_list(source, *n_source, att_info, traced, n_traced);
	 return(source);
   }
}

/* EXIST_INTERSECTION
   21oct94 wmt: = changed to ==
   */
int exist_intersection( int *fl1,int *fl2,int l1,int l2)
{
   int i, j;

   for (i=0; i<l1; i++)
      for (j=0; j<l2; j++)
	 if (fl1[i] == fl2[j])
	    return(TRUE);
   return(FALSE);
}


/* Sorts the model-group by interaction types and merges duplicate type lists.
   Insures all attribute sets are lists.  Sorts the attribute numbers in each
   set, and the sets in a type group by lowest attribute number.
   remove default model term type */


char ***canonicalize_model_group( char ***model_group)
{
/*    fprintf(stderr,"canonicalize_model_group is incomplete\n"); */

   return(model_group);
}


/* PRINT_ATT_LOCS_AND_IGNORE_IDS
   03mar95 wmt: new

*/
void print_att_locs_and_ignore_ids( model_DS model, int model_index)
{
  int n_att;

  for (n_att=0; n_att<model->n_att_locs; n_att++) {
    fprintf( stderr, "%d=%d:%s\n", model_index, n_att, model->att_locs[n_att]);
    fprintf( stderr, "%d=%d:%s\n", model_index, n_att, model->att_ignore_ids[n_att]);
  }
}

