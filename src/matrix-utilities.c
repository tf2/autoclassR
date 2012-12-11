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
/* Sets the values of vector V1 to the values of V2. */


float *setf_v_v( float *v1, float *v2, int num)
{
   int i;

   for (i=0; i<num; i++)
      v1[i] = v2[i];
   return(v1);
}


/* Increments the values of vector V1 by the values of vector V2. */

float *incf_v_v(float *v1, float *v2, int num)
{
   int i;

   for (i=0; i<num; i++)
      v1[i] += v2[i];
   return(v1);
}


/* Decrements the values of vector V1 by the values of vector V2. */

float *decf_v_v(float *v1, float *v2, int num)
{
   int i;

   for (i=0; i<num; i++)
      v1[i] -= v2[i];
   return(v1);
}


/* INCF_V_VS
    27dec94 wmt: pass type double, rather than float

   Increments the values of vector V1 by (* scale V2).
   */
float *incf_v_vs( float *v1, float *v2, double scale, int num)
{
   int i;
   float float_scale = (float) scale;

   for (i=0; i<num; i++)
      v1[i] += (v2[i] * float_scale);
   return(v1);
}


/* SETF_V_VS
   27dec94 wmt: pass type double, rather than float

   Sets the values of vector V1 to (* scale V2).
 */
float *setf_v_vs(float *v1, float *v2, double scale, int num)
{
   int i;
   float float_scale = (float) scale;

   for (i=0; i<num; i++)
      v1[i] = v2[i] * float_scale;
   return(v1);
}


/* INCF_M_VVS
   27dec94 wmt: pass type double, rather than float
   */
fptr *incf_m_vvs( fptr *m1, float *v1, float *v2, double scale, int num)
{
   int i, j;
   float value2, float_scale = (float) scale;

   for (i=0; i<num; i++) {
      value2 = v2[i] * float_scale;
      for (j=0; j<num; j++)
	 m1[i][j] += (v1[j] * value2);
   }
   return(m1);
}


/* DIAGONAL_PRODUCT
   27dec94 wmt: return type double, rather than float
   02mar95 wmt: make product double to prevent overflow

   Generates the product of the diagonal elements of M1. */

double diagonal_product( fptr *m1, int num)
{
  int i;
  double product = 1.0;

  for (i=0; i<num; i++)
    product *= (double) m1[i][i];
  return( product);
}


/* Generates the diagonal matrix of the square matrix m1, using M-diagonal
   when it is given. */

fptr *extract_diagonal_matrix( fptr *m1, fptr *m_diagonal, int num)
/* original had to adjust for -mij in lower. this version never makes it neg*/
{
   int i, j;


   for (i=0; i<num; i++)
      for (j=0; j<num; j++)
	 m_diagonal[i][j] = (i != j)?0.0 : m1[i][i];
   return(m_diagonal);
}


/* UPDATE_MEANS_AND_COVARIANCE
   27nov94 wmt: use percent_equal for float tests
   27dec94 wmt: remove FLOAT_UNKNOWN test

   A variation of CENTRAL-MEASURES specially hacked to avoid local allocation
   of vectors or matrices.  To this end, the Values, Means and Covar are used
   as accumulaters in the mean and covariance calculations.
   Treats any values vector containing one or more nil elements as unknown.
   This version calculates moments with respect to est-means in order to obtain
   reasonable accuracy when the variance is small relative to the true means.
   RETURNS multiple values: Unknown-wt Known-wt Means Covar.
   updates the means and covar arguments.  modifies the values. 
*/
void update_means_and_covariance( float **data, int n_data, float *att_indices,
     float *wts, float *est_means, float *means, fptr *covar, float *values, int num) 

{
  int i, j;
  float unknown = 0.0, known = 0.0, wt, *datum;

  for (i=0; i<num; i++) {
    means[i] = 0.0;
    for (j=0; j<num; j++)
      covar[i][j] = 0.0;
  }
  for (i=0; i<n_data; i++) {
    wt = wts[i];
    if (wt > 0.0) {
      datum = data[i];
      collect_indexed_values(values, att_indices, datum, num);
      j=num;
      while ( j-- > 0 && !(percent_equal( (double) values [j], FLOAT_UNKNOWN,
                                         REL_ERROR)));
      if (j < 0){
        known += wt;
        decf_v_v(values, est_means, num);
        incf_v_vs(means, values, (double) wt, num);
        incf_m_vvs(covar, values, values, (double) wt, num);
      }
      else 
        unknown += wt;
    }
  }
  for (i=0; i<num; i++)
    values[i] = 0.0;
  if (known != 0.0) {
    n_sv( (double) (1.0 / known), means, num);
    n_sm( (double) (1.0 / known), covar, num);
    incf_m_vvs(covar, means, means, (double) -1.0, num);
    incf_v_v(means, est_means, num);
  }
}


