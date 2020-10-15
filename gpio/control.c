#include <bcm2835.h>
#include <stdio.h>
 
#define PIN_COOLER RPI_GPIO_P1_18
#define PIN_RESISTOR RPI_GPIO_P1_16
 
void control(int histerese, float TI, float TR) {

    float upper_limit = TR + (float)histerese/2;
    float lower_limit = TR - (float)histerese/2;

    if(TI < lower_limit) {
        bcm2835_gpio_write(PIN_RESISTOR, LOW);
        bcm2835_delay(50);
        bcm2835_gpio_write(PIN_COOLER, HIGH);
        bcm2835_delay(50);
    } else if (TI > upper_limit) {
        bcm2835_gpio_write(PIN_RESISTOR, HIGH);
        bcm2835_delay(50);
        bcm2835_gpio_write(PIN_COOLER, LOW);
        bcm2835_delay(50);
    }
}

void initialize_gpio(){
    if (!bcm2835_init())
        return 1;

    bcm2835_gpio_fsel(PIN_COOLER, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(PIN_RESISTOR, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_write(PIN_COOLER, LOW);
    bcm2835_gpio_write(PIN_RESISTOR, HIGH);
}

void close_gpio(){
    bcm2835_gpio_write(PIN_COOLER, HIGH);
    bcm2835_gpio_write(PIN_RESISTOR, HIGH);
    bcm2835_close();
}
