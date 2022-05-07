#include "vogle.h"

static  Pstack  *psfree = (Pstack *)NULL;

/*
 * getgp
 *
 *	return the current (x, y, z) graphics position
 */
void
getgp(float *x, float *y, float *z)
{
   *x = vdevice.pos->cpW[V_X];
   *y = vdevice.pos->cpW[V_Y];
   *z = vdevice.pos->cpW[V_Z];
}

/*
 * getgp2
 *
 *	return the current (x, y) graphics position
 */
void
getgp2(float *x, float *y)
{
   *x = vdevice.pos->cpW[V_X];
   *y = vdevice.pos->cpW[V_Y];
}

/*
* getgpt
 *
 *	return the current transformed graphics position.
 */
void
getgpt(float *x, float *y, float *z, float *w)
{
   multvector(vdevice.pos->cpWtrans, vdevice.pos->cpW, vdevice.transmat->m);

   *x = vdevice.pos->cpWtrans[V_X];
   *y = vdevice.pos->cpWtrans[V_Y];
   *z = vdevice.pos->cpWtrans[V_Z];
   *w = vdevice.pos->cpWtrans[V_W];
}

/*
 * sgetgp2
 *
 *	return the current (x, y) graphics position in screen coordinates
 */
void
sgetgp2(float *x, float *y)
{
   float	sx, sy;

   sx = vdevice.maxVx - vdevice.minVx;
   sy = vdevice.maxVy - vdevice.minVy;

   multvector(vdevice.pos->cpWtrans, vdevice.pos->cpW, vdevice.transmat->m);
   *x = 2.0 * WtoVx(vdevice.pos->cpWtrans) / sx - 1.0;
   *y = 2.0 * WtoVy(vdevice.pos->cpWtrans) / sy - 1.0;
}

/*
 * pushpos()
 *
 * Saves the current position on a stack
 */
void
pushpos(void)
{
   Pstack	*ptmp;
   Token	*p;

   if (vdevice.inobject)
   {
      p = newtokens(1);

      p->i = PUSHPOS;

      return;
   }

   if (psfree != (Pstack *)NULL)
   {
      ptmp = vdevice.pos;
      vdevice.pos = psfree;
      psfree = psfree->back;
      vdevice.pos->back = ptmp;
      copyvector(vdevice.pos->cpW, ptmp->cpW);
      copyvector(vdevice.pos->cpWtrans, ptmp->cpWtrans);
   }
   else
   {
      ptmp = (Pstack *)vallocate(sizeof(Pstack));
      ptmp->back = vdevice.pos;
      copyvector(ptmp->cpW, vdevice.pos->cpW);
      copyvector(ptmp->cpWtrans, vdevice.pos->cpWtrans);
      vdevice.pos = ptmp;
   }
}

/*
* poppos()
 *
 * Restores the last pushed position.
 */
void
poppos(void)
{
   Token	*p;
   Pstack	*oldtop;

   if (vdevice.inobject)
   {
      p = newtokens(1);

      p->i = POPPOS;

      return;
   }

   if (vdevice.pos->back == (Pstack *)NULL)
   {
      verror(VERR_STACKUF, "poppos");
   }
   else
   {
      oldtop = vdevice.pos;
      vdevice.pos = vdevice.pos->back;
      oldtop->back = psfree;
      psfree = oldtop;
   }

   vdevice.cpVvalid = 0;	/* For sure ! */
}
