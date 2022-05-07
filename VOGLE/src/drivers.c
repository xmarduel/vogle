#include <stdlib.h>
#include <string.h>
#include "vogle.h"

struct vdev	vdevice;

static FILE	*fp;// = stdout;

static int	allocated = 0;

struct	devs
{
   char	*name;
   void (*copyfunc)(char *name);
};

#ifdef TG
extern	void	_TG_devcpy(char *);
#endif

#ifdef PPM
extern	void	_PPM_devcpy(char *);
#endif

#ifdef GIF
extern	void	_GIF_devcpy(char *);
#endif

#ifdef TIFF
extern	void	_TIFF_devcpy(char *); /* to do */
#endif

#ifdef X11
extern  void	_X11_devcpy(char *);
#endif

#ifdef CARBON
extern  void	_CARBON_devcpy(char *);
#endif

#ifdef COCOA
extern  void	_COCOA_devcpy(char *);
#endif

#ifdef POSTSCRIPT
extern void	_PS_devcpy(char *);
#endif

#ifdef HPDXY
extern void _HPGL_devcpy(char *);
extern void _DXY_devcpy(char *);
#endif

#ifdef TEK
extern void _TEK_devcpy(char *);
#endif

struct devs devices[] = {
#ifdef TG
   {".", _TG_devcpy},
#endif
#ifdef PPM
   {"p3", _PPM_devcpy},
   {"p6", _PPM_devcpy},
   {"ppm", _PPM_devcpy},
#endif
#ifdef GIF
   {"gif", _GIF_devcpy},
#endif
#ifdef TIFF
   {"tiff", _TIFF_devcpy},
#endif
#ifdef X11
   {"X11", _X11_devcpy},
#endif
#ifdef NeXT
   {"NeXT", _NeXT_devcpy},
#endif
#ifdef CARBON
   {"CARBON", _CARBON_devcpy},
#endif
#ifdef COCOA
   {"COCOA", _COCOA_devcpy},
#endif
#ifdef POSTSCRIPT
   {"cps", _PS_devcpy},
   {"pcps", _PS_devcpy},
   {"ps", _PS_devcpy},
   {"pps", _PS_devcpy},
#endif
#ifdef HPDXY
   {"hpgl", _HPGL_devcpy},
   {"hpgla1", _HPGL_devcpy},
   {"hpgla3", _HPGL_devcpy},
   {"hpgla4", _HPGL_devcpy},
   {"hpgla2", _HPGL_devcpy},
   {"dxy", _DXY_devcpy},
#endif
#ifdef HPGT
   {"hpgt", _HPGT_devcpy},
#endif
#ifdef GRX
   {"grx", _grx_devcpy},
#endif
#ifdef TEK
   {"tek", _TEK_devcpy},
#endif
#ifdef HERCULES
   {"hercules", _hgc_devcpy},
#endif
#ifdef MSWIN
   {"mswin", _mswin_devcpy},
#endif
#ifdef OS2
   {"os2pm", _PM_devcpy},
#endif
#ifdef CGA
   {"cga", _cga_devcpy},
#endif
#ifdef EGA
   {"ega", _ega_devcpy},
#endif
#ifdef VGA
   {"vga", _vga_devcpy},
#endif
#ifdef SIGMA
   {"sigma", _sigma_devcpy}
#endif
};

#define NDEVS	(sizeof(devices) / sizeof(struct devs))


/* device-independent function routines */

/*
 * voutput
 *
 *	redirect output - only for postscript, hpgl (this is not a feature)
 */
void
voutput(const char *path)
{
   char	buf[128];

   if ((fp = fopen(path, "w")) == (FILE *)NULL)
   {
      sprintf(buf, "voutput: couldn't open %s", path);
      verror(VERR_FILEIO, buf);
   }
}

/*
 * Set the current output file  pointer (could be a pipe from popen).
 */
void
voutputfp(FILE	*vfp)
{
   fp = vfp;
}

/*
 * _voutfile
 *
 *	return a pointer to the current output file - designed for internal
 * use only.
 */
FILE *
_voutfile(void)
{
   return(fp);
}

void
vlistdevs(void)
{
   int	i;
   printf("Devices compiled into this library are:\n");
   for (i = 0; i < NDEVS; i++)
   {
      printf("%s%c", devices[i].name, i == NDEVS - 1 ? '\n' : ' ');
   }
}

/*
 * Return the number of devices supported.
 */
int
vndevs(void)
{
   return(NDEVS);
}

/*
 * Return the name of the n-th device.
 */
