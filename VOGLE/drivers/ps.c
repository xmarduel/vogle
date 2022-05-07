
#define VOGLE 1
/*
 * The next bit is site and UNIX specific...
 */

/*
 * Some more bug fixes from  ralf@physik3.gwdg.de (Ralf Fassel)
 * regarding %%Pages and devname .... 28/03/94
 */

#ifdef VOGLE
#include "vogle.h"
#else
#include "vogl.h"
#endif

#include <stdlib.h>
#include <string.h>
 
static unsigned long	localpsid = 0L;

typedef struct {
	float	r, g, b;
} Coltab;



typedef struct {
	unsigned long	localid;	/* Must be first in structure */
	int	id;
	Coltab	*coltab;
	int	ps_first_time;
	int	drawn;
	int 	curcol;	
	int 	pslstx;
	int 	pslsty;
	int 	colour;
	int	page;
	FILE	*fp;
} Psdev;

static Psdev	*ps;

/*
 * Functions
 */
static int	PS_color(int col);
static int	PS_mapcolor(int indx, int r, int g, int b);
static int	PS_common_init(void);
static void 	*PS_winopen(const char *dev, const char *title, int id);
static void 	*CPS_winopen(const char *dev, const char *title, int id);
static void 	*PPS_winopen(const char *dev, const char *title, int id);
static void 	*PCPS_winopen(const char *dev, const char *title, int id);
static int	PS_winset(void *p);
static int	PS_winclose(void *p);
static int	PS_winraise(void *p);
static int	PS_windel(void *p);
static int	PS_draw(int x, int y);
static int	PS_pnt(int x, int y);
static int	PS_font(const char *font);
static int	PS_clear(void);
static int	PS_char(char c);
static int	PS_string(const char *s);
static int	PS_fill(int n, int x[], int y[]);
static int	PS_setlw(int w);
#ifndef VOGLE
static int	PS_setls(int lss);
#endif
static void	PS_sync(void);
static unsigned long	PS_getevent(Vevent *vev, int block);
static int	PS_backb(void *p, int old, int bw, int bh);
static int	PS_frontb(void);
static int	PS_swapb(void);
static int	PS_checkkey(void);
static int	PS_getkey(void);
static int	PS_locator(int *x, int *y);
void		_PS_devcpy(char *name);

/*
 * gray scale map for our standard colours
 */
static float	graymap[8] = {
			0.0,
			0.30,
			0.59,
			0.89,
			0.11,
			0.41,
			0.70,
			0.99
};



/*
 * PS_color
 *
 *	change the grey value of the ink
 */
static int
PS_color(int col)
{
	ps->curcol = col;
	if (ps->colour) {
		ps->curcol %= 256;
		fprintf(ps->fp, "%3.2f %3.2f %3.2f c\n", ps->coltab[ps->curcol].r, ps->coltab[ps->curcol].g, ps->coltab[ps->curcol].b);
		return(1);
	}

	if (col > 7)
		return(1);


#ifdef GREY_LINES
	fprintf(fp, "%3.2f g\n", graymap[curcol]);
#endif

	return(1);
}

/*
 * PS_mapcolor
 *
 *	Set our values in our pseudo colour map.
 */
static int
PS_mapcolor(int indx, int r, int g, int b)
{
	if (ps->colour && indx < 256 && indx >= 0) {
		ps->coltab[indx].r = r / 255.0;
		ps->coltab[indx].g = g / 255.0;
		ps->coltab[indx].b = b / 255.0;
	}

	return(1);
}

/*
 * PS_common_init
 *
 *	 Initialization that is common to both layouts
 */
static int
PS_common_init(void)
{

	vdevice.dev.depth = ps->colour ? 8 : 1;

	/*	Set other line drawing parameters	*/

	fprintf(ps->fp, "2 setlinewidth\n1 setlinejoin\n1 setlinecap\n");

	/*	Speed up symbol font handling	*/

	fprintf(ps->fp, "/sf /Courier findfont def\n");

	/*	Move	*/

	fprintf(ps->fp, "/m /moveto load def\n");

	/*	Draw	*/

	fprintf(ps->fp, "/d { lineto currentpoint stroke moveto } def\n");

	/*	Polygon Draw	*/

	fprintf(ps->fp, "/p /lineto load def\n");

	/*	Set character height	*/

	fprintf(ps->fp, "/h { sf exch scalefont setfont } def\n");

	/*	Show character string	*/

	fprintf(ps->fp, "/s /show load def\n");

	/*	Set gray scale	*/

	fprintf(ps->fp, "/g /setgray load def\n");


	/*	Set a default font height	*/
	
	fprintf(ps->fp, "45 h\n");

	ps->pslstx = ps->pslsty = -1;

	return(1);
}


