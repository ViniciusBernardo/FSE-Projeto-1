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
    SensorBME280 *sensor_temperatura_externa = new SensorBME280("/dev/i2c-1");
    while(1){
        printf("Temperatura Externa: %.2lf °C\n", sensor_temperatura_externa->get_temperature());
	sleep(1);
    }
    return 0;
}