char *
vgetdevname(int n, char *name)
{
   printf("vgetdevname: %d\n", n);
   if (n < 0 || n >= NDEVS)
      verror(VERR_BADDEV, "vgetdevname");
   else
      strcpy(name, devices[n].name);

   return(name);
}


/*
 * getdev
 *
 *	get the appropriate device table structure
 */
int
_vgetvogledev(const char *dev)
{
   char	*device = "";
   char	buf[100];
   int	i;

   if (dev == (char *)NULL || *dev == 0)
   {
      if ((device = getenv("VDEVICE")) == (char *)NULL)
         device = "";
   }
   else
   {
      device = (char *)dev;
   }
   
   for (i = 0; i < NDEVS; i++)
   {
      if (strcmp(devices[i].name, device) == 0)
      {
         devices[i].copyfunc(device);
         return(1);
      }
   }

   if (*device == 0)
      sprintf(buf, "vogle: expected the enviroment variable VDEVICE to be set to the desired device.");
   else
      sprintf(buf, "vogle: %s is an invalid device type", device);

#ifdef MSWIN
   return(mswin_verror(VERR_BADDEV, buf));
#else
#ifdef OS2
   return(PM_verror(VERR_BADDEV, buf));
#else
   return(verror(VERR_BADDEV, buf));
#endif
#endif
}


/*
 * vinit
 *
 * 	initialise VOGLE
 *
 */
void
_vdovogleinit(void)
{
   if (!allocated)
   {
      allocated = 1;
      vdevice.transmat = (Mstack *)vallocate(sizeof(Mstack));
      vdevice.transmat->back = (Mstack *)NULL;
      vdevice.attr = (Astack *)vallocate(sizeof(Astack));
      vdevice.attr->back = (Astack *)NULL;
      vdevice.viewport = (Vstack *)vallocate(sizeof(Vstack));
      vdevice.viewport->back = (Vstack *)NULL;
      vdevice.pos = (Pstack *)vallocate(sizeof(Pstack));
      vdevice.pos->back = (Pstack *)NULL;
   }

   vdevice.clipoff = 0;
   vdevice.sync = 1;
   vdevice.upset = 0;
   vdevice.pos->cpW[V_W] = 1.0;			/* never changes. (much?) */

   vdevice.attr->a.font[0] = '\0';
   vdevice.attr->a.color = 0;
   vdevice.attr->a.fill = 0;
   vdevice.attr->a.hatch = 0;
   vdevice.attr->a.backface = 0;
   vdevice.attr->a.justify = V_LEFT | V_BOTTOM;
   vdevice.attr->a.bold = 0;
   vdevice.attr->a.skew = 0.0;
   vdevice.attr->a.textcos = 1.0;
   vdevice.attr->a.textsin = 0.0;
   vdevice.attr->a.softtext = 0;
   vdevice.attr->a.fixedwidth = 0;
   vdevice.attr->a.hatchcos = 1.0;
   vdevice.attr->a.hatchsin = 0.0;
   vdevice.attr->a.hatchpitch = 0.1;
   vdevice.attr->a.style = (char *)NULL;
   vdevice.attr->a.dashp = (char *)NULL;
   vdevice.attr->a.adist = 0.0;
   vdevice.attr->a.dash = 0.0;
   vdevice.attr->a.exvp = 0;
   vdevice.pickwin.pickm = 0;
   vdevice.inobject = 0;
   vdevice.inpolygon = 0;

   vdevice.initialised = 1;


   if (getenv("VEXPANDVP") != (char *)NULL)
      expandviewport();

   viewport(-1.0, 1.0, -1.0, 1.0);

   identmatrix(vdevice.transmat->m);

   move(0.0, 0.0, 0.0);

   if (!hershfont("futura.l")) /* Try a Hershey font */
   {
      font("small");	/* set up default font */
   }

   textsize(0.05, 0.05);

   return;
}

#if 0
/*
 * getkey
 *
 *	returns the next key pressed.
 */
#ifndef GRX	/* Has it's own getkey */
int
getkey(void)
{
   if (!vdevice.initialised)
      verror(VERR_UNINIT, "getkey");

   return((*vdevice.dev.Vgetkey)());
}
#endif

/*
 * checkkey
 *
 *	returns true if a key has been hit, or 0 otherwise
 *	(doesn't wait around like getkey)
 */
int
checkkey(void)
{
   if (!vdevice.initialised)
      verror(VERR_UNINIT, "checkkey");

   return((*vdevice.dev.Vcheckkey)());
}
#endif

