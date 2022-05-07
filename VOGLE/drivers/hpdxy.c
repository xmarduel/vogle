#define VOGLE 1

/*
 *	DXY & HPGL Driver for vogle/vogl.
 */
#ifdef VOGLE
#include "vogle.h"
#else
#include "vogl.h"
#endif

#include <string.h>


static unsigned long localhpid = 0L;

typedef struct {
	unsigned long	localid;	/* Must be first in structure */
	int	id;
	int	plotlstx, plotlsty;	/* position of last draw */
	char	**plotcmds;
	FILE	*fp;
} Hpdxydev;

static	Hpdxydev	*hp;

#define	P_RESET		0
#define	P_MOVE		1
#define	P_DRAW		2
#define	P_TXTSIZE	3
#define	P_BEGTXT	4
#define	P_ENDTXT	5
#define	P_PEN		6

/*
 * basic commands for hpgl
 */
/* 
 * Changed to the delimiters to commas and removed spaces (for older plotter).
 * (mike@penguin.gatech.edu) 
 */
static char	*hpgl[] = {
	"DF;\n",
	"PU%d,%d;\n",
	"PD%d,%d;\n",
	"SI%.4f,%.4f;\n",
	"LB",
	"\003\n",
	"SP%d;\n"
};

/*
 * basic commands for dxy
 */
static char	*dxy[] = {
	"",
	"M %d,%d\n",
	"D %d,%d\n",
	"S %d\n",
	"P",
	"\n",
	"J %d\n"
};

/*
 * Functions
 */
static void 	*HPGL_winopen(const char *dev, const char *title, int id, int minx, int maxx, int miny, int maxy);
static void 	*HPGL_A4_winopen(const char *dev, const char *title, int id);
static void 	*HPGL_A3_winopen(const char *dev, const char *title, int id);
static void 	*HPGL_A2_winopen(const char *dev, const char *title, int id);
static void	*HPGL_A1_winopen(const char *dev, const char *title, int id);
static void 	*DXY_winopen(const char *dev, const char *title, int id);
static int	PLOT_draw(int x, int y);
static int	PLOT_pnt(int x, int y);
static int	PLOT_winset(void *p);
static int	PLOT_winclose(void *p);
static int	PLOT_winraise(void *p);
static int	PLOT_windel(void *p);
static int	PLOT_color(int i);
static int	HPGL_font(const char *font);
static int	DXY_font(const char *font);
static int	PLOT_char(char c);
static int	PLOT_string(const char *s);
static int	PLOT_fill(int n, int x[], int y[]);
static void	PLOT_sync(void);
static unsigned long	PLOT_getevent(Vevent *vev, int block);
static int	PLOT_backb(void *p, int old, int bw, int bh);
static int	PLOT_frontb(void);
static int	PLOT_swapb(void);
static int	PLOT_checkkey(void);
static int	PLOT_getkey(void);
static int	PLOT_locator(int *x, int *y);
static int	PLOT_clear(void);
static int	PLOT_mapcolor(int i, int r, int g, int b);
static int	PLOT_setlw(int n);
#ifndef VOGLE
static int	PLOT_setls(int n);
#endif
void		_HPGL_devcpy(char *name);
void		_DXY_devcpy(char *name);

/*
 * HPGL_winopen()
 *
 * Performs the common parts of HPGL initialization.
 */
static void *
HPGL_winopen(const char *dev, const char *title, int id, int minx, int maxx, int miny, int maxy)
{
	hp = (Hpdxydev *)vallocate(sizeof(Hpdxydev));
	memset(hp, 0, sizeof(Hpdxydev));
	hp->id = id;
	hp->localid = localhpid++;

	vdevice.dev.depth = 4;

	hp->fp = _voutfile();

	/*
	 * The next line is for serial lines if you need to set modes
	 */
	fprintf(hp->fp, "\033.(;\033.I81;;17;\033.N;19:IN;");

	/*
	 * Cause scaling to be 0 to maxX maxY.
	 */
	fprintf(hp->fp, "IP%d,%d,%d,%d;", minx, miny, maxx, maxy);
	fprintf(hp->fp, "SC0,%d,0,%d;", vdevice.dev.sizeX, vdevice.dev.sizeY);

	hp->plotcmds = hpgl;
	hp->plotlstx = -1111111;
	hp->plotlsty = -1111111;

	return((void *)hp);
}

