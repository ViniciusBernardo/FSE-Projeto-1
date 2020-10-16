#include "sensor_bme280/sensor_bme280.c"
#include "lcd_16x2/lcd_display.c"
#include "uart/uart.c"
#include "gpio/control.c"
#include "utils/csv_operations.c"
#include "utils/menu.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

pthread_mutex_t mutex_bme;
pthread_mutex_t mutex_uart;
pthread_mutex_t mutex_control;
pthread_mutex_t mutex_show;
pthread_mutex_t mutex_csv;
pthread_cond_t condition_bme;
pthread_cond_t condition_uart;
pthread_cond_t condition_control;
pthread_cond_t condition_show;
pthread_cond_t condition_csv;
unsigned int run_bme = 0;
unsigned int run_uart = 0;
unsigned int run_control = 0;
unsigned int run_show = 0;
unsigned int run_csv = 0;
unsigned int n_executions = 0;
int execute = 1;

void *read_bme(void *measurement){
    pthread_mutex_lock(&mutex_bme);
    while(!run_bme){
        pthread_cond_wait(&condition_bme, &mutex_bme);
        struct measurements * parameters = (struct measurements *)measurement;
        parameters->TE = get_external_temperature(parameters->sensor_bme280);
        run_bme = 0;
    }
    pthread_mutex_unlock(&mutex_bme);
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

void *control_temperature(void *measurement){
    pthread_mutex_lock(&mutex_control);
    while(!run_control){
        pthread_cond_wait(&condition_control, &mutex_control);
        struct measurements * parameters = (struct measurements *)measurement;
        control(parameters->histerese, parameters->TI, parameters->TR);
        run_control = 0;
    }
    pthread_mutex_unlock(&mutex_control);
    return NULL;
}

void *write_csv_file(void *measurement){
    pthread_mutex_lock(&mutex_csv);
    while(!run_csv){
        pthread_cond_wait(&condition_csv, &mutex_csv);
        struct measurements * parameters = (struct measurements *)measurement;
        write_to_csv(parameters->TE, parameters->TI, parameters->TR);
        run_csv = 0;
    }
    pthread_mutex_unlock(&mutex_csv);
    return NULL;
}

void *show_information(void *measurement){
    pthread_mutex_lock(&mutex_show); //mutex lock
    while(!run_show){
        pthread_cond_wait(&condition_show, &mutex_show); //wait for the condition

        struct measurements * parameters = (struct measurements *)measurement;
        printf("TE: %.2f°C, TI: %.2f°C, TR: %.2f°C\n", parameters->TE, parameters->TI, parameters->TR);
        char *line_1 = malloc(16*sizeof(char));
        char *line_2 = malloc(16*sizeof(char));
        sprintf(line_1, "TI:%.2f TE:%.1f", parameters->TI, parameters->TE);
        sprintf(line_2, "TR:%.2f", parameters->TR);
        showLines(line_1, line_2);

        run_show = 0;
    }
    pthread_mutex_unlock(&mutex_show);
    return NULL;
}

void sig_handler(int signum){
	n_executions++;

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

    // every 2 seconds
    if(n_executions == 4){
        n_executions = 0;
        // start csv
        pthread_mutex_lock(&mutex_csv);
        if(run_csv == 0){
            run_csv = 1;
            pthread_cond_signal(&condition_csv);
        }
        pthread_mutex_unlock(&mutex_csv);
    }
    ualarm(5e5, 5e5);
}

void exit_program(int signal){
    execute = 0;
};

int main(int argc, char ** argv){
    struct measurements temperatures;
    temperatures.sensor_bme280 = create_sensor("/dev/i2c-1");

	build_csv();
    initialize_lcd();
    initialize_gpio();
    initialize_menu(&temperatures);

    signal(SIGALRM, sig_handler);
    signal(SIGINT, &exit_program);

    ualarm(5e5, 5e5);

    pthread_mutex_init(&mutex_bme, NULL);
    pthread_mutex_init(&mutex_uart, NULL);
    pthread_mutex_init(&mutex_control, NULL);
    pthread_mutex_init(&mutex_show, NULL);
    pthread_mutex_init(&mutex_csv, NULL);
    pthread_cond_init(&condition_bme, NULL);
    pthread_cond_init(&condition_uart, NULL);
    pthread_cond_init(&condition_control, NULL);
    pthread_cond_init(&condition_show, NULL);
    pthread_cond_init(&condition_csv, NULL);

    pthread_t thread_id[5];
    pthread_create(&thread_id[0], NULL, read_bme, (void *)&temperatures);
    pthread_create(&thread_id[1], NULL, read_uart, (void *)&temperatures);
    pthread_create(&thread_id[2], NULL, control_temperature, (void *)&temperatures);
    pthread_create(&thread_id[3], NULL, show_information, (void *)&temperatures);
    pthread_create(&thread_id[4], NULL, write_csv_file, (void *)&temperatures);

    while(execute){sleep(1);}

    close_gpio();
    fclose(csv_file);

    pthread_join(&thread_id[0], NULL);
    pthread_join(&thread_id[1], NULL);
    pthread_join(&thread_id[2], NULL);
    pthread_join(&thread_id[3], NULL);
    pthread_join(&thread_id[4], NULL);

    pthread_exit(NULL);
    return 0;
}
