#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#include <sys/types.h>
/* for char_input_test => fcntl */
/* fcntl.h not available under gcc 2.6.3
   put needed flags in "fcntlcom-ac.h" & include the next 2 files */
/* #include <sys/fcntlcom.h> */
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
/* for safe_sprintf */
#include <stdarg.h>
#include "autoclass.h"
#include "minmax.h"
#include "globals.h"
#include "fcntlcom-ac.h" 


/* SUPRESS CODECENTER WARNING MESSSAGES */

/* empty body for 'while' statement */
/*SUPPRESS 570*/
/* formal parameter '<---->' was not used */
/*SUPPRESS 761*/
/* automatic variable '<---->' was not used */
/*SUPPRESS 762*/
/* automatic variable '<---->' was set but not used */
/*SUPPRESS 765*/


/* TO_SCREEN_AND_LOG_FILE
   20oct94 wmt: modified

   Dump a message to the screen and to the specified log file. */

void to_screen_and_log_file( char *msg, FILE *log_file_fp, FILE *stream, int output_p)
{
  char caller[] = "to_screen_and_log_file";

  if (output_p == TRUE) {
    if (stream != NULL) {
      fprintf(stream, "%s", msg);
      /* wmtdebug - 
      fprintf( stderr, "%s", msg);
      fflush( stderr); */
    }
    if (log_file_fp != NULL)
      safe_fprintf(log_file_fp, caller, "%s", msg);
  }
}


/* GET_UNIVERSAL_TIME
   26apr95 wmt: remove check for NULL return value from time
   */
time_t get_universal_time (void)
{
   static time_t current_time;

   current_time = time(&current_time);
   return (current_time);
}


/* Ellen's version 
time_t get_universal_time(void)
{
   time_t *current_time;

   current_time = (time_t *)malloc(sizeof(time_t));
   time(current_time); printf("time = %s\n", ctime(current_time));
   if (*current_time == NULL)
      fprintf(stderr, "Current time not available\n");
   return (*current_time);
}
*/


/* FORMAT_UNIVERSAL_TIME
   01oct94 wmt: new
   26apr95 wmt: NULL => '\0'

   call library funcion ctime (requires time.h) to format universal time
   */
char *format_universal_time( time_t universal_time) 
{ 
    char *date_time_string_ptr;

    date_time_string_ptr = ctime(&universal_time);  
    /* chop off new-line */
    date_time_string_ptr[strlen(date_time_string_ptr) - 1] = '\0';
    return(date_time_string_ptr); 
} 


/* FORMAT_TIME_DURATION
   03oct94 wmt: new

   format delta universal time - returns leading space, but no trailing space
   */
char *format_time_duration (time_t delta_universal_time) 
{
/*   static so that this var can be passed back to caller */
/*   simple types (int, float) do not have to be static */
/*   compound types (char *, other arrays, structs) must be static or */
/*   allocated by global define or malloc */
    static char time_string[50];
    char temp_string[20];
    int days, hours, minutes, seconds, remainder_days;
    int remainder_hours, delta_time;
    int seconds_per_minute, seconds_per_hour;
    int seconds_per_day;

    seconds_per_minute = 60;
    seconds_per_hour = 3600;    /* 60 * 60  */
    seconds_per_day = 86400;    /* 24 * 60 * 60 */

    delta_time = (int) delta_universal_time;
    days = delta_time / seconds_per_day;
    remainder_days = delta_time % seconds_per_day;

    hours = remainder_days / seconds_per_hour;
    remainder_hours = remainder_days % seconds_per_hour;

    minutes = remainder_hours / seconds_per_minute;
    seconds = remainder_hours % seconds_per_minute;

    time_string[0] = '\0';      /* reset static string to null */

    if (days != 0)
      sprintf(time_string," %d day%s", days, (days > 1) ? "s" : "");
    if (hours != 0) {
      sprintf(temp_string," %d hour%s", hours, (hours > 1) ? "s" : "");
      strcat(time_string, temp_string);
    }
    if (minutes != 0) {
      sprintf(temp_string," %d minute%s", minutes, (minutes > 1) ? "s" : "");
      strcat(time_string, temp_string);
    }
    if (seconds != 0) {
      sprintf(temp_string," %d second%s", seconds, (seconds > 1) ? "s" : "");
      strcat(time_string, temp_string);
    }
    if ((days == 0) && (hours == 0) && (minutes == 0) && (seconds == 0))
      sprintf(time_string," 0 seconds");
    return(time_string);
}


