DEFAULT_BUILD = release
ifeq ($(wildcard build/Makefile),)
	build = $(DEFAULT_BUILD)
else
	build = existing
endif

default: $(build)

existing:
	cd build; make

debug: 
	mkdir -p build
	cd build ; rm -rf * ; cmake -DCMAKE_BUILD_TYPE=Debug ../ ; make

release: 
	mkdir -p build
	cd build ; rm -rf * ; cmake -DCMAKE_BUILD_TYPE=Release ../ ; make

install: $(build)
	cd build; make install

test:
	cd build; ctest
#	cd build; ctest -V

clean: 
	rm -r build include lib
