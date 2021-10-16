### @file lib/Makefile
DIR = lib
ROOT = ..

include $(ROOT)/Makefile.template

LIBRARY = libthreadpool.a
OBJECTS = threadpool.o threadpool_generic.o
INC = $(INCLUDE)/threadpool

all::	$(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar r $@ $(OBJECTS)

threadpool.o: threadpool.cpp \
		$(INC)/impl/threadpool_interface_virtual.h \
		$(INC)/threadpool.h \
		$(INC)/impl/threadpool_impl.h \
		$(INC)/impl/threadpool_inst.h \
		$(INC)/impl/threadpool_impl_util.h \
		$(INC)/impl/threadpool_impl_homogenous.h \
		$(INC)/impl/threadpool_interface_generic.h \
		$(INC)/impl/threadpool_generic.h

threadpool_generic.o: threadpool_generic.cpp \
		$(INC)/impl/threadpool_interface_generic.h \
		$(INC)/impl/threadpool_generic.h \
		$(INC)/impl/threadpool_generic_impl.h \
		$(INC)/impl/threadpool_generic_inst.h

clean::
	rm -f $(LIBRARY) $(OBJECTS)
