
#ifndef VOGLE_H_
#define VOGLE_H_

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static char *vogle_version = "vogle2.0b0";


/*
 * default location for font library
 */
#ifndef FONTLIB
#define FONTLIB	"/usr/local/lib/hershey/"
#endif

/*
 * The maximum number of devices/windows we can allow to be open.
 */
#define MAXWIN	64

/*
 * standard colour indices
 */
#define	BLACK    0
#define	RED      1
#define	GREEN    2
#define	YELLOW   3
#define	BLUE     4
#define	MAGENTA  5
#define	CYAN     6
#define	WHITE    7

/*
 * Hershey text justification
 */
#define V_XCENTERED  1
#define V_YCENTERED  2
#define V_LEFT       4	/* The default */
#define V_RIGHT      8
#define V_TOP        16
#define V_BOTTOM     32	/* The default */
/*
 * Text weights 
 */
#define NORMAL		0	/* The default */
#define BOLD		1

/*
 * Line thickness
 */
#define THIN		0
#define THICK		1

/*
 * when (if ever) we need the precision
 */
#ifndef VOGLE_DOUBLE
#define VOGLE_DOUBLE 1
#endif

#ifdef VOGLE_DOUBLE
#define	float	double 
#endif 

/*
 * How to convert degrees to radians
 */
#ifndef PI
#define	PI	3.14159265358979323844
#endif
#define D2R	(PI / 180.0)

/*
 * miscellaneous typedefs and type defines
 */
typedef float	Vector[4];
typedef float	Matrix[4][4];
typedef float	Tensor[4][4][4];

/*
 * when register variables get us into trouble
 */
#ifdef NOREGISTER
#define	register
#endif

/*
 * max number of vertices in a ploygon
 */
#define	MAXVERTS	256

/*
 * max number of characters in a font name
 */
#define	FONTNAMELEN	256

/*
 * object definitions
 */
#define MAXENTS		101		/* size of object table */
#define MAXTOKS		100		/* num. of tokens alloced at once in an object  */

/*
 * functions which can appear in objects
 */
#define	ARC            1
#define	BOXTEXT        2
#define	CALLOBJ        3
#define	CENTERTEXT     4
#define	CIRCLE         5
#define	CLEAR          6
#define	COLOR          7
#define	DRAW           8
#define	DRAWCHAR       9
#define	DRAWSTR        10
#define	FIXEDWIDTH     11
#define	VFONT          12
#define	HATCHANG       13
#define	HATCHPITCH     14
#define	LOADMATRIX     15
#define	MAPCOLOR       16
#define	MOVE           17
#define	MULTMATRIX     18
#define	POLY           19
#define	POLYFILL       20
#define	POLYHATCH      21
#define	POPATTRIBUTES  22
#define	POPMATRIX      23
#define	POPVIEWPORT    24
#define	PUSHATTRIBUTES 25
#define	PUSHMATRIX     26
#define	PUSHVIEWPORT   27
#define	RCURVE         28
#define	RPATCH         29
#define	SECTOR         30
#define	TEXTANG        31
#define	TEXTSIZE       32
#define	VIEWPORT       33
#define	BACKBUFFER     34
#define	FRONTBUFFER    35
#define	SWAPBUFFER     36
#define	BACKFACING     37
#define	TRANSLATE      38
#define	ROTATE         39
#define	SCALE          40
#define	VFLUSH         41
#define	POSTMULTMATRIX 42
#define	LINESTYLE      43
#define	SETDASH        44
#define	LINEWIDTH      45
#define	PUSHPOS        46
#define	POPPOS         47
#define	CALCVIEWPORT   48

/*
 * Vogle error codes (passed to verror).
 */
