#include "gpioutils.h"

namespace gpioutil {

    uint64_t translateGpioToExtPin(GpioCommon::Reg reg) {
        uint64_t ext = 0;
        for (uint64_t i = 0; i < 24; i++)  // Max Pin is 32, see SiFive HiFive1
                                           // Getting Started Guide 1.0.2 p. 20
        {
            // cout << i << " to ";;
            if (i >= 16) {
                ext |= (reg & (1l << i)) >> 16;
                // cout << i - 16 << endl;
            } else if (i <= 5) {
                ext |= (reg & (1l << i)) << 8;
                // cout << i + 8 << endl;
            } else if (i >= 9 && i <= 13) {
                ext |= (reg & (1l << i)) << 6;  // Bitshift Ninja! (with broken
                                                // legs) cout << i + 6 << endl;
            }
            // rest is not connected.
        }
        return ext;
    }

    uint8_t translatePinNumberToRGBLed(uint64_t pinmap) {
        uint8_t ret = 0;
        ret |= (~pinmap & (1 << 6)) >> 6;  // R
        ret |= (~pinmap & (1 << 3)) >> 2;  // G
        ret |= (~pinmap & (1 << 5)) >> 3;  // B
        return ret;
    }

    uint8_t translatePinToGpioOffs(uint8_t pin) {
        if (pin < 8) {
            return pin + 16;  // PIN_0_OFFSET
        }
        if(pin >= 8 && pin < 14) {
            return pin - 8;
        }
        //ignoring non-wired pin 14 <==> 8
        if(pin > 14 && pin < 20){
            return pin - 6;
        }

        return 0;
    }

} //namespace
