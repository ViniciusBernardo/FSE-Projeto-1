#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

struct uart_temperatures{
    float TI;
    float TR;
};

int * initialize_uart(){
    int * uart0_filestream = (int *)malloc(sizeof(int));

    *uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    if(*uart0_filestream == -1) {
        printf("Erro - Porta serial não pode ser aberta. Confirme se não está sendo usada por outra aplicação.\n");
    }

    struct termios options;
    tcgetattr(*uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(*uart0_filestream, TCIFLUSH);
    tcsetattr(*uart0_filestream, TCSANOW, &options);
    return uart0_filestream;
}

void get_temperature(unsigned char command, int * uart0_filestream, float * measurement, float TE) { // temperature_name = TI || TR

    unsigned char tx_buffer[5] = {command, 1, 3, 3, 1};

    if (*uart0_filestream != -1) {
        int count = write(*uart0_filestream, tx_buffer, 5);
        if (count < 0){
            printf("UART TX error\n");
        }
    }

    usleep(2e5);

    //----- CHECK FOR ANY RX BYTES -----
    if (*uart0_filestream != -1) {
        unsigned char rx_buffer[100];
        int rx_length = read(*uart0_filestream, (void*)rx_buffer, 10);
        if (rx_length < 0) {
            //printf("Erro na leitura da UART.\n");
        } else if (rx_length == 0) {
            printf("Nenhum dado disponível.\n");
        } else {
            //Bytes received
	    float dado = 0;
            memcpy(&dado, &rx_buffer[0], 4);
	    if(command == 0xA1){
	        if(dado < *measurement + 5 && dado > *measurement - 5 || *measurement == 0){
	            *measurement = dado;
	        }
	    } else if(command == 0xA2) {
		if(dado >= TE || dado <= 75){
	            *measurement = dado;
		}
	    }
        }
    }
}

void close_uart(int * uart0_filestream) {
    close(*uart0_filestream);
}
