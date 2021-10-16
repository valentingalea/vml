### @file test/compile-tests/Makefile
DIR = test/compile-tests
ROOT = ../..

include $(ROOT)/Makefile.template

all::
test::
	@for f in testcases/*.cpp; do \
		echo -n "`basename $$f`: " && \
		$(CXX) $(CXXFLAGS) -o ./a.out $$f $(LIBRARIES) && \
		./a.out && rm ./a.out || exit 1; \
	done && echo "All test cases PASSED" || (echo "Tests FAILED" && exit 1)

clean::
	rm -f a.out
