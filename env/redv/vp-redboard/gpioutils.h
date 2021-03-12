#ifndef GPIOUTILS_H
#define GPIOUTILS_H

#include <stdint.h>

#include <gpio/gpio-client.hpp>

namespace gpioutil {

    uint64_t translateGpioToExtPin(GpioCommon::Reg reg);
    uint8_t translatePinNumberToRGBLed(uint64_t pinmap);
    uint8_t translatePinToGpioOffs(uint8_t pin);

} //namespace

#endif // GPIOUTILS_H
