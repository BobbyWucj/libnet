 #!/bin/bash

# set -x

if [ $# -gt 1 ]; then
    echo "Invalid number of parameters"
    echo "Usage: $0 [Debug/Release/clean]"
    exit 1 # Exit with error code 1
fi

BUILD_TYPE="Debug"
SOURCE_DIR=$(cd `dirname $0`; pwd)

if [ $# -eq 1 ]; then
    if [ $1 = "Debug" ] || [ $1 = "Release" ]; then
        BUILD_TYPE=$1
    elif [ $1 = "clean" ]; then
        sudo rm -rf $SOURCE_DIR/build
        exit 0
    else
        echo "Invalid parameter $1"
        echo "Usage: $0 [Debug/Release/clean]"
        exit 1 # Exit with error code 1
    fi
fi

BUILD_DIR=${SOURCE_DIR}/build/${BUILD_TYPE}
mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_INSTALL_PREFIX=$BUILD_DIR \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
      $SOURCE_DIR \
make && make install

# Default : Build Debug & Release together
if [ $# -eq 0 ]; then
    cd $SOURCE_DIR
    BUILD_TYPE="Release"
    BUILD_DIR=${SOURCE_DIR}/build/${BUILD_TYPE}
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR

    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$BUILD_DIR \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        $SOURCE_DIR \
    make && make install
fi