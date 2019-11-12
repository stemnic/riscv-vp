#include <err.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <systemc>

#include "gdb_server.h"
#include "protocol/protocol.h"

GDBServer::GDBServer(sc_core::sc_module_name name,
                     std::vector<debugable*> harts,
                     DebugMemoryInterface *mm,
                     uint16_t port) {
	prevpkt = NULL;
	create_sock(port);

	thr = std::thread(&GDBServer::serve, this);
	thr.detach();
}

void GDBServer::create_sock(uint16_t port) {
	struct sockaddr_in6 addr;
	int optval;

	sockfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sockfd == -1)
		throw std::system_error(errno, std::generic_category());

	optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1)
		goto err;

	optval = 0;
	if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval)) == -1)
		goto err;

	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_loopback;
	addr.sin6_port = htons(port);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		goto err;
	if (listen(sockfd, 0) == -1)
		goto err;

	return;
err:
	close(sockfd);
	throw std::system_error(errno, std::generic_category());
}

void GDBServer::writeall(int fd, char *data, size_t len) {
	ssize_t ret, w;

	w = 0;
	do {
		assert(len >= (size_t)w);
		ret = write(fd, &data[w], len - (size_t)w);
		if (ret < 0)
			throw std::system_error(errno, std::generic_category());

		w += ret;
	} while ((size_t)w < len);
}

void GDBServer::send_packet(int conn, std::string data) {
	char *serialized;

	serialized = gdb_serialize(GDB_KIND_PACKET, data.c_str());
	try {
		writeall(conn, serialized, strlen(serialized));
	} catch (const std::system_error& e) {
		warnx("writeall failed: %s", e.what());
		goto ret;
	}

	free(prevpkt);
	if (!(prevpkt = strdup(serialized))) {
		prevpkt = NULL;
		free(serialized);
		throw std::system_error(errno, std::generic_category());
	}

ret:
	free(serialized);
}

void GDBServer::retransmit(int conn) {
	if (!prevpkt)
		return;

	try {
		writeall(conn, prevpkt, strlen(prevpkt));
	} catch (const std::system_error& e) {
		warnx("writeall failed: %s", e.what());
	}

	/* memory for prevpkt is freed on next successfull
	 * packet transmit in the send_packet function */
}

void GDBServer::dispatch(FILE *stream) {
	gdb_packet_t *pkt;

	while ((pkt = gdb_parse(stream))) {
		printf("%s: received packet { kind: %d, data: '%s', csum: 0x%c%c }\n",
		       __func__, pkt->kind, (pkt->data) ? pkt->data : "", pkt->csum[0], pkt->csum[1]);

		switch (pkt->kind) {
		case GDB_KIND_NACK:
			retransmit(fileno(stream));
			/* fall through */
		case GDB_KIND_ACK:
			goto next;
		}

		if (!gdb_is_valid(pkt))
			printf("\tchecksum is invalid\n");

next:
		gdb_free_packet(pkt);
	}
}

void GDBServer::serve(void) {
	int conn;
	FILE *stream;

	for (;;) {
		if ((conn = accept(sockfd, NULL, NULL)) == -1) {
			warn("accept failed");
			continue;
		}

		if (!(stream = fdopen(conn, "r+")))
			throw std::system_error(errno, std::generic_category());

		dispatch(stream);
		fclose(stream);
	}
}
