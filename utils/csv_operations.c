#include <stdio.h>
#include <time.h>

FILE * build_csv(){
    FILE *csv_file;
    csv_file = fopen("./measurements.csv", "w+");

    if(csv_file == NULL){
        printf("Error to open CSV!");
        exit(1);
    }

    fprintf(csv_file, "%s,%s,%s,%s,%s\n", "Data", "Hora", "Temperatura Interna", "Temperatura Externa", "Temperatura de Referência");

    return csv_file;
}


void write_to_csv(FILE *csv_file, float TE, float, TI, float TR) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    fprintf(
        csv_file,
        "%02d-%02d-%d,%02d:%02d:%02d,%.2f,%.2f,%.2f\n",
        local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
        local->tm_hour, local->tm_min, local->tm_sec,
        TI, TE, TR
    );
}
