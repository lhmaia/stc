#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rslpStemmer.h"

#define TAMPALAVRA 200

int main (int argc, char **argv){

    if (argc < 4) {
        printf("\nNumero de parametros incorretos.\n");
        printf("Utilize: \n");
        printf("./stem arquivo_entrada arquivo_saida t/w\n\n");
        return (1);
    }

    char *nomearq = argv[1];
    char *nomearqsaida = argv[2];
    char *tipo_entrada = argv[3];

    if (strcmp(tipo_entrada, "t") && strcmp(tipo_entrada, "w")) {
    	printf("O terceiro parametro deve ser: t para entrada do Twitter ou w para entrada da Wikipedia\n");
    	return (1);
    }

    rslpLoadStemmer(&rslpMainStruct, "rslpconfig.txt");

    FILE *arquivo;
    FILE *arqsaida;

    arquivo = fopen(nomearq, "r");
    arqsaida = fopen(nomearqsaida, "w");

    if (arquivo && arqsaida){
        printf ("Lendo o arquivo %s\n", nomearq);

        char palavra[TAMPALAVRA];

        printf("\n");
        do {
            int conta = 0;
            char aux;
            do{ //lendo uma linha
                aux = getc(arquivo);
                if (aux != '\n' && !feof(arquivo)){
                    if (aux == '.'){
                        palavra[conta] = ' ';
                        conta++;
                    }
                    palavra[conta] = aux;
                    conta++;
                }
                //printf("%c", aux);
            }while (aux != '\n' && !feof(arquivo));
            palavra[conta] = '\0';

            char *tok;
            if (strcmp(tipo_entrada, "t") == 0){
				//desprezando o nome do usuario a data e possivel informacao de
				//retweet

				tok = strtok(palavra, " ");   //usuario que postou
			   //nao imprime: if(tok != NULL) fprintf(arqsaida, "%s ", tok);

				tok = strtok(NULL, " ");      //data do tweet
				//nao imprime: if(tok != NULL) fprintf(arqsaida, "%s ", tok);


				tok = strtok(NULL, " ");
				if (tok != NULL && strcmp(tok, "rt") == 0)  {
					//nao imprime: if(tok != NULL) fprintf(arqsaida, "%s ", tok);
					tok = strtok(NULL, " ");       //nome do usuario que teve post retwitado
					//nao imprime: if(tok != NULL) fprintf(arqsaida, "%s ", tok);
					tok = strtok(NULL, " ");
				}
            }
            
            if (strcmp(tipo_entrada, "w") == 0){
            	//desprezando links no inicio do artigo
            	strtok(palavra, ">");
            	strtok(NULL, ">");
            	tok = strtok(NULL, " ");

            }

            //trabalhando cada token da linha
            
            while(tok != NULL){
                rslpProcessWord(tok, &rslpMainStruct);
                fprintf(arqsaida, "%s", tok);
                tok = strtok(NULL, " ");
                fprintf(arqsaida, " ");
            }
            //fscanf(arquivo, "%s", palavra);
            //intf("%s", palavra);
            fprintf(arqsaida, "\n");
        }while (!feof(arquivo));
    }
    else{
        printf("Nao foi possivel ler o arquivo %s\n", nomearq);
        return 1;
    }

    rslpUnloadStemmer(&rslpMainStruct);

    fclose(arquivo);
    fclose(arqsaida);

    printf("\n***TERMINOU OK***\n");
    return 0;
}
