#!/bin/sh

NPROCS=$(sysctl -n hw.ncpu)

cd "${0%/*}"

DIR="$(pwd)"
PREFIX=$DIR/systemc-dist

version=2.3.3
source=systemc-$version.tar.gz

if [ ! -f "$source" ]; then
	wget http://www.accellera.org/images/downloads/standards/systemc/$source
fi

tar xzf $source
cd systemc-$version
mkdir -p build && cd build
#../configure CXXFLAGS='-std=c++14' --prefix=$PREFIX --with-arch-suffix=
cmake .. -DENABLE_PTHREADS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=14
make -j$NPROCS
sudo make install

cd $DIR
