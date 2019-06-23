#pragma once

#include <functional>
#include <thread>
#include "connector-common.hpp"

class NLVConnectorServer {
	int fd;
	volatile bool stop;
	std::function<void(const char* command)> fun;
	void handleConnection(int conn);
	std::thread* listenerThread;

public:
	NLVConnectorServer();
	~NLVConnectorServer();
	bool setupConnection(const char* port);
	void quit();
	bool isStopped();
	void registerInput(std::function<void(const char* command)> fun);
	void startListening();
private:
	void listener();
};
