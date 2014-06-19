DEFAULT_BUILD = release

default: 
ifeq ($(wildcard build/Makefile),)
	make $(DEFAULT_BUILD)
else
	cd build; make
endif

debug: 
	mkdir -p build
	cd build ; rm -rf * ; cmake -DCMAKE_BUILD_TYPE=Debug ../ ; make

release: 
	mkdir -p build
	cd build ; rm -rf * ; cmake -DCMAKE_BUILD_TYPE=Release ../ ; make

install:
ifeq ($(wildcard build/Makefile),)
	make $(DEFAULT_BUILD)
	make install
else
	cd build; make install
endif

test:
	cd build; ctest

#	cd build; ctest -V

clean: 
	rm -r build include lib
