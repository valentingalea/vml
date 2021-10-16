### @file examples/Makefile
DIR = examples
ROOT = ..

include $(ROOT)/Makefile.template

EXECUTABLES = \
	make_function_input_range_container.exe \
	make_function_input_range_for_loop.exe \
	make_function_input_range_transform.exe \
	make_function_output_iterator.exe \
	parallel_for_each_integral.exe \
	parallel_for_each_iterator.exe \
	parallel_for_each_vector.exe \
	parallel_transform_integral.exe \
	parallel_transform_iterator.exe \
	parallel_transform_vector.exe \
	threadpool_for_each.exe \
	threadpool_helloworld.exe \
	threadpool_helloworld_future.exe \
	threadpool_pass_by_value.exe

all:: $(EXECUTABLES)

test:: $(EXECUTABLES)
	@for f in $(EXECUTABLES); do \
		echo -n "`basename $$f`: " && ./$$f || exit 1; \
	done && echo "All test cases PASSED" || (echo "Test FAILED" && exit 1)

clean::
	rm -f $(EXECUTABLES)
