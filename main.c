#include "sensor_bme280/sensor_bme280.c"
#include "lcd_16x2/lcd_display.c"
#include "uart/uart.c"
#include "gpio/control.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

int execute;
void trap(int signal){ execute = 0; }

void *measure_external_temperature(void *temperature){
    float * external_temperature = (float *)arg;
    *external_temperature = get_external_temperature(sensor_bme280);
}

int main(int argc, char ** argv){
    int histerese = 4;
    float external_temperature;
    float internal_temperature;
    float reference_temperature;
    struct bme280_dev * sensor_bme280 = create_sensor("/dev/i2c-1");

    //if (wiringPiSetup () == -1) exit (1);

    //fd = wiringPiI2CSetup(I2C_ADDR);

    //lcd_init(); // setup LCD

    //if (!bcm2835_init())
    //    return 1;

    //// Set the pin to be an output
    //bcm2835_gpio_fsel(PIN_COOLER, BCM2835_GPIO_FSEL_OUTP);
    //bcm2835_gpio_fsel(PIN_RESISTOR, BCM2835_GPIO_FSEL_OUTP);

    //bcm2835_gpio_write(PIN_COOLER, HIGH);
    //bcm2835_gpio_write(PIN_RESISTOR, LOW);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, measure_external_temperature, (void *)&external_temperature);

    signal(SIGINT, &trap);
    execute = 1;
    while(execute){
        //internal_temperature = get_temperature("TI");
        //reference_temperature = get_temperature("TR");
        //printf("\n");
        printf("Temperatura Externa: %.2f °C\n", external_temperature);
        //printf("Temperatura De Referência: %.2f °C\n", reference_temperature);
        //printf("Temperatura Interna: %.2f °C\n", internal_temperature);
        //printf("\n");

        //char *line_1 = malloc(16*sizeof(char));
        //char *line_2 = malloc(16*sizeof(char));
        //sprintf(line_1, "TI: %.2f TE: %.1f", internal_temperature, external_temperature);
        //sprintf(line_2, "TR: %.2f", reference_temperature);

        //showLines(line_1, line_2);

        //control(histerese, internal_temperature, reference_temperature);
    }

    //bcm2835_gpio_write(PIN_COOLER, HIGH);
    //bcm2835_gpio_write(PIN_RESISTOR, HIGH);
    //bcm2835_close();

    return 0;
}
