#include "nlvhandler.hpp"
#include <iostream>

namespace nlv
{
IMPL_ENUM(SType);
IMPL_ENUM(Direction);

NLElement::~NLElement(){};
Commandlist NLElement::update(){ return Commandlist(); };

Connectable::Connectable(Connectable::Type type) : type(type){};
Connectable::~Connectable(){};
Connectable::Type Connectable::getType(){ return type; };

Port::Port(std::string name, Direction direction) : Connectable(Connectable::Type::port), name(name), direction(direction){};
std::string Port::getName() { return name; };
Direction& Port::getDirection() { return direction; };
Commandlist Port::load()
{
	Command cmd("port " + name + " " + ~direction);
	return Commandlist({cmd});
}

Pin::Pin(std::string name, Direction direction) : name(name), direction(direction){};
std::string Pin::getName() { return name; };
Direction& Pin::getDirection() { return direction; };
Commandlist Pin::load()
{
	Command cmd("pin " + name + " " + ~direction);
	return Commandlist({cmd});
};

Symbol::Symbol(std::string name, std::string viewname, SType stype) : name(name), viewname(viewname), stype(stype){};
Symbol::Symbol(std::string name, std::string viewname, SType stype, std::vector<Pin> pins) : name(name), viewname(viewname), stype(stype), pins(pins){};
std::string Symbol::getName() { return name; };
std::string Symbol::getViewname() { return viewname; };
SType Symbol::getStype() { return stype; };
std::vector<Pin>& Symbol::getPins() { return pins; };
Commandlist Symbol::load()
{
	Command cmd("symbol " + name + " " + viewname + " " + ~stype);
	for (std::vector<Pin>::iterator it = pins.begin() ; it != pins.end(); ++it)
		cmd += " " + it->load().front();
	std::cout << "Symbol: " << cmd << std::endl;
	return Commandlist({cmd});
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
void Instance::setText(std::string text){
	this->text = text;
	changed = true;
};
Commandlist Instance::load()
{
	Commandlist list;
	Command cmd("inst " + name + " " + symbol.getName() + " " + viewname);
	std::cout << "Instance: " << cmd << std::endl;
	list.push_back(cmd);
	cmd = "cgraphic " + name + "text linkto {inst " + name + "} text \"" + text + "\" -ll 0 0 5 place bot 10 0";
	list.push_back(cmd);
	return list;
};
Commandlist Instance::update()
{
	if(!changed)
		return Commandlist();

	Commandlist list;
	std::string cmd("unload cgraphic " + name + "text");
	list.push_back(cmd);
	cmd = "load cgraphic " + name + "text linkto {inst " + name + "} text \"" + text + "\" -ll 0 0 5 place bot 10 0";
	list.push_back(cmd);
	changed = false;
	return list;
}


Connection::Connection(std::string name) : name(name){};
Connection::Connection(std::string name, std::vector<Connectable*> list) : name(name), connectables(list){};
void Connection::add(Connectable* element)
{
	connectables.push_back(element);
}
void Connection::setText(std::string text){
	this->text = text;
	changed = true;
};
Commandlist Connection::load()
{
	Command cmd;
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
	return Commandlist({cmd});
}


Commandlist Connection::update()
{
	if(!changed)
		return Commandlist();

	Commandlist list;
	std::string cmd("unload cgraphic " + name + "text");
	list.push_back(cmd);
	cmd = "load cgraphic " + name + "text linkto {net " + name + "} text \"" + text + "\" -ll 0 0 5 place top 0 0";
	list.push_back(cmd);
	changed = false;
	return list;
}

}; //end namespace nlv


NLVhandler::NLVhandler(std::function<bool(const char*)> command) : sendCommand(command){};



void NLVhandler::add(std::list<nlv::NLElement*>& elements)
{
	mElements.splice(mElements.end(), elements);
}

void NLVhandler::add(nlv::NLElement& elem)
{
	mElements.push_back(&elem);
}

bool NLVhandler::init()
{
	sendCommand("clear");
	sendCommand("module new module");
	for(nlv::NLElement* elem : mElements)
	{
		for(nlv::Command cmd : elem->load())
		{
			if(!sendCommand(("load " + cmd).c_str()))
			{
				std::cerr << "NLVHandler: Connection lost" << std::endl;
				return false;
			}
		}
	}
	return true;
}

bool NLVhandler::show()
{
	if(!sendCommand("show")) return false;
	if(!sendCommand("fullfit")) return false;
	//sendCommand("increment");
	return true;
}

bool NLVhandler::update()
{
	//Loop des todes of doom
	for(nlv::NLElement* elem : mElements)
	{
		for(nlv::Command cmd : elem->update()){
			std::cout << cmd << std::endl;

			if(!sendCommand(cmd.c_str()))
			{
				std::cerr << "NLVHandler: Connection lost" << std::endl;
				return false;
			}
		}
	}
	return true;
}
