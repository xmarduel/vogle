#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vogle.h"

static	Astack	*asfree = (Astack *)NULL;

/*
 * copyattributes
 *
 *	Copies attribute stack entries from b to a
 */
static	void
copyattributes(Attribute *a, Attribute *b)
{
   if (b->style)
   {
      if (a->style)
         free(a->style);

      a->style = (char *)vallocate(strlen(b->style) + 1);
      strcpy(a->style, b->style);
   }
   else
   {
      a->style = NULL;
   }
   
   a->dashp	= b->dashp;
   a->dash	= b->dash;
   a->adist	= b->adist;
   a->color	= b->color;
   a->fill    	= b->fill;
   a->hatch 	= b->hatch;
   a->textcos 	= b->textcos;
   a->textsin 	= b->textsin;
   a->hatchcos 	= b->hatchcos;
   a->hatchsin 	= b->hatchsin;
   a->hatchpitch = b->hatchpitch;
   a->justify 	= b->justify;
   a->skew 	= b->skew;
   a->bold	= b->bold;
   a->fixedwidth = b->fixedwidth;
   a->fontwidth	 = b->fontwidth;
   a->fontheight = b->fontheight;
   a->softtext	 = b->softtext;
   a->exvp	 = b->exvp;
   strcpy(a->font, b->font);
}

/*
 * pushattributes
 *
 * save the current attributes on the matrix stack
 *
 */
void
pushattributes(void)
{
   Astack	*nattr;
   Token	*p;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "pushattributes");

   if (vdevice.inobject)
   {
      p = newtokens(1);

      p[0].i = PUSHATTRIBUTES;

      return;
   }

   if (asfree != (Astack *)NULL)
   {
      nattr = vdevice.attr;
      vdevice.attr = asfree;
      asfree = asfree->back;
      vdevice.attr->back = nattr;
      copyattributes(&vdevice.attr->a, &nattr->a);
   }
   else
   {
      nattr = (Astack *)vallocate(sizeof(Astack));
      nattr->back = vdevice.attr;
      copyattributes(&nattr->a, &vdevice.attr->a);
      vdevice.attr = nattr;
   }
}

/*
 * popattributes
 *
 * pop the top entry on the attribute stack
 *
 */
void
popattributes(void)
{
   Astack	*nattr;
   Token	*p;

   if (!vdevice.initialised)
      verror(VERR_UNINIT, "popattributes");

   if (vdevice.inobject)
   {
      p = newtokens(1);

      p[0].i = POPATTRIBUTES;

      return;
   }

   if (vdevice.attr->back == (Astack *)NULL)
   {
      verror(VERR_STACKUF, "popattributes");
   }
   else
   {
      nattr = vdevice.attr;
      font(vdevice.attr->back->a.font);
      vdevice.attr = vdevice.attr->back;
      nattr->back = asfree;
      asfree = nattr;
      /* Must zap the contents of ones that are on the free list */
      if (nattr->a.style)
         free(nattr->a.style);
      memset(&nattr->a, 0, sizeof(Attribute));

   }

   /*
    * Restore some stuff...
    */
   color(vdevice.attr->a.color);

   if (vdevice.attr->a.softtext)
      textsize(vdevice.attr->a.fontwidth, vdevice.attr->a.fontheight);

   if (vdevice.attr->a.exvp)
      expandviewport();

}

#ifdef	DEBUG

printattribs(char *s)
{
   printf("%s\n", s);
   printf("clipoff    = %d\n", vdevice.clipoff);
   printf("color      = %d\n", vdevice.attr->a.color);
   printf("fill       = %d\n", vdevice.attr->a.fill);
   printf("hatch      = %d\n", vdevice.attr->a.hatch);
   printf("textcos    = %f\n", vdevice.attr->a.textcos);
   printf("textsin    = %f\n", vdevice.attr->a.textsin);
   printf("hatchcos   = %f\n", vdevice.attr->a.hatchcos);
   printf("hatchsin   = %f\n", vdevice.attr->a.hatchsin);
   printf("hatchpitch = %f\n", vdevice.attr->a.hatchpitch);
   printf("justify    = %d\n", vdevice.attr->a.justify);
   printf("skew       = %f\n", vdevice.attr->a.skew);
   printf("fixedwidth = %d\n", vdevice.attr->a.fixedwidth);
   printf("fontwidth  = %f\n", vdevice.attr->a.fontwidth);
   printf("fontwidth  = %f\n", vdevice.attr->a.fontheight);
   printf("font       = %s\n", vdevice.attr->a.font);
}

#endif
