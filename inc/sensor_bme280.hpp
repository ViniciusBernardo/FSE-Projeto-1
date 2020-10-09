#ifndef INC_SENSOR_BME280_HPP_
#define INC_SENSOR_BME280_HPP_

#include "bme280_defs.hpp"

using namespace std;

class SensorBME280 {
 private:
    /*< Chip Id */
    uint8_t chip_id;

    /*< Interface function pointer used to enable the device
        address for I2C and chip selection for SPI
    */
    void *interface_pointer;

    /*< Interface Selection
     * For SPI, intf = BME280_SPI_INTF
     * For I2C, intf = BME280_I2C_INTF
     * */
    enum bme280_intf interface;

    /*< Read function pointer */
    bme280_read_fptr_t read;

    /*< Write function pointer */
    bme280_write_fptr_t write;

    /*< Delay function pointer */
    bme280_delay_us_fptr_t delay_us;

    /*< Trim data */
    struct bme280_calib_data calibration_data;

    /*< Sensor settings */
    struct bme280_settings settings;

    /*< Variable to store result of read/write function */
    BME280_INTF_RET_TYPE interface_result;

 public:
    SensorBME280(
        uint8_t chip_id,
        void *interface_pointer,
        enum bme280_intf interface,
        bme280_read_fptr_t read,
        bme280_write_fptr_t write,
        bme280_delay_us_fptr_t delay_us,
        bme280_calib_data calibration_data,
        bme280_settings settings,
        BME280_INTF_RET_TYPE interface_result);

    ~SensorBME280();

    void set_chip_id(uint8_t chip_id);

    void set_interface_pointer(void *interface_pointer);

    void set_interface(enum bme280_intf interface);

    bme280_read_fptr_t get_read();
    void set_read(bme280_read_fptr_t read);

    bme280_write_fptr_t get_write();
    void set_write(bme280_write_fptr_t write);

    bme280_delay_us_fptr_t get_delay_us();
    void set_delay_us(bme280_delay_us_fptr_t delay_us);

    void set_calibration_data(bme280_calib_data calibration_data);

    void set_settings(bme280_settings settings);

    void set_interface_result(BME280_INTF_RET_TYPE interface_result);
}
#endif  // INC_SENSOR_BME280_HPP_
