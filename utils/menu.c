#include<stdio.h>

struct measurements {
    struct bme280_dev * sensor_bme280;
    int * uart_filestream;
    float TE;
    float TI;
    float TR;
    int histerese;
};

void initialize_menu(struct measurements * input){
    float TR;
	int histerese, temp, status;

	printf("Entre com a histerese: ");
	status = scanf("%d", &histerese);
	while(status!=1){
		while((temp=getchar()) != EOF && temp != '\n');
		printf("Entrada inválida... Por favor entre com um número: ");
		status = scanf("%d", &histerese);
	}

    input->histerese = histerese;

	printf("Entre com a Temperatura de Referência (TR): ");
	status = scanf("%f", &TR);
	while(status!=1){
		while((temp=getchar()) != EOF && temp != '\n');
		printf("Entrada inválida... Por favor entre com um número: ");
		status = scanf("%f", &TR);
	}

    input->TR = TR;

	printf("Histerese: %d\n", histerese);
	printf("Temperatura de Referência(TR): %.2f\n", TR);
	printf("Você pode mudar a TR a qualquer momento pelo Potenciômetro.\n");
}
