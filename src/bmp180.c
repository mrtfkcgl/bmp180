#include "../inc/bmp180.h"
#include <stdio.h>

#define BMP180_ADDR             0x77
#define BMP180_REG_ID           0xD0
#define BMP180_REG_CALIB_START  0xAA  /* Start of 22-byte calibration block */

/**
 * @brief Checks if the BMP180 sensor is physically connected and responding.
 */
i2c_status_t bmp180_check_id(bmp180_t *dev) {
    uint8_t chip_id = 0;
    i2c_status_t status = i2c_read_reg(&dev->i2c, BMP180_REG_ID, &chip_id, 1);
    
    if (status == I2C_OK && chip_id == 0x55) {
        printf("BMP180: Sensor found! ID: 0x%02X\n", chip_id);
        return I2C_OK;
    }
    
    fprintf(stderr, "BMP180: Sensor not found or ID mismatch (ID: 0x%02X)\n", chip_id);
    return I2C_ERR_IOCTL;
}

/**
 * @brief Reads 22 bytes of calibration data and parses them into the struct.
 */
i2c_status_t bmp180_load_calibration(bmp180_t *dev) {
    if (dev == NULL) return I2C_ERR_INVALID_PARAM;

    uint8_t buf[22]; /* 11 coefficients * 2 bytes each */
    
    /* 1. Read the entire 22-byte block in one atomic transaction */
    i2c_status_t status = i2c_read_reg(&dev->i2c, BMP180_REG_CALIB_START, buf, 22);
    if (status != I2C_OK) return status;

    /* 2. Parse Big-Endian data into the struct */
    /* Using bit-shifting ensures portability across different CPU architectures */
    dev->calib.ac1 = (int16_t)  ((buf[0] << 8)  | buf[1]);
    dev->calib.ac2 = (int16_t)  ((buf[2] << 8)  | buf[3]);
    dev->calib.ac3 = (int16_t)  ((buf[4] << 8)  | buf[5]);
    dev->calib.ac4 = (uint16_t) ((buf[6] << 8)  | buf[7]);
    dev->calib.ac5 = (uint16_t) ((buf[8] << 8)  | buf[9]);
    dev->calib.ac6 = (uint16_t) ((buf[10] << 8) | buf[11]);
    dev->calib.b1  = (int16_t)  ((buf[12] << 8) | buf[13]);
    dev->calib.b2  = (int16_t)  ((buf[14] << 8) | buf[15]);
    dev->calib.mb  = (int16_t)  ((buf[16] << 8) | buf[17]);
    dev->calib.mc  = (int16_t)  ((buf[18] << 8) | buf[19]);
    dev->calib.md  = (int16_t)  ((buf[20] << 8) | buf[21]);

    printf("BMP180: Calibration data loaded successfully.\n");
    return I2C_OK;
}

#include "../inc/bmp180.h"
#include <unistd.h>
#include <math.h>

#define BMP180_REG_CONTROL      0xF4
#define BMP180_REG_OUT_MSB      0xF6
#define BMP180_CMD_TEMP         0x2E
#define BMP180_CMD_PRESS        0x34

/**
 * @brief Reads raw temperature and calculates the true temperature in Celsius.
 * This also updates dev->b5, which is REQUIRED for pressure calculation.
 */
i2c_status_t bmp180_read_temperature(bmp180_t *dev, float *temperature) {
    uint8_t data[2];
    uint8_t reg_val = BMP180_CMD_TEMP;
    
    /* 1. Trigger temperature measurement */
    i2c_write_reg(&dev->i2c, BMP180_REG_CONTROL, &reg_val, 1);
    usleep(5000); /* Wait 4.5ms (min) as per datasheet */

    /* 2. Read raw temperature (UT) */
    i2c_status_t status = i2c_read_reg(&dev->i2c, BMP180_REG_OUT_MSB, data, 2);
    if (status != I2C_OK) return status;

    int32_t UT = (data[0] << 8) | data[1];

    /* 3. True Temperature Calculation (Datasheet Formulas) */
    int32_t X1 = (UT - (int32_t)dev->calib.ac6) * (int32_t)dev->calib.ac5 >> 15;
    int32_t X2 = ((int32_t)dev->calib.mc << 11) / (X1 + (int32_t)dev->calib.md);
    dev->b5 = X1 + X2; /* Global B5 updated */
    
    if (temperature) {
        *temperature = ((dev->b5 + 8) >> 4) / 10.0f;
    }

    return I2C_OK;
}

/**
 * @brief Reads raw pressure and calculates true pressure in Pascals (Pa).
 */
i2c_status_t bmp180_read_pressure(bmp180_t *dev, bmp180_oss_t oss, int32_t *pressure) {
    uint8_t data[3];
    uint8_t reg_val = BMP180_CMD_PRESS + (oss << 6);
    
    /* 1. Trigger pressure measurement */
    i2c_write_reg(&dev->i2c, BMP180_REG_CONTROL, &reg_val, 1);
    
    /* Wait time depends on OSS */
    const uint32_t delays[] = {5000, 8000, 14000, 26000};
    usleep(delays[oss]);

    /* 2. Read raw pressure (UP) - 3 bytes (MSB, LSB, XLSB) */
    i2c_status_t status = i2c_read_reg(&dev->i2c, BMP180_REG_OUT_MSB, data, 3);
    if (status != I2C_OK) return status;

    int32_t UP = ((data[0] << 16) | (data[1] << 8) | data[2]) >> (8 - oss);

    /* 3. True Pressure Calculation (Extremely sensitive formulas) */
    int32_t b6 = dev->b5 - 4000;
    int32_t x1 = (dev->calib.b2 * (b6 * b6 >> 12)) >> 11;
    int32_t x2 = dev->calib.ac2 * b6 >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = ((( (int32_t)dev->calib.ac1 * 4 + x3) << oss) + 2) / 4;

    x1 = dev->calib.ac3 * b6 >> 13;
    x2 = (dev->calib.b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = (uint32_t)dev->calib.ac4 * (uint32_t)(x3 + 32768) >> 15;
    uint32_t b7 = ((uint32_t)UP - b3) * (50000 >> oss);

    int32_t p;
    if (b7 < 0x80000000) p = (b7 * 2) / b4;
    else p = (b7 / b4) * 2;

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    *pressure = p + ((x1 + x2 + 3791) >> 4);

    return I2C_OK;
}