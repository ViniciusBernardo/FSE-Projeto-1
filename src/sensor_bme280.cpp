#include "sensor_bme280.hpp"
#include "bme280.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

using namespace std;

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);
void user_delay_us(uint32_t period, void *intf_ptr);

/*!
 *  @brief This API is the entry point.
 *  It reads the chip-id and calibration data from the sensor.
 */
SensorBME280::SensorBME280(const char path_ic2_bus[]){
    int8_t rslt = BME280_OK;
    struct identifier id;

    if ((id.fd = open(path_ic2_bus, O_RDWR)) < 0){
        fprintf(stderr, "Failed to open the i2c bus %s\n", path_ic2_bus);
        exit(1);
    }

    id.dev_addr = BME280_I2C_ADDR_PRIM;

    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0){
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    this->device.intf = BME280_I2C_INTF;
    this->device.read = user_i2c_read;
    this->device.write = user_i2c_write;
    this->device.delay_us = user_delay_us;

    /* Update interface pointer with the structure that contains both device address and file descriptor */
    this->device.intf_ptr = &id;

    /* Initialize the bme280 */
    rslt = bme280_init(&this->device);
    if (rslt != BME280_OK){
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
        exit(1);
    }
}

/*!
 * @brief This API reads the pressure, temperature and humidity data from the
 * sensor, compensates the data and store it in the bme280_data structure
 * instance passed by the user.
 */
double SensorBME280::get_temperature(){
    /* Variable to define the result */
    int8_t rslt = BME280_OK;

    /* Variable to define the selecting sensors */
    uint8_t settings_sel = 0;

    /* Variable to store minimum wait time between consecutive measurement in force mode */
    uint32_t req_delay;

    /* Structure to get the pressure, temperature and humidity values */
    struct bme280_data comp_data;

    /* Recommended mode of operation: Indoor navigation */
    this->device.settings.osr_h = BME280_OVERSAMPLING_1X;
    this->device.settings.osr_p = BME280_OVERSAMPLING_16X;
    this->device.settings.osr_t = BME280_OVERSAMPLING_2X;
    this->device.settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    /* Set the sensor settings */
    rslt = bme280_set_sensor_settings(settings_sel, &this->device);
    if (rslt != BME280_OK){
        fprintf(stderr, "Failed to set sensor settings (code %+d).", rslt);

        return rslt;
    }

    /*Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
     *  and the oversampling configuration. */
    req_delay = bme280_cal_meas_delay(&this->device.settings);
    rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &this->device);
    if (rslt != BME280_OK){
        fprintf(stderr, "Failed to set sensor mode (code %+d).", rslt);
        exit(1);
    }

    this->device.delay_us(req_delay, this->device.intf_ptr);
    rslt = bme280_get_sensor_data(BME280_TEMP, &comp_data, &this->device);
    if (rslt != BME280_OK){
        fprintf(stderr, "Failed to get sensor data (code %+d).", rslt);
        exit(1);
    }

    printf("Temp: %.2lf\n", comp_data.temperature);
    return comp_data.temperature;
}

/*!
 * @brief This function reading the sensor's registers through I2C bus.
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr){
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr){
    uint8_t *buf;
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    buf = (uint8_t*)malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.fd, buf, len + 1) < (uint16_t)len){
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void user_delay_us(uint32_t period, void *intf_ptr){
    usleep(period);
}