#define VERR_UNINIT     1
#define VERR_STACKUF    2
#define VERR_STACKOF    3
#define VERR_NUMSEGS    4
#define VERR_NUMPNTS    5
#define VERR_FILEIO     6
#define VERR_NOFONT     7
#define VERR_NOHFONT    8
#define VERR_NUMCRVS    9
#define VERR_CLIPERR    10
#define VERR_BADAXIS    11
#define VERR_BADASPECT  12
#define VERR_BADPLANE   13
#define VERR_BADFOV     14
#define VERR_BADDEV     15
#define VERR_MALLOC     16
#define VERR_NOOBJECT   17
#define VERR_MAXPNTS    18
#define VERR_BADCOORD   19
#define VERR_BADVECTOR  20
#define VERR_BADVP      21
#define VERR_BADWIN     22
#define VERR_INVALIDW   23
#define VERR_DRIVER     24
#define VERR_RANGE      25
#define	MAXERRS        25

/*
 * Basic event types
 */
#define VKEYPRESS       1
#define VKEYRELEASE     2
#define VBUTTONPRESS    3
#define VBUTTONRELEASE  4
#define VREDRAW         5
#define VRESIZE         6
#define VMOTION         7



/*
 * data types for object tokens
 */
typedef union tk {
	int		i;
	float		f;
} Token;

typedef struct tls {
	int		count;
	Token		*toks;
	struct tls	*next;
} TokList;

/*
 * attributes
 */
typedef struct {
	char		*style,
			*dashp,
			fill,
			hatch,
			backface,
			justify,
			bold,
			exvp,
			softtext,
			fixedwidth;
	int		color;
	float		fontheight;
	float		fontwidth;
	float		skew;
	float		hatchcos,
			hatchsin,
			hatchpitch;
	float		textcos,
			textsin;
	float		dash,
			adist;
	char		font[FONTNAMELEN];
} Attribute;

/*
 * viewport
 */
typedef struct vp {
	float	left;
	float	right;
	float	bottom;
	float	top;
} Viewport; 

/*
 * stacks
 */
typedef	struct	ms {	/* Matrix stack entries	*/
	Matrix		m;
	struct	ms	*back;
} Mstack;

typedef	struct	as {	/* Attribute stack entries */
	Attribute	a;
	struct	as	*back;
} Astack;

typedef	struct	vs {	/* Viewport stack entries */
	Viewport	v;
	struct	vs	*back;
} Vstack;

typedef	struct	ps {	/* Graphics pos stack entries */
	Vector cpW;
	Vector cpWtrans;	/* world coords transformed */
	struct	ps	*back;
} Pstack;

typedef struct {
	int	w;		/* The window id that got the event */
	int	type;
	int	data;
	int	x, y;
	/*.... keep these as integers, so that easily map to a Fortran
	array... */
} Vevent;

/*
 * vogle device structures
 */
typedef struct dev {
   char		*devname;			/* name of device */
   char		*large,				/* name of large font */
            *small;				/* name of small font */
   int		depth,				/* # bit planes on screen */
            sizeX, sizeY, 			/* size of square on screen */
	         sizeSx, sizeSy;		/* side in x, side in y (# pixels) */
   int		(*Vbackb)(void *, int, int, int);			/* Set drawing in back buffer */
   int		(*Vchar)(char);			/* Draw a hardware character */
   int		(*Vcheckkey)(void);		/* Clear the screen to current color */
   int		(*Vclear)(void);		/* Clear the screen to current color */
   int		(*Vcolor)(int);			/* Set current color */
   int		(*Vdraw)(int, int);		/* Draw a line */
   int		(*Vfill)(int, int [], int []);	/* Fill a polygon */
   int		(*Vfont)(const char *);		/* Set hardware font */
   int		(*Vfrontb)(void);		/* Set drawing in front buffer */
   int 		(*Vgetkey)(void);		/* Wait for and get the next key hit */
   unsigned long (*Vgetevent)(Vevent *vev, int block);	/* Get the next event */
   void *	(*Vwinopen)(const char *, const char *, int id); /* Initialise a window on the device */
   int		(*Vwinset)(void *devwin);	/* Use an already open window */
   int		(*Vwinclose)(void *devwin);	/* Close an already open window */
   int		(*Vwinraise)(void *devwin);	/* Open an already created window */
   int		(*Vwindel)(void *devwin);	/* Close and delete an already open window */
   int		(*Vlocator)(int *, int *);	/* Get mouse/cross hair position */
   int		(*Vmapcolor)(int, int, int, int);		/* Set color indicies */
   int		(*Vpnt)(int, int);		/* Set color indicies */
   int		(*Vsetlw)(int);			/* Set line thickness */
   int		(*Vstring)(const char *);	/* Draw a hardware string */
   int		(*Vswapb)(void);		/* Swap front and back buffers */
   void		(*Vsync)(void);			/* Syncronise the display */
} DevEntry;


