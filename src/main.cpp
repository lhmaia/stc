/*
 * main.cpp
 *
 *  Created on: 23/10/2010
 *      Author: luizhenrique
 */


#include "reader.h"
#include "stree.h"

#define NUM_TO_PRINT 50

extern int verbose;

int main(int argc, char** argv)
{
	//cout << "tamanha de um edge: " << sizeof(Edge) << endl;

	char *input_file_name = NULL;
	char *input_file_origin = NULL;
	int numtoprint = 0;
	int numtostatistic = 0;
	float threshold = 0.5;
	bool considera_usuario = false;
	int num_docs = 0;


	// check the number of arguments
	if (argc < 5)
    {
		cout << "./a.out -i <nome_arquivo> -o <nome_arquivo_original>" << endl;
  	    exit(1);
    }

	int option;
	// loop to receive information from the command line
    while ( (option = getopt (argc, argv, "i:o:t:n:p:d:v:u:")) != EOF )
	{
    	switch (option)
		{
			// input file
    	case 'i':
	  	input_file_name = strdup(optarg);
			break;

			// arquivo original
    	case 'o':
		input_file_origin = strdup(optarg);
			break;

			//valor do threshold
		case 't':
		threshold = atof(optarg);
			break;

			//numero de documentos
		case 'n' :
		num_docs = atoi(optarg);
			break;

			//numero de clusters para impressao na tela
    	case 'p':
			numtoprint = atoi(optarg);
				break;

    	case 'd':
			numtostatistic = atoi(optarg);
				break;

    	case 'v':
    		verbose = true;
    		break;

    	case 'u':
    		considera_usuario = true;
    		break;

		// mensagem de erro
		default:
			cout << "./a.out -i <nome_arquivo>" << endl;
			exit(1);
		}
	}
    //considera_usuario = true;
	if (considera_usuario) usuario::carregar_usuarios();

	Reader r (input_file_name, input_file_origin);
	r.readDocument(num_docs, considera_usuario);

	//execucao do processamento dos clusters

	clust::processa_clusters(threshold, considera_usuario, numtoprint);

    return 0;
}