/*
 * HPGL_winopen
 *
 *	set up hp plotter. Returns 1 on success.
 */

void *
HPGL_A4_winopen(const char *dev, const char *title, int id)
{
	/*
	 * A4 paper
	 */
	vdevice.dev.sizeX = vdevice.dev.sizeY = 7320;

	vdevice.dev.sizeSx = 10200;
	vdevice.dev.sizeSy = 7320;

	/* 
	 * Changed to 7000 (from 7721) as noted by Michael J. Gourlay 
	 * (mike@penguin.gatech.edu) 
	 */
	return(HPGL_winopen(dev, title, id, -7000, 7000, -7000, 7000));
}

void *
HPGL_A3_winopen(const char *dev, const char *title, int id)
{
	/*
	 * A3 paper
	 */
	vdevice.dev.sizeX = vdevice.dev.sizeY = 10560;

	vdevice.dev.sizeSx = 14720; 
	vdevice.dev.sizeSy = 10560; 

	return(HPGL_winopen(dev, title, id, -10000, 10000, -10000, 10000));
}

void *
HPGL_A2_winopen(const char *dev, const char *title, int id)
{
	/*
	 * A2 paper
	 */
	vdevice.dev.sizeX = vdevice.dev.sizeY = 13440;

	vdevice.dev.sizeSx = 18734;
	vdevice.dev.sizeSy = 13440;

	return(HPGL_winopen(dev, title, id, -13000, 13000, -13000, 13000));
}

void *
HPGL_A1_winopen(const char *dev, const char *title, int id)
{
	/*
	 * A1 paper
	 */
	vdevice.dev.sizeX = vdevice.dev.sizeY = 21360;

	vdevice.dev.sizeSx = 29774;
	vdevice.dev.sizeSy = 21360;

	return(HPGL_winopen(dev, title, id, -21000, 21000, -21000, 21000));
}


/*
 * DXY_winopen
 *
 *	set up dxy plotter. Returns 1 on success.
 */
void *
DXY_winopen(const char *dev, const char *title, int id)
{
	hp = (Hpdxydev *)vallocate(sizeof(Hpdxydev));
	memset(hp, 0, sizeof(Hpdxydev));
	hp->id = id;
	hp->localid = localhpid++;

	vdevice.dev.depth = 4;

	hp->fp = _voutfile();

	vdevice.dev.sizeX = vdevice.dev.sizeY = 1920; 

	vdevice.dev.sizeSx = 2668; 
	vdevice.dev.sizeSy = 1920; 

	hp->plotcmds = dxy;
	hp->plotlstx = -1;
	hp->plotlsty = -1;

	fprintf(hp->fp, hp->plotcmds[P_RESET]);

	return((void *)hp);
}

/*
 * PLOT_draw
 *
 *	print the commands to draw a line from the current graphics position
 * to (x, y).
 */
static int
PLOT_draw(int x, int y)
{
	if (hp->plotlstx != vdevice.cpVx || hp->plotlsty != vdevice.cpVy)
		fprintf(hp->fp, hp->plotcmds[P_MOVE], vdevice.cpVx, vdevice.cpVy);

	fprintf(hp->fp, hp->plotcmds[P_DRAW], x, y);
	hp->plotlstx = x;
	hp->plotlsty = y;

	return(1);
}

static int
PLOT_pnt(int x, int y)
{
	fprintf(hp->fp, hp->plotcmds[P_MOVE], x, y);
	fprintf(hp->fp, hp->plotcmds[P_DRAW], x, y);
	hp->plotlstx = x;
	hp->plotlsty = y;

	return(1);
}

