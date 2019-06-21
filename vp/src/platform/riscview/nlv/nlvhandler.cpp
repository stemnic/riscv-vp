#include "nlvhandler.hpp"
#include <iostream>

namespace nlv
{
IMPL_ENUM(SType);
IMPL_ENUM(Direction);

void print(NlviewArgs& cmd)
{
	for(unsigned i = 0; i < cmd.length(); i++)
	{
		std::cout << ">" << cmd[i] << "< ";
	}
	std::cout << std::endl;
}

void extend(NlviewArgs& to, const NlviewArgs&& cmd)
{
	for(unsigned i = 0; i < cmd.length(); i++)
	{
		to << cmd[i];
	}
}

void prefixAdd(NlviewArgs& to, const std::string p, const NlviewArgs cmd)
{
	to << (p + cmd[0]);
	for(unsigned i = 1; i < cmd.length(); i++)
	{
		to << cmd[i];
	}
}

} //namespace nlv

using namespace nlv;

NLElement::~NLElement(){};

Connectable::Connectable(Connectable::Type type) : type(type){};
Connectable::~Connectable(){};
Connectable::Type Connectable::getType(){ return type; };

Port::Port(std::string name, Direction direction) : Connectable(Connectable::Type::port), name(name), direction(direction){};
std::string Port::getName() { return name; };
Direction& Port::getDirection() { return direction; };
NlviewArgs Port::toCommand()
{
	NlviewArgs cmd;
	cmd << "port" << name << ~direction;
	return cmd;
}

Pin::Pin(std::string name, Direction direction) : name(name), direction(direction){};
std::string Pin::getName() { return name; };
Direction& Pin::getDirection() { return direction; };
NlviewArgs Pin::toCommand()
{
	NlviewArgs cmd;
	cmd << "pin" << name << ~direction;
	return cmd;
};

Symbol::Symbol(std::string name, std::string viewname, SType stype) : name(name), viewname(viewname), stype(stype){};
Symbol::Symbol(std::string name, std::string viewname, SType stype, std::vector<Pin> pins) : name(name), viewname(viewname), stype(stype), pins(pins){};
std::string Symbol::getName() { return name; };
std::string Symbol::getViewname() { return viewname; };
SType Symbol::getStype() { return stype; };
std::vector<Pin>& Symbol::getPins() { return pins; };
NlviewArgs Symbol::toCommand()
{
	NlviewArgs cmd;
	cmd << "symbol" << name << viewname << ~stype;
	for (std::vector<Pin>::iterator it = pins.begin() ; it != pins.end(); ++it)
		extend(cmd, it->toCommand());
	std::cout << "Symbol: ";
	print(cmd);
	return cmd;
};
Instance Symbol::instantiate(std::string name)
{
	return Instance(name, viewname, *this);
};

PinInstance::PinInstance(Pin& pin, Instance& instance) : Connectable(Connectable::Type::pin), pin(pin), instance(instance){};
Pin& PinInstance::getPin(){ return pin; };
std::string PinInstance::getName(){ return pin.getName(); };
Instance& PinInstance::getInstance(){ return instance; };

Instance::Instance(std::string name, std::string viewname, Symbol& symbol): name(name), viewname(viewname), symbol(symbol)
{
	for (auto &pin : symbol.getPins())
		pins[pin.getName()] = new PinInstance(pin, *this);
};
std::string Instance::getName() { return name; };
Symbol& Instance::getSymbol() { return symbol; };
std::string Instance::getViewname() { return viewname; };
PinInstance* Instance::getPin(Pin& pin)
{
	return pins[pin.getName()];
}
NlviewArgs Instance::toCommand()
{
	NlviewArgs cmd;
	cmd << "inst" << name << symbol.getName() << viewname;
	std::cout << "Instance: ";
	print(cmd);
	return cmd;
};

Connection::Connection(std::string name) : name(name){};
Connection::Connection(std::string name, std::vector<Connectable*> list) : name(name), connectables(list){};
void Connection::add(Connectable* element)
{
	connectables.push_back(element);
}

NlviewArgs Connection::toCommand()
{
	NlviewArgs cmd;
	cmd << "net" << name;
	for (std::vector<Connectable*>::iterator it = connectables.begin() ; it != connectables.end(); ++it)
	{
		switch((*it)->getType())
		{
		case Connectable::Type::pin:
			cmd << "-pin" << static_cast<PinInstance*>(*it)->getInstance().getName() << (*it)->getName();
			break;
		case Connectable::Type::port:
			cmd << "-port" << (*it)->getName();
			break;
		case Connectable::Type::hierPin:
			//not handled
			break;
		}
	}
	return cmd;
}


NLVhandler::NLVhandler(NlvQWidget* nlview) : nlview(nlview){};

bool NLVhandler::command(const char* command)
{
	std::cout << command << std::endl;
	bool r;
	const char* err = nlview->commandLine(&r, command);
	if(!r)
	{
		std::cerr << err << std::endl;
	}
	return r;
}

void NLVhandler::init()
{
	command("module new module");
}

bool NLVhandler::add(NLElement& elem)
{
	bool r;
	NlviewArgs arg;
	arg << "load";
	extend(arg, elem.toCommand());
	print(arg);
	const char* err = nlview->command(&r, arg);
	if(!r)
	{
		std::cerr << err << std::endl;
	}
	return r;
}

void NLVhandler::show()
{
	command("show");
	command("fullfit");
	//command("increment");
}
