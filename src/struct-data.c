#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "autoclass.h"
#include "minmax.h"
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


database_DS find_database( char *data_file_ptr, char *header_file_ptr, int n_data)
{
   int i;
   database_DS temp;

/*    validate_data_header_model_files(0, data_file, header_file, 0); */

   for (i=0; i<G_db_length; i++) {
      temp = G_db_list[i];
      if ((eqstring(temp->data_file, data_file_ptr) == TRUE) &&
	  (eqstring(temp->header_file, header_file_ptr)) &&
	  ((temp->n_data == 0) || (temp->n_data == n_data)))
	 return(temp);
   }
   return(NULL);
}


/* EVERY_DB_DS_SAME_SOURCE_P
   18may95 wmt: do string compares for header_file & data_file

   This tests if two db's (compressed or expanded) come from the same source and
   have the same size.  It is intended to allow avoiding having to reread a
   database. 
*/
int every_db_DS_same_source_p( database_DS db1, model_DS *models)
{
  int i, length;
  database_DS db2;

  if (models == NULL)
    length = 0;
  else
    length = sizeof( models) / sizeof(struct model);
  for (i=0; i<length; i++) {
    db2 = models[i]->database;
    if (((eqstring( db1->header_file, db2->header_file) != TRUE) ||
         (eqstring( db1->data_file, db2->data_file) != TRUE)  ||
         (db1->n_data != db2->n_data)))
      return(FALSE);
  }
  return(TRUE);
}


database_DS compress_database( database_DS db)
{
   /*if (compressed_db_DS_p(db))*/
      return(db);
}


/* DB_DS_EQUAL_P
   18may95 wmt: take out: if (db1 == db2) -- always false

   If two databases have the same number of data and attributes, and if all the
   attributes are equal-p.  Note that attribute transformation may cause a
   database and a previously compressed version of that database to fail this
   test.
*/

int db_DS_equal_p( database_DS db1, database_DS db2)
{
   int i;

   if ((db1->n_data == db2->n_data) && (db1->n_atts == db2->n_atts)) {
      for (i=0; i<db1->n_atts; i++)
	 if (att_DS_equal_p( db1->att_info[i], db2->att_info[i]) == FALSE)
	    return(FALSE);
      return(TRUE);
   }
   return(FALSE);
}


/* Attribute predicates: */


/* ATT_DS_EQUAL_P
   18may95 wmt: check for type = dummy
   */
int att_DS_equal_p( att_DS att1,att_DS att2)
{
  int i, num;

  if (eqstring(att1->type, att2->type) == FALSE)
    return(FALSE);
  if (eqstring(att1->sub_type, att2->sub_type) == FALSE)
    return(FALSE);
  if (eqstring(att1->dscrp, att2->dscrp) == FALSE)
    return(FALSE);
  if (att1->n_props != att2->n_props)
    return(FALSE);
  if (eqstring(att1->type, "discrete") == TRUE) {
    if (att1->d_statistics->range != att2->d_statistics->range)
      return(FALSE);
    num = att1->d_statistics->n_observed;
    if (num != att2->d_statistics->n_observed)
      return(FALSE);
    for (i=0; i<num; i++)
      if (att1->d_statistics->observed[i] != att2->d_statistics->observed[i])
        return(FALSE);
  }
  else if (eqstring(att1->type, "real") == TRUE) {
    if (att1->r_statistics->count != att2->r_statistics->count)
      return(FALSE);
    if (att1->r_statistics->mx != att2->r_statistics->mx)
      return(FALSE);
    if (att1->r_statistics->mn != att2->r_statistics->mn)
      return(FALSE);
    if (att1->r_statistics->mean != att2->r_statistics->mean)
      return(FALSE);
    if (att1->r_statistics->var != att2->r_statistics->var)
      return(FALSE);
  }
  return(TRUE);
}


/* CREATE_DATABASE
   15nov94 wmt: initialize data and n_data
   28nov94 wmt: initialize datum_length
   21jun95 wmt: initialize att_info[]
   */
database_DS create_database(void)
{
   database_DS temp;
   int i;

   temp = (database_DS) malloc(sizeof(struct database));
   strcpy(temp->data_file, "");
   strcpy(temp->header_file, "");
   temp->data = NULL;
   temp->datum_length = NULL;
   temp->n_data = 0;
   temp->n_atts = 0;
   temp->allo_n_atts = 50; /* arbitrary start; bump by 1.5 when needed*/
   temp->att_info = (att_DS *) malloc( temp->allo_n_atts * sizeof(att_DS));
   for (i=0; i<temp->allo_n_atts; i++) 
     temp->att_info[i] = NULL;
   temp->translations_supplied_p = NULL;
   temp->num_tsp = 0;
   temp->invalid_value_errors = NULL;
   temp->num_invalid_value_errors = 0;
   temp->incomplete_datum = NULL;
   temp->num_incomplete_datum = 0;
   return(temp);
}