/* ROUND 
   13oct94 wmt: new function
   round a float to an integer, using ceil & floor
   gcc ceil & floor return double

   int i_number, rounded_number;
   i_number = number;
   if ((number - (float) i_number) < 0.5)
     rounded_number = i_number;
   else
     rounded_number = i_number++;
   return(rounded_number);

   does not always produce the correct answer 

   06mar06 wmt: round => iround because of gcc 4.0
*/
int iround (double number)
{  
  int absolute_value, rounded_value;

  absolute_value = (int) number;
  if ((number - (double) absolute_value) >= 0.5)
    rounded_value = (int) ceil( number);
  else
    rounded_value = (int) floor( number);

  return(rounded_value);
}

/* INT_COMPARE_LESS
   14oct94 wmt: new
   compare function for qsort - smallest integer first
   */
int int_compare_less (int *i_ptr, int *j_ptr)
{
  return(*i_ptr - *j_ptr);
}


/* INT_COMPARE_GREATER
   14oct94 wmt: new
   compare function for qsort - largest integer first
   */
int int_compare_greater (int *i_ptr, int *j_ptr)
{
  return(*j_ptr - *i_ptr);
}


/* EQSTRING
   */
int eqstring( char *str1, char *str2)
{
  if (strcmp(str1, str2) == 0)
    return(TRUE);
  else
    return(FALSE);
}


float *fill( float *wts, double info, int num, int  end)
{
   int i;

   for (i=0; i<num && i<end; i++)
      wts[i] = (float) info;
   return(wts);
}


/* CHECKPOINT_CLSF
   16nov94 wmt: add log messages

   Checkpointing option, see file checkpoint.text
   */
void checkpoint_clsf( clsf_DS clsf)
{
  fxlstr msg_string;
  clsf_DS *temp;
  time_t now;

  now = get_universal_time();
  sprintf( msg_string, " [checkpt clsf (j=%d, cycle=%d) at %s] ", clsf->n_classes,
          clsf->checkpoint->current_cycle, format_universal_time( now));
  clsf->checkpoint->accumulated_try_time += now - G_search_cycle_begin_time;
  
  if (eqstring( G_checkpoint_file, "") == TRUE) {
    fprintf( stderr, "ERROR: checkpoint_clsf called with G_checkpoint_file = "
            "\"\" does nothing\n");
    exit(1);
  }
  else {
    temp = (clsf_DS *) malloc( sizeof( clsf_DS));
    temp[0] = clsf;

    save_clsf_seq( temp, 1, G_checkpoint_file, G_save_compact_p, "chkpt");

    free( temp);
    to_screen_and_log_file( msg_string, G_log_file_fp, G_stream, TRUE);
    G_last_checkpoint_written = get_universal_time();
    G_search_cycle_begin_time = now;    /* start cycle time after writing file */
  }
}


int *delete_duplicates( int *list, int num)
{
  int *new = NULL;
/********** this routine appears only to be used by 
get_sources_list which was commented so this was too

   int i, j, found, length, new_length, *new;

   new = (int *) malloc(num * sizeof(int));
   new_length = 0;
   for (i=0; i<num; i++) {
      found = 0;
      for (j=i+1; j<num; j++)
	 if (list[i] == list[j]) {
	    found = 1;
	    break;
         }
      if (found == 0)
	 new[new_length++] = list[i];
   }

    free(list); <<< is this ok
   if (new == NULL)
      new = (int *) malloc(new_length * sizeof(int));
   else new = (int *) realloc(new, new_length * sizeof(int));
   return(new);

   *************************/
   fprintf(stderr," all code commented in delete_duplicates\n");
   abort();

   return(new);

}


/* MAX_PLUS
   19dec94 wmt: return double rather than float -- float returns 32768.0
                regardless of the input args

   Maximum value of a numerical sequence of any length, or any sequence
   from which 'key generates a number from an element, or NIL for an
   empty sequence.
   */
double max_plus( float *fl, int num)
{
  int i;
  float mx;

  mx = 0;
  for (i=0; i<num; i++) 
    if (mx < fl[i]) 
      mx = fl[i];    
  return (mx); 
}


