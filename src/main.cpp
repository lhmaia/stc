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


	// check the number of arguments
	if (argc < 5)
    {
		cout << "./a.out -i <nome_arquivo> -o <nome_arquivo_original>" << endl;
  	    exit(1);
    }

	int option;
	// loop to receive information from the command line
    while ( (option = getopt (argc, argv, "i:o:t:p:d:v:")) != EOF )
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

		// mensagem de erro
		default:
			cout << "./a.out -i <nome_arquivo>" << endl;
			exit(1);
		}
	}

	Reader r (input_file_name, input_file_origin);
	r.readDocument();

	//execucao do processamento dos clusters

	clust::processa_clusters(threshold);

	clust::imprime_clusters(numtoprint);

    return 0;
}
