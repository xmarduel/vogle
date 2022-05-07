#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#ifdef PC
#include <string.h>
#else
#ifdef SYS5
#include <string.h>
#define rindex strrchr
#else
#include <strings.h>
#endif
#endif
#include "vogle.h"

#define	MAX(a, b)	((a) < (b) ? (b) : (a))
#define	XCOORD(x)	((x) - 'R')
#define	YCOORD(y)	('R' - (y))
#define SKEW(x, y)      (x  + vdevice.attr->a.skew * y)

static	float	SCSIZEX = 1.0, SCSIZEY = 1.0;
static	int	Loaded = 0;
static	short	nchars;


static	struct
{
	char	*p;	/* All the vectors in the font */
	char	**ind;	/* Pointers to where the chars start in p */
	int	as;	/* Max ascender of a character in this font */
	int	dec;	/* Max decender of a character in this font */
	int	mw;	/* Max width of a character in this font */
} ftab;

float  strlength(const char *s);
static void actual_move(void);
static void drawhardchar(char c);
static void drawhstr(const char *string);

float	getfontwidth(void);
float	getfontheight(void);

/*
 * font
 * 	loads in a font.
 */
void
font(const char *name)
{
   Token	*tok;

   if (!vdevice.initialised)
   {
      verror(VERR_UNINIT, "font");
   }
   
   if (vdevice.inobject)
   {
      tok = newtokens(2 + strlen(name) / sizeof(Token));
      tok[0].i = VFONT;
      strcpy((char *)&tok[1], name);

      return;
   }

   /*
    * check we aren't loading the same font twice in a row
    */
#ifdef PC
   if (*name == '\\')
   {
      if (strcmp(strrchr(name, '\\') + 1, vdevice.attr->a.font) == 0)
         return;
#else
   if (*name == '/')
   {
      if (strcmp(rindex(name, '/') + 1, vdevice.attr->a.font) == 0)
         return;
#endif
   }
   else if (strcmp(name, vdevice.attr->a.font) == 0)
      return;

   vdevice.attr->a.softtext = 0;
   if (hershfont(name))
   {
      return;
   }
   else if (!vdevice.initialised)
   {
      verror(VERR_UNINIT, "font");
   }
   else if (strcmp(name, "large") == 0)
   {
      if (!(*vdevice.dev.Vfont)(vdevice.dev.large))
         verror(VERR_FILEIO, "font(large)");
   }
   else if (strcmp(name, "small") == 0)
   {
      if (!(*vdevice.dev.Vfont)(vdevice.dev.small))
         verror(VERR_FILEIO, "font(small)");
   }
   else if (!(*vdevice.dev.Vfont)(name))
   {
      verror(VERR_FILEIO, name);
   }

#ifdef PC
   if (*name == '\\')
      strcpy(vdevice.attr->a.font, strrchr(name, '\\') + 1);
#else
   if (*name == '/')
      strcpy(vdevice.attr->a.font, rindex(name, '/') + 1);
#endif
   else
      strcpy(vdevice.attr->a.font, name);
}

/*
 * numchars
 *
 *	return the number of characters in the currently loaded hershey font.
 *	(The 128 is the number of chars in a hardware font)
 */
int
numchars(void)
{
   if (vdevice.attr->a.softtext)
      return((int)nchars);

   return(128);
}

/*
 * hershfont
 *
 * Load in a hershey font. First try the font library, if that fails try
 * the current directory, otherwise return 0.
 */
int
hershfont(const char *fontname)
{
   FILE	*fp;
   int	i;
   short	nvects, n;
   char	path[120], *flib;

   if ((flib = getenv("VFONTLIB")) == (char *)NULL)
   {
      strcpy(path, FONTLIB);
#ifdef PC
      strcat(path, "\\");
#else
      strcat(path, "/");
#endif
      strcat(path, fontname);
   }
   else
   {
      strcpy(path, flib);
#ifdef PC
      strcat(path, "\\");
#else
      strcat(path, "/");
#endif
      strcat(path, fontname);
   }

#ifdef PC
   if ((fp = fopen(path, "r+b")) == (FILE *)NULL)
      if ((fp = fopen(fontname, "r+b")) == (FILE *)NULL)
#else
   if ((fp = fopen(path, "r")) == (FILE *)NULL)
      if ((fp = fopen(fontname, "r")) == (FILE *)NULL)
#endif
         return (0);

   if (fread(&nchars, sizeof(nchars), 1, fp) != 1)
      return (0);

   if (fread(&nvects, sizeof(nvects), 1, fp) != 1)
      return(0);

   if (fread(&n, sizeof(n), 1,  fp) != 1)
      return(0);

   ftab.as = (int)n;

   if (fread(&n, sizeof(n), 1, fp) != 1)
      return(0);

   ftab.dec = (int)n;

   if (fread(&n, sizeof(n), 1, fp) != 1)
      return(0);

   ftab.mw = (int)n + vdevice.attr->a.skew * ftab.as;

   /*
    *  Allocate space for it all....
    */
   if (Loaded)
   {
      if (ftab.ind[0])
         free(ftab.ind[0]);
      if (ftab.ind)
         free(ftab.ind);
      Loaded = 0;
   }

   ftab.ind = (char **)vallocate(sizeof(char *)*(nchars + 1));

   ftab.p = (char *)vallocate((unsigned)(2 * nvects));

   /*
    *  As we read in each character, figure out what ind should be
    */

   for (i = 0; i < nchars; i++)
   {
      if (fread(&n , sizeof(n), 1, fp) != 1)
         return(0);

      if (fread(ftab.p, 1, (unsigned)n, fp) != (unsigned)n)
         return(0);

      ftab.ind[i] = ftab.p;
      ftab.p += n;
   }

   ftab.ind[nchars] = ftab.p;	/* To Terminate the last one */

   fclose(fp);
   vdevice.attr->a.softtext = Loaded = 1;

#ifdef PC
   if (*fontname == '\\')
      strcpy(vdevice.attr->a.font, strrchr(fontname, '\\') + 1);
#else
   if (*fontname == '/')
      strcpy(vdevice.attr->a.font, rindex(fontname, '/') + 1);
#endif
   else
      strcpy(vdevice.attr->a.font, fontname);

   return(1);
}

/*
 * getcharsize
 *
 *	get the width and height of a single character. At the moment, for
 * the hershey characters, the height returned is always that of the
 * difference between the maximun descender and ascender.
 *
 */
void
getcharsize(char c, float *width, float *height)
{
   float	a, b;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "getcharsize");

   if (vdevice.attr->a.softtext)
   {
      if (!Loaded)
         verror(VERR_NOHFONT, "getcharsize");

      *height = (float)(ftab.as - ftab.dec) * SCSIZEY;

      if (vdevice.attr->a.fixedwidth)
         *width = ftab.mw * SCSIZEX;
      else
         *width = (ftab.ind[c - 32][1] - ftab.ind[c - 32][0]) * SCSIZEX;
   }
   else
   {
      VtoWxy(vdevice.hwidth, vdevice.hheight, width, height);
      VtoWxy(0.0, 0.0, &a, &b);
      *height -= b;
      *width -= a;
   }
}

