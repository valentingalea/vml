### @file Makefile
DIR = .
ROOT = .
DIST = ../threadpool-0.2

include $(ROOT)/Makefile.template

all::
	cd lib && make $@

test::
	cd test && make $@

examples::
	cd examples && make test

clean::
	cd lib && make clean
	cd test && make clean
	cd examples && make clean
	find . -name '*~' | xargs -r rm

dist:: # Make dist directory. Overwrite existing dist if it looks like ours.
	test \! -d $(DIST) -o -f $(DIST)/README.md -a -f $(DIST)/Doxyfile
	rm -rf $(DIST)
	mkdir $(DIST)
	tar cf - . | (cd $(DIST) && tar xf -)
	find $(DIST) -name CVS -o -name '~*' -o -name '*.a' -o -name '*~' -o -name '*.o' -o -name 'a.out' -o -name '*.exe' | xargs -r rm -rf
	sed -e '/##### END OF DIST/,$$d' $(DIST)/Makefile.template > $(DIST)/Makefile.template.new && mv $(DIST)/Makefile.template.new $(DIST)/Makefile.template
	sed -e 's/THREADPOOL_USE_LIBRARY *1/THREADPOOL_USE_LIBRARY 0/' $(DIST)/include/threadpool/threadpool_config.h > $(DIST)/include/threadpool/threadpool_config.h.new && mv $(DIST)/include/threadpool/threadpool_config.h.new $(DIST)/include/threadpool/threadpool_config.h
	cd .. && d=`echo "$(DIST)" | sed -e 's/^\.\.\///'` && zip -D -r $$d.zip $$d

doc::
	doxygen