/*
 * PS_winopen
 *
 *	set up the postcript environment. Returns 1 on success.
 */
static void *
PS_winopen(const char *dev, const char *title, int id)
{
	ps = (Psdev *)vallocate(sizeof(Psdev));
	memset(ps, 0, sizeof(Psdev));
	ps->id = id;

	ps->localid = localpsid++;

	ps->fp = _voutfile(); 

	ps->page = 1;
	fputs("%!PS-Adobe-2.0 EPSF-1.2\n", ps->fp);
	fputs("%%BoundingBox: 74 96 528 728\n", ps->fp);
	fprintf(ps->fp, "%%%%Page: %d %d\n", ps->page, ps->page);
	fputs("%%EndComments\n", ps->fp);
	fprintf(ps->fp, "72 300 div dup scale\n90 rotate\n400 -2200 translate\n");

	vdevice.dev.sizeSy = 1890; 
	vdevice.dev.sizeSx = 2634; 
	vdevice.dev.sizeX = vdevice.dev.sizeY = 1890; 

	PS_common_init();

	return(ps);
}

static void *
CPS_winopen(const char *dev, const char *title, int id)
{
	ps = (Psdev *)PS_winopen(dev, title, id);

	ps->colour = 1;
	fprintf(ps->fp, "/c /setrgbcolor load def\n");
	ps->coltab = (Coltab *)vallocate(256 * sizeof(Coltab));
	PS_mapcolor(0, 0, 0, 0);
	PS_mapcolor(1, 255, 0, 0);
	PS_mapcolor(2, 0, 255, 0);
	PS_mapcolor(3, 255, 255, 0);
	PS_mapcolor(4, 0, 0, 255);
	PS_mapcolor(5, 255, 0, 255);
	PS_mapcolor(6, 0, 255, 255);
	PS_mapcolor(7, 255, 255, 255);

	return((void *)ps);
}

/*
 * PPS_init
 *
 *	set up the postscript (Portrait) environment. Returns 1 on success.
 */
static void *
PPS_winopen(const char *dev, const char *title, int id)
{
	ps = (Psdev *)vallocate(sizeof(Psdev));
	memset(ps, 0, sizeof(Psdev));
	ps->id = id;
	ps->localid = localpsid++;
	ps->fp = _voutfile();

	ps->page = 1;
	fputs("%!PS-Adobe-2.0 EPSF-1.2\n", ps->fp);
	fputs("%%BoundingBox: 72 96 526 728\n", ps->fp);
	fprintf(ps->fp, "%%%%Page: %d %d\n", ps->page, ps->page);
	fputs("%%EndComments\n", ps->fp);

	fprintf(ps->fp, "72 300 div dup scale\n300 400 translate\n");

	vdevice.dev.sizeSy = 2634; 
	vdevice.dev.sizeSx = 1890; 
	vdevice.dev.sizeX = vdevice.dev.sizeY = 1890; 

	PS_common_init();

	return (ps);
}

void *
PCPS_winopen(const char *dev, const char *title, int id)
{
	ps = (Psdev *)PPS_winopen(dev, title, id);

	ps->colour = 1;
	fprintf(ps->fp, "/c /setrgbcolor load def\n");
	ps->coltab = (Coltab *)vallocate(256 * sizeof(Coltab));
	PS_mapcolor(0, 0, 0, 0);
	PS_mapcolor(1, 255, 0, 0);
	PS_mapcolor(2, 0, 255, 0);
	PS_mapcolor(3, 255, 255, 0);
	PS_mapcolor(4, 0, 0, 255);
	PS_mapcolor(5, 255, 0, 255);
	PS_mapcolor(6, 0, 255, 255);
	PS_mapcolor(7, 255, 255, 255);

	return((void *)ps);
}

static int
PS_winset(void *p)
{
	ps = (Psdev *)p;

	return(ps->id);
}

/* 
 * PS_winclose
 * 	Close a window
 */
static int
PS_winclose(void *p)
{
	/* Nothing */

	return(1);
}

/* 
 * PS_winraise
 * 	Raise a window
 */
