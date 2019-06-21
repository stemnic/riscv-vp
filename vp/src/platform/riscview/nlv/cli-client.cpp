/*
 * cli-client.cpp
 *
 *  Created on: 7 Nov 2018
 *      Author: dwd
 */

#include <unistd.h>
#include <iostream>

#include "connector-client.hpp"

using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 3) {
		cout << "usage: " << argv[0] << " host port (e.g. localhost 1339)" << endl;
		exit(-1);
	}

	NLVConnectorClient ncc;

	if (!ncc.setupConnection(argv[1], argv[2])) {
		cout << "can't setup connection" << endl;
		return -1;
	}

	while (true) {
		ncc.command("hey, i bims, 1 prog");
		usleep(125000);
	}
}
