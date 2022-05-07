
#include <stdio.h>

#ifdef SYS5
#include <string.h>
#define rindex	strrchr
#else
#include <strings.h>
#endif


#define MAXPNTS	5000

#ifdef DEFDEV
#define DEFAULTDEV DEFDEV
#else
#define DEFAULTDEV "X11"
#endif

#define	VERSION	"678-983-132773-1.1"

/*
 * graph types
 */
#define	X		1
#define	Y		2
#define	Z		4

/*
 * single graph structure
 */
typedef struct gr {
	int		type;
	char		*legend;
	int		npnts;
	float		*x, *y, *z;
	struct gr	*nxt;
} graph;

extern graph	*gp;

extern char	device[], fontname[];

extern char	*xlabel, *ylabel, *zlabel;

extern int	ngraphs, do_legend, do_dash, do_markers, uxscale, uyscale, uzscale;

extern FILE	*infile;

extern	float	wholescale;

extern	int	maxpnts;
