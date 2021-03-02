CC=g++
CFLAGS=-std=c++14 -g2

all: i2cexample

i2cclient.o: i2cclient.cpp
	$(CC) $(CFLAGS) -c i2cclient.cpp

i2cexample.o: i2cexample.cpp
	$(CC) $(CFLAGS) -c i2cexample.cpp

i2cexample: i2cexample.o i2cclient.o
	$(CC) $(CFLAGS) -o i2cexample i2cexample.o i2cclient.o


clean:
	rm -rf *.o
	rm -rf i2cexample