int class_duplicatesp( int n_classes, class_DS *classes)
{
   int i, j;

   for (i=0; i<n_classes; i++)
      for (j=i+1; j<n_classes; j++)
         if (classes[i] == classes[j])
	    return(TRUE);
   return(FALSE);
}


int find_term( term_DS term, term_DS *terms, int n_terms)
{
   int i;

   for (i=0; i<n_terms; i++)
      if (terms[i] == term)
	 return(i);
   return(-1);
}


int find_class( class_DS class, class_DS class_store)
{
   while(class_store != NULL){
      if (class_store == class)
	 return(TRUE);
      class_store=class_store->next;
   }
   return(FALSE);
}


int find_class_test2( class_DS class, clsf_DS  clsf, double rel_error)
{
   int i;
   for(i=0;i<clsf->n_classes;i++)
      if (class_DS_test(clsf->classes[i], class, rel_error) == TRUE)
	 return(TRUE);
   
   return(FALSE);
}


/* FIND_DATABASE_P
   23jan95 wmt: renamed from find_data
   */
int find_database_p( database_DS data, database_DS *databases, int n_data)
{
  int i;

  for (i=0; i<n_data; i++)
    if (databases[i] == data)
      return(TRUE);
  return(FALSE);
}


/* FIND_MODEL_P
   23jan95 wmt: renamed from find_model
   */
int find_model_p( model_DS model, model_DS *models, int n_models)
{
  int i;

  for (i=0; i<n_models; i++)
    if (models[i] == model)
      return(TRUE);
  return(FALSE);
}


int member_int( int val, int *list, int num)
{
   int i;

   for (i=0; i<num; i++)
      if (list[i] == val)
	 return(TRUE);
   return(FALSE);
}


int find_str_in_table( char *str, shortstr table[], int num)
{
  int i;
  
  /* fprintf (stderr, "find_str_in_table: str %s num %d\n", str, num); */
  for (i=0; i<num; i++) {
    /* fprintf (stderr, "find_str_in_table: table[i] %s\n", table[i]); */
    if (eqstring(table[i], str) == TRUE)
      return(i);
  }
  return(-1);
}


/* NEW_RANDOM
   05jan95 wmt: use normalization
   17may95 wmt: Convert from srand/rand to srand48/lrand48

   pick unused element of used_list, randomly
   */
int new_random( int n_data, int *used_list, int num)
{
  int i, temp, limit;
  double normalizer;

  normalizer = G_rand_base_normalizer / (double) n_data;
  limit = min( 1000, 10 * n_data);
  for (i=0; i<limit; i++) {
    temp = (int) (min( (double) lrand48( ), G_rand_base_normalizer) / normalizer);
    /*       temp = lrand48(); */
    /*       temp = temp % n_data; */
    if (member_int( temp, used_list, num) == FALSE)
      return (temp);
  }
  fprintf( stderr, "WARNING: new_random: unable to find an unused number\n");
  return (0);
}

#ifdef _WIN32
/* 980612 aju: This function does not exist in win32.  I kludged this one up.  Yuck.*/
long lrand48() {
    unsigned long temp;
    temp = rand() << 16;
    temp |= rand() << 1;
    temp |= (rand() & 0x1);
    return temp;
}
#endif

/* RANDOMIZE_LIST 
   18oct94 wmt: modified
   17may95 wmt: Convert from srand/rand to srand48/lrand48

   to randomize a list of length n; done in place with no need to return
   */
float *randomize_list( float *y, int n)
{
   int i, max_list_index = n - 1;
   float temp;
   double normalizer;

   normalizer = G_rand_base_normalizer/ (double) n;
   while (n > 1) {
      i = (int) (min( (double) lrand48( ), G_rand_base_normalizer) / normalizer);
      i = min( i, max_list_index);
      if(i != --n){
         temp=y[i];
         y[i]=y[n];
         y[n]=temp;
      }
   }
   return y;
 }


/* modified and moved to struct-clsf clsf_list_DS push_clsf(clsf, clsf_list)
   clsf_DS clsf;
   clsf_list_DS clsf_list;
{
   clsf_list_DS temp;

   temp = (clsf_list_DS) malloc(sizeof(struct clsf_list));
   temp->clsf = clsf;
   temp->next = clsf_list;
   return(temp);
} whole routine commented here JTP */


