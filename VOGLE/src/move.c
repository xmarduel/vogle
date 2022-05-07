#include "vogle.h"

/*
 * move
 *
 * Move the logical graphics position to the world coordinates x, y, z.
 *
 */
void
move(float x, float y, float z)
{
   Token	*p;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "move");

   vdevice.pos->cpW[V_X] = x;
   vdevice.pos->cpW[V_Y] = y;
   vdevice.pos->cpW[V_Z] = z;

   vdevice.cpVvalid = 0;

   if (vdevice.inpolygon)
   {
      (*vdevice.pmove)(x, y, z);
      return;
   }

   if (vdevice.inobject)
   {
      p = newtokens(4);

      p[0].i = MOVE;
      p[1].f = x;
      p[2].f = y;
      p[3].f = z;

      return;
   }

   if (vdevice.clipoff)
   {		/* update device coords as well */
      multvector(vdevice.pos->cpWtrans, vdevice.pos->cpW, vdevice.transmat->m);
      vdevice.cpVx = WtoVx(vdevice.pos->cpWtrans);
      vdevice.cpVy = WtoVy(vdevice.pos->cpWtrans);
   }
}

/*
 * move2
 *
 * Move the logical graphics position to the world coords x, y, 0.0
 * (I.e. a 2D move is defined as a 3D move with the Z-coord set to zero)
 *
 */
void
move2(float x, float y)
{
   move(x, y, 0.0);
}

/*
 * rmove
 *
 * move the logical graphics position from the current world
 * coordinates by dx, dy, dz
 *
 */
void
rmove(float dx, float dy, float dz)
{
   move((vdevice.pos->cpW[V_X] + dx), (vdevice.pos->cpW[V_Y] + dy), (vdevice.pos->cpW[V_Z] + dz));
}

/*
 * rmove2
 *
 * Move Relative in 2D.
 *
 */
void
rmove2(float dx, float dy)
{
   move((vdevice.pos->cpW[V_X] + dx), (vdevice.pos->cpW[V_Y] + dy), 0.0);
}

/*
 * smove2
 *
 * Move directly as a fraction of the screen size.
 */
void
smove2(float xs, float ys)
{
   if (!vdevice.initialised)
      verror(VERR_UNINIT, "smove2");

   vdevice.cpVx = (xs / 2 + 0.5) * vdevice.dev.sizeX;
   vdevice.cpVy = (0.5 + ys / 2) * vdevice.dev.sizeY;
}

/*
 * rsmove2
 *
 * Relative move as a fraction of the screen size.
 */
void
rsmove2(float dxs, float dys)
{
   if (!vdevice.initialised)
      verror(VERR_UNINIT, "rsmove2");

   vdevice.cpVx += dxs / 2 * vdevice.dev.sizeX;
   vdevice.cpVy += dys / 2 * vdevice.dev.sizeY;
}