static int
PLOT_winset(void *p)
{
        hp = (Hpdxydev *)p;
	return(hp->id);
}
 
/*
 * PLOT_winclose
 *      Close a window
 */
static int
PLOT_winclose(void *p)
{
	/* Nothing */

	return(1);
}

/*
 * PLOT_winraise
 *      Raise a window
 */
static int
PLOT_winraise(void *p)
{
	/* Nothing */

	return(1);
}
 

/*
 * PLOT_windel
 *
 *	exit from vogle printing the command to put away the pen and flush
 * the buffer.
 */
static int
PLOT_windel(void *p)
{
	Hpdxydev *hp = (Hpdxydev *)p;

	fprintf(hp->fp, hp->plotcmds[P_PEN], 0);
	fprintf(hp->fp, "\033.)");
	fflush(hp->fp);

	if (hp->fp != stdout)
		fclose(hp->fp);

	return(1);
}

/*
 * PLOT_color
 *
 *	change the current pen number.
 */
static int
PLOT_color(int i)
{ 
	fprintf(hp->fp, hp->plotcmds[P_PEN], i);

	return(1);
}

/*
 * HPGL_font
 *
 *	load in large or small
 */
static int
HPGL_font(const char *font)
{
	if (strcmp(font, "small") == 0) {
		vdevice.hwidth = 97.01;	/* Size in plotter resolution units */
		vdevice.hheight = vdevice.hwidth * 2.0;
		fprintf(hp->fp, hp->plotcmds[P_TXTSIZE], 0.16, 0.32);
	} else if (strcmp(font, "large") == 0) {
		vdevice.hwidth = 145.5;
		vdevice.hheight = vdevice.hwidth * 2.0;
		fprintf(hp->fp, hp->plotcmds[P_TXTSIZE], 0.24, 0.48);
	} else 
		return(0);

	return(1);
}

/*
 * DXY_font
 *
 *	load in large or small.
 */
static int
DXY_font(const char *font)
{
	if (strcmp(font, "small") == 0) {
		vdevice.hwidth = 24.25;
		vdevice.hheight = vdevice.hwidth * 2.0;
		fprintf(hp->fp, hp->plotcmds[P_TXTSIZE], 3);
	} else if (strcmp(font, "large") == 0) {
		vdevice.hwidth = 36.375;
		vdevice.hheight = vdevice.hwidth * 2.0;
		fprintf(hp->fp, hp->plotcmds[P_TXTSIZE], 5);
	} else 
		return(0);

	return(1);
}

/*
 * PLOT_char
 *
 *	draw a character.
 */
static int
PLOT_char(char c)
{
	if (hp->plotlstx != vdevice.cpVx || hp->plotlsty != vdevice.cpVy)
		fprintf(hp->fp, hp->plotcmds[P_MOVE], vdevice.cpVx, vdevice.cpVy);

	fprintf(hp->fp, hp->plotcmds[P_BEGTXT]);

	fprintf(hp->fp, "%c", c);

	fprintf(hp->fp, hp->plotcmds[P_ENDTXT]);

	hp->plotlstx = hp->plotlsty = -1111111;

	return(1);
}

/*
 * PLOT_string
 *
 *	output a string.
 */
static int
PLOT_string(const char *s)
{
	if (hp->plotlstx != vdevice.cpVx || hp->plotlsty != vdevice.cpVy)
		fprintf(hp->fp, hp->plotcmds[P_MOVE], vdevice.cpVx, vdevice.cpVy);

	fprintf(hp->fp, hp->plotcmds[P_BEGTXT]);

	fputs(s, hp->fp);

	fprintf(hp->fp, hp->plotcmds[P_ENDTXT]);

	hp->plotlstx = hp->plotlsty = -1111111;

	return(1);
}

/*
 * PLOT_fill
 *
 *      "fill" a polygon
 */
