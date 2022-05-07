
/** \file mem.c
 *
 */

#include "vopl.h"


/*
 * newm1
 *
 *	allocate a one dimensional array
 */
double *newm1(int n)
{
   double	*p;

   if ((p = (double *)malloc(sizeof(double) * n)) == (double *)NULL)
   {
      fprintf(stderr, "newm1: request for %d bytes returns NULL\n", n);
      exit(1);
   }

   return(p);
}

/*
 * newm2
 *
 *	allocate an array of arrays
 */
double **newm2(int m, int n)
{
   double  **mat;
   int i;

   if ((mat = (double **)malloc((unsigned)sizeof(double *) * m)) == (double **)NULL)
   {
      fprintf(stderr, "newm2: request for %d bytes returns NULL\n", n);
      exit(1);
   }

   for (i = 0; i < m; i++)
   {
      if ((mat[i] = (double *)malloc(n * sizeof(double))) == (double *)NULL)
      {
         fprintf(stderr, "newm2: request for %d bytes returns NULL\n", n);
         exit(1);
      }
   }

   return(mat);
}
