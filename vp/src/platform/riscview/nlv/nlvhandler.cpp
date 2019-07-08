#include "nlvhandler.hpp"
#include <iostream>

namespace nlv
{
IMPL_ENUM(SType);
IMPL_ENUM(Direction);

NLElement::~NLElement(){};

Connectable::Connectable(Connectable::Type type) : type(type){};
Connectable::~Connectable(){};
Connectable::Type Connectable::getType(){ return type; };

Port::Port(std::string name, Direction direction) : Connectable(Connectable::Type::port), name(name), direction(direction){};
std::string Port::getName() { return name; };
Direction& Port::getDirection() { return direction; };
std::string Port::toCommand()
{
	std::string cmd;
	cmd += "port " +  name + " " + ~direction;
	return cmd;
}

Pin::Pin(std::string name, Direction direction) : name(name), direction(direction){};
std::string Pin::getName() { return name; };
Direction& Pin::getDirection() { return direction; };
std::string Pin::toCommand()
{
	std::string cmd;
	cmd += "pin " + name + " " + ~direction;
	return cmd;
};

Symbol::Symbol(std::string name, std::string viewname, SType stype) : name(name), viewname(viewname), stype(stype){};
Symbol::Symbol(std::string name, std::string viewname, SType stype, std::vector<Pin> pins) : name(name), viewname(viewname), stype(stype), pins(pins){};
std::string Symbol::getName() { return name; };
std::string Symbol::getViewname() { return viewname; };
SType Symbol::getStype() { return stype; };
std::vector<Pin>& Symbol::getPins() { return pins; };
std::string Symbol::toCommand()
{
	std::string cmd;
	cmd += "symbol " + name + " " + viewname + " " + ~stype;
	for (std::vector<Pin>::iterator it = pins.begin() ; it != pins.end(); ++it)
		cmd += " " + it->toCommand();
	std::cout << "Symbol: " << cmd << std::endl;
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
std::string Instance::toCommand()
{
	std::string cmd;
	cmd += "inst " + name + " " + symbol.getName() + " " + viewname;
	std::cout << "Instance: " << cmd << std::endl;
	return cmd;
};

Connection::Connection(std::string name) : name(name){};
Connection::Connection(std::string name, std::vector<Connectable*> list) : name(name), connectables(list){};
void Connection::add(Connectable* element)
{
	connectables.push_back(element);
}

std::string Connection::toCommand()
{
	std::string cmd;
	cmd += "net " + name;
	for (std::vector<Connectable*>::iterator it = connectables.begin() ; it != connectables.end(); ++it)
	{
		switch((*it)->getType())
		{
		case Connectable::Type::pin:
			cmd += " -pin " + static_cast<PinInstance*>(*it)->getInstance().getName() + " " + (*it)->getName();
			break;
		case Connectable::Type::port:
			cmd += " -port " + (*it)->getName();
			break;
		case Connectable::Type::hierPin:
			//not handled
			break;
		}
	}
	return cmd;
}

}; //end namespace nlv


NLVhandler::NLVhandler(std::function<bool(const char*)> command) : command(command){};


void NLVhandler::init()
{
	command("clear");
	command("module new module");
}

bool NLVhandler::add(nlv::NLElement& elem)
{
	std::string arg;
	arg = "load ";
	arg += elem.toCommand();
	std::cout << arg << std::endl;

	if(!command(arg.c_str()))
	{
		std::cerr << "NLVHandler: Connection lost" << std::endl;
		return false;
	}
	return true;
}

void NLVhandler::show()
{
	command("show");
	command("fullfit");
	//command("increment");
}
