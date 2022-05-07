#include "vogle.h"

/*
 * backbuffer
 *
 *	swap drawing to backbuffer - returns -1 if no
 * backbuffer is available.
 */
int
backbuffer(void)
{
   int	win;

   vdevice.sync = 0;

   win = vdevice.curwin;

   if ((*vdevice.dev.Vbackb)(vdevice.wins[win].devwin, vdevice.wins[win].havebackb, 0, 0) != -1)
   {
      vdevice.wins[win].havebackb = 1;
      vdevice.inbackbuffer = 1;
      return(1);
   }
   return(-1);
}

/*
 * frontbuffer
 *
 *	start drawing in the front buffer again. This
 * will always work!
 */
void
frontbuffer(void)
{
   (*vdevice.dev.Vfrontb)();

   vdevice.inbackbuffer = 0;
   vdevice.sync = 1;
}

/*
 * swapbuffers
 *
 *	swap the back and front buffers - returns -1 if
 * no backbuffer is available.
 */
int
swapbuffers(void)
{

   if (!vdevice.wins[vdevice.curwin].havebackb)
      return(-1);

   if (!vdevice.inbackbuffer)
      vdevice.inbackbuffer = 1;

   return((*vdevice.dev.Vswapb)());
}
