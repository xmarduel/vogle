#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifdef PC
#include <fcntl.h>
#endif

#ifndef PC
#ifdef SYS5
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#endif
#include "vogle.h"

extern void		polyobj(int n, Token dp[]);

typedef struct o
{
   int		obno;
   TokList	*tlist;
   struct o	*next;
} Object;

static Object	*object_table[MAXENTS];
static Object	*save_o = NULL;

static int	obno = -1, omax = 0;

/*
 * makeobj
 *
 *	start a new object.
 *
 */
void
makeobj(int n)
{
   Object	*o;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "makeobj");

   if (n < 0)
      verror(VERR_RANGE, "makeobj");

   for (o = object_table[n % MAXENTS]; o != (Object *)NULL; o = o->next)
   {
      if (o->obno == n)
      {
         delobj(n);
         break;
      }
   }
   
   obno = n;
   vdevice.tokens = (TokList *)NULL;

   vdevice.inobject = 1;
   save_o = NULL;

   if (omax <= n)
      omax = n + 1;
}

/*
* appendobj
 *
 * Append to an existing object
 */
void
appendobj(int n)
{
   Object	*o;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "appendobj");

   if (n < 0)
      verror(VERR_RANGE, "appendobj");

   for (o = object_table[n % MAXENTS]; o != (Object *)NULL; o = o->next)
   {
      if (o->obno == n)
      {
         break;
      }
   }

   if (o == NULL)
   {	/* it's really a new one */
      makeobj(n);
      return;
   }

   obno = n;

   vdevice.tokens = o->tlist;
   _setcurrenttoklist(vdevice.tokens);
   save_o = o;

   vdevice.inobject = 1;
}


/*
 * closeobj
 *
 *	close an object
 */
void
closeobj(void)
{
   Object	*o;

   if (!vdevice.inobject)
      verror(VERR_NOOBJECT, "closeobj");

   vdevice.inobject = 0;

   if (!save_o)
   {	/* It's from a makeobj() */
      o = (Object *)vallocate(sizeof(Object));
      o->obno = obno;
      o->tlist = vdevice.tokens;
      o->next = object_table[obno % MAXENTS];

      object_table[obno % MAXENTS] = o;
   }
   else /* It's from an appendobj() */
   {
      save_o = NULL;
   }

   obno = -1;
}

/*
 * delobj
 *
 *	deletes an object, freeing its memory
 */
void
delobj(int n)
{
   Object	*o, *lo;
   TokList	*tl, *ntl;

   if (n < 0)
      verror(VERR_RANGE, "delobj");

   for (lo = o = object_table[n % MAXENTS]; o != (Object *)NULL; lo = o, o = o->next)
   {
      if (o->obno == n)
         break;
   }
   
   if (o != (Object *)NULL)
   {
      for (tl = o->tlist; tl != (TokList *)NULL; tl = ntl)
      {
         ntl = tl->next;
         if (tl->toks)
            free(tl->toks);
         free(tl);
      }
      
      if (lo == object_table[n % MAXENTS])
      {
         object_table[n % MAXENTS] = (Object *)NULL;
      }
      else
      {
         lo->next = o->next;
      }
      free(o);
   }
}

/*
 * genobj
 *
 *	generates a unique object identifier
 */
int
genobj(void)
{
   return(omax++);
}

/*
 * getopenobj
 *
 *	returns the object currently being edited, -1 if none.
 */
int
getopenobj(void)
{
   return(obno);
}

/*
 * loadobj
 *
 *	reads in object file file and makes it the object refered to by n.
 */
