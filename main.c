#include "sensor_bme280/sensor_bme280.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char ** argv){
    while(1){
        printf("Temperatura Externa: %.2f °C\n", get_external_temperature("/dev/i2c-1"));
    }
    return 0;
}
