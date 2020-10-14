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

void *measure_external_temperature(void *measurement){
    struct external_measurement * parameters = (struct external_measurement *)measurement;
    parameters->TE = get_external_temperature(parameters->sensor_bme280);
}

void *read_uart(void *TI_TR_structure){
    struct uart_temperatures * uart_measurements = (struct uart_temperatures *)TI_TR_structure;
    uart_measurements.TI = get_temperature("TI");
    uart_measurements.TR = get_temperature("TR");
}

int main(int argc, char ** argv){
    int histerese = 4;
    struct external_measurement external_temperature;
    struct uart_temperatures uart_measurements;
    //float internal_temperature;
    //float reference_temperature;
    //struct bme280_dev * sensor_bme280 = create_sensor("/dev/i2c-1");

    external_temperature.sensor_bme280 = create_sensor("/dev/i2c-1");

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

    signal(SIGINT, &trap);
    execute = 1;
    while(execute){

        pthread_t thread_id[2];
        pthread_create(&thread_id[0], NULL, measure_external_temperature, (void *)&external_temperature);
        pthread_create(&thread_id[1], NULL, read_uart, (void *)&uart_measurements);
        //internal_temperature = get_temperature("TI");
        //reference_temperature = get_temperature("TR");
        printf("\n");
        printf("Temperatura Externa: %.2f °C\n", external_temperature.TE);
        printf("Temperatura De Referência: %.2f °C\n", uart_measurements.TR);
        printf("Temperatura Interna: %.2f °C\n", uart_measurements.TI);
        printf("\n");

        //char *line_1 = malloc(16*sizeof(char));
        //char *line_2 = malloc(16*sizeof(char));
        //sprintf(line_1, "TI: %.2f TE: %.1f", internal_temperature, external_temperature);
        //sprintf(line_2, "TR: %.2f", reference_temperature);

        //showLines(line_1, line_2);

        //control(histerese, internal_temperature, reference_temperature);
        sleep(1);
    }

    //bcm2835_gpio_write(PIN_COOLER, HIGH);
    //bcm2835_gpio_write(PIN_RESISTOR, HIGH);
    //bcm2835_close();
    pthread_exit(NULL);

    return 0;
}
