#!/bin/sh
PROGRAM="spur"

echo "### Cleaning..."

# clean demo web app
rm ./demo/bundle.js
rm ./demo/$PROGRAM.wasm

# clean release folder
mkdir -p release
rm ./release/Wrapper.js
rm ./release/$PROGRAM.js
rm ./release/$PROGRAM.wasm

# build
echo "\n### Building release build..."
emcc -o release/$PROGRAM.js \
    src/src/alt_component_analyzer.cpp \
    src/src/component_cache.cpp \
    src/src/component_management.cpp \
    src/src/instance.cpp \
    src/src/main.cpp \
    src/src/model_sampler.cpp \
    src/src/rand_distributions.cpp \
    src/src/solver.cpp \
    src/src/stack.cpp \
    src/src/statistics.cpp \
    src/src/component_types/base_packed_component.cpp \
    src/src/component_types/component_archetype.cpp \
    -I include/ -L lib/ -lgmpxx -lgmp \
    -std=c++11 \
    -O3 \
    -s ALLOW_MEMORY_GROWTH=1 -s INVOKE_RUN=0 -s FORCE_FILESYSTEM=1 -s EXIT_RUNTIME=1 -s MODULARIZE=1 -s 'EXPORT_NAME="$PROGRAM"' \
    --pre-js src_js/prerun.js 

cp ./src_js/Wrapper.js ./release/Wrapper.js

# optionally build and run demo web app
if [ -n "$1" -a "$1" = "demo" ]
then
	echo "\n### Building demo web app..."
	cp ./release/$PROGRAM.wasm ./demo/$PROGRAM.wasm
	browserify ./src_js/demo.js -o ./demo/bundle.js

	echo "\n### Running demo web app..."
	http-server ./demo
fi