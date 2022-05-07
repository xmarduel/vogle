
#include "vogle.h"

static	int	px = -1, py = -1, pxs = -1, pys = -1;

/*
 * prefposition
 *
 *	Specify a prefered position for a window that is
 *	under control of a window manager.
 *	Position is the location of the upper left corner.
 *	Should be called before vinit.
 */
void
prefposition(int x, int y)
{
   if (x < 0 || y < 0)
   {
      px = py = -1;
   }
   else
   {
      px = x;
      py = y;
   }
}

/*
 * prefsize
 *
 *	Specify the prefered size for a window under control of
 *	a window manager.
 *	Should be called before vinit.
 */
void
prefsize(int x, int y)
{
   if (x < 0 || y < 0)
   {
      pxs = pys = -1;
   }
   else
   {
      pxs = x;
      pys = y;
   }
}

/*
 * getprefposandsize
 *
 *	Returns the prefered position and size of a window under
 *	control of a window manager. (-1 for unset parameters)
 */
void
getprefposandsize(int *x, int *y, int * xs, int *ys)
{
   *x = px;
   *y = py;
   *xs = pxs;
   *ys = pys;
}

