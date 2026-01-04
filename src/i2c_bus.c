#include"../inc/i2c_bus.h"
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<linux/i2c-dev.h>
#include<linux/i2c.h>


i2c_status_t i2c_init(i2c_device_t *dev, const char *bus, uint8_t addr){

    if(dev == NULL || bus == NULL || addr == 0)
    {
        return I2C_ERR_INVALID_PARAM;
    }

    
    //Initialize the structure to zero to avoid garbage values
    memset(dev, 0, sizeof(i2c_device_t));

    if((dev->fd = open(bus,O_RDWR))<0)
    {
        perror("I2C Error: Failed to open bus.");
        return I2C_ERR_OPEN;
    }

    dev->device_addr = addr;

    return I2C_OK;
}
i2c_status_t i2c_write_reg(i2c_device_t *dev, uint8_t reg, uint8_t *data, uint16_t len) {

    if (dev == NULL) {
        return I2C_ERR_INVALID_PARAM;
    }

    if (len > I2C_MAX_WRITE_LEN) {
        fprintf(stderr, "I2C Error: Write length %d exceeds limit of %d\n", len, I2C_MAX_WRITE_LEN);
        return I2C_ERR_INVALID_PARAM;
    }

    if (len > 0 && data == NULL) {
        return I2C_ERR_INVALID_PARAM;
    }


    uint8_t write_buf[I2C_FIXED_BUF_SIZE]; 

    memset(write_buf, 0, sizeof(write_buf));
    

    write_buf[0] = reg;
    
    /* Copy data if present */
    if (len > 0) {
        memcpy(&write_buf[1], data, len);
    }

    struct i2c_rdwr_ioctl_data msgset;
    struct i2c_msg msg;

    /* Initialize ioctl structures */
    memset(&msg, 0, sizeof(msg));
    memset(&msgset, 0, sizeof(msgset));

    msg.addr  = dev->device_addr;
    msg.flags = I2C_WRITE_FLAG;
    msg.len   = (uint16_t)(len + 1); /* Total size: Register + Data */
    msg.buf   = write_buf;

    msgset.msgs  = &msg;
    msgset.nmsgs = 1;

    /* Perform the atomic write transaction */
    if (ioctl(dev->fd, I2C_RDWR, &msgset) < 0) {
        perror("I2C Error: Write failed");
        return I2C_ERR_IOCTL;
    }

    return I2C_OK;
}

i2c_status_t i2c_read_reg(i2c_device_t *dev, uint8_t reg, uint8_t *data, uint16_t len) {

    if (dev == NULL || data == NULL) {
        return I2C_ERR_INVALID_PARAM;
    }

    if (len == 0 || len > I2C_MAX_READ_LEN) {
        fprintf(stderr, "I2C Error: Invalid read length %d (Max %d)\n", len, I2C_MAX_READ_LEN);
        return I2C_ERR_INVALID_PARAM;
    }

    struct i2c_rdwr_ioctl_data msgset;
    struct i2c_msg msgs[2];


    memset(msgs, 0, sizeof(msgs));
    memset(&msgset, 0, sizeof(msgset));


    msgs[0].addr  = dev->device_addr;
    msgs[0].flags = I2C_WRITE_FLAG; 
    msgs[0].len   = 1;
    msgs[0].buf   = &reg;

    /* * 2nd Message: Read (Fetch Data)
     * This follows the write message with a 'Repeated Start' condition.
     */
    msgs[1].addr  = dev->device_addr;
    msgs[1].flags = I2C_READ_FLAG; 
    msgs[1].len   = len;
    msgs[1].buf   = data;

    /* Link messages to the set */
    msgset.msgs  = msgs;
    msgset.nmsgs = 2;

    /* 4. Atomic Execution: Ensures the bus isn't released between Write and Read */
    if (ioctl(dev->fd, I2C_RDWR, &msgset) < 0) {
        perror("I2C Error: Combined read/write transaction failed");
        return I2C_ERR_IOCTL;
    }

    return I2C_OK;
}