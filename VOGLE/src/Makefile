#
# Make the vogle library.
#
# Below are our defaults for compliers etc. These are set in the
# main Makefile.
#

MCFLAGS = -g

LIB = libvogle.a
SHAREDLIB = libvogle.so

RANLIB = ranlib		# = "ar ts" for SYS5

#DEVICES = DTEK -DHPGL -DPOSTSCRIPT -DPOSTSCRIPTP -DDXY
DEVICES = -DX11

FONTLIB=/usr/local/lib/hershey/

HDRS =	vogle.h

CSRC =	arcs.c aspect.c attr.c buffer.c clip.c curves.c draw.c drivers.c \
	viewing.c viewp.c mapping.c matrix.c move.c objects.c patches.c \
	points.c polygons.c rect.c tensor.c text.c newtoken.c trans.c \
	scale.c getstr.c getgp.c valloc.c yobbaray.c pref.c verror.c \		window.c

COBJS =	arcs.o aspect.o attr.o buffer.o clip.o curves.o draw.o drivers.o \
	viewing.o viewp.o mapping.o matrix.o move.o objects.o patches.o \
	points.o polygons.o rect.o tensor.o text.o newtoken.o trans.o \
	scale.o getstr.o getgp.o valloc.o yobbaray.o pref.o verror.o \
	window.o

#
#FOBJS =	sunfort/farcs.o sunfort/faspect.o sunfort/fattr.o sunfort/fcurves.o \
#	sunfort/fclip.o sunfort/fdraw.o sunfort/fdrivers.o sunfort/fgetgp.o  \
#	sunfort/fviewing.o sunfort/fviewp.o sunfort/fmatrix.o sunfort/fmove.o \
#	sunfort/fobjects.o sunfort/fpatches.o sunfort/fpoints.o sunfort/fpoly.o\
#	sunfort/frect.o sunfort/ftrans.o sunfort/fyobbaray.o sunfort/ftext.o \
#	sunfort/fgetstr.o sunfort/fbuffer.o sunfort/fpref.o sunfort/fwindow.o
#
#POBJS =	sunpas/parcs.o sunpas/paspect.o sunpas/pattr.o \
#	sunpas/pcurves.o sunpas/pclip.o sunpas/pdraw.o \
#	sunpas/pdrivers.o sunpas/pgetgp.o sunpas/pviewing.o \
#	sunpas/pviewp.o sunpas/pmatrix.o sunpas/pmove.o \
#	sunpas/pobjects.o sunpas/ppatches.o sunpas/ppoints.o \
#	sunpas/ppoly.o sunpas/prect.o sunpas/ptrans.o \
#	sunpas/pyobbaray.o sunpas/ptext.o sunpas/pgetstr.o \
#	sunpas/pbuffer.o sunpas/ppref.o
#
#
#  Where the fonts live
#
FONTLIB = /tmp/hfonts/

CARCH= -arch x86_64
CFLAGS= $(CARCH) $(DEVICES) -DFONTLIB=\"$(FONTLIB)\" $(MCFLAGS)
LINTFLAGS=


all: $(LIB) #$(SHAREDLIB)

$(LIB): $(DOBJS) $(COBJS)  vogle.h
	rm -f $@
	ar rcv $@ $(DOBJS) $(COBJS)
	$(RANLIB) $@

$(SHAREDLIB): $(DOBJS) $(COBJS)  vogle.h
	rm -f $@
	ld -shared -o $@ $(DOBJS) $(COBJS)
	$(RANLIB) $@

lint:
	lint -hac $(DEVICES) $(CSRC) sunfort/*.c sunpas/*.c > vogle.lint

clean:
	rm -f $(COBJS) core
	cd sunfort; make clean
	cd sunpas; make clean
	cd ../drivers; make clean

clobber:
	rm -f $(LIB) $(COBJS) $(FOBJS) core 
	cd sunfort; make clean
	cd sunpas; make clean

$(COBJS): vogle.h



#
$(DOBJS): vogle.h
	cd ../drivers; make -f Makefile \
				CC="$(CC)" \
				MCFLAGS="$(MCFLAGS)" \
				DOBJS="$(DOBJS)"