/*
 * Some place holders for multiple windows/devices
 */
typedef struct {
   int		id;		/* Our id for a window */
   unsigned long localid;	/* Driver's local id for a window */
   void		*devwin;	/* Points to driver private data */
   char		havebackb;	/* Is this window double buffered */
   DevEntry	dev;		/* The functions within the driver */
} Vwindow;

typedef struct {
   float   	px, py;
   Matrix	pickmat;
   char	    	pickm, pick;
} Pickwin;


typedef struct vdev {
   char		initialised,
		clipoff,
		inobject,
		inpolygon,
		upset,			/* is up vector set */
		cpVvalid,		/* is the current device position valid */
		sync,			/* Do we syncronise the display */
		inbackbuffer,		/* are we in the backbuffer */
		clipplanes;		/* active clipping planes */
   void		(*pmove)(float, float, float),		/* Polygon moves */
		(*pdraw)(float, float, float);		/* Polygon draws */
   TokList	*tokens;		/* ptr to list of tokens for current object */
   Mstack	*transmat;		/* top of transformation stack */
   Astack	*attr;			/* top of attribute stack */
   Vstack	*viewport;		/* top of viewport stack */
   Pstack	*pos;			/* top of pos stack */
   float	hheight, hwidth;	/* hardware character height, width */
   Vector	upvector;		/* world up */
   int		maxVx, minVx,
		maxVy, minVy,
		cpVx, cpVy;
   DevEntry	dev;			/* The current active one */
   Vwindow	*wins;
   int		nwin;
   int		curwin;
   Pickwin	pickwin;
} Device;

extern Device	vdevice;		/* device structure */

#define	V_X	0			/* x axis in cpW */
#define	V_Y	1			/* y axis in cpW */
#define	V_Z	2			/* z axis in cpW */
#define	V_W	3			/* w axis in cpW */

/*
 * function definitions
 */

/*
 * allocate memory
 */
extern	void	*vallocate(unsigned size);

/*
 * arc routines
 */
extern void	arc(float x,float y,float radius, float startang, float endang);
extern void	circle(float x, float y, float radius);
extern void	arcprecision(int noseg);
extern void	circleprecision(int noseg);
extern void	sector(float x, float y,float radius,float startang,float endang);

/*
 * attr routines
 */
extern void	popattributes(void);
extern void	pushattributes(void);

/*
 * curve routines
 */
extern void	curve(float geom[4][3]);
extern void	rcurve(Matrix geom);
extern void	curven(int n, float geom[][3]);
extern void	drcurve(int n, Matrix r);
extern void	curvebasis(Matrix basis);
extern void	curveprecision(int nsegments);

/*
 * draw routines
 */
extern void	draw(float x, float y, float z);
extern void	draw2(float x, float y);
extern void	rdraw(float dx, float dy, float dz);
extern void	rdraw2(float dx, float dy);
extern void	sdraw2(float xs, float ys);
extern void	rsdraw2(float dxs, float dys);
extern void	setdash(float d);
extern void	dashline(Vector p0, Vector p1);
extern void	linestyle(const char *l);

/*
 * device routines
 */
extern void	clear(void);
extern void	color(int i);
extern int	getdepth(void);
extern int	getkey(void);
extern int	checkkey(void);
extern int	vgetevent(Vevent *vev, int noblock);
extern int	getplanes();
extern int	locator(float *wx, float *wy);
extern int	slocator(float *wx, float *wy);
extern void	mapcolor(int i, short r, short g, short b);

/*
 * Old ones...
 */
extern int	vinit(const char *device);
extern void	vexit(void);
/*
 * Should be replaced by...
 */
extern int	winopen(const char *device, const char *title);
extern int	winset(int win);
extern int	winclose(int win);
extern int	winraise(int win);
extern int	windel(int win);
extern int	vgetwin(void);
extern int	vwinidvalid(int winid);

