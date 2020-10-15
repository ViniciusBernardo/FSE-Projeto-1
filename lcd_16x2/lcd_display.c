#include "i2c_lcd.c"


void showLines(char *line_1, char *line_2){
    lcdLoc(LINE1);
    typeln(line_1);
    lcdLoc(LINE2);
    typeln(line_2);
}
