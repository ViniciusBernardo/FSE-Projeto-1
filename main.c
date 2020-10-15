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

struct measurements {
    struct bme280_dev * sensor_bme280;
    float TE;
    float TI;
    float TR;
};

struct measurements temperatures;

pthread_mutex_t mutex_bme;
pthread_mutex_t mutex_uart;
pthread_mutex_t mutex_control;
pthread_mutex_t mutex_show;
pthread_cond_t condition_bme;
pthread_cond_t condition_uart;
pthread_cond_t condition_control;
pthread_cond_t condition_show;
unsigned int run_bme = 0;
unsigned int run_uart = 0;
unsigned int run_control = 0;
unsigned int run_show = 0;

void *read_bme(void *measurement){
    pthread_mutex_lock(&mutex_bme);
    while(!run_bme){
        pthread_cond_wait(&condition_bme, &mutex_bme);
        struct measurements * parameters = (struct measurements *)measurement;
        parameters->TE = get_external_temperature(parameters->sensor_bme280);
        run_bme = 0;
    }
    pthread_mutex_unlock(&mutex_bme);
    return NULL;
}

void *read_uart(void *measurement){
    pthread_mutex_lock(&mutex_uart);
    while(!run_uart){
        pthread_cond_wait(&condition_uart, &mutex_uart);
        struct measurements * parameters = (struct measurements *)measurement;
        parameters->TI = get_temperature("TI");
        parameters->TR = get_temperature("TR");
        run_uart = 0;
    }
    pthread_mutex_unlock(&mutex_uart);
    return NULL;
}

void *control_unit(void *measurement){
    pthread_mutex_lock(&mutex_control);
    while(!run_control){
        pthread_cond_wait(&condition_control, &mutex_control);
        struct measurements * parameters = (struct measurements *)measurement;
        control(2, parameters->TI, parameters->TR);
        run_control = 0;
    }
    pthread_mutex_unlock(&mutex_control);
    return NULL;
}

void *show_information(void *measurement){
    pthread_mutex_lock(&mutex_show); //mutex lock
    while(!run_show){
        pthread_cond_wait(&condition_show, &mutex_show); //wait for the condition

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

        run_show = 0;
    }
    pthread_mutex_unlock(&mutex_show);
    return NULL;
}

void sig_handler(int signum){
    // start bme
    pthread_mutex_lock(&mutex_bme);
    if(run_bme == 0){
        run_bme = 1;
        pthread_cond_signal(&condition_bme);
    }
    pthread_mutex_unlock(&mutex_bme);

    // start uart
    pthread_mutex_lock(&mutex_uart);
    if(run_uart == 0){
        run_uart = 1;
        pthread_cond_signal(&condition_uart);
    }
    pthread_mutex_unlock(&mutex_uart);

    // start control
    pthread_mutex_lock(&mutex_control);
    if(run_control == 0){
        run_control = 1;
        pthread_cond_signal(&condition_control);
    }
    pthread_mutex_unlock(&mutex_control);

    // start show
    pthread_mutex_lock(&mutex_show);
    if(run_show == 0){
        run_show = 1;
        pthread_cond_signal(&condition_show);
    }
    pthread_mutex_unlock(&mutex_show);
    ualarm(5e5, 5e5);
}

void exit_program(int signal){
    bcm2835_gpio_write(PIN_COOLER, HIGH);
    bcm2835_gpio_write(PIN_RESISTOR, HIGH);
    bcm2835_close();

    exit(0);
};

int main(int argc, char ** argv){
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

    signal(SIGALRM, sig_handler);
    signal(SIGINT, &exit_program);

    ualarm(5e5, 5e5);

    pthread_t thread_id[4];
    pthread_create(&thread_id[0], NULL, read_bme, (void *)&temperatures);
    pthread_create(&thread_id[1], NULL, read_uart, (void *)&temperatures);
    pthread_create(&thread_id[2], NULL, control_unit, (void *)&temperatures);
    pthread_create(&thread_id[3], NULL, show_information, (void *)&temperatures);

    pthread_join(&thread_id[0], NULL);
    pthread_join(&thread_id[1], NULL);
    pthread_join(&thread_id[2], NULL);
    pthread_join(&thread_id[3], NULL);

    pthread_exit(NULL);
    return 0;
}
