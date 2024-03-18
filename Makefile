all: build

configure:
	mkdir -p build
	cd build && cmake ..

configure-debug:
	mkdir -p buildDebug
	cd buildDebug && cmake -DCMAKE_BUILD_TYPE=Debug ..

build: configure
	$(MAKE) -C build

build-debug: configure-debug
	$(MAKE) -C buildDebug

run: build
	./build/inferencer-cpp $(MODEL_FPATH) $(SOURCE)

run-debug: build-debug
	./buildDebug/inferencer-cpp $(MODEL_FPATH) $(SOURCE)

clean:
	rm -rf build
	rm  -rf buildDebug

.PHONY: all configure build run clean configure-debug build-debug run-debug