all : main.c sum.c bootstrap.S
	riscv64-unknown-elf-gcc main.c sum.c bootstrap.S -o main -march=rv32i -mabi=ilp32 -nostartfiles -Wl,--no-relax
	
sim: all
	riscv-vp --intercept-syscalls main
	
dump-elf: all
	riscv64-unknown-elf-readelf -a main
	
dump-code: all
	riscv64-unknown-elf-objdump -D main
	
dump-comment: all
	objdump -s --section .comment main
	
clean:
	rm -f main