extern void	voutput(const char *path);
extern void	voutputfp(FILE *vfp);

extern int	vndev(void);
extern char	*vgetdevname(int n, char *name);
extern char	*vgetdev(char *buf);
extern void	linewidth(int w);

extern int	_vgetvogledev(const char *dev);
extern void	_vdovogleinit(void);
extern int	_vgetfreewindowslot(void);


/*
 * mapping routines
 */
extern int	WtoVx(float p[]);
extern int	WtoVy(float p[]);
extern void	VtoWxy(float xs, float ys, float *xw, float *yw);
extern void	CalcW2Vcoeffs(void);

/*
 * general matrix and vector routines
 */
extern void	mult4x4(Matrix a, Matrix b,Matrix c);
extern void	copymatrix(Matrix a, Matrix b);
extern void	identmatrix(Matrix a);
extern void	copytranspose(Matrix a, Matrix b);

extern void	multvector(Vector v, Vector a, Matrix b);
extern void	copyvector(Vector a, Vector b);
extern void	premultvector(Vector v, Vector a, Matrix b);

/*
 * matrix stack routines
 */
extern void	getmatrix(Matrix m);
extern void	popmatrix(void);
extern void	loadmatrix(Matrix mat);
extern void	pushmatrix(void);
extern void	multmatrix(Matrix mat);
extern void	postmultmatrix(Matrix mat);
extern Matrix	*getmstackaddress();

/*
 * move routines
 */
extern void	move(float x, float y, float z);
extern void	move2(float x, float y);
extern void	rmove(float dx, float dy, float dz);
extern void	rmove2(float dx, float dy);
extern void	smove2(float xs, float ys);
extern void	rsmove2(float dxs, float dys);

/*
 * object routines
 */
extern int	isobj(int n);
extern int	genobj(void);
extern void	delobj(int n);
extern void	makeobj(int n);
extern void	appendobj(int n);
extern void	_setcurrenttoklist(TokList *tl);
extern void	loadobj(int n, char *file);
extern void	saveobj(int n, char *file);
extern void	callobj(int n);
extern void	closeobj(void);
extern int	getopenobj(void);
extern Token	*newtokens(int num);

/*
 * patch routines.
 */
extern void	patch(Matrix geomx, Matrix geomy, Matrix geomz);
extern void	rpatch(Matrix geomx, Matrix geomy, Matrix geomz, Matrix geomw);
extern void	drpatch(Tensor R, int ntcurves, int nucurves, int ntsegs, int nusegs, int ntiter, int nuiter);
extern void	patchbasis(Matrix tb, Matrix ub);
extern void	patchcurves(int nt, int nu);
extern void	patchprecision(int tseg, int useg);
extern void	transformtensor(Tensor S, Matrix m);

/*
 * point routines
 */
extern void	point(float x, float y, float z);
extern void	point2(float x, float y);
extern void	spoint2(float xs, float ys);

/*
 * polygon routines.
 */
extern void	poly(int n, float dp[][3]);
extern void	poly2(int n, float dp[][2]);
extern void hatchang(float a);
extern void	makepoly(void);
extern void	polyfill(int onoff);
extern void	closepoly(void);
extern void	polyhatch(int onoff);
extern void hatchpitch(float a);
extern void	backface(int onoff);
extern void	backfacedir(int cdir);

extern void	polyobj(int n, Token dp[]);
extern void	pmove(float x, float y, float z);
extern void	pdraw(float x, float y, float z);

/*
 * rectangle routine
 */
extern void	rect(float x1, float y1, float x2, float y2);
extern void	srect(float x1, float y1, float x2, float y2);

/*
 * tensor routines
 */
extern void multtensor(Tensor c, Matrix a, Tensor b);
extern void copytensor(Tensor b, Tensor a);
extern void premulttensor(Tensor c, Matrix a, Tensor b);
extern void copytensortrans(Tensor b, Tensor a);

/*
 * text routines
 */
