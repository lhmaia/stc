/*
 * main.cpp
 *
 *  Created on: 10/05/2011
 *      Author: luizhenrique
 */

#include <fstream>
#include "usuario.h"
#include <string.h>

int main(int argc, char** argv){

	if (argc < 1) {
		cout << endl << "Numero de argumentos invalidos" << endl
		           << "./graf <nome arquivo>" << endl;
		return 1;
	}


	string nome_arq = argv[1];

	cout << endl << "Processando arquivo: " << argv[1] << endl;

	usuario::GeraPesos(usuario::p);

	ifstream stream_doc(nome_arq.c_str(), ios::in);

	if (!stream_doc.is_open()) {
		cout << "arquivo nao encontrado." << endl;
		exit(1);
	}

	//cout << sizeof(usuario) << endl; return 0;

	cout << "Preenchendo Hash ..." << endl;

	string linha;
	getline(stream_doc, linha);

	unsigned long int conta = 0;
	unsigned long int contalinha = 1;

	while(!stream_doc.eof()){

		char *nomeusuario;
		char docaux[linha.size() + 1];
		strcpy(docaux, linha.c_str());

		//lendo o nome do usuario
		nomeusuario = strtok(docaux, "	");
		//cout << conta << ": " << nomeusuario << endl;

		if (usuario::pesquisa_hash(nomeusuario) == NULL){
			usuario usu_tmp(nomeusuario);

			usuario::usuarios[conta] = usu_tmp;

			usuario::insere_hash(&(usuario::usuarios[conta]));

			conta++;
		}

		//cout << nomeusuario << "-";


        //if(conta%100000 == 0) cout << '\r' << conta << " -  "  << contalinha << " - " << nomeusuario  << endl;

		//lendo o nome do seguidor
		nomeusuario = strtok(NULL, " ");

		if (nomeusuario != NULL){
			if (usuario::pesquisa_hash(nomeusuario) == NULL){

				usuario usu_tmp(nomeusuario);

				usuario::usuarios[conta] = usu_tmp;

				usuario::insere_hash(&(usuario::usuarios[conta]));

				conta++;
			}
		}

		//cout << nomeusuario << ",";

		getline(stream_doc, linha);
		contalinha++;

	}

	int numusuarios = conta;

	cout << "Preenchendo Grafo ..." << endl;

	//retorna o arquivo para a primeira linha
	stream_doc.close();

	ifstream stream_doc1 (nome_arq.c_str(), ios::in);

	getline(stream_doc1, linha);

    contalinha = 1;

	char *nomeusuario;
	char docaux[MAX_LENGTH * 2 + 2];
	strcpy(docaux, linha.c_str());

	nomeusuario = strtok(docaux, "	");

	char aux [MAX_LENGTH];

	while(!stream_doc1.eof()){


		usuario* usu_aux = usuario::pesquisa_hash(nomeusuario);

		do{
			strcpy(aux, nomeusuario);

			nomeusuario = strtok(NULL, "	");

			if (nomeusuario != NULL){
				usuario* usu =  usuario::pesquisa_hash(nomeusuario);

				if (usu == NULL)
					cout << "Usuario " << nomeusuario << " nao encontrado" << endl;
				else{
					usu_aux->seguidores.push_back(usu);
					usu->segue.push_back(usu_aux);
				}

				//cout << usuario::usuarios[conta].nome << "	" << nomeusuario << endl;
			}

			getline(stream_doc1, linha);
			contalinha++;
			strcpy(docaux, linha.c_str());
			nomeusuario = strtok(docaux, "	");

		}while(!stream_doc1.eof() && (strcmp(aux, nomeusuario) == 0));

	}

	stream_doc1.close();

	cout << "Calculando coeficientes locais ..." << endl;
	usuario::num_triangulos_total = 0;
	usuario::num_triplas_total = 0;

	double sum_ci = 0;

	for(int it = 0; it < numusuarios; it++){
		sum_ci += usuario::usuarios[it].calcula_coefficient();
		//cout << usuario::usuarios[it].lcoefficient << ", ";
	}

	cout << "Soma dos coeficientes locais: " << sum_ci << endl;

	double cluster_coefficient_local = sum_ci / (double) numusuarios;

	cout << "Total de usuarios: " << numusuarios << endl;
	cout << endl << "Clustering Coefficient média local: " << cluster_coefficient_local << endl;

	cout << endl << "Total de triplas: " << usuario::num_triplas_total << endl;
	cout << "Total de triangulos: " << usuario::num_triangulos_total << endl;

    //getchar();

    cout << "TERMINOU OK" << endl;

	return 0;
}
