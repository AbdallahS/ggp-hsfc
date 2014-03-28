all: release

debug: 
	mkdir -p build
	cd build ; cmake -DCMAKE_BUILD_TYPE=Debug ../ ; make

release: 
	mkdir -p build
	cd build ; cmake -DCMAKE_BUILD_TYPE=Release ../ ; make

install:
	cd build ; make install

test:
	cd build; ctest

clean: 
	rm -r build include lib