extern int	hershfont(const char *fontname);
extern void	font(const char *name);
extern void	boxfit(float l, float h, int nchars);
extern void	boxtext(float x, float y, float l, float h, char *s);
extern void	drawstr(const char *string);
extern int	getstring(int bcol, char *s);
extern void	textang(float ang);
extern int	numchars(void);
extern void	drawchar(int c);
extern void	textsize(float width, float height);
extern float	strlength(const char *s);
extern float	sstrlength();
extern void	centertext(int onoff);
extern void	xcentertext(void);
extern void	ycentertext(void);
extern void	fixedwidth(int onoff);
extern void	topjustify(void);
extern void	bottomjustify(void);
extern void	leftjustify(void);
extern void	rightjustify(void);
extern void	textjustify(unsigned val);
extern void	textslant(float val);
extern void	textweight(int val);
extern void	getcharsize(char c, float *width, float *height);
extern void	getfontsize(float *cw, float *ch);
extern float	getfontwidth(void);
extern float	getfontheight(void);

/*
 * transformation routines
 */
extern void	scale(float x, float y, float z);
extern void	translate(float x, float y, float z);
extern void	rotate(float r, char axis);

/*
 * window definition routines
 */
extern void	ortho(float left, float right, float bottom, float top, float hither, float yon);
extern void	ortho2(float left, float right, float bottom, float top);
extern void	lookat(float vx, float vy, float vz, float px, float py, float pz, float twist);
extern void	window(float left, float right, float bottom, float top, float hither, float yon);
extern void	polarview(float dist, float azim, float inc, float twist);
extern void	perspective(float fov, float aspect, float hither, float yon);
extern void	up(float x, float y, float z);

/*
 * routines for manipulating the viewport
 */
extern void	viewport(float xlow, float xhigh, float ylow, float yhigh);
extern void	getviewport(float *left, float *right,float *bottom,float *top);
extern void	popviewport(void);
extern void	pushviewport(void);
extern void	calcviewport(void);

/*
 * routines for retrieving the graphics position
 */
extern void	getgp(float *x, float *y, float *z);
extern void	getgpt(float *x, float *y, float *z, float *w);
extern void	getgp2(float *x, float *y);
extern void	sgetgp2(float *x, float *y);
extern void	pushpos(void);
extern void	poppos(void);

/*
 * routines for retrieving the aspect details of the device
 */
extern float	getaspect(void);
extern void	getfactors(float *x, float *y);
extern void	getdisplaysize(float *x, float *y);
extern void	expandviewport(void);
extern void	unexpandviewport(void);

/*
 * routines for handling the buffering
 */
extern int	backbuffer(void);
extern void	frontbuffer(void);
extern int	swapbuffers(void);

/*
 * routines for window sizing and positioning
 */
extern void	prefsize(int x, int y);
extern void	prefposition(int x, int y);
extern void	getprefposandsize(int *x, int *y, int * xs, int *ys);

/* 
 * Misc control routines
 */
extern void	clip(Vector p0, Vector p1);
extern void	quickclip(Vector p0, Vector p1);
extern void	clipping(int onoff);
extern void	vsetflush(int yn);
extern void	vflush(void);
extern void	yobbarays(int onoff);
extern FILE	*_voutfile(void);

/*
 * Error handling routines
 */
extern int	verror(int xerrno, const char *str);
extern void	verrorhandler(int (*f)(int en, const char *m));
extern void	verrorhandler_(int (*f)(int *en, const char *m, int len));
extern void	verrignore(int yes);
extern int	vgeterrno(void);
extern char 	*vgeterrtext(int xerrno, char *buf);

extern void	vlistdevs(void);
/*
 * Picking routines
 */
extern void	pickmode(int yes);
extern int	picked(void);
extern void	setpicksize(float px, float py);
extern void	setpickat(float x, float y);
extern void	setpickatdev(int x, int y, int w, int h);

/*
 * Misc specials
 */
#if  defined(_POSIX_C_SOURCE) || defined(_POSIX_SOURCE) || defined(NEEDPOPENDEF) || defined(__STDC__)
extern FILE	*popen(const char *, const char *);
extern int	pclose(FILE *);
#endif

#endif

