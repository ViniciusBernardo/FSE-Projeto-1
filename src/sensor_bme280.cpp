#include "sensor_bme280.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>

using namespace std;

/*!
 *  @brief This API is the entry point.
 *  It reads the chip-id and calibration data from the sensor.
 */
SensorBME280::SensorBME280(
    uint8_t device_address,
    int8_t file_descriptor
){
    int8_t result;

    /* chip id read try count */
    uint8_t try_count = 5;
    uint8_t chip_id = 0;

    this->settings.osr_t = BME280_OVERSAMPLING_2X;
    this->settings.filter = BME280_FILTER_COEFF_16;
    this->device_address = device_address;
    this->file_descriptor = file_descriptor;

    while (try_count){
        /* Read the chip-id of bme280 sensor */
        result = get_register_data(BME280_CHIP_ID_ADDR, &chip_id, 1);

        /* Check for chip id validity */
        if ((result == BME280_OK) && (chip_id == BME280_CHIP_ID)){
            this->chip_id = chip_id;

            /* Reset the sensor */
            result = soft_reset();

            if (result == BME280_OK){
                /* Read the calibration data */
                result = get_calib_data();
            }

            break;
        }

        /* Wait for 1 ms */
        delay_us(1000);
        --try_count;
    }

    /* Chip id check failed */
    if (!try_count){
        result = BME280_E_DEV_NOT_FOUND;
    }
}

/*!
 * @brief This API reads the pressure, temperature and humidity data from the
 * sensor, compensates the data and store it in the bme280_data structure
 * instance passed by the user.
 */
double SensorBME280::get_temperature(){
    int8_t rslt;
    double temperature;

    /* Array to store the pressure, temperature and humidity data read from
     * the sensor
     */
    uint8_t reg_data[BME280_P_T_H_DATA_LEN] = { 0 };
    struct bme280_uncomp_data uncomp_data = { 0 };

    /* Read the pressure and temperature data from the sensor */
    rslt = get_register_data(BME280_DATA_ADDR, reg_data, BME280_P_T_H_DATA_LEN);

    if (rslt == BME280_OK){
        uint32_t data_xlsb;
        uint32_t data_lsb;
        uint32_t data_msb;

        /* Parse the read data from the sensor */
        data_msb = (uint32_t)reg_data[3] << 12;
        data_lsb = (uint32_t)reg_data[4] << 4;
        data_xlsb = (uint32_t)reg_data[5] >> 4;
        uncomp_data.temperature = data_msb | data_lsb | data_xlsb;

        /* Compensate the pressure and/or temperature and/or
         * humidity data from the sensor
         */
        temperature = compensate_temperature(&uncomp_data);
    }

    return temperature;
}

/*!
 * @brief This internal API is used to compensate the raw temperature data and
 * return the compensated temperature data in double data type.
 */
double SensorBME280::compensate_temperature(const struct bme280_uncomp_data *uncomp_data){
    double var1;
    double var2;
    double temperature;
    double temperature_min = -40;
    double temperature_max = 85;

    var1 = ((double)uncomp_data->temperature) / 16384.0 - ((double)this->calibration_data.dig_t1) / 1024.0;
    var1 = var1 * ((double)this->calibration_data.dig_t2);
    var2 = (((double)uncomp_data->temperature) / 131072.0 - ((double)this->calibration_data.dig_t1) / 8192.0);
    var2 = (var2 * var2) * ((double)this->calibration_data.dig_t3);
    this->calibration_data.t_fine = (int32_t)(var1 + var2);
    temperature = (var1 + var2) / 5120.0;

    if (temperature < temperature_min){
        temperature = temperature_min;
    } else if (temperature > temperature_max){
        temperature = temperature_max;
    }

    return temperature;
}

/*!
 * @brief This function reading the sensor's registers through I2C bus.
 */
int8_t SensorBME280::read_data(uint8_t register_address, uint8_t *data, uint32_t length){
    write(this->file_descriptor, &register_address, 1);
    read(this->file_descriptor, data, length);

    return 0;
}

/*!
 * @brief This function for writing the sensor's registers through I2C bus.
 */