/* N_SM
   27dec94 wmt: pass type double, rather than float

   Destructively alters m1 by multiplying the elements by scale.
 */
fptr *n_sm( double scale, fptr *m1, int num)
{
   int i, j;
   float float_scale = (float) scale;

   for (i=0; i<num; i++)
      for (j=0; j<num; j++)
	 m1[i][j] *= float_scale;
   return(m1);
}


/* This makes a vector V2 that represents the diagonal root of a diagonal
   matrix. */

float *vector_root_diagonal_matrix( fptr *m1, int num)
{
   int i;
   float *v2;

   v2 = (float *) malloc(num * sizeof(float));
   for (i=0; i<num; i++)
      v2[i] = (float) sqrt((double) m1[i][i]);
   return(v2);
}


/* DOT_VV
    27dec94 wmt: return type double, rather than float. make
                sum type double
    
   Generates the dot product of two vectors, using double precision
   accumulation.
   */
double dot_vv( float *row, float *col, int num)
{
   int i;
   double sum = 0.0;

   for (i=0; i<num; i++)
      sum += row[i] * col[i];
   return(sum);
}


/* DOT_MM
    27dec94 wmt: return type double, rather than float

   Generates the dot product of two matrices, which equals the trace of the
   product of one with the transpose of the other, or simply the trace of the
   product if either matrix is symmetric.
   */
double dot_mm( fptr *m1, fptr *m2, int num)
{
   int i, j;
   double sum = 0.0;

   for (i=0; i<num; i++)
      for (j=0; j<num; j++)
	 sum += m1[i][j] * m2[i][j];
   return(sum);
}


/* This collects into vector 'acc-V the indexed values of vector 'values-V. */

float *collect_indexed_values( float *acc_v,float *index_list,float *values_v,
				     int num)
{
   int i;

   for (i=0; i<num; i++) {
      acc_v[i] = values_v[(int) index_list[i]];
   }
   return(acc_v);
}


/* Sets the elements of matrix 'to to be the elements of matrix 'from. */

fptr *copy_to_matrix( fptr *from, fptr *to, int num)
{
   int i, j;

   for (i=0; i<num; i++)
      for (j=0; j<num; j++)
	 to[i][j] = from[i][j];
   return(to);
}


/* N_SV
    27dec94 wmt: pass type double, rather than float

   Destructivly alters vec by multiplying the elements by scale.
   */
float *n_sv( double scale, float *vec, int num)
{
   int i;
   float float_scale = (float) scale;

   for (i=0; i<num; i++)
      vec[i] *= float_scale
      ;
   return(vec);

}


/* SETF_M_MS
    27dec94 wmt: pass type double, rather than float

   Sets matrix M1 to (* scale M2).
 */
fptr *setf_m_ms( fptr *m1, fptr *m2, double scale, int num)
{
   int i, j;
   float float_scale = (float) scale;

   for (i=0; i<num; i++)
      for (j=0; j<num; j++)
	 m1[i][j] = m2[i][j] * float_scale;
   return(m1);
}


/* INCF_M_MS
    27dec94 wmt: pass type double, rather than float

   Increments matrix M1 by (* scale M2).
 */
fptr *incf_m_ms( fptr *m1, fptr *m2, double scale, int num)
{
   int i,j;
   float float_scale = (float) scale;

   for (i=0; i<num; i++)
      for (j=0; j<num; j++)
         m1[i][j] += m2[i][j] * float_scale;
   return(m1);
}


/* Alters square matrix M to have a minimum value of min-value on the
   diagonal. */

fptr *limit_min_diagonal_values( fptr *m, float *mins_vec, int num)
{
   int i;

   for (i=0; i<num; i++)
      m[i][i] = max(mins_vec[i], m[i][i]);
   return(m);
}


/* Sets M-invert to be the invert of the matrix from factor F1 was generated. */

fptr *invert_factored_square_matrix( fptr *a, fptr *ainv, int n)

/* inverts nxn matrix a given LU deomposition of a
 accomplished by solving AX=I 1 column at a time*/
{
   float temp;
   int i,j;

    for(i=0;i<n;i++)
    {
	 /* for conveninece do each col in row and transpose later*/
       for (j=0;j<n;j++)
	  ainv[i][j]=(i != j)?0.0:1.0;
       solve(a,ainv[i],n);
     }

     for(i=0;i<n-1;i++)
	for (j=i+1;j<n;j++)
       {
	 temp=ainv[i][j];
         ainv[i][j]=ainv[j][i];
         ainv[j][i]=temp;
       }
   return(ainv);
}


