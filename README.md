# FSE-Projeto-1

## Compilando e executando o projeto
* Para compilar o projeto, entre com o seguinte comando na pasta raiz:
```
make
```

* Para executar o projeto, entre com o seguinte comando na pasta raiz:
```
make run
```

* Para excluir o binário do projeto, execute o seguinte comando na pasta raiz:
```
make clean
```

## Limitações conhecidas
* Os valores salvos no arquivo csv são sobrescritos a cada execução do projeto;
* O valor de TR (Temperatura de Referência) definido pelo usuário como input do teclado é sobrescrito pelo valor retornado da UART;
* Não é possivel entrar com a TR pelo teclado em tempo de execução, apenas pelo Potenciômetro.
