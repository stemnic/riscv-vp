all : main.c entry.S
	riscv32-unknown-elf-gcc main.c entry.S -o main -Wl,-e_reset_vector
	
sim: all
	riscv-vp main --intercept-syscalls
	
dump-elf: all
	riscv32-unknown-elf-readelf -a main
	
dump-code: all
	riscv32-unknown-elf-objdump -D main
	
clean:
	rm -f main
