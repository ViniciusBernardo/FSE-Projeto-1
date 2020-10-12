#include <bcm2835.h>
#include <stdio.h>
 
#define PIN_COOLER RPI_GPIO_P1_18
#define PIN_RESISTOR RPI_GPIO_P1_16
 
void control(int histerese, float TI, float TR) {

    float upper_limit = TR + (float)histerese/2;
    float lower_limit = TR - (float)histerese/2;

    if(TI < lower_limit) {
        bcm2835_gpio_write(PIN_COOLER, LOW);
        bcm2835_gpio_write(PIN_RESISTOR, HIGH);
    } else if (TI > upper_limit) {
        bcm2835_gpio_write(PIN_COOLER, HIGH);
        bcm2835_gpio_write(PIN_RESISTOR, LOW);
    }
}
