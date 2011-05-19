/*
 * main.cpp
 *
 *  Created on: 10/05/2011
 *      Author: luizhenrique
 */

#include <fstream>
#include "usuario.h"
#include <string.h>

int main(){

	string nome_arq = "/home/luizhenrique/Desktop/POC2/rede_seguidores.csv";

	usuario::GeraPesos(usuario::p);

	ifstream stream_doc(nome_arq.c_str(), ios::in);

	if (!stream_doc.is_open()) {
		cout << "arquivo nao encontrado." << endl;
		exit(1);
	}

	//cout << sizeof(usuario) << endl; return 0;

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

		getline(stream_doc, linha);
		contalinha++;
	}

	//retorna o arquivo para a primeira linha
	stream_doc.seekg(0, ios::beg);

	getline(stream_doc, linha);

	conta = 0;

	while(!stream_doc.eof()){

		char *nomeusuario;
		char docaux[linha.size() + 1];
		strcpy(docaux, linha.c_str());

		//while (usu)

		getline(stream_doc, linha);
	}

	stream_doc.close();

    getchar();

	return 0;
}