/* Y_OR_N_P
   modified 11oct94 wmt 

   ask user for yes or no answer  */

int y_or_n_p( char* str)
{
   char answer[] = "     ", line[] = "     ";

   while (1) {
      fprintf( stdout, "%s", str);
/*       scanf("%s", &answer); */
      fgets( line, sizeof(line), stdin);
      sscanf( line, "%s", answer);
      if ((answer[0] != 'y') && (answer[0] != 'n'))
	 fprintf( stdout, "Type \"y\" for yes or \"n\" for no.\n");
      else
        break;
   }
   if (answer[0] == 'y')
     return(TRUE);
   else return(FALSE);
}



/* this routine commented JTP 6/29 wont work as is need to pass length.
 should also consier doing in place (call recursively after swapping ends?) 
float *reverse( float *flist)
{
   int i, length = sizeof(flist) / sizeof(float);
   float *new;

   new = (float *) malloc(sizeof(flist));
   for (i=0; i<length; i++)
      new[i] = flist[length - (i + 1)];
   free(flist);< is this ok ?
   return(new);
}********************commented**************/


/* SIGMA_SQ
   27nov94 wmt: use percent_equal for float tests
   19dec94 wmt: return double rather than float -- float returns 32768.0
                regardless of the input args
   27dec94 wmt: not referenced by store_real_stats anymore             
   */
double sigma_sq( int n, double sum, double sum_sq, double min_variance)
{
  double mean, var;

  mean = ((double) sum / 1.0) / (double) n;
  var = (((double) sum_sq / 1.0) / (double) n) - (double) (mean * mean);
  if (!percent_equal( min_variance, FLOAT_UNKNOWN, REL_ERROR))
    var = max(var, min_variance);
  return(var);
}


/* CHAR_INPUT_TEST
   05nov94 wmt: use fcntl to make getc be "no-hang" (non-blocking)
   14aug95 wmt: if G_interactive_p = FALSE, do not do the test

   Detects a character, currently one of '(~ q Q), in the input stream,
   and replaces it in the stream.

    980612 aju: Added a completely separate version for Win32, which doesn't 
    behave. 
   */
#ifndef _MSC_VER
int char_input_test()
{
  int c, fcntl_flags, stdin_fd = 0;

  if (G_interactive_p == FALSE)
    return (FALSE);

  fcntl_flags = fcntl( stdin_fd, F_GETFL );
  fcntl_flags |= O_NDELAY;
  fcntl( stdin_fd, F_SETFL, fcntl_flags );

  c = getc(stdin);

  fcntl_flags &= ~O_NDELAY;
  fcntl( stdin_fd, F_SETFL, fcntl_flags );

  if (c == '~')
    return(TRUE);
  else if ((c == 'q') || (c == 'Q')) {
    ungetc(c, stdin);
    return(TRUE);
  }
  else
    return(FALSE);
}
#else
int char_input_test()
{
    int c, stdin_fd = 0;
    DWORD nc, dConsMode;
    HANDLE hCons;
    INPUT_RECORD sInp;
      
    if (G_interactive_p == FALSE)
        return (FALSE);
      
    hCons = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hCons,&dConsMode);
    dConsMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    SetConsoleMode(hCons,dConsMode );
    
    if( ! GetNumberOfConsoleInputEvents(hCons,&nc) )
        return FALSE;
    if( !nc )
        return FALSE;
    if( !ReadConsoleInput(hCons,&sInp,1,&nc) )
        return FALSE;
    if( (sInp.EventType == KEY_EVENT) &&
        (sInp.Event.KeyEvent.bKeyDown) ) {
        c = sInp.Event.KeyEvent.uChar.AsciiChar;
        if ((char)c == '~') {
            return(TRUE);
        } else if (((char)c == 'q') || ((char)c == 'Q')) {
            WriteConsoleInput(hCons,&sInp,1,&nc);
            return(TRUE);
        }
    }
    return(FALSE);
}
#endif


/* PERCENT_EQUAL
   18oct94 wmt: modified

   Returns t if difference is less than rel-error times the mean.
   Note that if one value is zero and rel-error is < 2.0, the other
   value must also be zero.
   With rel-error >= 2.0, any two values are acceptable.
   */
