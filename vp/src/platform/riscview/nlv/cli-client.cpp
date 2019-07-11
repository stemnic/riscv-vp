/*
 * cli-client.cpp
 *
 *  Created on: 7 Nov 2018
 *      Author: dwd
 */
#include "connector-client.hpp"
#include "nlvhandler.hpp"

#include <unistd.h>
#include <iostream>
#include <functional>
#include <list>

using namespace std;

int main(int argc, char* argv[]) {
	string domain = "localhost";
	string port = "1333";
	if (argc < 3) {
		cout << "usage: " << argv[0] << " host port (e.g. " << domain << " " << port << ")" << endl;
	}
	else{
		domain = argv[1];
		port   = argv[2];
	}

	NLVConnectorClient ncc;

	if (!ncc.setupConnection(domain.c_str(), port.c_str())) {
		cout << "can't setup connection" << endl;
		return -1;
	}

	/*
	ncc.command("module new module");
	ncc.command("load symbol ABOX v BOX pin input1 IN pin input2 IN pin output1 OUT");
			//load symbol ABOX vn BOX pin A IN pin B IN pin C OUT);
	ncc.command("load inst ABOX1 ABOX v");
	ncc.command("show");
	ncc.command("load cgraphic sparta2 linkto {inst BOX1} text \"Booger\nAids\nBrewt\" -ll 0 0 5 place bot 10 0);
	ncc.command("load cgraphic sparta3 linkto {inst BOX1} text \"Booger\nAids\nBrewt\" -ll 0 0 5 place bot 10 0);
	*/


	NLVhandler nlv(std::bind(&NLVConnectorClient::command, &ncc, placeholders::_1));


	nlv::Port in1("IN1", nlv::Direction::IN), in2("IN2", nlv::Direction::IN);
	nlv::Port out1("OUT1", nlv::Direction::OUT);
	nlv.add(in1);
	nlv.add(in2);
	nlv.add(out1);

	nlv::Pin a("A", nlv::Direction::IN), b("B", nlv::Direction::IN), c("C", nlv::Direction::OUT);
	nlv::Symbol abox("ABOX", "vn", nlv::SType::BOX, {a,b,c});
	nlv.add(abox);

	nlv::Instance box1 = abox.instantiate("BOX1");
	nlv::Instance box2 = abox.instantiate("BOX2");
	nlv.add(box1);
	nlv.add(box2);

	nlv::Connection l1("l1");
	l1.add(&in1);
	l1.add(box1.getPin(a));
	l1.add(box2.getPin(a));
	nlv.add(l1);
	uint8_t l1ExampleCounter = 0;

	nlv::Connection l2("l2", {box1.getPin(c), box2.getPin(c), &out1});
	nlv.add(l2);
	uint8_t l2ExampleCounter = 0;

	nlv::Connection l3("l3", {&in2, box1.getPin(b), box2.getPin(b)});
	nlv.add(l3);
	uint8_t l3ExampleCounter = 0;

	std::vector<std::string> textlist({"Bloop", "bop", "bleep"});
	uint8_t textIndice = 0;

	box1.setText(textlist[(textIndice++) % textlist.size()]);
	box2.setText(textlist[(textIndice++) % textlist.size()]);

	if(!nlv.init()) return -1;
	if(!nlv.show()) return -2;

	while(true)
	{
		usleep(50000);
		box1.setText(textlist[(textIndice++) % textlist.size()]);
		box2.setText(textlist[(textIndice++) % textlist.size()]);

		l1ExampleCounter+= 1;
		l2ExampleCounter+= 2;
		l3ExampleCounter+= 3;

		l1.setText(to_string(l1ExampleCounter));
		l2.setText(to_string(l2ExampleCounter));
		l3.setText(to_string(l3ExampleCounter));

		if(!nlv.update()) return -3;
	}
}
