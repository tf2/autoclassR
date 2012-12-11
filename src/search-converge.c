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
/* Performs a `recalculate likelihoods & update' cycle, eliminating
   null classes. */


/* BASE_CYCLE
   30jan95 wmt: add checkpointing logic from ac-x
   */
double base_cycle( clsf_DS clsf, FILE *stream, int display_wts, int converge_cycle_p)
{
  int n_stored, i;

  update_wts( clsf, clsf);

  n_stored = delete_null_classes(clsf);

  if ((display_wts == TRUE) && (n_stored > 0) && (stream != NULL))
    fprintf(stream, "%d null classes stored from base-cycle.\n", n_stored);

  update_parameters(clsf);

  update_approximations(clsf);

  if ((display_wts == TRUE) && (stream != NULL))
    display_step(clsf, stream);

  if ((eqstring( G_checkpoint_file, "") != TRUE) && (converge_cycle_p == TRUE) &&
      (clsf->n_classes > 1) && ((get_universal_time() - G_last_checkpoint_written) >
                                G_min_checkpoint_period))
    checkpoint_clsf( clsf);

  if ((converge_cycle_p == TRUE) && (stream != NULL))
    fprintf(stream, ".");       /* show cycle  */
  /*     fprintf(stderr, "base_cycle - log_a_x_h=%e\n", clsf->log_a_x_h);  */
  if (G_clsf_storage_log_p == TRUE) {
    fprintf( stream, "\n");
    for (i=0; i<clsf->n_classes; i++)
      fprintf( stream, "%.4d ", iround( (double) clsf->classes[i]->w_j));
  }
  return (clsf->log_a_x_h);
}