int percent_equal( double n1, double n2, double rel_error)
{
   double val1, val2;

   val1 = 0.5 * rel_error * (fabs( n1) + fabs( n2));
   val2 = fabs( n1 - n2);
   if (val1 >= val2)
      return(TRUE);
   else 
     return(FALSE);
}


int prefix(char *str,char *substr)
{
   int i, l1 = strlen(str), l2 = strlen(substr);

   if (l1 < l2)
      return(FALSE);
   else
      for (i=0; i<l2; i++)
	 if (str[i] != substr[i])
	    return(FALSE);
   return(TRUE);
}


void *getf( void ***list, char *property, int num)
{
   int i;
   char *str;

   for (i=0; i<num; i++) {
      str = list[i][0];
      if (eqstring(str, property) == TRUE)
	 return(list[i][1]);
   }
   return(NULL);
}


void *get( char *target, char *property)
{
   int i;
   char t1[STRLIMIT], t2[STRLIMIT];

   for (i=0; i<G_plength; i++) {
      sprintf(t1, "%s", (char *) G_plist[i][0]);
      sprintf(t2, "%s", (char *) G_plist[i][1]);

/*       if (eqstring(t1, "single_normal_cm")) */
/*           printf(" i=%d get-0:%s, get-1:%s, get-2str:%s, get-2dec:%d\n", */
/*                  i, t1, t2, G_plist[i][2], ((int *) G_plist[i][2]) [0]); */

      if ((eqstring(t1, target) == TRUE) && (eqstring(t2, property) == TRUE))
         return(G_plist[i][2]);
   }
   return(NULL);
}


void add_property( shortstr target, shortstr pname, void *value)
{
   int n;

   n = G_plength;
   G_plength++;
   if (G_plist == NULL)
      G_plist = (void ***) malloc(sizeof(void **));
   else 
      G_plist = (void ***) realloc(G_plist, G_plength * sizeof(void **));

   G_plist[n] = (void **) malloc(3 * sizeof(void *));

   G_plist[n][0] = (char *) malloc(sizeof(fxlstr));
   strcpy(G_plist[n][0], target);

   G_plist[n][1] = (char *) malloc(sizeof(fxlstr));
   strcpy(G_plist[n][1], pname);

   G_plist[n][2] = value;
}


/* ADD_TO_PLIST
   21dec94 wmt: add param to specify value type, for use of write att_DS
                in io-results.c
*/
void add_to_plist ( att_DS att, char *target, void *value, char *type)
{
   int n;

   if ( (n = (att->n_props)++) == 0)
      att->props = (void ***) malloc(sizeof(void **));
   else 
      att->props = (void ***) realloc(att->props, att->n_props * sizeof(void **));

   att->props[n] = (void **) malloc(3 * sizeof(void *));
   att->props[n][0] = target;
   att->props[n][1] = value;
   att->props[n][2] = type;
}


/* WRITE_VECTOR_FLOAT
   21nov94 wmt: new

   variation on print_vector_f, used for writing .results files
   */
void write_vector_float(float *vector, int n, FILE *stream)
{
  int i;
  char caller[] = "write_vector_float";

  for (i=0; i<n; i++) {
    if ((i > 0) && ((i % NUM_TOKENS_IN_FXLSTR) == 0))
      safe_fprintf(stream, caller, "\n");
    safe_fprintf(stream, caller, "%.7e ", vector[i]);
  }
  if (n > 0)
    safe_fprintf(stream, caller, "\n");
}


/* WRITE_MATRIX_FLOAT
   21nov94 wmt: new

   variation on print_matrix_f, used for writing .results files
   */
void write_matrix_float( float **vector, int m, int n, FILE *stream)
{
  int i, j, first_row = TRUE;
  char caller[] = "write_matrix_float";

  for (i=0; i<m ;i++) {
    if (first_row == TRUE)
      first_row = FALSE;
    else
      safe_fprintf(stream, caller, "\n");
    safe_fprintf(stream, caller, "row %d\n", i);
    for (j=0; j<n; j++) {
      if ((j > 0) && ((j % NUM_TOKENS_IN_FXLSTR) == 0))
        safe_fprintf(stream, caller, "\n");
      safe_fprintf(stream, caller, "%.7e ", vector[i][j]);
    }
  }
  if (m > 0)
    safe_fprintf(stream, caller, "\n");
}


