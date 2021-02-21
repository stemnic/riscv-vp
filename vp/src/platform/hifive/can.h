#pragma once

#include "can/mcp_can_dfs.h"
#include "spi.h"

#ifdef __APPLE__
#define PF_CAN 29
#define AF_CAN PF_CAN

#define CAN_MAX_DLEN	8
#define CANFD_MAX_DLEN 64

/* protocols of protocol family PF_CAN */
#define CAN_RAW			1 /* RAW sockets */
#define CAN_BCM			2 /* Broadcast Manager */
#define CAN_TP16		3 /* VAG Transport Protocol v1.6 */
#define CAN_TP20		4 /* CAG Transport Protocol v2.0 */
#define CAN_MCNET		5 /* Bosch MCNet */
#define CAN_ISOTP		6 /* ISO 15765-2 Transport Protocol */

typedef uint32_t canid_t;

struct can_frame {
	canid_t can_id;		/* 32 bit CAN_ID + EFF/RTR/ERR flags */
	uint8_t	can_dlc;	/* frame payload length in byte (0 .. CAN_MAX_DLEN) */
	uint8_t	__pad;		/* padding */
	uint8_t	__res0;		/* reserved / padding */
	uint8_t	__res1;		/* reserved / padding */
	uint8_t	data[CAN_MAX_DLEN] __attribute__((aligned(8)));
};


struct sockaddr_can {
	uint8_t can_family;
	int         can_ifindex;
	union {
		/* transport protocol class address information (e.g. ISOTP) */
		struct { canid_t rx_id, tx_id; } tp;
		
		/* reserved for future CAN protocols address information */
	} can_addr;
};

#else
#include <linux/can.h>
#endif

#include <net/if.h>

#include <functional>
#include <thread>

class CAN : public SpiInterface {
	enum class State {
		init,
		readRegister,
		writeRegister,
		bitmod,

		loadTX0,
		loadTX1,
		loadTX2,

		sendTX0,
		sendTX1,
		sendTX2,
		sendALL,

		readRX0,
		readRX1,

		getStatus,

		shit,
		wank,
		fuck,
		arse,
		crap,
		dick,
	} state;

	std::thread listener;

	uint8_t registers[MCP_RXB1SIDH + 1];

	struct MCPFrame {
		union {
			uint8_t raw[5 + CANFD_MAX_DLEN];
			struct {
				union {
					uint8_t id[4];
					struct {
						/*
						 *	MCP_SIDH        0
						    MCP_SIDL        1
						    MCP_EID8        2
						    MCP_EID0		3
						 */
						uint16_t sid;
						uint16_t eid;
					};
				};
				uint8_t length;
				uint8_t payload[CANFD_MAX_DLEN];
			};
		};
	};

	MCPFrame txBuf[3];
	MCPFrame rxBuf[2];

	uint8_t status;

	int s;
	struct sockaddr_can addr;
	struct ifreq ifr;

	volatile bool stop;

   public:
	CAN();
	~CAN();

	uint8_t write(uint8_t byte) override;

	const char* registerName(uint8_t id);
	const char* regValueName(uint8_t id);
	const char* spiInstName(uint8_t id);

	void command(uint8_t byte);
	uint8_t readRegister(uint8_t byte);
	uint8_t writeRegister(uint8_t byte);
	uint8_t modifyRegister(uint8_t byte);

	uint8_t loadTxBuf(uint8_t no, uint8_t byte);
	uint8_t sendTxBuf(uint8_t no, uint8_t byte);

	uint8_t readRxBuf(uint8_t no, uint8_t byte);

	void mcp2515_id_to_buf(const unsigned long id, uint8_t* idField, const bool extended = false);
	void mcp2515_buf_to_id(unsigned& id, bool& extended, uint8_t* idField);

	void enqueueIncomingCanFrame(const struct can_frame& frame);
	void listen();
};
