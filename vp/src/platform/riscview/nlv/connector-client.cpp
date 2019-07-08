/*
 * connector-client.cpp
 *
 *  Created on: 21 Jun 2019
 *      Author: dwd
 */

#include "connector-client.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

NLVConnectorClient::NLVConnectorClient(): fd(-1){};
NLVConnectorClient::~NLVConnectorClient(){
	if (fd >= 0) {
		close(fd);
	}
}

bool NLVConnectorClient::setupConnection(const char *host, const char *port) {
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return false;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(fd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		fd = -1;
		return false;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);  // all done with this structure

	return true;
}

bool NLVConnectorClient::command(const char* command){
	if(fd < 0)
		return false;

	uint16_t cmdLen = strlen(command);

	if (write(fd, &cmdLen, sizeof(uint16_t)) != sizeof(uint16_t)) {
		std::cerr << "nlv-connector: Error in write (header)" << std::endl;
		return false;
	}
	if (write(fd, command, cmdLen) != cmdLen) {
		std::cerr << "nlv-connector: Error in write (body)" << std::endl;
		return false;
	}
	return true;
}