void
loadobj(int n, char *file)
{
   int	objfd;
   TokList	*tl;
   Object	*o;
   char	buf[100];

   if ((objfd = open(file, O_RDONLY)) == EOF)
   {
      sprintf(buf, "loadobject: unable to open object file %s", file);
      verror(VERR_FILEIO, buf);
   }

   if (n < 0)
   {
      verror(VERR_RANGE, "loadobj");
   }
   
   o = (Object *)vallocate(sizeof(Object));
   o->obno = n;
   o->next = object_table[n % MAXENTS];

   object_table[n % MAXENTS] = o;

   o->tlist = tl = (TokList *)vallocate(sizeof(TokList));
   read(objfd, &tl->count, sizeof(tl->count));

   tl->toks = (Token *)vallocate(tl->count * sizeof(Token));
   tl->next = (TokList *)NULL;

   read(objfd, tl->toks, tl->count * sizeof(Token));

   close(objfd);
}

/*
 * saveobj
 *
 *	saves the object n into file file.
 */
void
saveobj(int n, char *file)
{
   int	objfd, count;
   Object	*o;
   TokList	*tl;
   char	buf[100];


   if ((objfd = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0664)) == EOF)
   {
      sprintf(buf, "saveobject: unable to open object file %s", file);
      verror(VERR_FILEIO, buf);
   }

   if (n < 0)
      verror(VERR_RANGE, "saveobj");

   for (o = object_table[n % MAXENTS]; o != (Object *)NULL; o = o->next)
   {
      if (o->obno == n)
         break;
   }
   
   if (o == (Object *)NULL)
      return;

   count = 0;
   for (tl = o->tlist; tl != (TokList *)NULL; tl = tl->next)
   {
      count += tl->count;
   }
   
   write(objfd, &count, sizeof(count));

   for (tl = o->tlist; tl != (TokList *)NULL; tl = tl->next)
   {
      write(objfd, tl->toks, tl->count * sizeof(Token));
   }
   
   close(objfd);
}

/*
 * doarc
 *
 *	draw an arc or circle.
 */
static void
doarc(float x, float y, float xoff, float yoff,float cosine,float sine,int nsegs)
{
   float	cx, cy, dx, dy;
   int	i;

   cx = x + xoff;
   cy = y + yoff;
   move2(cx, cy);

   for (i = 0; i < nsegs; i++)
   {
      dx = cx - x;
      dy = cy - y;
      cx = x + dx * cosine - dy * sine;
      cy = y + dx * sine + dy * cosine;
      draw2(cx, cy);
   }
}

/*
 * callobj
 *
 *	draws an object
 */