static int
PLOT_fill(int n, int x[], int y[])
{
	int     i;

	if (hp->plotlstx != x[0] || hp->plotlsty != y[0])
		fprintf(hp->fp, hp->plotcmds[P_MOVE], x[0], y[0]);

	for (i = 1; i < n; i++)
		fprintf(hp->fp, hp->plotcmds[P_DRAW], x[i], y[i]);

	fprintf(hp->fp, hp->plotcmds[P_DRAW], x[0], y[0]);

	hp->plotlstx = vdevice.cpVx = x[n - 1];
	hp->plotlsty = vdevice.cpVy = y[n - 1];

	return(1);
}


void
PLOT_sync(void)
{
	fflush(hp->fp);
}

/*
 * Do nothing functions.
 */
unsigned long
PLOT_getevent(Vevent *vev, int block)
{
	vev->type = -1;
	vev->data = 0;
	vev->w = 0;
	return(hp->localid);
}

static int
PLOT_backb(void *p, int old, int bw, int bh)
{
	return(-1);
}

static int
PLOT_frontb(void)
{
	return(-1);
}

static int
PLOT_swapb(void)
{
	return(-1);
}

static int
PLOT_checkkey(void)
{
	return(-1);
}

static int
PLOT_getkey(void)
{
	return(-1);

}

static int
PLOT_locator(int *x, int *y)
{
	return(-1);
}

static int
PLOT_clear(void)
{
	return(-1);
}

static int
PLOT_mapcolor(int i, int r, int g, int b)
{
	return(-1);
}

static int
PLOT_setlw(int n)
{
	return(-1);
}

#ifndef VOGLE
static int
PLOT_setls(int n)
{
	return(-1);
}
#endif


static DevEntry hpgldev = {
	"hpgl",
	"large",
	"small",
	4, 0, 0, 0, 0,
	PLOT_backb,
	PLOT_char,
	PLOT_checkkey,
	PLOT_clear,
	PLOT_color,
	PLOT_draw,
	PLOT_fill,
	HPGL_font,
	PLOT_frontb,
	PLOT_getkey,
	PLOT_getevent,
	HPGL_A2_winopen,
	PLOT_winset,
	PLOT_winclose,
	PLOT_winraise,
	PLOT_windel,
	PLOT_locator,
	PLOT_mapcolor,
	PLOT_pnt,
#ifndef VOGLE
	PLOT_setls,
#endif
	PLOT_setlw,
	PLOT_string,
	PLOT_swapb,
	PLOT_sync
};

/*
 * _HPGL_devcpy
 *
 *	copy the HPGL device into vdevice.dev.
 */
void
_HPGL_devcpy(char *name)
{
	vdevice.dev = hpgldev;
	vdevice.dev.devname = name;

	if (strcmp(name, "hpgla3") == 0)
		vdevice.dev.Vwinopen = HPGL_A3_winopen;
	else if (strcmp(name, "hpgla4") == 0)
		vdevice.dev.Vwinopen = HPGL_A4_winopen;
	else if (strcmp(name, "hpgla1") == 0)
		vdevice.dev.Vwinopen = HPGL_A1_winopen;
}

static DevEntry dxydev = {
	"dxy",
	"large",
	"small",
	4, 0, 0, 0, 0,
	PLOT_backb,
	PLOT_char,
	PLOT_checkkey,
	PLOT_clear,
	PLOT_color,
	PLOT_draw,
	PLOT_fill,
	DXY_font,
	PLOT_frontb,
	PLOT_getkey,
	PLOT_getevent,
	DXY_winopen,
	PLOT_winset,
	PLOT_winclose,
	PLOT_winraise,
	PLOT_windel,
	PLOT_locator,
	PLOT_mapcolor,
	PLOT_pnt,
#ifndef VOGLE
	PLOT_setls,
#endif
	PLOT_setlw,
	PLOT_string,
	PLOT_swapb,
	PLOT_sync,
};

/*
 * _DXY_devcpy
 *
 *	copy the DXY device into vdevice.dev.
 */
void
_DXY_devcpy(char *name)
{
	vdevice.dev = dxydev;
}
