#ifndef RISCV_VP_I2C_H
#define RISCV_VP_I2C_H

#include <systemc>

#include <tlm_utils/simple_target_socket.h>

#include "core/common/irq_if.h"
#include "util/tlm_map.h"

#include <map>
#include <queue>

#define I2C_CMD_STA 7
#define I2C_CMD_STO 6
#define I2C_CMD_RD 5
#define I2C_CMD_WR 4
#define I2C_CMD_ACK 3
#define I2C_CMD_IACK 0

struct I2C : public sc_core::sc_module {
	tlm_utils::simple_target_socket<I2C> tsock;

	// memory mapped configuration registers
	uint32_t prelo = 0;
	uint32_t prehi = 0;
	uint32_t ctrl = 0;
	uint32_t txrx = 0;
	uint32_t crsr = 0;
	
	enum I2CReg{
		PRELO_REG_ADDR = 0x00,
		PREHI_REG_ADDR = 0x04,
		CTRL_REG_ADDR = 0x08,
		TXRX_REG_ADDR = 0x0C,
		CRSR_REG_ADDR = 0x10,
	};

	vp::map::LocalRouter router = {"I2C"};
	interrupt_gateway *plic = nullptr;

	// indicate if transmission is happening 
	bool m_transmission = false;


	I2C(sc_core::sc_module_name) {
		tsock.register_b_transport(this, &I2C::transport);

		router
		    .add_register_bank({
		        {PRELO_REG_ADDR, &prelo},
		        {PREHI_REG_ADDR, &prehi},
		        {CTRL_REG_ADDR, &ctrl},
		        {TXRX_REG_ADDR, &txrx},
		        {CRSR_REG_ADDR, &crsr},
			})
		    .register_handler(this, &I2C::register_access_callback);
	}

	virtual ~I2C() {
	}

	virtual uint8_t process_i2c(uint8_t f_reg, uint8_t f_in) {
		return f_in;
	}

	void register_access_callback(const vp::map::register_access_t &r) {				

		if (r.read) {
		}

		r.fn();

		// wishbone behaviour 
		if (r.write) {
			// decide with crsr 
			if (r.vptr == &crsr)
			{
				uint8_t l_in = static_cast<uint8_t>(*r.vptr) & 0xff;
				// first check start of transmission
				if (l_in & (1 << I2C_CMD_STA))
				{
					crsr = process_i2c(CRSR_REG_ADDR, l_in);
					l_in &= ~(1 << I2C_CMD_STA);
					m_transmission = true;
				}
				// only then consider other flags, one by one in the following order
				if (m_transmission)
				{
					if (l_in & (1 << I2C_CMD_WR))
					{
						//write
						process_i2c(TXRX_REG_ADDR, txrx);
						l_in &= ~(1 << I2C_CMD_WR);
					} 
					if (l_in & (1 << I2C_CMD_RD))
					{
						//read
						txrx = process_i2c(TXRX_REG_ADDR, 0);
						l_in &= ~(1 << I2C_CMD_RD);
					}
					if (l_in & (1 << I2C_CMD_ACK))
					{
						crsr = process_i2c(CRSR_REG_ADDR, l_in);
						l_in &= ~(1 << I2C_CMD_ACK);
					}
					if (l_in & (1 << I2C_CMD_STO))
					{
						crsr = process_i2c(CRSR_REG_ADDR, l_in);
						l_in &= ~(1 << I2C_CMD_STO);
						m_transmission = false;
					}
				}
			}
			else if (r.vptr == &prelo)
			{
				std::cout << "I2C: Prescaler low set: " << std::hex << prelo << std::endl;			
			}
			else if (r.vptr == &prehi)
			{
				std::cout << "I2C: Prescaler high set: " << std::hex << prehi << std::endl;			
			}
			else if (r.vptr == &ctrl)
			{
				std::cout << "I2C: Control: " << std::hex << ctrl << std::endl;
			}
		}
	}

	void transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
		router.transport(trans, delay);
	}

};

#endif  // RISCV_VP_I2C_H
