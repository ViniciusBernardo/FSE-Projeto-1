#include "sensor_bme280/sensor_bme280.c"
#include "lcd_16x2/lcd_display.c"
#include "uart/uart.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char ** argv){
    float external_temperature;
    float reference_temperature;
    float internal_temperature;
    while(1){
        external_temperature = get_external_temperature("/dev/i2c-1");
        reference_temperature = get_temperature("TR");
        internal_temperature = get_temperature("TI");
        printf("Temperatura Externa: %.2f °C\n", external_temperature);
        printf("Temperatura De Referência: %.2f °C\n", reference_temperature);
        printf("Temperatura Interna: %.2f °C\n", internal_temperature);

        char line_1[16];
        char line_2[16];
        sprintf(line_1, "TR: %.2f TE: %.1f", reference_temperature, external_temperature);
        sprintf(line_2, "TI: %.2f", internal_temperature);
        
        showLines(line_1, line_2);
    }
    return 0;
}
