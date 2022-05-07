

CEX1 = cex1
CEX2 = cex2
CEX3 = cex3
CEX1TRANSIENT = cex1transient

CEXCONTOUR = cexcontour

CFLAGS = -I../SRC -I../../VOGLE/src
CFLAGS_MESCHACH = -I../../.. 

LDFLAGS = -L/Users/xavier/x_local/lib  -lvopl-2.0 -lvogle-2.0
LDFLAGS_MESCHACH = -L/Users/xavier/x_local/lib  -lmeschach_adds -lmeschach

$(CEX1).o: $(CEX1).c
	$(CC) $(CFLAGS) -c $(CEX1).c
$(CEX1): $(CEX1).o
	$(CC) -o $(CEX1) $(LDFLAGS) $(CEX1).o

$(CEX2).o: $(CEX2).c
	$(CC) $(CFLAGS) -c $(CEX2).c
$(CEX2): $(CEX2).o
	$(CC) -o $(CEX2) $(LDFLAGS) $(CEX2).o

$(CEX3).o: $(CEX3).c
	$(CC) $(CFLAGS) -c $(CEX3).c
$(CEX3): $(CEX3).o
	$(CC) -o $(CEX3) $(LDFLAGS) $(CEX3).o

$(CEX1TRANSIENT).o: $(CEX1TRANSIENT).c
	$(CC) $(CFLAGS) -c $(CEX1TRANSIENT).c
$(CEX1TRANSIENT): $(CEX1TRANSIENT).o
	$(CC) -o $(CEX1TRANSIENT) $(LDFLAGS) $(CEX1TRANSIENT).o
	
$(CEXCONTOUR).o: $(CEXCONTOUR).c
	$(CC) $(CFLAGS) $(CFLAGS_MESCHACH)-c $(CEXCONTOUR).c
$(CEXCONTOUR): $(CEXCONTOUR).o
	$(CC) -o $(CEXCONTOUR) $(CEXCONTOUR).o $(LDFLAGS) $(LDFLAGS_MESCHACH)

all: $(CEX1) $(CEX2) $(CEX3) $(CEX1TRANSIENT) $(CEXCONTOUR)

clean::
	rm -f *.o $(CEX1) $(CEX2) $(CEX3) $(CEX1TRANSIENT) $(CEXCONTOUR)