static struct
{
   float	x, y;
} btab[] = {
   {-0.5, -0.5},
   {0.0, 0.0},
   {-0.5, 0.5},
   {0.5, 0.5},
   {0.5, -0.5}
};

/*
 * drawchar
 *
 * Display a character from the currently loaded font.
 */
void
drawchar(int c)
{
   char	*p, *e;
   Token	*pr;
   int	Move, b, i, x, y, xt, yt, sync;
   float	xp, yp, tmp, xsave, ysave;
   float	tcos, tsin;

   if (vdevice.inobject)
   {
      pr = newtokens(2);

      pr[0].i = DRAWCHAR;
      pr[1].i = c;

      return;
   }

   if (!vdevice.attr->a.softtext)
   {
      if (!vdevice.cpVvalid)
         actual_move();
      drawhardchar(c);
      rmove(getfontwidth(), 0.0, 0.0);
      return;
   }

   if (!Loaded)
      verror(VERR_NOFONT, "drawchar");

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   tcos = vdevice.attr->a.textcos;
   tsin = vdevice.attr->a.textsin;


   if ((i = c - 32) < 0)
      i = 0;
   if (i >= nchars)
      i = nchars - 1;

   xsave = vdevice.pos->cpW[V_X];
   ysave = vdevice.pos->cpW[V_Y];

   Move = 1;
   xt = (vdevice.attr->a.fixedwidth ? -ftab.mw / 2 : XCOORD(ftab.ind[i][0]));
   yt = ftab.dec;

   /* Justify in the x direction */
   if (vdevice.attr->a.justify & V_XCENTERED)
   {
      xt = 0;
   }
   else if (vdevice.attr->a.justify & V_RIGHT)
   {
      xt = (vdevice.attr->a.fixedwidth ? ftab.mw / 2 : -XCOORD(ftab.ind[i][0]));
   }

   /* Justify in the y direction */
   if (vdevice.attr->a.justify & V_YCENTERED)
   {
      yt = 0;
   }
   else if (vdevice.attr->a.justify & V_TOP)
   {
      yt = -ftab.dec;
   }

   e = ftab.ind[i+1];
   b = 0;
   do
   {
      Move = 1;
      p = ftab.ind[i] + 2;
      while(p < e) {
         x = XCOORD((int)(*p++));
         y = YCOORD((int)(*p++));
         x = SKEW(x, y);
         if (x != -50)
         {			/* means move */
            xp = (btab[b].x + (float)(x - xt))*SCSIZEX;
            yp = (btab[b].y + (float)(y - yt))*SCSIZEY;
            tmp = xp;
            xp = tcos*tmp - tsin*yp + xsave;
            yp = tsin*tmp + tcos*yp + ysave;
            if (Move)
            {
               Move = 0;
               move(xp, yp, vdevice.pos->cpW[V_Z]);
            }
            else
            {
               draw(xp, yp, vdevice.pos->cpW[V_Z]);
            }
         }
         else
         {
            Move = 1;
         }
      }
   } while (++b < vdevice.attr->a.bold);

   /*
    * Move to right hand of character.
    */

   tmp = vdevice.attr->a.fixedwidth ? (float)ftab.mw : (float)(ftab.ind[i][1] - ftab.ind[i][0]);
   tmp *= SCSIZEX;
   xsave += tcos*tmp;
   ysave += tsin*tmp;
   move(xsave, ysave, vdevice.pos->cpW[V_Z]);

   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }
}

