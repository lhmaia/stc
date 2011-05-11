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

	string nome_arq = "/var/tmp/rede_seguidores.csv";

	usuario::GeraPesos(usuario::p);

	ifstream stream_doc(nome_arq.c_str(), ios::in);

	if (!stream_doc.is_open()) {
		cout << "arquivo nao encontrado." << endl;
		exit(1);
	}

	string linha;
	getline(stream_doc, linha);

	int conta = 0;

	while(!stream_doc.eof()){

		char *nomeusuario;
		char docaux[linha.size() + 1];
		strcpy(docaux, linha.c_str());

		//lendo o nome do usuario
		nomeusuario = strtok(docaux, " ");
		cout << conta << ": " << nomeusuario << endl;

		if (usuario::pesquisa_hash(nomeusuario) == NULL){
			usuario usu_tmp(nomeusuario);

			usuario::usuarios[conta] = usu_tmp;

			usuario::insere_hash(&(usuario::usuarios[conta]));

			conta++;
		}

		//lendo o nome do seguidor
		nomeusuario = strtok(NULL, " ");
		cout << conta << ": " << nomeusuario << endl;

		if (usuario::pesquisa_hash(nomeusuario) == NULL){
			cout << "nao esta no hash" << endl;
			usuario usu_tmp(nomeusuario);

			usuario::usuarios[conta] = usu_tmp;

			usuario::insere_hash(&(usuario::usuarios[conta]));

			conta++;
		}

		getline(stream_doc, linha);
	}


	return 0;
}