/* WRITE_MATRIX_INTEGER
   21nov94 wmt: new

   variation on print_matrix_i, used for writing .results files
   */
void write_matrix_integer( int **vector, int m, int n, FILE *stream)
{
  int i, j, first_row = TRUE;
  char caller[] = "write_matrix_integer";

  for (i=0; i<m ;i++) {
    if (first_row == TRUE)
      first_row = FALSE;
    else
      safe_fprintf(stream, caller, "\n");
    safe_fprintf(stream, caller, "row %d\n", i);
    for (j=0; j<n; j++) {
      if ((j > 0) && ((j % NUM_TOKENS_IN_FXLSTR) == 0))
        safe_fprintf(stream, caller, "\n");
      safe_fprintf(stream, caller, "%d ", vector[i][j]);
    }
  }
  if (m > 0)
    safe_fprintf(stream, caller, "\n");
}


/* READ_VECTOR_FLOAT
   19jan95 wmt: new

   variation on print_vector_f, used for writing .results files.
   use fscanf, rather than fgets & sscanf since fscanf keeps track
   of file ptr, and sscanf does not keep track of string ptr.
   */
void read_vector_float( float *vector, int n, FILE *stream)
{
  int i;

  for (i=0; i<n; i++) {
    if ((i > 0) && ((i % NUM_TOKENS_IN_FXLSTR) == 0))
      fscanf( stream, "\n");
    fscanf( stream, "%e ", &vector[i]);
  }
  if (n > 0)
    fscanf( stream, "\n");
}


/* READ_MATRIX_FLOAT
   19jan95 wmt: new

   variation on print_matrix_f, used for writing .results files
   use fscanf, rather than fgets & sscanf since fscanf keeps track
   of file ptr, and sscanf does not keep track of string ptr.
    */
void read_matrix_float( float **vector, int m, int n, FILE *stream)
{
  int i, j, first_row = TRUE, i_file;

  for (i=0; i<m ;i++) {
    if (first_row == TRUE)
      first_row = FALSE;
    else
      fscanf( stream, "\n");
    fscanf( stream, "row %d\n", &i_file);
    for (j=0; j<n; j++) {
      if ((j > 0) && ((j % NUM_TOKENS_IN_FXLSTR) == 0))
        fscanf( stream, "\n");
      fscanf( stream, "%e ", &vector[i][j]);
    }
  }
  if (m > 0)
    fscanf( stream, "\n");
}


/* READ_MATRIX_INTEGER
   19jan95 wmt: new

   variation on print_matrix_i, used for writing .results files
   use fscanf, rather than fgets & sscanf since fscanf keeps track
   of file ptr, and sscanf does not keep track of string ptr.
    */
void read_matrix_integer( int **vector, int m, int n, FILE *stream)
{
  int i, j, first_row = TRUE, i_file;

  for (i=0; i<m ;i++) {
    if (first_row == TRUE)
      first_row = FALSE;
    else
      fscanf( stream, "\n");
    fscanf( stream, "row %d\n", &i_file);
    for (j=0; j<n; j++) {
      if ((j > 0) && ((j % NUM_TOKENS_IN_FXLSTR) == 0))
        fscanf( stream, "\n");
      fscanf( stream, "%d ", &vector[i][j]);
    }
  }
  if (m > 0)
    fscanf( stream, "\n");
}


/* DISCARD_COMMENT_LINES
   28nov94 wmt: new

   check first column for '#', '!', ' ', or ';', then read to \n
   return EOF, if found
   */
int discard_comment_lines (FILE *stream)
{
  int c;

  c = fgetc(stream);
  if (c == EOF)
    return(EOF);
  while ((c == ';') || (c == '!') || (c == '#') || (c == ' ') || 
    (c == '\n')) {
    /*     printf("\ndiscard "); */
    if (c != '\n')
      flush_line(stream);
    c = fgetc(stream);
  }
  ungetc(c, stream);
  return(c);
}


/* FLUSH_LINE
   28nov94 wmt: new

   read to \n or \r or EOF
   */
