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
	*/


	NLVhandler nlv(std::bind(&NLVConnectorClient::command, &ncc, placeholders::_1));


	std::list<nlv::NLElement*> elements;

	nlv::Port in1("IN1", nlv::Direction::IN), in2("IN2", nlv::Direction::IN);
	nlv::Port out1("OUT1", nlv::Direction::OUT);
	elements.push_back(&in1);
	elements.push_back(&in2);
	elements.push_back(&out1);

	nlv::Pin a("A", nlv::Direction::IN), b("B", nlv::Direction::IN), c("C", nlv::Direction::OUT);
	nlv::Symbol abox("ABOX", "vn", nlv::SType::BOX, {a,b,c});
	elements.push_back(&abox);

	nlv::Instance box1 = abox.instantiate("BOX1");
	nlv::Instance box2 = abox.instantiate("BOX2");
	elements.push_back(&box1);
	elements.push_back(&box2);

	nlv::Connection l1("l1");
	l1.add(&in1);
	l1.add(box1.getPin(a));
	l1.add(box2.getPin(a));
	elements.push_back(&l1);

	nlv::Connection l2("l2", {box1.getPin(c), box2.getPin(c), &out1});
	elements.push_back(&l2);

	nlv::Connection l3("l3", {&in2, box1.getPin(b), box2.getPin(b)});
	elements.push_back(&l3);

	nlv.init();

	std::string textlist[3] = {"Booger\nAids", "Aids\nBoogers", "CANCER"};
	uint8_t textIndice = 0;

	box1.setText(textlist[(textIndice++) % 3]);
	box2.setText(textlist[(textIndice++) % 3]);

	for (nlv::NLElement* element : elements)
	{
		nlv.add(*element);
	}

	nlv.show();

	while(true)
	{
		usleep(500000);
		box1.setText(textlist[(textIndice++) % 3]);
		box2.setText(textlist[(textIndice++) % 3]);

		nlv.update(box1);
		nlv.update(box2);
	}
}
