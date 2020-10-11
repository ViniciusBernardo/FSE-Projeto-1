#include "sensor_bme280/sensor_bme280.c"
#include "lcd_16x2/i2c_lcd.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char ** argv){
    float external_temperature;
    while(1){
        external_temperature = get_external_temperature("/dev/i2c-1");
        printf("Temperatura Externa: %.2f °C\n", external_temperature);

        char line_1[16];
        sprintf(line_1, "TE: %.2f", external_temperature); 
        
        writeOnLCD(line_1, "");
    }
    return 0;
}
