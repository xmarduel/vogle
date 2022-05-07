/*
 *  cexcontour.c
 *
 */


#include <stdlib.h>
#include <math.h>

#include "vogle.h"
#include "vopl.h"

#include "MESCHACH/INCLUDES/matrix.h"
#include "MESCHACH_ADDS/INCLUDES/matrix_adds.h"


#define NB_LEVELS 10


int main(int ac, char **av)
{
   int	i;
   int	w;
	
	char device[9];
	
   int SIZE = 800;
   
   FILE *fp1 = fopen("xysomm_1.dat", "r");
   FILE *fp2 = fopen("sol_1.dat", "r");
   FILE *fp3 = fopen("nselmt_1.dat", "r");
	
   MAT *xysomm  = m_finput  (fp1, MNULL );
   VEC *sol     = v_finput  (fp2, VNULL );
   IMAT *mesh   = xim_finput(fp3, IMNULL);
	
	fclose(fp1);
   fclose(fp2);
   fclose(fp3);

	VEC *levels = v_get(NB_LEVELS);

	int idx = 0;
	double minvalue = v_min(sol, &idx);
	double maxvalue = v_max(sol, &idx);
	
	for (i=0; i<NB_LEVELS; i++)
	{
		levels->ve[i] = minvalue + (maxvalue - minvalue)/(NB_LEVELS)*(i+1);
	}
	
	
	fprintf(stderr,"Enter output device: ");
	gets(device);
	
   prefsize(SIZE, SIZE);
	
   w = vopl_winopen(device, "Solution FEM");
   backbuffer();
	
	setwindow( 0.0, 1.0, 'x');
	setwindow( 0.0, 1.0, 'y');
	
	color(BLACK);
	clear();
	
	color(RED);
	contour_mesh(xysomm->me, sol->ve, sol->dim, mesh->im, mesh->m);
	color(WHITE);
	contour_border(xysomm->me, sol->ve, sol->dim, mesh->im, mesh->m);
	/* colors set automatically */
	contour(levels->ve, NB_LEVELS, xysomm->me, sol->ve, sol->dim, mesh->im, mesh->m);
	
	swapbuffers();
   
	getkey();
	
   vopl_winclose(w);
	
   return(0);
}