/* EXPAND_DATABASE
   22jan95 wmt: new
   25apr95 wmt: allow binary data files
   09may95 wmt: to handle partial databases, read G_s_params_n_data
   27apr97 wmt: make error msg more informative
   24oct97 wmt: use comp_database->n_data rather than G_s_params_n_data,
                since G_s_params_n_data does not do the right thing
                when expand_database is called from intf-reports.c

   if not found in global store, read data from disk
   */
database_DS expand_database( database_DS comp_database)
{
  static fxlstr data_file, header_file;
  int valid_file_p = TRUE, reread_p = FALSE, num_models = 0;
  int n_att, n_prop, i; 
  att_DS att;
  database_DS database;
  model_DS *models = NULL;
  FILE *header_file_fp, *log_file_fp = NULL;
  int exit_if_error_p = FALSE, silent_p = FALSE;

  data_file[0] = header_file[0] = '\0';
  
  if (comp_database->compressed_p == FALSE)
    return (comp_database);

  if (validate_data_pathname( comp_database->data_file, &data_file, exit_if_error_p,
                               silent_p) != TRUE) 
      valid_file_p = FALSE;
  if (make_and_validate_pathname( "header", comp_database->header_file, &header_file,
                                 TRUE) != TRUE)
      valid_file_p = FALSE;
  if (valid_file_p == FALSE)
    exit(1);
  if ((database = find_database( data_file, header_file, comp_database->n_data))
      != NULL) {

    return (database);
  }

  header_file_fp = fopen( header_file, "r");
  /* pass stderr as stream arg to allow error msgs out */
  database = read_database( header_file_fp, log_file_fp, data_file,
                           header_file, comp_database->n_data, reread_p, stderr);

  fclose( header_file_fp);
  check_errors_and_warnings( database, models, num_models);

  /* Now check to see if comp_database has been expanded relative to database 
  if ((database->n_atts != comp_database->n_atts) ||
      (att_info_equal( database, comp_database) != TRUE))
    extend_database( database, comp_database);
    */

  if (db_same_source_p( database, comp_database) != TRUE) {
    fprintf( stderr, "ERROR: expand_database has db and comp_db from differing sources\n"
            "       check: data_file_path, header_file_path, or n_data\n");
    exit(1);
  }
  /* Check for equality of the input attributes, which preceed the transformed attributes,
     if any. */
  if (att_info_equal( database, comp_database) != TRUE) {
    fprintf( stderr, "ERROR: expand_database found unmatched common attributes defs in "
            "<.results[-bin] file> and %s\n", header_file);
    exit(1);
  }

  /* free storage of compressed database */
  for (n_att=0; n_att<comp_database->n_atts; n_att++) {
    att = comp_database->att_info[n_att];
    if (att->n_props > 0) {
      for (n_prop=0; n_prop< att->n_props; n_prop++) {
        free( att->props[n_prop][0]);  
        free( att->props[n_prop][1]);
        /* free( att->props[n_prop][2]);  not freeable, points to static strings */
        free( att->props[n_prop]);
      }
      free( att->props);
    }
    if (eqstring( att->type, "real"))
      free( att->r_statistics);
    else if (eqstring( att->type, "discrete"))
      free( att->d_statistics);
    if (att->n_trans > 0) {
      for (i=0; i< att->n_trans; i++)
        free( att->translations[i]);
      free( att->translations);
    }
    if (att->warnings_and_errors != NULL)
      free( att->warnings_and_errors);
    free( att);
  }
  free( comp_database->att_info);
  free( comp_database);
  return (database);
}

/* EXTEND_DATABASE
   27feb95 wmt: new, from ac-x
   06mar95 wmt: could not get this to work

   This extends the attributes of db to match the att_info of comp_db,
   which is presumed to have origionated from the same source via transformations.
   */
int extend_database( database_DS db, database_DS comp_db)
{
  int db_match = TRUE, att_index, *att_list;
  shortstr transform;
  FILE *log_file_fp = NULL, *stream = NULL;
  att_DS att_i;

  if (db_same_source_p( db, comp_db) != TRUE) {
    fprintf( stderr, "ERROR: extend_database has db and comp_db from differing sources\n");
    exit(1);
  }
  /* Check for equality of the input attributes, which preceed the transformed attributes. */
  if (att_info_equal( db, comp_db) != TRUE) {
    fprintf( stderr, "ERROR: extend_database found unmatched common attributes in "
            "db and comp_db");
    exit(1);
  }
  /* The problem is now to determine what transforms were made to comp_db, and
     in what order and then to reproduce these transformations on db */
  for (att_index=db->n_atts; att_index<comp_db->n_atts; att_index++) {
    att_i = comp_db->att_info[att_index];
    strcpy( transform, att_i->sub_type);
    att_list = (int *) malloc( sizeof( int));
    att_list[0] = *((int *) getf( att_i->props, "source", att_i->n_props));
    find_transform( db, transform, att_list, 1, log_file_fp, stream);
  }
  if ((db->n_atts != comp_db->n_atts) ||
      (att_info_equal( db, comp_db) != TRUE)) {
    fprintf( stderr, "ERROR: extend_database failed to produce corresponding attributes");
    exit(1);
  }
  return( db_match);
}


