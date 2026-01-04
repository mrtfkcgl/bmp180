#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "bmp180.h"

/* Flag to handle graceful shutdown on Ctrl+C */
static volatile bool keep_running = true;

void signal_handler(int sig) {
    (void)sig; /* Explicitly tell the compiler we intentionally don't use this */
    keep_running = false;
}

int main(void) {
    bmp180_t sensor;
    i2c_status_t status;
    float temperature = 0.0f;
    int32_t pressure = 0;

    /* Register signal handler for clean exit */
    signal(SIGINT, signal_handler);

    printf("--- BMP180 Senior Driver Test ---\n");

    /* 1. Initialize I2C Bus and Sensor Structure */
    /* Device address is 0x77, bus is /dev/i2c-1 */
    status = i2c_init(&sensor.i2c, "/dev/i2c-1", 0x77);
    if (status != I2C_OK) {
        fprintf(stderr, "Fatal: Could not initialize I2C bus.\n");
        return -1;
    }

    /* 2. Check if sensor is actually there */
    if (bmp180_check_id(&sensor) != I2C_OK) {
        close(sensor.i2c.fd);
        return -1;
    }

    /* 3. Load factory calibration coefficients */
    if (bmp180_load_calibration(&sensor) != I2C_OK) {
        fprintf(stderr, "Fatal: Failed to load calibration data.\n");
        close(sensor.i2c.fd);
        return -1;
    }

    printf("Starting measurement loop (Press Ctrl+C to stop)...\n");
    printf("----------------------------------------------\n");

    while (keep_running) {
        /* 4. Read Temperature (Mandatory to run before pressure for B5 update) */
        status = bmp180_read_temperature(&sensor, &temperature);
        if (status != I2C_OK) {
            fprintf(stderr, "Error: Failed to read temperature.\n");
            break;
        }

        /* 5. Read Pressure (Using Ultra High Resolution mode) */
        status = bmp180_read_pressure(&sensor, BMP180_OSS_ULTRA_RES, &pressure);
        if (status != I2C_OK) {
            fprintf(stderr, "Error: Failed to read pressure.\n");
            break;
        }

        /* 6. Display Data */
        /* Pressure is in Pa, converting to hPa (hectopascals) for standard use */
        printf("\rTemp: %.2f °C | Pressure: %.2f hPa", temperature, (float)pressure / 100.0f);
        fflush(stdout);

        /* Wait 1 second before next read */
        sleep(1);
    }

    /* 7. Graceful Shutdown */
    printf("\nShutting down and closing I2C bus...\n");
    close(sensor.i2c.fd);

    return 0;
}