#ifndef BMP180_H
#define BMP180_H

#include "i2c_bus.h"

/* 1. Enums */
typedef enum {
    BMP180_OSS_LOW_POWER = 0,
    BMP180_OSS_STANDARD  = 1,
    BMP180_OSS_HIGH_RES  = 2,
    BMP180_OSS_ULTRA_RES = 3
} bmp180_oss_t;

/* 2. Structs */
typedef struct {
    int16_t  ac1, ac2, ac3;
    uint16_t ac4, ac5, ac6;
    int16_t  b1, b2, mb, mc, md;
} bmp180_calib_t;

typedef struct {
    i2c_device_t i2c;
    bmp180_calib_t calib;
    int32_t b5;
} bmp180_t;

/* 3. Prototypes (They must be AFTER the struct definitions) */
i2c_status_t bmp180_check_id(bmp180_t *dev);
i2c_status_t bmp180_load_calibration(bmp180_t *dev);
i2c_status_t bmp180_read_temperature(bmp180_t *dev, float *temperature);
i2c_status_t bmp180_read_pressure(bmp180_t *dev, bmp180_oss_t oss, int32_t *pressure);

#endif