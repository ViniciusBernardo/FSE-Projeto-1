#ifndef INC_SENSOR_BME280_HPP_
#define INC_SENSOR_BME280_HPP_

#include "bme280_defs.h"

using namespace std;

class SensorBME280 {
 private:
    struct bme280_dev device;

 public:
    SensorBME280(const char path_ic2_bus[]);
    ~SensorBME280();
    double get_temperature();
};
#endif  // INC_SENSOR_BME280_HPP_
