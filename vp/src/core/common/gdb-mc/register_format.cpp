#include <iomanip>

#ifdef __APPLE__
#define bswap_16(value) ((((value) & 0xff) << 8) | ((value) >> 8))
#define bswap_32(value) (((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | (uint32_t)bswap_16((uint16_t)((value) >> 16)))
#define bswap_64(value) (((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) << 32) | (uint64_t)bswap_32((uint32_t)((value) >> 32)))
#else
#include <byteswap.h>
#endif

#include "register_format.h"

RegisterFormater::RegisterFormater(Architecture arch) {
	this->arch = arch;
	this->stream << std::setfill('0') << std::hex;
}

void RegisterFormater::formatRegister(uint64_t value) {
	switch (arch) {
	case RV32:
		stream << std::setw(8) << bswap_32(value);
		break;
	case RV64:
		stream << std::setw(16) << bswap_64(value);
		break;
	default:
		throw std::invalid_argument("Architecture not implemented");
	}
}

std::string RegisterFormater::str(void) {
	return this->stream.str();
}
