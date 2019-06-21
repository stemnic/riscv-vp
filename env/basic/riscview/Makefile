default: build

Makefile-riscview.qmake:
	qmake-qt5 riscview.pro -o Makefile-riscview.qmake

build: Makefile-riscview.qmake
	make -f Makefile-riscview.qmake

.PHONY: clean
clean:
	make -f Makefile-riscview.qmake clean
	rm -f Makefile-riscview.qmake