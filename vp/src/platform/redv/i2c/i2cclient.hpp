#ifndef I2C_CLIENT_H
#define I2C_CLIENT_H

#define I2C_CMD_STA 7
#define I2C_CMD_STO 6
#define I2C_CMD_ACK 3

#define I2C_STAT_RXACK 7
#define I2C_STAT_BUSY 6
#define I2C_STAT_ARLO 5
#define I2C_STAT_TIP 1
#define I2C_STAT_IF 0

class I2CClient
{
public:
    enum State
    {
        IDLE,
        ADDRESS,
        REGISTER,
        READ,
        WRITE
    };

    I2CClient(const uint8_t f_id) : m_id(f_id)
    {
    }

    virtual ~I2CClient()
    {
    }

    // start processing of i2c transmissions
    void run();

protected:
    // required for specific i2c handling
    virtual uint8_t readRegister(uint8_t f_reg) = 0;
    virtual void writeRegister(uint8_t f_reg, uint8_t f_val) = 0;

private:
    const uint8_t m_id;
    State m_state = IDLE;
    uint32_t m_regPointer = 0x0;
};

#endif
