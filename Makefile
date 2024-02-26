all: build

configure:
	mkdir -p build
	cd build && cmake ..

build: configure
	$(MAKE) -C build

run: build
	./build/inferencer-cpp

clean:
	rm -rf build

.PHONY: all configure build run clean