void flush_line (FILE *stream)
{
  int c;

  /* printf("\nflush "); */
  while (((c = fgetc(stream)) != '\n') && (c != '\r') &&
         (c != EOF))
    /* printf("%c", (char) c); */
  ;
}


/* READ_CHAR_FROM_SINGLE_QUOTES
   30nov94 wmt: new

   read char c from 'c', and flush rest of line
   */
int read_char_from_single_quotes (char *param_name, FILE *stream)
{
  int c;
  
  c = fgetc(stream);
  while ((c == ' ') || (c == '\n'))
    c = fgetc(stream);
  if (c == '\'') {
    c = fgetc(stream);
    flush_line(stream);
    return(c);
  }
  else {
    fprintf(stderr,
            "ERROR: for %s, expected to read first ' from 'c', read %c instead!\n",
            param_name, (char) c);
    exit(1);
  }
  return(0);    /* must return something */
}


/* 02dec94 wmt: moved from io-read-model.c
   not currently used
   */
int strcontains( char *str, int c)
{
   int i, length = strlen(str);

   for (i=0; i<length; i++)
      if (str[i] == (char) c)
	 return(TRUE);
   return(FALSE);
}


/* OUTPUT_INT_LIST
   25jan95 wmt: new

   output list of integers to log file & stream
   return number if integers output
   */
int output_int_list( int_list i_list, FILE *log_file_fp, FILE *stream)
{
  int first_list_item = TRUE, cnt = 0;
  shortstr str;

  for ( ; *i_list != END_OF_INT_LIST; i_list++) {
    if (first_list_item == TRUE)
      first_list_item = FALSE;
    else
      to_screen_and_log_file( ",", log_file_fp, stream, TRUE);
    cnt++;
    sprintf( str, "%d", *i_list);
    to_screen_and_log_file(str, log_file_fp, stream, TRUE);
  }
  return (cnt);
}


/* POP_INT_LIST
   30jan95 wmt: new

   take value from top of list; return false if list empty
   */
int pop_int_list( int *list, int *n_list_ptr, int *value_ptr)
{
  int found_j_in = TRUE, i;

  *n_list_ptr = 0;
  for (i=0; list[i] != END_OF_INT_LIST; i++) {
    (*n_list_ptr)++;
    if (i == 0)
      *value_ptr = list[i];
  }
  if (*n_list_ptr > 0) {
    for (i=0; i<*n_list_ptr; i++)
      list[i] = list[i+1];
    (*n_list_ptr)--;
  }
  else
    found_j_in = FALSE;
  return (found_j_in);
}


/* PUSH_INT_LIST
   30jan95 wmt: new

   push value onto top of list
   */
void push_int_list( int *list, int *n_list_ptr, int value, int max_n_list)
{
  int i;

  *n_list_ptr = 0;
  for (i=0; list[i] != END_OF_INT_LIST; i++) 
    (*n_list_ptr)++;
  if (*n_list_ptr > (max_n_list - 2)) {
    fprintf( stderr, "ERROR: integer list of type \"int_list\" is full\n");
    abort();
  }
  for (i=*n_list_ptr; i>=0; i--)
    list[i+1] = list[i];
  list[0] = value;
  (*n_list_ptr)++;
}


/* MEMBER_INT_LIST
   02feb95 wmt: new

   return true if val in is list of type int_list
   */
int member_int_list( int val, int_list list)
{
  int i;

  for (i=0; list[i] != END_OF_INT_LIST; i++)
    if (list[i] == val)
      return (TRUE);
  return (FALSE);
}


/* FLOAT_SORT_CELL_COMPARE_GTR
   03feb95 wmt: new
   compare function for qsort - largest float first in list of struct sort_cell.
   when equal, sort by lowest int_value
   */
int float_sort_cell_compare_gtr( sort_cell_DS i_cell, sort_cell_DS j_cell)
{
  if (i_cell->float_value < j_cell->float_value)
    return (1);
  else if (i_cell->float_value > j_cell->float_value)
    return (-1);
  else {                        /* equal */
    if (i_cell->int_value < j_cell->int_value)
      return (-1);
    else if (i_cell->int_value > j_cell->int_value)
      return (1);
    else
      return (0);
  }
}


/* CLASS_CASE_SORT_COMPARE_LSR
   09feb95 wmt: new
   compare function for qsort - lowest class first in xref_data structs
   keep case ordering within the class
   */
