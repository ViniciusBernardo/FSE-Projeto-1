#include "sensor_bme280.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char ** argv){
    struct bme280_dev device = create_sensor("/dev/i2c-1");
    while(1){
        printf("Temperatura Externa: %.2lf °C\n", get_temperature(&device));
        sleep(1);
    }
    return 0;
}
