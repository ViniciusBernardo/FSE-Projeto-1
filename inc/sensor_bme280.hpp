#ifndef INC_SENSOR_BME280_HPP_
#define INC_SENSOR_BME280_HPP_

#include "bme280_defs.h"

using namespace std;

class SensorBME280 {
 private:
    /*< Chip Id */
    uint8_t chip_id;

    uint8_t device_address;

    int8_t file_descriptor;

    /*< Trim data */
    struct bme280_calib_data calibration_data;

    /*< Sensor settings */
    struct bme280_settings settings;

    /*< Variable to store result of read/write function */
    BME280_INTF_RET_TYPE interface_result;

    int8_t get_register_data(uint8_t register_address, uint8_t *register_data, uint16_t length);
    void parse_temperature_calib_data(const uint8_t *reg_data);
    int8_t get_calib_data();
    double compensate_temperature(const struct bme280_uncomp_data *uncomp_data);
    int8_t set_sensor_mode(uint8_t sensor_mode);
    int8_t write_power_mode(uint8_t sensor_mode);
    int8_t read_data(uint8_t register_address, uint8_t *data, uint32_t length);
    int8_t write_data(uint8_t register_address, const uint8_t *data, uint32_t length);
    int8_t set_register_data(uint8_t *register_address, const uint8_t *register_data, uint8_t length);
    int8_t soft_reset();
    int8_t put_device_to_sleep();
    int8_t reload_device_settings(const struct bme280_settings *settings);
    void interleave_reg_addr(const uint8_t *register_address, uint8_t *temp_buff, const uint8_t *register_data, uint8_t length);
    void delay_us(uint32_t period);

 public:
    SensorBME280(uint8_t device_address, int8_t file_descriptor);
    ~SensorBME280();
    double get_temperature();
};
#endif  // INC_SENSOR_BME280_HPP_
