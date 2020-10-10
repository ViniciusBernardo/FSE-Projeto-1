#include "sensor_bme280.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char ** argv){
    uint8_t device_address = BME280_I2C_ADDR_PRIM;
    int8_t file_descriptor;

    if ((file_descriptor = open("/dev/i2c-1", O_RDWR)) < 0){
        fprintf(stderr, "Failed to open the i2c bus %s\n", argv[1]);
        exit(1);
    }

    if (ioctl(file_descriptor, I2C_SLAVE, device_address) < 0){
        fprintf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }
    SensorBME280 *sensor_temperatura_externa = new SensorBME280(device_address, file_descriptor);
    while(1){
        printf("Temperatura Externa: %.2lf °C\n", sensor_temperatura_externa->get_temperature());
	sleep(1);
    }
    return 0;
}
