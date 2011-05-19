/*
 * reader.cpp
 *
 *  Created on: 24/10/2010
 *      Author: luizhenrique
 */

#include "reader.h"

string Reader::arquivo_original;

vector<pair<set<Usuario>::iterator, bool> > Reader::lista_usu_doc;
set <Usuario, comp_usuario> Reader::lista_usuarios;

Reader::Reader(string Path, string Path_origin){
	path = Path;
	arquivo_original = Path_origin;
}

void Reader::readDocument(){
	ifstream stream_doc(path.c_str(), ios::in);
	ifstream stream_doc_origin(arquivo_original.c_str(), ios::in);

	if (!stream_doc.is_open() || !stream_doc_origin.is_open()) {
		cout << "arquivo nao encontrado." << endl;
		exit(1);
	}
	string document;
	string document_origin;
	getline(stream_doc, document);
	getline(stream_doc_origin, document_origin);

	//gerando pesos para a funcao hash da arvore de sufixos
	Edge::GeraPesos (Edge::p);
	int doc = 1;
	int conta = 0;

	while(!stream_doc.eof()){
		char docaux [document.size()];
		char docaux_origin[document_origin.size()];
		clust::documents.push_back(document);

		strcpy(docaux, document.c_str());
		strcpy(docaux_origin, document_origin.c_str());

		//cout << docaux_origin << endl;

		set<tipo_termo, comp_tipotermo> termos_doc;

		//nome de usuario, data e informacao de retweet
		string usu = strtok(docaux_origin, " ");    //nome de usuario
		Usuario user_tmp(usu);

		//incluindo usuario na lista
		pair<set<Usuario, comp_usuario>::iterator, bool> retorno;
		retorno = lista_usuarios.insert(user_tmp);
		lista_usu_doc.push_back(retorno);

/*
		strtok(NULL, " ") << endl;      //data do tweet
		char *auxTok = strtok(NULL, " ");      //rt

		if (auxTok != NULL && strcmp(auxTok, "rt") == 0) {
			strtok(NULL, " ");
		}
*/


		char *frase = NULL;
		frase = strtok(docaux, ".;?!");
		int conta_frase = 0;

		while(frase != NULL){
			//cout << tmp_frases.size() << " " << conta_frase << endl;
			if (frase != NULL) inserir_frase(frase, doc, termos_doc);
			//cout << frase << endl;
			frase = strtok(NULL, ".;?!");
			conta_frase++;
			conta++;

		}
		//cout << "numero de edges: " << Edge::numedges << endl;

		pair <set<tipo_termo, comp_tipotermo>::iterator, bool > aux;
		//inserindo o termo no conjunto geral de termos e contando o numero de documentos onde aparece
		for(set<tipo_termo, comp_tipotermo>::iterator itermo = termos_doc.begin(); itermo != termos_doc.end(); itermo++){
			aux = Edge::conjunto_termos.insert(*itermo);
			if(aux.second == false) {

				tipo_termo tmp = *(aux.first);
				Edge::conjunto_termos.erase(aux.first);
				tmp.incrementa();

				Edge::conjunto_termos.insert(tmp);
			}
		}

		doc++;

		getline(stream_doc, document);
		getline(stream_doc_origin, document_origin);
	}
	//dump_edges(Edge::N);
	//walk_tree(0, 0, 0);
	/*
	cout << "====================================" << endl;
	cout << "lista de usuarios:" << endl << endl;

	int usu = 1;
	for(set<Usuario, comp_usuario>::iterator it = lista_usuarios.begin(); it != lista_usuarios.end(); it++){
		cout << usu << " " << (*it).nome << endl;
		usu++;
	}
	cout << "====================================" << endl;

	cout << "====================================" << endl;

	cout << "lista de usuarios por doc:" << endl << endl;
	usu = 1;
	for(vector<pair<set<Usuario, comp_usuario>::iterator, bool> >::iterator it = lista_usu_doc.begin(); it < lista_usu_doc.end(); it++){
		cout << usu << " " << (*(*it).first).nome << endl;
		usu++;
	}
	cout << "====================================" << endl;
	*/
}
