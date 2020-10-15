/**\
 * Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **/

/**
 * \ingroup bme280
 * \defgroup bme280Examples Examples
 * @brief Reference Examples
 */

/*!
 * @ingroup bme280Examples
 * @defgroup bme280GroupExampleLU linux_userspace
 * @brief Linux userspace test code, simple and mose code directly from the doco.
 * compile like this: gcc linux_userspace.c ../bme280.c -I ../ -o bme280
 * tested: Raspberry Pi.
 * Use like: ./bme280 /dev/i2c-0
 * \include linux_userspace.c
 */

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

/******************************************************************************/
/*!                         System header files                               */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

/******************************************************************************/
/*!                         Own header files                                  */
#include "bme280.c"

/******************************************************************************/
/*!                               Structures                                  */

/* Structure that contains identifier details used in example */
struct identifier
{
    /* Variable to hold device address */
    uint8_t dev_addr;

    /* Variable that contains file descriptor */
    int8_t fd;
};

/* Structure that contains sensor_bme280 struct and the external temperature */
struct external_measurement
{
    struct bme280_dev * sensor_bme280;

    float TE;
};

/****************************************************************************/
/*!                         Functions                                       */

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs.
 *
 * @param[in] period              : Delay in microseconds.
 * @param[in, out] intf_ptr       : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *  @return void.
 *
 */
void user_delay_us(uint32_t period, void *intf_ptr);

/*!
 *  @brief Function for reading the sensor's registers through I2C bus.
 *
 *  @param[in] reg_addr       : Register address.
 *  @param[out] data          : Pointer to the data buffer to store the read data.
 *  @param[in] len            : No of bytes to read.
 *  @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs.
 *
 *  @return Status of execution
 *
 *  @retval 0 -> Success
 *  @retval > 0 -> Failure Info
 *
 */
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);

/*!
 *  @brief Function for writing the sensor's registers through I2C bus.
 *
 *  @param[in] reg_addr       : Register address.
 *  @param[in] data           : Pointer to the data buffer whose value is to be written.
 *  @param[in] len            : No of bytes to write.
 *  @param[in, out] intf_ptr  : Void pointer that can enable the linking of descriptors
 *                                  for interface related call backs
 *
 *  @return Status of execution
 *
 *  @retval BME280_OK -> Success
 *  @retval BME280_E_COMM_FAIL -> Communication failure.
 *
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);

float get_external_temperature(struct bme280_dev * device);

struct bme280_dev * create_sensor(const char path_ic2_bus[]);

struct bme280_dev * create_sensor(const char path_ic2_bus[]){
    struct bme280_dev * device = (struct bme280_dev*)malloc(sizeof(struct bme280_dev));

    struct identifier * id = (struct identifier*)malloc(sizeof(struct identifier));

    /* Variable to define the result */
    int8_t rslt = BME280_OK;

    if ((id->fd = open(path_ic2_bus, O_RDWR)) < 0){
        fprintf(stderr, "Failed to open the i2c bus %s\n", path_ic2_bus);
        exit(1);
    }

    /* Make sure to select BME280_I2C_ADDR_PRIM or BME280_I2C_ADDR_SEC as needed */
    id->dev_addr = BME280_I2C_ADDR_PRIM;

    if (ioctl(id->fd, I2C_SLAVE, id->dev_addr) < 0){
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    device->intf = BME280_I2C_INTF;
    device->read = user_i2c_read;
    device->write = user_i2c_write;
    device->delay_us = user_delay_us;

    /* Update interface pointer with the structure that contains both device address and file descriptor */
    device->intf_ptr = id;

    /* Initialize the bme280 */
    rslt = bme280_init(device);
    if (rslt != BME280_OK){
        fprintf(stderr, "Failed to initialize the device (code %+d).\n", rslt);
        exit(1);
    }

    /* Variable to define the selecting sensors */
    uint8_t settings_sel = 0;

    /* Recommended mode of operation: Indoor navigation */
    device->settings.osr_h = BME280_OVERSAMPLING_1X;
    device->settings.osr_p = BME280_OVERSAMPLING_16X;
    device->settings.osr_t = BME280_OVERSAMPLING_2X;
    device->settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    /* Set the sensor settings */
    rslt = bme280_set_sensor_settings(settings_sel, device);
    if (rslt != BME280_OK){
        fprintf(stderr, "Failed to set sensor settings (code %+d).\n", rslt);
        exit(1);
    }

    return device;
}

float get_external_temperature(struct bme280_dev * device){

    /* Variable to define the result */
    int8_t rslt = BME280_OK;

    /* Variable to store minimum wait time between consecutive measurement in force mode */
    uint32_t req_delay;

    /* Structure to get the pressure, temperature and humidity values */
    struct bme280_data comp_data;

    /*Calculate the minimum delay required between consecutive measurement based upon the sensor enabled
     *  and the oversampling configuration. */
    req_delay = bme280_cal_meas_delay(&device->settings);

    /* Set the sensor to forced mode */
    rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, device);
    if (rslt != BME280_OK){
        fprintf(stderr, "Failed to set sensor mode (code %+d).\n", rslt);
        exit(1);
    }

    /* Wait for the measurement to complete and print data */
    device->delay_us(8e4, device->intf_ptr);
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, device);
    if (rslt != BME280_OK){
        fprintf(stderr, "Failed to get sensor data (code %+d).\n", rslt);
        exit(1);
    }

    float temperature;

#ifdef BME280_FLOAT_ENABLE
    temperature = comp_data.temperature;
#else
#ifdef BME280_64BIT_ENABLE
    temperature = 0.01f * comp_data.temperature;
#else
    temperature = 0.01f * comp_data.temperature;
#endif
#endif

    return temperature;
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
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void user_delay_us(uint32_t period, void *intf_ptr){
    usleep(period);
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr){
    uint8_t *buf;
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.fd, buf, len + 1) < (uint16_t)len)
    {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}
