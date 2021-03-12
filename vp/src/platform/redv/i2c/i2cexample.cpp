#include <iostream>
#include <vector>

#include "i2cclient.hpp"

#define I2C_SL_ADDR 0x76


class MyI2CClient : public I2CClient
{
    public:
        MyI2CClient(uint8_t f_id) : I2CClient(f_id) 
        {}

    protected:
        virtual uint8_t readRegister(uint8_t f_reg)
        {
            // return inverse of f_reg
            uint8_t val = ~f_reg;
            std::cout << std::hex << "Read reg: " << int(f_reg) << " val: " << int(val) << std::endl;
            return val;
        }

        virtual void writeRegister(uint8_t f_reg, uint8_t f_val)
        {
            // answer if ok.
            std::cout << std::hex << "Write reg: " << int(f_reg) << " val: " << int(f_val) << std::endl;
        }

    private:
};

MyI2CClient i2csclient(I2C_SL_ADDR);

int main()
{
  i2csclient.run();

  return 0;
}