int8_t SensorBME280::write_data(uint8_t register_address, const uint8_t *data, uint32_t length){
    uint8_t *buf;

    buf = (uint8_t*)malloc(length + 1);
    buf[0] = register_address;
    memcpy(buf + 1, data, length);
    if (write(this->file_descriptor, buf, length + 1) < (uint16_t)length){
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

/*!
 * @brief This API reads the data from the given register address of the sensor.
 */
int8_t SensorBME280::get_register_data(uint8_t register_address, uint8_t *register_data, uint16_t length){
    int8_t result;

    /* Read the data  */
    this->interface_result = read_data(register_address, register_data, length);

    /* Check for communication error */
    if (this->interface_result != BME280_INTF_RET_SUCCESS){
        result = BME280_E_COMM_FAIL;
    }

    return result;
}

/*!
 * @brief This API writes the given data to the register address
 * of the sensor.
 */
int8_t SensorBME280::set_register_data(uint8_t *register_address, const uint8_t *register_data, uint8_t length){
    int8_t result;
    uint8_t temp_buff[20]; /* Typically not to write more than 10 registers */

    if (length > 10){
        length = 10;
    }

    uint16_t temp_len;
    uint8_t reg_addr_cnt;

    if (length != 0){
        temp_buff[0] = register_data[0];

        /* Burst write mode */
        if (length > 1){
            /* Interleave register address w.r.t data for
             * burst write
             */
            interleave_reg_addr(register_address, temp_buff, register_data, length);
            temp_len = ((length * 2) - 1);
        } else {
            temp_len = length;
        }

        this->interface_result = write_data(register_address[0], temp_buff, temp_len);

        /* Check for communication error */
        if (this->interface_result != BME280_INTF_RET_SUCCESS){
            result = BME280_E_COMM_FAIL;
        }
    } else {
        result = BME280_E_INVALID_LEN;
    }

    return result;
}

/*!
 *  @brief This internal API is used to parse the temperature and
 *  pressure calibration data and store it in device structure.
 */
void SensorBME280::parse_temperature_calib_data(const uint8_t *reg_data){
    this->calibration_data.dig_t1 = BME280_CONCAT_BYTES(reg_data[1], reg_data[0]);
    this->calibration_data.dig_t2 = (int16_t)BME280_CONCAT_BYTES(reg_data[3], reg_data[2]);
    this->calibration_data.dig_t3 = (int16_t)BME280_CONCAT_BYTES(reg_data[5], reg_data[4]);
}

/*!
 * @brief This internal API reads the calibration data from the sensor, parse
 * it and store in the device structure.
 */
int8_t SensorBME280::get_calib_data(){
    int8_t rslt;
    uint8_t reg_addr = BME280_TEMP_PRESS_CALIB_DATA_ADDR;

    /* Array to store calibration data */
    uint8_t calib_data[BME280_TEMP_PRESS_CALIB_DATA_LEN] = { 0 };

    /* Read the calibration data from the sensor */
    rslt = get_register_data(reg_addr, calib_data, BME280_TEMP_PRESS_CALIB_DATA_LEN);

    if (rslt == BME280_OK){
        /* Parse temperature and pressure calibration data and store
         * it in device structure
         */
        parse_temperature_calib_data(calib_data);
    }

    return rslt;
}

/*!
 * @brief This API performs the soft reset of the sensor.
 */
int8_t SensorBME280::soft_reset(){
    int8_t result;
    uint8_t register_address = BME280_RESET_ADDR;
    uint8_t status_reg = 0;
    uint8_t try_run = 5;

    /* 0xB6 is the soft reset command */
    uint8_t soft_rst_cmd = BME280_SOFT_RESET_COMMAND;

    /* Write the soft reset command in the sensor */
    result = set_register_data(&register_address, &soft_rst_cmd, 1);

    if (result == BME280_OK){
        /* If NVM not copied yet, Wait for NVM to copy */
        do{
            /* As per data sheet - Table 1, startup time is 2 ms. */
            delay_us(2000);
            result = get_register_data(BME280_STATUS_REG_ADDR, &status_reg, 1);

        } while ((result == BME280_OK) && (try_run--) && (status_reg & BME280_STATUS_IM_UPDATE));

        if (status_reg & BME280_STATUS_IM_UPDATE){
            result = BME280_E_NVM_COPY_FAILED;
        }
    }

    return result;
}

/*!
 * @brief This internal API interleaves the register address between the
 * register data buffer for burst write operation.
 */
void SensorBME280::interleave_reg_addr(const uint8_t *register_address, uint8_t *temp_buff, const uint8_t *register_data, uint8_t length){
    uint8_t index;

    for (index = 1; index < length; index++){
        temp_buff[(index * 2) - 1] = register_address[index];
        temp_buff[index * 2] = register_data[index];
    }
}

/*!
 * @brief This function provides the delay for required time (Microseconds) as per the input provided in some of the
 * APIs
 */
void SensorBME280::delay_us(uint32_t period){
    usleep(period);
}
