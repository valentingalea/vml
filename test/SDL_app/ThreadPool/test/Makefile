### @file test/Makefile
DIR = test
ROOT = ..

include $(ROOT)/Makefile.template

INC = $(INCLUDE)/threadpool

all:: test

test:: threadpool_test.exe
	./threadpool_test.exe
	cd compile-tests && make test

threadpool_test.exe: threadpool_test.cpp \
		$(INC)/singlethreaded/threadpool.h \
		$(INC)/singlethreaded/parallel_transform.h \
		$(INC)/singlethreaded/parallel_for_each.h \
		$(INC)/threadpool.h \
		$(INC)/impl/threadpool_inst.h \
		$(INC)/impl/threadpool_impl.h \
		$(INC)/impl/threadpool_generic.h \
		$(INC)/impl/threadpool_generic_inst.h \
		$(INC)/impl/threadpool_generic_impl.h \
		$(INC)/impl/threadpool_interface_virtual.h \
		$(INC)/impl/threadpool_impl_homogenous.h \
		$(INC)/impl/threadpool_impl_util.h \
		$(INC)/impl/threadpool_interface_generic.h \
		$(INC)/parallel_transform.h \
		$(INC)/threadpool_config.h \
		$(INC)/parallel_for_each.h \
		$(INC)/make_iterator.h \
		$(LIB)/libthreadpool.a
	$(CXX) $(CXXFLAGS) -o $@ ${@:.exe=.cpp} $(LIBRARIES)

$(LIB)/libthreadpool.a: $(LIB)/threadpool_generic.o $(LIB)/threadpool.o
	cd $(LIB) && make libthreadpool.a

$(LIB)/threadpool_generic.o: $(LIB)/threadpool_generic.cpp \
		$(INC)/impl/threadpool_interface_generic.h \
		$(INC)/impl/threadpool_generic.h \
		$(INC)/impl/threadpool_generic_impl.h \
		$(INC)/impl/threadpool_generic_inst.h
	cd $(LIB) && make threadpool_generic.o

$(LIB)/threadpool.o: $(LIB)/threadpool.cpp \
		$(INC)/impl/threadpool_interface_virtual.h \
		$(INC)/threadpool.h \
		$(INC)/impl/threadpool_impl.h \
		$(INC)/impl/threadpool_inst.h \
		$(INC)/impl/threadpool_impl_util.h \
		$(INC)/impl/threadpool_impl_homogenous.h \
		$(INC)/impl/threadpool_interface_generic.h \
		$(INC)/impl/threadpool_generic.h
	cd $(LIB) && make threadpool.o


clean::
	rm -f threadpool_test.exe
	cd compile-tests && make $@
