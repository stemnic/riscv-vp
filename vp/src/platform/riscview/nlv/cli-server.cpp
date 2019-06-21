/*
 * gpio-server.cpp
 *
 *  Created on: 5 Nov 2018
 *      Author: dwd
 */

#include <unistd.h>
#include <csignal>
#include <functional>
#include <iostream>
#include <thread>

#include "connector-server.hpp"

using namespace std;

bool stop = false;

void signalHandler(int signum) {
	cout << "Interrupt signal (" << signum << ") received.\n";

	if (stop)
		exit(signum);
	stop = true;
	raise(SIGUSR1);  // this breaks wait in thread
}

void command(const char* cmd)
{
	cout << cmd << endl;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "usage: " << argv[0] << " port (e.g. 1339)" << endl;
		exit(-1);
	}

	NLVConnectorServer ncs;

	if (!ncs.setupConnection(argv[1])) {
		cerr << "cant set up server" << endl;
		exit(-1);
	}

	signal(SIGINT, signalHandler);

	ncs.registerInput(bind(command, placeholders::_1));
	thread server(bind(&NLVConnectorServer::startListening, &ncs));

	while (!stop && !ncs.isStopped()) {
		usleep(100000);
	}
	ncs.quit();
	server.join();
}
