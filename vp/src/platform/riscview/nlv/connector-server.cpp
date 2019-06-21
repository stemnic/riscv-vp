/*
 * connector-server.cpp
 *
 *  Created on: 21 Jun 2019
 *      Author: dwd
 */


#include "connector-server.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <thread>

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

NLVConnectorServer::NLVConnectorServer() : fd(-1), stop(false), fun(nullptr) {};
NLVConnectorServer::~NLVConnectorServer(){
	if (fd >= 0) {
		std::cout << "closing nlv-connector-server socket " << fd << std::endl;
		close(fd);
		fd = -1;
	}
}

bool NLVConnectorServer::setupConnection(const char *port) {
	struct addrinfo hints, *servinfo, *p;
	int yes = 1;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;  // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("gpio-server: socket");
			continue;
		}

		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			return false;
		}

		if (bind(fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(fd);
			perror("gpio-server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);  // all done with this structure

	if (p == NULL) {
		fprintf(stderr, "gpio-server: failed to bind\n");
		return false;
	}

	return true;
}

void NLVConnectorServer::quit() {
	stop = true;
}

bool NLVConnectorServer::isStopped() {
	return stop;
}

void NLVConnectorServer::registerInput(std::function<void(const char* command)> fun) {
	this->fun = fun;
}

void NLVConnectorServer::startListening() {
	if (listen(fd, 1) == -1) {
		std::cerr << "nlv-connector: fd " << fd << " ";
		perror("listen");
		stop = true;
		return;
	}
	// printf("gpio-server: accepting connections (%d)\n", fd);

	struct sockaddr_storage their_addr;  // connector's address information
	socklen_t sin_size = sizeof their_addr;
	char s[INET6_ADDRSTRLEN];

	while (!stop)  // this would block a bit
	{
		int new_fd = accept(fd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd < 0) {
			std::cerr << "nlv-connector-server accept return " << new_fd << std::endl;
			perror("accept");
			stop = true;
			return;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("gpio-server: got connection from %s\n", s);
		handleConnection(new_fd);
	}
}


void NLVConnectorServer::handleConnection(int conn) {
	static char buffer[10000]; //This should not overflow, probably.
	int bytes;
	//TODO: Perhaps prepend two bytes for "size"
	while ((bytes = read(conn, buffer, sizeof(buffer))) > 0) {
		fun(buffer);
	}
	std::cout << "gpio-client disconnected. (" << bytes << ")" << std::endl;
	close(conn);
}