/*
 * drawhardchar
 *
 *	Displays a hardware character.
 *	NOTE: Only does gross clipping to the viewport.
 *	      Current world position becomes undefined (ie you have
 *	      to do an explicit move after calling hardware text)
 */
static void
drawhardchar(char c)
{

   if (!vdevice.clipoff)
   {
      if (vdevice.cpVx - (int)vdevice.hwidth > vdevice.maxVx)
         return;

      if (vdevice.cpVx < vdevice.minVx)
         return;

      if (vdevice.cpVy - (int)vdevice.hheight > vdevice.maxVy)
         return;

      if (vdevice.cpVy < vdevice.minVy)
         return;
   }

   (*vdevice.dev.Vchar)(c);
}

/*
 * textsize
 *
 * set software character scaling values
 *
 * Note: Only changes software char size. Should be called
 * after a font has been loaded.
 *
 */
void
textsize(float width, float height)
{
   float	a;
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "textsize");

   if (!vdevice.attr->a.softtext)
      return;

   if (!Loaded)
      verror(VERR_NOFONT, "textsize");

   a = (float)MAX(ftab.mw, (ftab.as - ftab.dec));
   vdevice.attr->a.fontwidth = width;
   vdevice.attr->a.fontheight = height;
   SCSIZEX = width / a;
   SCSIZEY = height / a;

   if (vdevice.inobject)
   {
      /*
       * Note: need the above computation even if inobject
       * so that gettextsize works correctly in an object
       */
      tok = newtokens(3);

      tok[0].i = TEXTSIZE;
      tok[1].f = width;
      tok[2].f = height;
   }
}

/*
 * getfontwidth
 *
 * Return the maximum Width of the current font.
 *
 */
float
getfontwidth(void)
{
   float	a, b, c, d;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "getfontwidth");


   if (vdevice.attr->a.softtext)
   {
      if (!Loaded)
         verror(VERR_NOFONT, "getfontwidth");

      return((float)(SCSIZEX * MAX(ftab.mw, (ftab.as - ftab.dec))));
   }
   else
   {
      VtoWxy(vdevice.hwidth, vdevice.hheight, &c, &d);
      VtoWxy(0.0, 0.0, &a, &b);
      c -= a;
      return(c);
   }
}

/*
 * getfontheight
 *
 * Return the maximum Height of the current font
 */
