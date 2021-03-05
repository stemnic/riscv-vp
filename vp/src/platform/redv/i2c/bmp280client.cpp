#include <iostream>
#include <map>

#include "i2cclient.hpp"

#define DEBUG
#ifdef DEBUG
    #define DEBUGOUT std::cout
#else
    #define DEBUGOUT if(false) std::cout
#endif

#define I2C_SL_ADDR 0x76

typedef std::map<uint8_t, uint8_t> memorymap_t;

class BMP280Client : public I2CClient {
   public:
	BMP280Client(uint8_t f_id) : I2CClient(f_id) {
		// exemplary init 0xd0 to 0xfc
		m_memoryMap.insert(std::make_pair(0xd0, 0x58));
		m_memoryMap.insert(std::make_pair(0xe0, 0x00));
		m_memoryMap.insert(std::make_pair(0xf3, 0x00));
		m_memoryMap.insert(std::make_pair(0xf4, 0x00));
		m_memoryMap.insert(std::make_pair(0xf5, 0x00));
		m_memoryMap.insert(std::make_pair(0xf7, 0x80));
		m_memoryMap.insert(std::make_pair(0xf8, 0x00));
		m_memoryMap.insert(std::make_pair(0xf9, 0x00));
		m_memoryMap.insert(std::make_pair(0xfa, 0x80));
		m_memoryMap.insert(std::make_pair(0xfb, 0x00));
		m_memoryMap.insert(std::make_pair(0xfc, 0x00));

	}

   protected:
	virtual uint8_t readRegister(uint8_t f_reg) {
		memorymap_t::const_iterator it = m_memoryMap.find(f_reg);
		if (it != m_memoryMap.end()) {
            uint8_t val = it->second;
			DEBUGOUT << std::hex << "Read reg: " << int(f_reg) << " val: " << int(val) << std::endl;
			return val;
		} else {
			DEBUGOUT << "Read reg: " << int(f_reg) <<  " failed. No such register." << std::endl;
			return 0;
		}
	}

	virtual void writeRegister(uint8_t f_reg, uint8_t f_val) {
		// answer if ok.
		memorymap_t::iterator it = m_memoryMap.find(f_reg);
		if (it != m_memoryMap.end()) {
			it->second = f_val;;

            // do something.

    		DEBUGOUT << std::hex << "Write reg: " << int(f_reg) << " val: " << int(f_val) << std::endl;
		} else {
			DEBUGOUT << "Write reg: " << int(f_reg) <<  " failed. No such register." << std::endl;
		}
	}

   private:
	memorymap_t m_memoryMap;
	
};

BMP280Client i2csclient(I2C_SL_ADDR);

int main() {
	if (!i2csclient.run())
        exit(-1);

	return 0;
}