/* DB_SAME_SOURCE_P
   27feb95 wmt: new, from ac-x

   This tests if two db's (compressed or expanded) come from the same source
   and have the same size.  It is intended to allow avoiding having to reread
   a database.
   */
int db_same_source_p( database_DS db, database_DS comp_db)
{
  return (eqstring( db->header_file, comp_db->header_file) &&
          eqstring( db->data_file, comp_db->data_file) &&
          (db->n_data == comp_db->n_data));
}


/* ATT_INFO_EQUAL
   27feb95 wmt: new

   Checks the attribute type, subtype, relevant properties, description &
   statistics
   */
int att_info_equal( database_DS db1, database_DS db2)
{
  int equal_p = TRUE, n_att;
  att_DS att_1, att_2;

  for (n_att=0; n_att<min( db1->n_atts, db2->n_atts); n_att++) {
    att_1 = db1->att_info[n_att];
    att_2 = db2->att_info[n_att];
    if ((att_props_equivalent_p( att_1, att_2) != TRUE) ||
        (eqstring( att_1->dscrp, att_2->dscrp) != TRUE) ||
        (att_stats_equivalent_p( att_1, att_2) != TRUE)) {
      equal_p = FALSE;
      break;
    }
  }  
  return (equal_p);
}


/* ATT_PROPS_EQUIVALENT_P
   01mar95 wmt: new

   Checks for for equality of those properties relevant to the attributes
   type & subtype, using *Att-type-data* to identify the relevant properties
   */
int att_props_equivalent_p( att_DS att_1, att_DS att_2)
{
  int equal_p = TRUE, i;

  if ((eqstring( att_1->type, att_2->type) != TRUE) ||
      (eqstring( att_1->sub_type, att_2->sub_type) != TRUE))
    equal_p = FALSE;
  else {
    for (i=0; i<min( att_1->n_props, att_2->n_props); i++)
      if (((att_1->props[i][0] != att_2->props[i][0]) != TRUE) || /* target */
          ((att_1->props[i][1] != att_2->props[i][1]) != TRUE) || /* value */
          ((att_1->props[i][2] != att_2->props[i][2]) != TRUE)) { /* type */
        equal_p = FALSE;
        break;
      }
  }
  return (equal_p);
}


/* ATT_STATS_EQUIVALENT_P
   01mar95 wmt: new

   test for equality in attribute definition components
   */
int att_stats_equivalent_p( att_DS att_1, att_DS att_2)
{
  int equal_p = TRUE, i;

  if (eqstring( att_1->type, "real")) {
    if (((att_1->r_statistics->count == att_2->r_statistics->count) != TRUE) ||
        (percent_equal( (double) att_1->r_statistics->mx,
                       (double) att_2->r_statistics->mx, REL_ERROR) != TRUE) ||
        (percent_equal( (double) att_1->r_statistics->mn,
                       (double) att_2->r_statistics->mn, REL_ERROR) != TRUE) ||
        (percent_equal( (double) att_1->r_statistics->mean,
                       (double) att_2->r_statistics->mean, REL_ERROR) != TRUE) ||
        (percent_equal( (double) att_1->r_statistics->var,
                       (double) att_2->r_statistics->var, REL_ERROR) != TRUE))
      equal_p = FALSE;
  }
  else if (eqstring( att_1->type, "discrete")) {
    if (((att_1->d_statistics->range == att_2->d_statistics->range) != TRUE) ||
        ((att_1->d_statistics->n_observed == att_2->d_statistics->n_observed) != TRUE))
      equal_p = FALSE;
    for (i=0; i<min( att_1->d_statistics->n_observed,
                    att_2->d_statistics->n_observed); i++)
      if ((att_1->d_statistics->observed[i] == att_2->d_statistics->observed[i]) != TRUE) {
        equal_p = FALSE;
        break;
      }
  }
  else if (eqstring( att_1->type, "dummy"))
    ;
  else {
    fprintf( stderr, "ERROR: att_type %s not handled\n", att_1->type);
    abort();
  }
  return ( equal_p);
}