float
getfontheight(void)
{
   float	a, b, c, d;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "getfontheight");

   if (vdevice.attr->a.softtext)
   {
      if (!Loaded)
         verror(VERR_NOFONT, "getfontheight");

      return((float)(SCSIZEY * MAX(ftab.mw, (ftab.as - ftab.dec))));
   }
   else
   {
      VtoWxy(vdevice.hwidth, vdevice.hheight, &c, &d);
      VtoWxy(0.0, 0.0, &a, &b);
      d -= b;
      return(d);
   }
}

/*
 * getfontsize
 *
 * get the current character size in user coords.
 * Hardware text may or may not be really that accurate,
 * depending on what type of font you are using on the device.
 * For software Hershey fonts, the character width is that of
 * a the widest character and the height the height of the tallest.
 *
 */
void
getfontsize(float *cw, float *ch)
{
   *cw = getfontwidth();
   *ch = getfontheight();
}

/*
 * drawhstr
 *
 * Display the text string using the currently loaded Hershey font
 */
static void
drawhstr(const char *string)
{
   char	c;
   int	i, sync, oldClipoff, NeedClip, oldJustify;
   float	p[4], q[4];


   if (!vdevice.initialised)
      verror(VERR_UNINIT, "drawhstr");

   /*
    * For the duration of hershey strings, turn off
    * "vdevice.attr->a.justify" as we have already compensated
    * for it in drawstr()
    */
   oldJustify = vdevice.attr->a.justify;
   vdevice.attr->a.justify = V_LEFT;

   /*
    * Determine if we can get away with "clipoff"
    */
   oldClipoff = vdevice.clipoff;
   if (!oldClipoff)
   {  /* Only do this if we have to ... ie. if clipping is on */
      q[0] = vdevice.pos->cpW[V_X];
      q[1] = vdevice.pos->cpW[V_Y];
      q[2] = vdevice.pos->cpW[V_Z];
      q[3] = 1.0;
      multvector(p, q, vdevice.transmat->m);
      NeedClip = 0;
      for (i = 0; i < 3; i++)
         NeedClip = ((p[3] + p[i] < 0.0) ||
                     (p[3] - p[i] < 0.0)) || NeedClip;
      if (!NeedClip)
      {   	/* The other end, only if we have to */
         q[0] += strlength(string);
         q[1] += getfontheight();
         multvector(p, q, vdevice.transmat->m);
         NeedClip = 0;
         for (i = 0; i < 3; i++)
            NeedClip = ((p[3] + p[i] < 0.0) ||
                        (p[3] - p[i] < 0.0)) || NeedClip;
      }

      if (!NeedClip)
         vdevice.clipoff = 1; /* ie. Don't clip */

   }

   /*
    * Now display each character
    *
    */
   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   while ((c = *string++))
      drawchar(c);

   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }

   /*
    * Restore ClipOff
    */
   vdevice.clipoff = oldClipoff;
   vdevice.attr->a.justify = oldJustify;
}

/*
 * drawstr
 *
 * Draw a string from the current pen position.
 *
 */
