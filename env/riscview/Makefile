default: build

Makefile-riscview.qmake: riscview.pro
	qmake-qt5 riscview.pro -o Makefile-riscview.qmake

build: Makefile-riscview.qmake
	make -f Makefile-riscview.qmake

show: build
	./riscview

.PHONY: clean
clean:
	make -f Makefile-riscview.qmake clean
	rm -f Makefile-riscview.qmake