/* DETERMINENT_F
    27dec94 wmt: return type double, rather than float

   Finds the determinent from the LU factorization of a matrix. 

   (defun Determinent-F (fs)
   (declare (type factor-$ fs))
   (unless (factor-$-factored fs) (factor fs))
   (WITHOUT-FLOATING-UNDERFLOW-TRAPS
     (* (float (expt -1 (mod (factor-$-n-pivots fs) 2)))
        (Diagonal-Product (factor-$-matrix fs)))))
*/
double determinent_f(fptr *fs, int n)
{
   double det = 1.0;
   int i;

   for(i=0;i<n;i++)
      det *= fs[i][i]; /* since no pivoting was done, n-pivots=0 and sign=+1 */
   return(det);	
}


/* STAR_VMV
   23feb95 wmt: new (lisp name is *vmv)

   Computes the quadratic function V'*M*V, of vector V and symmetric square matrix M.
   Note that the symmetry of M is not tested.  Use *VpMpqVq for asymetric M.
   This does the symmetric calculation in O(n(n-1)/2) = O(n^2)
   */
double star_vmv( fptr *m, fptr v, int num)
{
  float sum = 0.0, v_i;
  int i, j;

  for (i=0; i<num; i++) {
    v_i = v[i];
    for (j=0; j<i; j++)
      sum += 2.0 * m[i][j] * v_i * v[j];
    sum += m[i][i] * square( v_i);
  }
  return (sum);
}


/* TRACE_STAR_MM
   23feb95 wmt: new (lisp name is trace-*mm), assume square matricies

   Generates the sum of the diagonal elements of M1*M2,
   without actually performing the multiplication.
   This gives O(ik) rather than O(ijk) time.
   */
double trace_star_mm( fptr *m1, fptr *m2, int num)
{
  float sum = 0.0;
  int i, j;

  for (i=0; i<num; i++)
    for (j=0; j<num; j++)
      sum += m1[i][j] * m2[j][i];
  return(sum);
}


/* EXTRACT_RHOS
   23feb95 wmt: new

   Generates the matrix of rho values corresponding to the off diagonal covariance values.
   */
fptr *extract_rhos( fptr *m, int num)
{
  float **inv_root_diag, **diagonal_matrix, **mm_matrix, **return_matrix;
  int i;

  diagonal_matrix = invert_diagonal_matrix( m, num);
  inv_root_diag = root_diagonal_matrix( diagonal_matrix, num);
  mm_matrix = star_mm( m, inv_root_diag, num);
  return_matrix = star_mm( inv_root_diag, mm_matrix, num);

  for (i=0; i<num; i++) {
    free( diagonal_matrix[i]);
    free( inv_root_diag[i]);
    free( mm_matrix[i]);
  }
  free( diagonal_matrix);
  free( inv_root_diag);
  free( mm_matrix);
  return (return_matrix);
}


/* INVERT_DIAGONAL_MATRIX
   23feb95 wmt: new, assumes square matrix

   This creates a matrix that is the invert of the input diagonal matrix.
   It may also be used to extract and invert the diagonal of any square matrix.
   */
fptr *invert_diagonal_matrix( fptr *m, int num)
{
  fptr *m2;
  int c;

  m2 = make_matrix( num, num);

  for (c=0; c<num; c++)
    m2[c][c] = 1.0 / m[c][c];

  return (m2);
}


/* ROOT_DIAGONAL_MATRIX
   23feb95 wmt: new, assumes square matrix

   This finds the root of a diagonal matrix, the M2 such that M2*M2 = M1.
   It may also be used to extract the root of the diagonal of any square matrix.
   */
fptr *root_diagonal_matrix( fptr *m, int num)
{
  int c;
  fptr *m2;

  m2 = make_matrix( num, num);

  for (c=0; c<num; c++)
    m2[c][c] = (float) sqrt( (double) m[c][c]);

  return (m2);
}


/* STAR_MM  
   23feb95 wmt: new (Lisp name is *mm)

   Generates the product of two square matrices.
   */
fptr *star_mm( fptr *m1, fptr *m2, int num)
{
  int n_cycle = num, n_rows = num, n_cols = num, c, r, k;
  fptr *m3, col_2, col_3;
  float sum;

  m3 = make_matrix( n_rows, n_cols);

  for (c=0; c<n_cols; c++) {
    col_2 = m2[c];
    col_3 = m3[c];
    for (r=0; r<n_rows; r++) {
      sum = 0.0;
      for (k=0; k<n_cycle; k++)
        sum += m1[r][k] * col_2[k];
      col_3[r] = sum;
    }
  }
  return (m3);
}
 

/* MAKE_MATRIX
   23feb95 wmt: new

   create, allocate, & initialize to 0.0 storage for a matrix
   */
fptr *make_matrix( int num_rows, int num_cols)
{
  fptr *m;
  int i, j;

  m = (fptr *) malloc( num_rows * sizeof( fptr));
  for (i=0; i<num_rows; i++) {
    m[i] = (fptr) malloc( num_cols * sizeof( float));
    for (j=0; j<num_cols; j++)
      m[i][j] = 0.0;
  }
  return (m);
}