int class_case_sort_compare_lsr( xref_data_DS i_xref, xref_data_DS j_xref)
{
  if (i_xref->class_case_sort_key > j_xref->class_case_sort_key)
    return (1);
  else if (i_xref->class_case_sort_key < j_xref->class_case_sort_key)
    return (-1);
  else
    return (0);
}


/* ATT_I_SUM_SORT_COMPARE_GTR
   13feb95 wmt: new
   10mar95 wmt: for equal att_i_sum, lower attribute number will
                come first

   compare function for ordered_normalized_influence_values
   */
int att_i_sum_sort_compare_gtr( ordered_influ_vals_DS i_influ_val,
                               ordered_influ_vals_DS j_influ_val)
{
  if (i_influ_val->att_i_sum < j_influ_val->att_i_sum)
    return (1);
  else if (i_influ_val->att_i_sum > j_influ_val->att_i_sum)
    return (-1);
  else  {                       /* equal */
    if (i_influ_val->n_att < j_influ_val->n_att)
      return (-1);
    else if (i_influ_val->n_att > j_influ_val->n_att)
      return (1);
    else
      return (0);
  }
}


/* FLOAT_P_P_STAR_COMPARE_GTR
   16feb95 wmt: new
   23jun95 wmt: sort the absolute value
   
   compare function for formatted_p_p_star_list
   */
int float_p_p_star_compare_gtr( formatted_p_p_star_DS i_formatted_p_p_star,
                               formatted_p_p_star_DS j_formatted_p_p_star)
{
  if (i_formatted_p_p_star->abs_att_value_influence <
      j_formatted_p_p_star->abs_att_value_influence)
    return (1);
  else if (i_formatted_p_p_star->abs_att_value_influence >
           j_formatted_p_p_star->abs_att_value_influence)
    return (-1);
  else
    return (0);
}


/* SAFE_FPRINTF
   13mar95 wmt: new

   NOTE: this function is not interpreted correctly by CodeCenter,
   and the middle three lines must be commented out when
   interpreted (probably some king of library/include problem)

   this is a wrapper to check for error returns from fprintf
   */
void safe_fprintf( FILE *stream, char *caller, char *format, ...) 
{
  int return_cnt = 0;
  va_list arg_addr;

  /* fprintf( stderr, "safe_fprintf: caller %s, format \"%s\"\n", caller, format);
  fflush( stderr); */

  va_start( arg_addr, format);
  return_cnt = vfprintf( stream, format, arg_addr);
  va_end( arg_addr);

  /* fprintf( stderr, "safe_fprintf: caller %s, return_cnt %d\n", caller, return_cnt);
  fflush( stderr); */

  if (return_cnt < 0) {
    fprintf( stderr, "ERROR: fprintf returned %d -- called by %s\n",
            return_cnt, caller);
    abort();
  }
}


/* SAFE_SPRINTF
   02may95 wmt: new

   NOTE: this function is not interpreted correctly by CodeCenter,
   and the middle three lines must be commented out when
   interpreted (probably some kind of library/include problem)

   detect overwriting string arrays while using sprintf
   */
void safe_sprintf( char *str, int str_length, char *caller, char *format, ...) 
{
  int return_cnt = 0;
  va_list arg_addr;

  /* fprintf( stderr, "safe_sprintf: len %d '%s' -- called by %s\n", str_length, str, caller); */



  va_start( arg_addr, format);
  return_cnt = (int) vsprintf( str, format, arg_addr);
  va_end( arg_addr); 

  /* vsprintf always retrurns negative value - ??? */
     if (return_cnt < 0) { 
    fprintf( stderr, "ERROR: vsprintf had an error return: %d) " 
            "-- called by %s\n", 
            return_cnt, caller); 
    abort(); 
    } /*  */

  /* debug 
  fprintf( stderr, "actual=%.3d, limit=%.3d, caller = %s\n", (int) strlen( str),
          str_length - 1, caller); */
     
  /*
  if ((int) strlen( str) > (str_length - 1)) {
    fprintf( stderr, "ERROR: vsprintf produced %d chars (max number is %d) "
            "-- called by %s\n",
            (int) strlen( str), (str_length - 1), caller);
    abort();
  }
  */ 
}


