all:
	mkdir -p build
	cd build ; cmake ../ ; make

install:
	cd build ; make install

test:
	cd build; make test

clean: 
	rm -r build include lib