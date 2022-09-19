all: build

build:
	mkdir -p build
	clang src/*.c -o build/sorting-algorithms

install:
	[ -f build/sorting-algorithms ] || (echo Please build first; exit 1)
	sudo cp build/sorting-algorithms /usr/bin/sorting-algorithms

uninstall: 
	sudo rm -f /usr/bin/sorting-algorithms