int
getkey(void)
{
   Vevent	vev;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "getkey");

   do
   {
      (void)(*vdevice.dev.Vgetevent)(&vev, 1);
   } while (vev.type != -1 && vev.type != VKEYPRESS);

   return(vev.data);
}

int
checkkey(void)
{

   Vevent	vev;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "checkkey");

   (void)(*vdevice.dev.Vgetevent)(&vev, 0);
   if (vev.type == VKEYPRESS)
      return(vev.data);

   return(0);
}

/*
 * locator
 *
 *	returns the current position of the crosshair or equivalent
 * in world coordinates, and the mouse buttons pressed (if any).
 */
int
locator(float *wx, float *wy)
{
   int	a, b, c;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "locator");

   c = (*vdevice.dev.Vlocator)(&a, &b);
   VtoWxy((float)a, (float)b, wx, wy);

   return(c);
}

/*
 * slocator
 *
 *	returns the current position of the crosshair or equivalent
 * in screen coordinates, and the mouse buttons pressed (if any).
 */
int
slocator(float *wx, float *wy)
{
   int	a, b, c;
   float	sx, sy;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "slocator");

   c = (*vdevice.dev.Vlocator)(&a, &b);
   sx = vdevice.dev.sizeX;
   sy = vdevice.dev.sizeY;

   *wx = a / (0.5 * sx) - 1.0;
   *wy = b / (0.5 * sy) - 1.0;

   return(c);
}

/*
 * clear
 *
 *	clears the screen to the current colour, excepting devices
 * like a laser printer where it flushes the page.
 *
 */
void
clear(void)
{
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "clear");

   if (vdevice.inobject)
   {
      tok = newtokens(1);
      tok->i = CLEAR;

      return;
   }

   (*vdevice.dev.Vclear)();
}

/*
 * vexit
 *
 *	exit the vogle system
 *
 */
void
vexit(void)
{
   int	i;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "vexit");

   for (i = 0; i < vdevice.nwin; i++)
   {
      if (vdevice.wins[i].id != -1)
      {
         DevEntry *ent;

         ent = &vdevice.wins[i].dev;
         (*ent->Vwindel)(vdevice.wins[i].devwin);
         vdevice.wins[i].id = -1;
         vdevice.wins[i].devwin = NULL;
      }
   }

   free(vdevice.wins);
   vdevice.wins = NULL;

   vdevice.initialised = 0;
   fp = stdout;
}

/*
 * color
 *
 *	set the current colour to colour index number i.
 *
 */
void
color(int i)
{
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "color");

   if (vdevice.inobject)
   {
      tok = newtokens(2);

      tok[0].i = COLOR;
      tok[1].i = i;
      return;
   }

   vdevice.attr->a.color = i;
   (*vdevice.dev.Vcolor)(i);
}

/*
 * mapcolor
 *
 *	set the color of index i.
 */
void
mapcolor(int i, short r, short g, short b)
{
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "mapcolor");

   if (vdevice.inobject)
   {
      tok = newtokens(5);

      tok[0].i = MAPCOLOR;
      tok[1].i = i;
      tok[2].i = r;
      tok[3].i = g;
      tok[4].i = b;

      return;
   }

   (*vdevice.dev.Vmapcolor)(i, r, g, b);
}

/*
 * getdepth
 *
 *	Returns the number if bit planes on a device.
 */
int
getdepth(void)
{
   if (!vdevice.initialised)
      verror(VERR_UNINIT, "getdepth");

   return(vdevice.dev.depth);
}

/*
 * vsetflush
 *
 * Controls flushing of the display - we can get considerable
 * Speed up's under X11 using this...
 */
void
vsetflush(int yn)
{
   vdevice.sync = yn;
}

/*
 * vflush
 *
 * Explicitly call the device flushing routine...
 * This is enabled for object so that you can force an update
 * in the middle of an object, as objects have flushing off
 * while they are drawn anyway.
 */
void
vflush(void)
{
   Token	*tok;

   if (!vdevice.initialised)
   {
      verror(VERR_UNINIT, "vflush");
   }
   
   if (vdevice.inobject)
   {
      tok = newtokens(1);
      tok->i = VFLUSH;

      return;
   }

   (*vdevice.dev.Vsync)();
}

/*
 * linewidth
 *
 *	Because it's so difficult to do this device independently,
 *	(it looks so different on each device) we will just have
 *	THICK(1) or THIN(0).
 */
void
linewidth(int w)
{
   Token	*tok;

   if (vdevice.inobject)
   {
      tok = newtokens(2);
      tok[0].i = LINEWIDTH;
      tok[1].i = w;

      return;
   }
   
   (*vdevice.dev.Vsetlw)(w);
}
