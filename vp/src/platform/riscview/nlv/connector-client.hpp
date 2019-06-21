#pragma once

#include "connector-common.hpp"
#include <inttypes.h>

class NLVConnectorClient {
	int fd;

public:
	NLVConnectorClient();
	~NLVConnectorClient();

	bool setupConnection(const char* host, const char* port);
	//zero-terminated ''string''
	bool command(const char* command);
};
