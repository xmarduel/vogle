#
LIBS = -lvogle -lX11

SRCS = gpp.c read.c legend.c doargs.c

OBJS = gpp.o read.o legend.o doargs.o

MCFLAGS = -fsingle -f68881 -g

DEFDEV = -DX11

CFLAGS = $(DEFDEV) $(MCFLAGS) -I../src

all:	gpp

gpp:     $(OBJS) gpp.h ../src/libvopl.a
	cc $(CFLAGS) -o gpp $(OBJS) ../src/libvopl.a $(LIBS) -lm
	
$(OBJS): gpp.h

lint: $(SRCS)
	lint -I../vogle/src $(SRCS) > gpp.lint

clean:
	rm -f *.o core

clobber:
	rm -f gpp *.o core

