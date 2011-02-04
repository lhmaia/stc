/*
 * main.cpp
 *
 *  Created on: 23/10/2010
 *      Author: luizhenrique
 */


#include "reader.h"
#include "stree.h"
#include <sys/time.h>
#include <unistd.h>
#include <sys/resource.h>

#define NUM_TO_PRINT 50

extern int verbose;

int main(int argc, char** argv)
{
	//cout << "tamanha de um edge: " << sizeof(Edge) << endl;

	char *input_file_name = NULL;
	int numtoprint = 0;
	int numtostatistic = 0;
	float threshold = 0.5;


	// check the number of arguments
	if (argc < 3)
    {
		cout << "./a.out -i <nome_arquivo>" << endl;
  	    exit(1);
    }

	int option;
	// loop to receive information from the command line
    while ( (option = getopt (argc, argv, "i:t:p:d:v:")) != EOF )
	{
    	switch (option)
		{
			// input file
    	case 'i':
	  	input_file_name = strdup(optarg);
			break;

			//valo do threshold
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

		// error message
		default:
			cout << "./a.out -i <nome_arquivo>" << endl;
			exit(1);
		}
	}

	Reader r (input_file_name);
	r.readDocument();

	/*
	 * ********** Medida do tempo de execucao ************
	 * variaveis para medida do tempo de execucao

		double tusuario;
		double tsistema;
		double texecucao;
		struct timeval tinicio;
		struct timeval tfim;
		struct rusage recursos;
		struct timeval usuario;
		struct timeval sistema;

		gettimeofday(&tinicio, NULL);


	 //* *****************************************************/



		//execucao do processamento dos clusters

		clust::processa_clusters(threshold);

	/*
	 * ********** Medida do tempo de execucao ************
	 * variaveis para medida do tempo de execucao


	//calculo do tempo
		gettimeofday(&tfim, NULL);

		getrusage(RUSAGE_SELF, &recursos);
		usuario = recursos.ru_utime;
		sistema = recursos.ru_stime;

		tusuario = (double) recursos.ru_utime.tv_sec + 1.e-6 * (double) recursos.ru_utime.tv_usec;
		tsistema = (double) recursos.ru_stime.tv_sec + 1.e-6 * (double) recursos.ru_stime.tv_usec;

		texecucao = (tfim.tv_sec-tinicio.tv_sec)*1.e6;
		texecucao = (texecucao+(tfim.tv_usec-tinicio.tv_usec))*1.e-6;
		/*
		printf("tempo de execucao: %5f\n", texecucao);
		printf("tempo de usuario: %5f\n", tusuario);
		printf("tempo de sistema: %5f\n", tsistema);
		printf("--------------------------------------------------\n");

		cout << clust::documents.size() << " " << texecucao << endl;


	 //* *****************************************************/


	clust::imprime_clusters(numtoprint);

	//cout << endl << "----------------------------------------------------------------" << endl;
	//cout << "***IMPRIMINDO ESTATISTICAS***" << endl;

	//clust::grafico_termo_por_doc(numtostatistic);

	//cout << endl << "----------------------------------------------------------------" << endl;

    return 0;
}
