CC=g++
CFLAGS=-std=c++14 -g2

all: cli-server cli-client

#gpiocommon.o: gpiocommon.cpp
#	$(CC) $(CFLAGS) -c gpiocommon.cpp


%.o : %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

cli-client: connector-client.o nlvhandler.o elegantEnums.o cli-client.cpp
	$(CC) $(CFLAGS) -o cli-client cli-client.cpp connector-client.o nlvhandler.o elegantEnums.o 

cli-server: connector-server.o cli-server.cpp
	$(CC) $(CFLAGS) -o cli-server cli-server.cpp connector-server.o -lpthread

clean:
	rm -rf *.o
	rm -rf cli-client cli-server