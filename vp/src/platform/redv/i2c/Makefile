CC=g++
CFLAGS=-g -std=c++14 

all: i2cexample bmp280client

i2cclient.o: i2cclient.cpp
	$(CC) $(CFLAGS) -c i2cclient.cpp

i2cexample.o: i2cexample.cpp
	$(CC) $(CFLAGS) -c i2cexample.cpp

i2cexample: i2cexample.o i2cclient.o
	$(CC) $(CFLAGS) -o i2cexample i2cexample.o i2cclient.o

bmp280client.o: bmp280client.cpp
	$(CC) $(CFLAGS) -c bmp280client.cpp

bmp280client: bmp280client.o i2cclient.o
	$(CC) $(CFLAGS) -o bmp280client bmp280client.o i2cclient.o

clean:
	rm -rf *.o
	rm -rf i2cexample
	rm -rf bmp280client
