#include $(ROOT)/usr/include/make/commondefs


CFLAGS= -I. 
LDLIBS= -ldmedia
LIBSRC=decode.c decodeScan.s
INCLUDES=decode.h decodeP.h
LIBOBJS=decodeScan.o decode.o

LIB=libjpeglld.a
SRC= decodeTest.c
OBJS= decodeTest.o

default: $(LIB) test



$(LIB): $(LIBOBJS)
	rm -f $(LIB)
	ar q $(LIB) $(LIBOBJS)

$(LIBOBJS): $(LIBSRC) $(INCLUDES)

test: jDecode
	
decodeScan.o : decodeScan.s
	as  decodeScan.s

jDecode: $(LIB) $(OBJS)
	cc -o jDecode $(OBJS) $(LDLIBS) $(LIB)

clean:
	rm *.o