static int
PS_winraise(void *p)
{
	/* Nothing */

	return(1);
}

/*
 * PS_windel
 *
 *	do a showpage and close the output file if neccessary.
 */
static int
PS_windel(void *p)
{
	Psdev	*ps = (Psdev *)p;

	fputs("showpage\n", ps->fp);
	fputs("%%Trailer\n", ps->fp);
	fflush(ps->fp);

	if (ps->fp != stdout)
		fclose(ps->fp);

	if (ps->colour)
		free(ps->coltab);

	free(ps);
	ps = NULL;

	return(1);
}

/*
 * PS_draw
 *
 *	draw to an x, y point.
 */
static int
PS_draw(int x, int y)
{
	if (ps->pslstx != vdevice.cpVx || ps->pslsty != vdevice.cpVy)
		fprintf(ps->fp, "%d %d m\n", vdevice.cpVx, vdevice.cpVy);

	fprintf(ps->fp, "%d %d d\n", x, y);
	ps->pslstx = x;
	ps->pslsty = y;
	ps->drawn = 1;

	return(1);
}

static int
PS_pnt(int x, int y)
{
	fprintf(ps->fp, "%d %d m\n", x, y);
	fprintf(ps->fp, "%d %d d\n", x, y);
	ps->pslstx = x;
	ps->pslsty = y;
	ps->drawn = 1;

	return(1);
}


/*
 * PS_font
 *
 * load in small or large - could be improved.
 */
static int
PS_font(const char *font)
{
	if (strcmp(font, "small") == 0) {
		vdevice.hwidth = 22.0;
		vdevice.hheight = vdevice.hwidth * 1.833;
		fprintf(ps->fp, "%d h\n", (int)vdevice.hheight);
	} else if (strcmp(font, "large") == 0) {
		vdevice.hwidth = 35.0;
		vdevice.hheight = vdevice.hwidth * 1.833;
		fprintf(ps->fp, "%d h\n", (int)vdevice.hheight);

	} else
		return(-1);

	return(1);
}

/*
 * PS_clear
 *
 *	flush the current page without resetting the graphics state of the
 * laser printer.
 */
static int
PS_clear(void)
{
	if (ps->drawn) {
		fprintf(ps->fp, "gsave showpage grestore\n");
		/* This is the end of the page, not of the document. */
		/*  ralf@physik3.gwdg.de (Ralf Fassel) */
		fputs("%%PageTrailer\n", ps->fp);
		ps->page++;
		fprintf(ps->fp, "%%%%Page: %d %d\n", ps->page, ps->page);
	}

	if (ps->colour) {
		fprintf(ps->fp, "gsave %3.2f %3.2f %3.2f c clippath fill grestore\n",
		ps->coltab[ps->curcol].r, ps->coltab[ps->curcol].g, ps->coltab[ps->curcol].b);
	}

	ps->drawn = 0;

	return(1);
}

	
/*
 * PS_char
 *
 *	output a character making sure that a '\' is sent first when
 * appropriate.
 */
static int
PS_char(char c)
{
	if (ps->pslstx != vdevice.cpVx || ps->pslsty != vdevice.cpVy)
		fprintf(ps->fp, "%d %d m\n", vdevice.cpVx, vdevice.cpVy);

	fprintf(ps->fp, "(");

	switch(c) {
	case '(':
		fprintf(ps->fp, "\\(");
		break;
	case ')':
		fprintf(ps->fp, "\\)");
		break;
	case '\\':
		fprintf(ps->fp, "\\");
		break;
	default:
		fprintf(ps->fp, "%c",c);
	}

	fprintf(ps->fp,") s \n");

	ps->drawn = 1;
	ps->pslstx = ps->pslsty = -1;

	return(1);
}

/*
 * PS_string
 *
 *	output a string one char at a time.
 */
static int
PS_string(const char *s)
{
	char	c;

	if (ps->pslstx != vdevice.cpVx || ps->pslsty != vdevice.cpVy)
		fprintf(ps->fp, "%d %d m\n", vdevice.cpVx, vdevice.cpVy);

	fprintf(ps->fp, "(");
	while ((c = *s++))
		switch(c) {
		case '(':
			fprintf(ps->fp, "\\(");
			break;
		case ')':
			fprintf(ps->fp, "\\)");
			break;
		case '\\':
			fprintf(ps->fp, "\\");
			break;
		default:
		fprintf(ps->fp, "%c",c);
		}

	fprintf(ps->fp,") s \n");
	ps->drawn = 1;
	ps->pslstx = ps->pslsty = -1;

	return(1);
}

