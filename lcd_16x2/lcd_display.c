#include "i2c_lcd.c"


void showLines(char *line_1, char *line_2){
    if (wiringPiSetup () == -1) exit (1);

    fd = wiringPiI2CSetup(I2C_ADDR);

    lcd_init(); // setup LCD

    lcdLoc(LINE1);
    typeln(line_1);
    lcdLoc(LINE2);
    typeln(line_2);
}
