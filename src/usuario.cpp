/*
 * usuario.cpp
 *
 *  Created on: 21/04/2011
 *      Author: luizhenrique
 */

#define VERB if(true)

#include "usuario.h"

usuario::usuario(string Nome){
	nome = Nome;
}

usuario::usuario(){

}

string usuario::getNome(){
	return nome;
}

vector<usuario*> usuario::hash [M];

TipoPesosUsu usuario::p;

usuario usuario::usuarios [TAM_VEC_USU];

int usuario::num_triangulos_total;
unsigned long usuario::num_triplas_total;

void usuario::GeraPesos (TipoPesosUsu p){
//funcao para gerar pesos
     int i;

     srand (time(NULL));//inciando gerador de numeros aleatorios utilizando o tempo
     for (i = 0; i < MAX_LENGTH_USU; i++)
          p[i] = 1 + (int)(10000.0 * rand()/(RAND_MAX+1.0));
}

int usuario::hashing (string chave){

	unsigned int soma = 0;

	int comp = chave.size();
	for(int i = 0; i < comp; i++){
		soma += (unsigned int) chave.at(i) * p[i];
	}

	return (soma % M);
}

usuario* usuario::pesquisa_hash (string chave){

	int indice = hashing(chave);

	for(vector<usuario*>::iterator it = hash[indice].begin(); it < hash[indice].end(); it++){
		if ( (*it)->nome.compare(chave) == 0) return *it;
	}
	return NULL;
}

void usuario::insere_hash (usuario* usu){
	string Nome = (*usu).nome;
	int indice = hashing(Nome);

	hash[indice].push_back(usu);
}

double usuario::calcula_coefficient (){
	int interseccao = 0;
	for (vector<usuario*>::iterator it = segue.begin(); it < segue.end(); it++){
		if (contem_usu(*it, seguidores)) interseccao++;
	}
	int num_triplas = ( (seguidores.size() * segue.size()) / 2 ) - ( ( interseccao * interseccao) / 2 );

	int num_triangulos = 0;
	for (vector<usuario*>::iterator it = seguidores.begin(); it < seguidores.end(); it++){
		if (contem_usu(*it, segue)) num_triangulos++;
	}

	num_triplas_total += num_triplas;
	num_triangulos_total += num_triangulos;

	if (num_triplas == 0) lcoefficient = 0;
	else
		lcoefficient = ( (double) num_triangulos ) / ( (double) num_triplas );

	return lcoefficient;
}

bool usuario::contem_usu (usuario* usu, vector<usuario*> &lista){
	for(vector<usuario*>::iterator it = lista.begin(); it < lista.end(); it++){
		if ((*it)->nome.compare(usu->nome) == 0) return true;
	}
	return false;
}

void usuario::carregar_usuarios (){
	string nome_arq = "/media/Documentos/Documentos/Computacao/Computacao201101/POC2/rede_ordenada.txt";

	VERB cout << endl << "Processando arquivo: " << endl;

	usuario::GeraPesos(usuario::p);

	ifstream stream_doc(nome_arq.c_str(), ios::in);

	if (!stream_doc.is_open()) {
		cout << "arquivo nao encontrado." << endl;
		exit(1);
	}

	//cout << sizeof(usuario) << endl; return 0;

	VERB cout << "Preenchendo Hash ..." << endl;

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

	VERB cout << "Preenchendo Grafo ..." << endl;

	//retorna o arquivo para a primeira linha
	stream_doc.close();

	ifstream stream_doc1 (nome_arq.c_str(), ios::in);

	getline(stream_doc1, linha);

	contalinha = 1;

	char *nomeusuario;
	char docaux[MAX_LENGTH_USU * 2 + 2];
	strcpy(docaux, linha.c_str());

	nomeusuario = strtok(docaux, "	");

	char aux [MAX_LENGTH_USU];

	while(!stream_doc1.eof()){


		usuario* usu_aux = usuario::pesquisa_hash(nomeusuario);

		do{
			strcpy(aux, nomeusuario);

			nomeusuario = strtok(NULL, "	");

			if (nomeusuario != NULL){
				usuario* usu =  usuario::pesquisa_hash(nomeusuario);

				if (usu == NULL)
					VERB cout << "Usuario " << nomeusuario << " nao encontrado" << endl;
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
}