void
drawstr(const char *string)
{
   float	sl, width, height, cx, cy;
   float	tcos, tsin;
   char	c;
   Token	*tok;

   if(!vdevice.initialised)
      verror(VERR_UNINIT, "drawstr");

   if (vdevice.inobject)
   {
      tok = newtokens(2 + strlen(string) / sizeof(Token));

      tok[0].i = DRAWSTR;
      strcpy((char *)&tok[1], string);

      return;
   }

#ifdef SUN_CC
   /* Note that SUN's unbundled ANSI C compiler bitches about this
      sl = (float)strlen(string);
   ... so we change it to */
   sl = (float)(size_t)strlen(string);
#else
   sl = (float)strlen(string);
#endif

   tcos = vdevice.attr->a.textcos;
   tsin = vdevice.attr->a.textsin;

   height = getfontheight();
   width = strlength(string);

   /* Justify in the x direction */
   if (vdevice.attr->a.justify & V_XCENTERED)
   {
      width /= 2.0;
   }
   else if (vdevice.attr->a.justify & V_RIGHT)
   {
      ;	/* NO change */
   }
   else
   {	/* V_LEFT as default */
      width = 0.0;
   }

   /* Justify in the y direction */
   if (vdevice.attr->a.justify & V_YCENTERED)
   {
      height /= 2.0;
   }
   else if (vdevice.attr->a.justify & V_TOP)
   {
      ;	/* NO change */
   }
   else
   {	/* V_BOTTOM as default */
      height = 0.0;
   }

   cx = vdevice.pos->cpW[V_X] + height * tsin - width * tcos;
   cy = vdevice.pos->cpW[V_Y] - height * tcos - width * tsin;

   move(cx, cy, vdevice.pos->cpW[V_Z]);


   if (vdevice.attr->a.softtext)
   {
      /*  As we are using software text then call the routine
          to display it in the current font */
      drawhstr(string);
   }
   else
   {
      actual_move();	/* Really move there */

      /*   If not clipping then simply display text and return  */

      if (vdevice.clipoff)
      {
         (*vdevice.dev.Vstring)(string);
      }
      else
      {
         /* Check if string is within viewport */
         if (vdevice.cpVx > vdevice.minVx &&
             vdevice.cpVx + (int)(sl * (vdevice.hwidth - 1)) < vdevice.maxVx &&
             vdevice.cpVy - (int)vdevice.hheight < vdevice.maxVy &&
             vdevice.cpVy > vdevice.minVy)
               (*vdevice.dev.Vstring)(string);
         else
            while ((c = *string++))
            {
               drawhardchar(c);
               vdevice.cpVx += vdevice.hwidth;
            }
               
      }

      move(cx + getfontwidth() * sl, cy, vdevice.pos->cpW[V_Z]);

   }
}

/*
 * istrlength
 *
 * Find out the length of a string in raw "Hershey coordinates".
 */
static	int
istrlength(const char *s)
{
   char	c;
   int	i, j, len = 0;

   if (vdevice.attr->a.fixedwidth)
   {
      return((int)(strlen(s) * ftab.mw));
   }
   else
   {
      while ((c = *s++))
      {
         if ((i = (int)c - 32) < 0 || i >= nchars)
            i = nchars - 1;

         j = ftab.ind[i][1] - ftab.ind[i][0];

         len += j;
      }
      len += (int)(0.5 * vdevice.attr->a.skew * ftab.as);
      return(len);
   }
}

/*
 * strlength
 *
 * Find out the length (in world coords) of a string.
 *
 */
float
strlength(const char *s)
{
   if (!vdevice.initialised)
      verror(VERR_UNINIT, "strlength");

   if (vdevice.attr->a.softtext)
      return((float)(istrlength(s) * SCSIZEX));
   else
#ifdef SUN_CC
      /* Note that SUN's unbundled ANSI C compiler bitches here
      return((float)(strlen(s) * getfontwidth()));
      ... so we write it as ... */
      return((float)((size_t)strlen(s) * getfontwidth()));
#else
   return((float)(strlen(s) * getfontwidth()));
#endif
}

/*
 * boxtext
 *
 * Draw text so it fits in a "box" - note only works with hershey text
 */
void
boxtext(float x, float y, float l, float h, char *s)
{
   float	oscsizex, oscsizey;
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "boxtext");

   if (!vdevice.attr->a.softtext)
      verror(VERR_NOHFONT, "boxtext");

   if (vdevice.inobject)
   {
      tok = newtokens(6 + strlen(s) / sizeof(Token));

      tok[0].i = BOXTEXT;
      tok[1].f = x;
      tok[2].f = y;
      tok[3].f = l;
      tok[4].f = h;
      strcpy((char *)&tok[5], s);

      return;
   }

   oscsizex = SCSIZEX;
   oscsizey = SCSIZEY;
   /*
    * set width so string length is the same a "l"
    */
   SCSIZEX = l / (float)istrlength(s);

   /*
    * set character height so it's the same as "h"
    */
   SCSIZEY = h / (float)(ftab.as - ftab.dec);
   move(x, y, vdevice.pos->cpW[V_Z]);

   drawstr(s);

   SCSIZEX = oscsizex;
   SCSIZEY = oscsizey;
}

/*
 * boxfit
 *
 * Set up the scales etc for text so that a string of "nchars" characters
 * of the maximum width in the font fits in a box.
 */
void
boxfit(float l, float h, int nchars)
{
   if (!vdevice.initialised)
      verror(VERR_UNINIT, "boxfit");

   if (!vdevice.attr->a.softtext)
      verror(VERR_NOHFONT, "boxfit");

   SCSIZEX = l / (float)(nchars * ftab.mw);
   SCSIZEY = h / (float)(ftab.as - ftab.dec);
}