/*
 * PS_fill
 *
 *      fill a polygon
 */
static int
PS_fill(int n, int x[], int y[])
{
	int     i;


	fprintf(ps->fp, "newpath \n");

	fprintf(ps->fp, "%d %d m\n", x[0], y[0]);

	for (i = 1; i < n; i++)
		fprintf(ps->fp, "%d %d p\n", x[i], y[i]);

	fprintf(ps->fp, "closepath\n");

	if (!ps->colour)
		fprintf(ps->fp, "closepath\n");

	fprintf(ps->fp, "%3.2f g\n", graymap[ps->curcol]);

	fprintf(ps->fp, "fill\n");

	if (!ps->colour)
		fprintf(ps->fp, "0 g\n");

	vdevice.cpVx = x[n - 1];
	vdevice.cpVy = y[n - 1];

	ps->pslstx = ps->pslsty = -1;		/* fill destroys current path */

	return(1);
}

#ifndef VOGLE
/*
 * Set the line width...
 */
static int
PS_setlw(int w)
{
	fprintf(fp, "%d setlinewidth\n", w * 2 + 1);

	return(1);
}

/*
 * Set the line style...
 */
static int
PS_setls(int lss)
{
	unsigned ls = lss;
	int	i, d, a, b, offset;

	if (ls == 0xffff) {
		fprintf(fp, "[] 0 setdash\n");
		return;
	}

	fputc('[', fp);

	for (i = 0; i < 16; i++)	/* Over 16 bits */
		if ((ls & (1 << i)))
			break;

	offset = i;

#define	ON	1
#define	OFF	0
		
	a = b = OFF;
	if (ls & (1 << 0))
		a = b = ON;

	d = 0;
	for (i = 0; i < 16; i++) {	/* Over 16 bits */
		if (ls & (1 << i))
			a = ON;
		else
			a = OFF;

		if (a != b) {
			b = a;
			fprintf(fp, "%d ", d * 2 + 1);
			d = 0;
		}

		d++;
	}

	fprintf(fp, "] %d setdash\n", offset);

	return(1);
}

#else
/*
 * Set the line width...
 */
static int
PS_setlw(int w)
{
	if (w == 0)
		w = 2;
	else if (w == 1)
		w = 4;

	fprintf(ps->fp, "%d setlinewidth\n", w);

	return(1);
}
#endif

static void
PS_sync(void)
{
	fflush(ps->fp);
}

/*
 * Do nothing functions.
 */
static unsigned long
PS_getevent(Vevent *vev, int block)
{
	vev->type = -1;
	vev->data = 0;
	vev->w = 0;
	return(ps->localid);
}

static int
PS_backb(void *p, int old, int bw, int bh)
{
	return(-1);
}

static int
PS_frontb(void)
{
	return(-1);
}

static int
PS_swapb(void)
{
	return(-1);
}

static int
PS_checkkey(void)
{
	return(-1);
}

static int
PS_getkey(void)
{
	return(-1);

}

static int
PS_locator(int *x, int *y)
{
	return(-1);
}

static DevEntry psdev = {
	"postscript",
	"large",
	"small",
	8, 
	0, 0, 0, 0,
	PS_backb,
	PS_char,
	PS_checkkey,
	PS_clear,
	PS_color,
	PS_draw,
	PS_fill,
	PS_font,
	PS_frontb,
	PS_getkey,
	PS_getevent,
	PS_winopen,
	PS_winset,
	PS_winclose,
	PS_winraise,
	PS_windel,
	PS_locator,
	PS_mapcolor,
	PS_pnt,
#ifndef VOGLE
	PS_setls,
#endif
	PS_setlw,
	PS_string,
	PS_swapb,
	PS_sync
};

/*
 * _PS_devcpy
 *
 *	copy the postscript device into vdevice.dev.
 * 	Set it so we can use colours.
 */
void
_PS_devcpy(char *name)
{
	vdevice.dev = psdev;
	vdevice.dev.devname = name;
	if (strcmp(name, "cps") == 0)
		vdevice.dev.Vwinopen = CPS_winopen;
	else if (strcmp(name, "pcps") == 0)
		vdevice.dev.Vwinopen = PCPS_winopen;
	else if (strcmp(name, "pps") == 0)
		vdevice.dev.Vwinopen = PPS_winopen;

}
