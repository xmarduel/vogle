#include <stdlib.h>
#include <string.h>
#include "vogle.h"

static int      (*fverrorhandlerfunc)(int *xerrno, const char *msg, int len) = NULL;
static int      (*verrorhandlerfunc)(int xerrno, const char *msg) = NULL;

static int	verrno = 0, verr_ignore = 0;

static char	*errmsgs[] = {
	"",
	"vogle not initialised",
	"stack underflow",
	"stack overflow",
	"number of segments <= 0",
	"not enough points in geometry matrix",
	"unable to open file",
	"no font loaded",
	"no software font loaded",
	"number of patch curves <= 0",
	"impossible error in the clipping routine",
	"illegal axis of rotation",
	"can't have zero aspect ratio!",
	"bad clipping plane specification",
	"bad field of view",
	"no device, or error opening device",
	"malloc returns NULL",
	"not in an object",
	"too many vertices",
	"bad coordinate specified",
	"bad vector specified",
	"bad viewport specified",
	"bad window specified",
	"attempt to access an invalid window id",
	"misc driver specific error",
	"value out of range"
};

/*
 * vgeterrno
 *	Returns the current errno.
 */
int
vgeterrno(void)
{
   return(verrno);
}

/*
 * verrignore
 *	What do we do on errors? Just return, or do all the other stuff?
 */
void
verrignore(int yes)
{
   verr_ignore = yes;
}

/*
 * vgeterrtext
 *
 * 	Get the text that corresponds to a verrno
 */
char *
vgeterrtext(int xerrno, char *buf)
{
   if (xerrno < 0 || xerrno >= MAXERRS)
      verror(VERR_RANGE, "vgeterrtext");

   strcpy(buf, errmsgs[xerrno]);

   return(buf);
}

/*
 * verror
 *
 *
 */
int
verror(int xerrno, const char *str)
{
   int     ret = -1;
   int	len;
   char	*s;

   verrno = xerrno;

   /*
    * If we said to ignore fatal errors then
    * Just return.
    */

   if (verr_ignore)
      return(-1);

   if (xerrno < 0 || xerrno >= MAXERRS)
      xerrno = 0;

   len = strlen(str) + strlen(errmsgs[xerrno]) + 3;
   s = (char *)vallocate(len);
   strcpy(s, str);
   strcat(s, ": ");
   strcat(s, errmsgs[xerrno]);

   if (verrorhandlerfunc != NULL)
      ret = verrorhandlerfunc(xerrno, s);
   else if (fverrorhandlerfunc != NULL)	/* fortran interface */
      ret = fverrorhandlerfunc(&xerrno, s, len);

   if (ret == -1)
   {
      verrignore(1);
      if (vdevice.initialised && vdevice.curwin >= 0 && vdevice.wins[vdevice.curwin].id != -1)
         vexit();

      fprintf(stderr, "%s\n", s);
      /*
       * This is to keep the behaviour the same as the old vogle
       */
      if (xerrno == VERR_BADDEV)
         vlistdevs();
      exit(1);
   }

   free(s);

   return(ret);
}

/*
 * verrorhandler (C interface).
 * Sets the user defined error function to be called.
 */
void
verrorhandler(int (*f)(int en, const char *m))
{
   verrorhandlerfunc = f;
   fverrorhandlerfunc = NULL;
}

/*
 * verrorhandler_ (Fortran interface).
 * Sets the user defined error function to be called.
 */
void
verrorhandler_(int (*f)(int *en, const char *m, int len))
{
   fverrorhandlerfunc = f;
   verrorhandlerfunc = NULL;
}
