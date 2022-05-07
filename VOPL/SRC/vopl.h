
/** \file vopl.h
 *
 */

#ifndef VOPL_H_
#define VOPL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "vogle.h"
#include "vopl_error.h"




#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#define ABS(a)		((a) < 0.0 ? (-a) : (a))

#define	EBUF_SIZE	100		/* size of error message buffer */
#define	LABEL_LEN	50		/* max length of an axis label */


/*
 * fits
 */
#define	NO_LINES        0
#define	STRAIGHT_LINE	1
#define LEAST_SQUARE	2
#define CUBIC_SPLINE	3
#define	POWER_EQN       4
#define	SGR_FIT		5

#define	MAX_FIT		5		/* must be maximum fit number */

/*
 * spline types
 */
#define	FREE	 	0
#define	CLAMPED		1

/*
 * scalings
 */
#define	LINEAR		0
#define	LOGARITHMIC	1

/*
 * boundaries in which the graph is plotted (viewport [-1:1]x[-1:1])
 */
#define	XMIN	-0.75
#define	YMIN	-0.75
#define	XMAX	 0.75
#define	YMAX	 0.75

/*
 * size of line-markers on axes
 */
#define	LINELEN		0.02

/*
 * textsizes
 */
#define	TEXTHEIGHT	0.05
#define	TEXTWIDTH	0.035

/*
 * marker size
 */
#define	MARKERSIZE	0.05


/*
 * axis indexes
 */
#define	XIND		0
#define	YIND		1
#define	ZIND		2

#define	AXES		3	/* number of axes */


/*
 * symbols
 */
#define VOPL_CIRCLE         "a"
#define VOPL_CUBE           "b"
#define VOPL_TRIANGLE       "c"
#define VOPL_LOSANGE        "d"
#define VOPL_CHRISMAS       "e"
#define VOPL_CROSS          "f"
#define VOPL_ADDITION       "g"
#define VOPL_MULTIPICATION  "h"
#define VOPL_STAR           "i"   
   


   

typedef struct AXISDATA {
   double min;
   double max;
   double div;
   int 	 scaling;
   int 	 annotate;
   int	 nticks;
   int	 ntspacing;
   int	 minorticks;
   int	 scaleset;
   char	 *format;
   char	 *title;

} axisdata;


typedef struct VOPL {
   double s1;
   double sn;
   double markerscale;

   int	fit;
   int	degree;
   int	splinetype;
   int	gridX;
   int  gridY;
   int  box;
   int	startind, arrayind;
   int	precision;
   int	markerspacing;
   int	forceticks;
		
   char	*marker;
   char	*graphtitle;

   axisdata	axes[AXES];

} vopldev;


typedef struct voplWindow_ {

   int     id;       /* id for a window */
   vopldev *plotdev; /* per window many sub-plot/plotdev */

   int nx;  /* number of subplots/vopldev in a window */
   int ny;  /* number of subplots/vopldev in a window */
   
} voplWindow;


typedef struct voplDevice_ {
   
   voplWindow	*window;  /* vopl can have many windows */
   int		nwins;
   int		curwin;

} voplDevice; /* this mirrors the vogle "Device" structure */


/* the static data -- initialized in vopl.c -- */
extern voplDevice  vopl_device ;                   /* mirror the vogle vdevice */
/* the static data -- initialized in vopl.c -- */

/* a pointer set to the current selected plotdev */
extern vopldev *plotdev;                            /* for backward compatibility with v-1.0 */
/* a pointer set to the current selected plotdev */



/* vopl version */
char *vopl_get_version(void);

/* vopl windows functions */
int vopl_winopen(const char *device, const char *title);
int vopl_wininit(int win);
int vopl_winset(int win);
int vopl_winclose(int win);
int vopl_winraise(int win);
int vopl_windel(int win);
int vopl_vgetwin(void);
int vopl_vwinidvalid(int win);

/* vopl subplot functions */
int  nxsubp();
int  nysubp();

/* new functions */
void  subp (int nx, int ny);
void  panel(int nx, int ny);
void  clearpanel(int color);

void  plotmarker(double x, double y, char* symbol);


#define WhatX(x) (plotdev->axes[XIND].scaling == LINEAR ? (x) : (double)log10((double)(x)))
#define WhatY(y) (plotdev->axes[YIND].scaling == LINEAR ? (y) : (double)log10((double)(y)))
#define WhatZ(z) (plotdev->axes[ZIND].scaling == LINEAR ? (z) : (double)log10((double)(z)))

/* utils */

double  *newm1(int n);
double **newm2(int m, int n);

char *savestr(char *old, char *string);


/* declarations */
void fit(int type);
void degree  (int ord);
void skip(int n);
void endslopes(double a, double b);
void gridspacing(int spacingX, int spacingY);
void scaling(int type, char axis);
void tickmarks(int num, char axis);
void tickspacing(int num, char axis);
void minorticks(int num, char axis);
void annotate(char *format, char axis);
void arrayindex(int i);
void marker(char *string);
void markerspacing(int spacing);
void markerscale(double s);
void graphtitle(char *s);
void range(double min, double max, char axis);
void withbox(int yes);

void avefit(double *x, double *y, int n);
void llsfit(double *x, double *y, int n);
int orthofit(double *xx, double *yy, int degree, int npnts);
int ortpol(int npnts, double *x, double *y, double *pjm1, double *pj, int degree);
void pefit(double *x, double *y, int n);
void sgrfit(double *x, double *y, int n);
void cubicsp(double *x, double *y, int n);
void spline(double *x, double *y, int n);
   
/* 2-D */
void adjustscale(double *x, int n, char axis);
void logscale (double xmin, double xmax, int n, double *xminp, double *xmaxp, double *dist);
void linscale2(double xmin, double xmax, int n, double *xminp, double *xmaxp, double *dist);
void linscale1(double xmin, double ymax, int n, double *xminp, double *xmaxp, double *dist);

void setwindow(double min, double max, char axis);
void unsetwindow(char axis);

void axistitle  (char *title , char ax);
void drawaxes   (void);
void drawtitle  (void);

/* behaviour depends in fit(n) etc */
void plot2   (double *x, double *y, int n);

/* contour on a unstructured mesh */
void* vopl_ucdmesh_build(double **nodes, int nb_nodes, int **mesh, int nb_zones);
void  vopl_ucdmesh_setdata(void *ucdmesh, double *data);
void  vopl_ucdmesh_free(void *ucdmesh);
/* contour on a unstructured mesh */
void contour(double *values, int nb_values, double **nodes, double *data, int nb_nodes, int **mesh, int nb_triangles);
void contour_with_vopl_ucdmesh(double *values, int nb_values, double *data, void *ucdmesh);
/* unstructured mesh */
void contour_mesh(double **nodes, double *data, int nb_nodes, int **mesh, int nb_zones);
/* unstructured mesh border */
void contour_border(double **nodes, double *data, int nb_nodes, int **mesh, int nb_zones);
				 
#ifdef __cplusplus
}
#endif

#endif