void
callobj(int n)
{
   Object		*o;
   TokList		*tl;
   Matrix		prod, tmpmat;
   Tensor		S;
   int		sync, i, j, inpoly;
   float		cx, cy, cz, *m;
   register Token	*t, *et, *pt;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "callobj");

   if (n < 0)
      verror(VERR_RANGE, "callobj");

   if (vdevice.inobject) {
      t = newtokens(2);

      t[0].i = CALLOBJ;
      t[1].i = n;

      return;
   }

   for (o = object_table[n % MAXENTS]; o != (Object *)NULL; o = o->next)
   {
      if (o->obno == n)
         break;
   }
   
   if (o == (Object *)NULL)
      return;

   if ((sync = vdevice.sync))
      vdevice.sync = 0;

   for (tl = o->tlist; tl != (TokList *)NULL; tl = tl->next)
   {
      t = tl->toks;
      et = &tl->toks[tl->count];
      while (t != et)
      {
         switch (t->i)
         {
            case ARC:
               doarc(t[1].f, t[2].f, t[3].f, t[4].f, t[5].f, t[6].f, t[7].i);
               t += 8;
               break;
            case BACKBUFFER:
               backbuffer();
               t++;
               break;
            case BACKFACING:
               backface(t[1].i);
               t += 2;
               break;
            case BOXTEXT:
               boxtext(t[1].f, t[2].f, t[3].f, t[4].f, (char *)&t[5]);
               t += 6 + strlen((char *)&t[5]) / sizeof(Token);
               break;
            case CALLOBJ:
               callobj(t[1].i);
               t += 2;
               break;
            case CENTERTEXT:
               centertext(t[1].i);
               t += 2;
               break;
            case CIRCLE:
               inpoly = vdevice.inpolygon;

               if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !inpoly)
                  makepoly();

                  doarc(t[1].f, t[2].f, t[3].f, 0.0, t[4].f, t[5].f, t[6].i);

               if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !inpoly)
                  closepoly();
               else
                  draw2(t[1].f + t[3].f, t[2].f);
               t += 7;
               break;
            case CLEAR:
               (*vdevice.dev.Vclear)();
               t++;
               break;
            case COLOR:
               color(t[1].i);
               t += 2;
               break;
            case DRAW:
               draw(t[1].f, t[2].f, t[3].f);
               t += 4;
               break;
            case DRAWCHAR:
               drawchar(t[1].i);
               t += 2;
               break;
            case DRAWSTR:
               drawstr((char *)&t[1]);
               t += 2 + strlen((char *)&t[1]) / sizeof(Token);
               break;
            case FIXEDWIDTH:
               fixedwidth(t[1].i);
               t += 2;
               break;
            case VFONT:
               font((char *)&t[1]);
               t += 2 + strlen((char *)&t[1]) / sizeof(Token);
               break;
            case HATCHANG:
               vdevice.attr->a.hatchcos = t[1].f;
               vdevice.attr->a.hatchsin = t[2].f;
               t += 3;
               break;
            case HATCHPITCH:
               vdevice.attr->a.hatchpitch = t[1].f;
               t += 2;
               break;
            case LOADMATRIX:
               m = (float *)vdevice.transmat->m;
               for (i = 0; i < 16; i++)
                  *m++ = (++t)->f;

                  vdevice.cpVvalid = 0;           /* may have changed mapping from world to device coords */
               t++;

               break;
            case MAPCOLOR:
               mapcolor(t[1].i, t[2].i, t[3].i, t[4].i);
               t += 5;
               break;
            case MOVE:
               move(t[1].f, t[2].f, t[3].f);
               t += 4;
               break;
            case MULTMATRIX:
               m = (float *)tmpmat;
               for (i = 0; i < 16; i++)
                  *m++ = (++t)->f;

                  mult4x4(prod, tmpmat, vdevice.transmat->m);
               loadmatrix(prod);
               t++;
               break;
            case POSTMULTMATRIX:
               m = (float *)tmpmat;
               for (i = 0; i < 16; i++)
                  *m++ = (++t)->f;

                  postmultmatrix(tmpmat);
               t++;
               break;
            case POLY:
               polyobj(t[1].i, &t[2]);
               t += 2 + 3 * t[1].i;
               break;
            case POLYFILL:
               polyfill(t[1].i);
               t += 2;
               break;
            case POLYHATCH:
               polyhatch(t[1].i);
               t += 2;
               break;
            case POPATTRIBUTES:
               popattributes();
               t++;
               break;
            case POPMATRIX:
               popmatrix();
               t++;
               break;
            case POPVIEWPORT:
               popviewport();
               t++;
               break;
            case POPPOS:
               poppos();
               t++;
               break;
            case PUSHATTRIBUTES:
               pushattributes();
               t++;
               break;
            case PUSHMATRIX:
               pushmatrix();
               t++;
               break;
            case PUSHVIEWPORT:
               pushviewport();
               t++;
               break;
            case PUSHPOS:
               pushpos();
               t++;
               break;
            case CALCVIEWPORT:
               calcviewport();
               t++;
               break;
            case RCURVE:
               i = (++t)->i;
               cx = (++t)->f;
               cy = (++t)->f;
               cz = (++t)->f;
               m = (float *)tmpmat;
               for (j = 0; j < 16; j++)
                  *m++ = (++t)->f;
                  mult4x4(prod, tmpmat, vdevice.transmat->m);
               drcurve(i, prod);
               vdevice.pos->cpW[V_X] = cx;
               vdevice.pos->cpW[V_Y] = cy;
               vdevice.pos->cpW[V_Z] = cz;
               t++;
               break;
            case RPATCH:
               pt = t + 10;
               cx = (++t)->f;
               cy = (++t)->f;
               cz = (++t)->f;
               for (i = 0; i < 4; i++)
               for (j = 0; j < 4; j++)
               {
                  S[0][i][j] = (pt++)->f;
                  S[1][i][j] = (pt++)->f;
                  S[2][i][j] = (pt++)->f;
                  S[3][i][j] = (pt++)->f;
               }

               transformtensor(S, vdevice.transmat->m);
               drpatch(S, t[1].i, t[2].i, t[3].i, t[4].i, t[5].i, t[6].i);

               vdevice.pos->cpW[V_X] = cx;
               vdevice.pos->cpW[V_Y] = cy;
               vdevice.pos->cpW[V_Z] = cz;
               t = pt;
               break;
            case SECTOR:
               inpoly = vdevice.inpolygon;

               if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !inpoly)
                  makepoly();

                  doarc(t[1].f, t[2].f, t[3].f, t[4].f, t[5].f, t[6].f, t[7].i);
               draw(t[1].f, t[2].f, 0.0);

               if ((vdevice.attr->a.fill || vdevice.attr->a.hatch) && !inpoly)
                  closepoly();
               else
                  draw2(t[1].f + t[3].f, t[2].f + t[4].f);
               t += 8;
               break;
            case TEXTANG:
               vdevice.attr->a.textcos = t[1].f;
               vdevice.attr->a.textsin = t[2].f;
               t += 3;
               break;
            case TEXTSIZE:
               textsize(t[1].f, t[2].f);
               t += 3;
               break;
            case VIEWPORT:
               viewport(t[1].f, t[2].f, t[3].f, t[4].f);
               t += 5;
               break;
            case TRANSLATE:
               translate(t[1].f, t[2].f, t[3].f);
               t += 4;
               break;
            case SCALE:
               /*
                * Do the operations directly on the top matrix of
                * the stack to speed things up.
                */

               vdevice.transmat->m[0][0] *= t[1].f;
               vdevice.transmat->m[0][1] *= t[1].f;
               vdevice.transmat->m[0][2] *= t[1].f;
               vdevice.transmat->m[0][3] *= t[1].f;

               vdevice.transmat->m[1][0] *= t[2].f;
               vdevice.transmat->m[1][1] *= t[2].f;
               vdevice.transmat->m[1][2] *= t[2].f;
               vdevice.transmat->m[1][3] *= t[2].f;

               vdevice.transmat->m[2][0] *= t[3].f;
               vdevice.transmat->m[2][1] *= t[3].f;
               vdevice.transmat->m[2][2] *= t[3].f;
               vdevice.transmat->m[2][3] *= t[3].f;

               t += 4;
               break;
            case ROTATE:
               rotate(t[1].f, (char)t[2].i);
               t += 3;
               break;
            case LINESTYLE:
               linestyle((char *)&t[1]);
               t += 2 + strlen((char *)&t[1]) / sizeof(Token);
               break;
            case LINEWIDTH:
               linewidth(t[1].i);
               t += 2;
               break;
            case SETDASH:
               setdash(t[1].f);
               t += 2;
               break;
            default:
               verror(0, "vogle: internal error in callobj");
               exit(1);
         }
      }
   }

   if (sync)
   {
      vdevice.sync = 1;
      (*vdevice.dev.Vsync)();
   }
}

/*
 * isobj
 *
 *	returns 1 if there is an object n, 0 otherwise.
 */
int
isobj(int n)
{
   Object	*o;

   if (n < 0)
      return(0);	/* Not an error, but not an object */

   for (o = object_table[n % MAXENTS]; o != (Object *)NULL; o = o->next)
   {
      if (o->obno == n)
      {
         break;
      }
   }
   
   return(o != (Object *)NULL);
}
