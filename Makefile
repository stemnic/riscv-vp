NPROCS:=$(shell sysctl -n hw.ncpu)

vps: vp/src/core/common/gdb-mc/libgdb/mpc/mpc.c vp/dependencies/systemc-dist vp/dependencies/softfloat-dist vp/build/Makefile
	make install -C vp/build -j$(NPROCS)

vp/dependencies/systemc-dist:
	cd vp/dependencies/ && ./build_systemc_233.sh

vp/dependencies/softfloat-dist:
	cd vp/dependencies/ && ./build_softfloat.sh
	
vp/src/core/common/gdb-mc/libgdb/mpc/mpc.c:
	git submodule update --init vp/src/core/common/gdb-mc/libgdb/mpc

all: vps vp-display vp-breadboard

vp/build/Makefile:
	mkdir vp/build || true
	cd vp/build && cmake ..

vp-eclipse:
	mkdir vp-eclipse || true
	cd vp-eclipse && cmake ../vp/ -G "Eclipse CDT4 - Unix Makefiles"

env/basic/vp-display/build/Makefile:
	mkdir env/basic/vp-display/build || true
	cd env/basic/vp-display/build && cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/qt@5/5.15.2/

vp-display: env/basic/vp-display/build/Makefile
	make -C  env/basic/vp-display/build -j$(NPROCS)

env/hifive/vp-breadboard/build/Makefile:
	mkdir env/hifive/vp-breadboard/build || true
	cd env/hifive/vp-breadboard/build && cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/qt@5/5.15.2/

vp-breadboard: env/hifive/vp-breadboard/build/Makefile
	make -C  env/hifive/vp-breadboard/build -j$(NPROCS)

vp-clean:
	rm -rf vp/build

qt-clean:
	rm -rf env/basic/vp-display/build
	rm -rf env/hifive/vp-breadboard/build

sysc-clean:
	rm -rf vp/dependencies/systemc*

softfloat-clean:
	rm -rf vp/dependencies/softfloat-dist

clean-all: vp-clean qt-clean sysc-clean softfloat-clean

clean: vp-clean

codestyle:
	find . -type d \( -name .git -o -name dependencies \) -prune -o -name '*.h' -o -name '*.hpp' -o -name '*.cpp' -print | xargs clang-format -i -style=file
