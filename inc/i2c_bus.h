#ifndef I2C_Wrapper_H_
#define I2C_Wrapper_H_

#include <stdint.h>


/* -------------------------------------------------------------------------- */
/* I2C BUFFER LIMITS                                                          */
/* -------------------------------------------------------------------------- */
/** * Max data length for a single write (excluding register byte).
 * Total transaction will be 64 bytes (1 byte reg + 63 bytes data).
 */
#define I2C_MAX_WRITE_LEN    63 

/** * Total size of the fixed stack buffer used in i2c_write_reg.
 */
#define I2C_FIXED_BUF_SIZE   (I2C_MAX_WRITE_LEN + 1) 

/** * Max data length for a single read operation.
 */
#define I2C_MAX_READ_LEN     I2C_FIXED_BUF_SIZE

typedef enum{
    I2C_OK = 0,
    I2C_ERR_OPEN,
    I2C_ERR_IOCTL,
    I2C_ERR_WRITE,
    I2C_ERR_READ,
    I2C_ERR_INVALID_PARAM,
    
}i2c_status_t;

typedef enum{

    I2C_WRITE_FLAG = 0,
    I2C_READ_FLAG  = 1
}i2c_msg_status_t;


//device handle
typedef struct{
    int fd;
    uint8_t device_addr;
}i2c_device_t;

i2c_status_t i2c_init(i2c_device_t *dev, const char *bus, uint8_t addr);
i2c_status_t i2c_write_reg(i2c_device_t *dev, uint8_t reg, uint8_t *data, uint16_t len);
i2c_status_t i2c_read_reg(i2c_device_t *dev, uint8_t reg, uint8_t *data, uint16_t len);


#endif