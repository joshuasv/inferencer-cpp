all: build

configure:
	mkdir -p build
	cd build && cmake ..

build: configure
	$(MAKE) -C build

run: build
	./build/inferencer-cpp $(MODEL_FPATH) $(SOURCE)

debug:
	mkdir -p buildDebug
	cd buildDebug && cmake -DCMAKE_BUILD_TYPE=Debug ..
	$(MAKE) -C buildDebug

clean:
	rm -rf build
	rm  -rf buildDebug

.PHONY: all configure build run clean