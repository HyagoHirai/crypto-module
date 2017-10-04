#include <stdio.h>
#include <string.h>

int tamanhoTabela = 10;

/*Rotina que transforma uma string em um número
que será usado depois na função hash
*/
int stringParaInt(char *string) {
    int tamanho, primeira, segunda; //Inteiros que representam o tamanho,
                                    //o código da primeira letra da string
                                    //e o código da segunda letra.
    tamanho =  strlen(string);      //Mede o tamanho da string
    primeira = string[0];           //Obtém o código da primeira letra
    segunda = string[1];            //Obtém o código da segunda letra
    int resultado = (tamanho * primeira) + segunda; //Função de transformação
    return resultado;  //Retorna número que representa a string
    }

/*A função mais simples de hash;
Para uma tabela com n posições (n == tamanhoTabela)
Toma-se o módulo n do valor inteiro gerado na
função "stringParaInt"
Retornará um número entre 0 e 19.
*/
int hash(int valor) {
    return valor % tamanhoTabela;  
    }


/*Rotina principal
Captura strings quaisquer e gera a chave correspondente para tabela hash.
*/
int main() {
    int i;
    char dado[50];

    printf("\nDefina o tamanho da tabela: ");
    scanf("%d", &tamanhoTabela);

    for (i=0; i<tamanhoTabela; i++) {
   
    printf("\nDigite uma palavra qualquer: ");
    gets(dado);

    printf("A chave para a tabela (de 0 a %d) é: %d", tamanhoTabela-1, hash(stringParaInt(dado)));
    }

}
