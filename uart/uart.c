#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>


float get_temperature(const char *temperature_name) { // temperature_name = TI || TR
    int uart0_filestream = -1;
    float temperature = -40;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    if(uart0_filestream == -1) {
        printf("Erro - Porta serial não pode ser aberta. Confirme se não está sendo usada por outra aplicação.\n");
        return temperature;
    }

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;		//<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;

    if(temperature_name == "TI") {
        p_tx_buffer = &tx_buffer[0];
        *p_tx_buffer++ = 0xA1;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 3;
        *p_tx_buffer++ = 3;
        *p_tx_buffer++ = 1;
    } else if (temperature_name == "TR") {
        p_tx_buffer = &tx_buffer[0];
        *p_tx_buffer++ = 0xA2;
        *p_tx_buffer++ = 1;
        *p_tx_buffer++ = 3;
        *p_tx_buffer++ = 3;
        *p_tx_buffer++ = 1;
    } else {
        printf("Invalid temperature_name!! (%s)\n", temperature_name);
        return temperature;
    }

    if (uart0_filestream != -1) {
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));		//Filestream, bytes to write, number of bytes to write
        if (count < 0){
            printf("UART TX error\n");
        }
    }

    sleep(1);

    //----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1) {
        // Read up to 255 characters from the port if they are there
        unsigned char rx_buffer[256];
        int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);		//Filestream, buffer to store in, number of bytes to read (max)
        if (rx_length < 0) {
            printf("Erro na leitura.\n");
        } else if (rx_length == 0) {
            printf("Nenhum dado disponível.\n");
        } else {
            //Bytes received
            rx_buffer[rx_length] = '\0';
            temperature = (*(float*)rx_buffer);
        }
    }

    close(uart0_filestream);
    return temperature;
}
