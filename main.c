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

struct measurements {
    struct bme280_dev * sensor_bme280;
    float TE;
    float TI;
    float TR;
};

struct measurements temperatures;

void *measure_external_temperature(void *measurement){
    struct measurements * parameters = (struct measurements *)measurement;
    parameters->TE = get_external_temperature(parameters->sensor_bme280);
    return NULL;
}

void *read_uart(void *measurement){
    struct measurements * parameters = (struct measurements *)measurement;
    parameters->TI = get_temperature("TI");
    usleep(5e4);
    parameters->TR = get_temperature("TR");
    return NULL;
}

void *control_unit(void *measurement){
    struct measurements * parameters = (struct measurements *)measurement;
    control(2, parameters->TI, parameters->TR);
    return NULL;
}

void *show_information(void *measurement){
    struct measurements * parameters = (struct measurements *)measurement;
    printf("\n");
    printf("Temperatura Externa: %.2f °C\n", parameters->TE);
    printf("Temperatura De Referência: %.2f °C\n", parameters->TR);
    printf("Temperatura Interna: %.2f °C\n", parameters->TI);
    printf("\n");
    char *line_1 = malloc(16*sizeof(char));
    char *line_2 = malloc(16*sizeof(char));
    sprintf(line_1, "TI: %.2fTE:%.1f", parameters->TI, parameters->TE);
    sprintf(line_2, "TR: %.2f", parameters->TR);
    
    showLines(line_1, line_2);
    return NULL;
}

void sig_handler(int signum){
    pthread_t thread_id[4];
    pthread_create(&thread_id[0], NULL, measure_external_temperature, (void *)&temperatures);
    pthread_create(&thread_id[1], NULL, read_uart, (void *)&temperatures);
    pthread_create(&thread_id[2], NULL, control_unit, (void *)&temperatures);
    pthread_create(&thread_id[3], NULL, show_information, (void *)&temperatures);
    ualarm(5e5, 5e5);
}

int main(int argc, char ** argv){
    int histerese = 2;
    //float internal_temperature;
    //float reference_temperature;
    //struct bme280_dev * sensor_bme280 = create_sensor("/dev/i2c-1");

    temperatures.sensor_bme280 = create_sensor("/dev/i2c-1");

    if (wiringPiSetup () == -1) exit (1);

    fd = wiringPiI2CSetup(I2C_ADDR);

    lcd_init(); // setup LCD
    ClrLcd();

    if (!bcm2835_init())
        return 1;

    // Set the pin to be an output
    bcm2835_gpio_fsel(PIN_COOLER, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(PIN_RESISTOR, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_write(PIN_COOLER, LOW);
    bcm2835_gpio_write(PIN_RESISTOR, HIGH);

    signal(SIGALRM, sig_handler); // Register signal handler

    ualarm(5e5, 5e5);

    signal(SIGINT, &trap);
    execute = 1;
    while(execute){

    //    //pthread_t thread_id[2];
    //    //pthread_create(&thread_id[0], NULL, measure_external_temperature, (void *)&external_temperature);
    //    //pthread_create(&thread_id[1], NULL, read_uart, (void *)&uart_measurements);
    //    //internal_temperature = get_temperature("TI");
    //    //reference_temperature = get_temperature("TR");
    //    //printf("\n");
    //    //printf("Temperatura Externa: %.2f °C\n", external_temperature.TE);
    //    //printf("Temperatura De Referência: %.2f °C\n", uart_measurements.TR);
    //    //printf("Temperatura Interna: %.2f °C\n", uart_measurements.TI);
    //    //printf("\n");

    //    //char *line_1 = malloc(16*sizeof(char));
    //    //char *line_2 = malloc(16*sizeof(char));
    //    //sprintf(line_1, "TI: %.2f TE: %.1f", internal_temperature, external_temperature);
    //    //sprintf(line_2, "TR: %.2f", reference_temperature);

    //    //showLines(line_1, line_2);

    //    //control(histerese, internal_temperature, reference_temperature);
    }

    bcm2835_gpio_write(PIN_COOLER, HIGH);
    bcm2835_gpio_write(PIN_RESISTOR, HIGH);
    bcm2835_close();

    pthread_exit(NULL);

    return 0;
}