/*
 * centertext
 *
 *	Turns centering of text on or off
 *	Turns off all other justifying.
 *	(Just like in old VOGLE).
 */
void
centertext(int onoff)
{
   if (onoff)
      vdevice.attr->a.justify = V_XCENTERED | V_YCENTERED;
   else
      vdevice.attr->a.justify = V_LEFT | V_BOTTOM;
}

/*
 * textjustify
 *
 *	Directly turns on/off justification
 */
void
textjustify(unsigned val)
{
   vdevice.attr->a.justify = val;
}

/*
 * xcentertext
 *
 *	Directly turns on xcentering
 */
void
xcentertext(void)
{
   vdevice.attr->a.justify |= V_XCENTERED;
   vdevice.attr->a.justify &= ~(V_LEFT | V_RIGHT);
}

/*
 * ycentertext
 *
 *	Directly turns on ycentering
 */
void
ycentertext(void)
{
   vdevice.attr->a.justify |= V_YCENTERED;
   vdevice.attr->a.justify &= ~(V_TOP | V_BOTTOM);
}

/*
 * leftjustify
 *
 *	Turns on leftjustification
 */
void
leftjustify(void)
{
   /*
    * If left justification is on, then V_XCENTER must be off
    * and V_RIGHT must be off
    */
   vdevice.attr->a.justify |= V_LEFT;
   vdevice.attr->a.justify &= ~(V_RIGHT | V_XCENTERED);
}

/*
 * rightjustify
 *
 *	Turns on rightjustification
 */
void
rightjustify(void)
{
   /*
    * If right justification is on, then V_XCENTER must be off
    * and V_LEFT must be off
    */
   vdevice.attr->a.justify |= V_RIGHT;
   vdevice.attr->a.justify &= ~(V_LEFT | V_XCENTERED);
}

/*
 * topjustify
 *
 *	Turns on topjustification
 */
void
topjustify(void)
{
   /*
    * If top justification is on, then V_YCENTER must be off
    * and V_BOTTOM must be off
    */
   vdevice.attr->a.justify |= V_TOP;
   vdevice.attr->a.justify &= ~(V_BOTTOM | V_YCENTERED);
}


/*
 * bottomjustify
 *
 *	Turns on bottomjustification
 */
void
bottomjustify(void)
{
   /*
    * If bottom justification is on, then V_YCENTER must be off
    * and V_TOP must be off
    */
   vdevice.attr->a.justify |= V_BOTTOM;
   vdevice.attr->a.justify &= ~(V_TOP | V_YCENTERED);
}

/*
 * fixedwidth
 *
 *	Turns fixedwidth text on or off
 */
void
fixedwidth(int onoff)
{
   vdevice.attr->a.fixedwidth = onoff;
}

/*
 * textang
 *
 * set software character angle in degrees
 *
 * strings will be written along a line 'ang' degrees from the
 * horizontal screen direction
 *
 * Note: only changes software character angle
 *
 */
void
textang(float ang)
{
   Token	*tok;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "textang");

   if (vdevice.inobject)
   {
      tok = newtokens(3);

      tok[0].i = TEXTANG;
      tok[1].f = cos((double)(ang * D2R));
      tok[2].f = sin((double)(ang * D2R));

      return;
   }

   vdevice.attr->a.textcos = cos((double)(ang * D2R));
   vdevice.attr->a.textsin = sin((double)(ang * D2R));
}

/*
 * textslant
 *
 *	Defines the obliqness of the fonts.
 */
void
textslant(float val)
{
   vdevice.attr->a.skew = val;
}

/*
 * textweight
 *
 *	Defines the weight of the fonts.
 */
void
textweight(int val)
{
   vdevice.attr->a.bold = val ? 5 : 0;
}

/*
 * Actually do a move (multplying by the current transform and updating the
                       * actual screen coords) instead of just setting the current spot in world
 * coords.
 */
static void
actual_move(void)
{
   Vector	v2;

   multvector(v2, vdevice.pos->cpW, vdevice.transmat->m);
   vdevice.cpVvalid = 0;

   vdevice.cpVx = WtoVx(v2);
   vdevice.cpVy = WtoVy(v2);

   copyvector(vdevice.pos->cpWtrans, v2);
}
