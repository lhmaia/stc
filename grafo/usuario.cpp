/*
 * usuario.cpp
 *
 *  Created on: 10/05/2011
 *      Author: luizhenrique
 */

#include "usuario.h"

usuario::usuario(string Nome){
	nome = Nome;
}

usuario::usuario(){

}

vector<usuario*> usuario::hash [M];

TipoPesos usuario::p;

usuario usuario::usuarios [TAM_VEC_USU];

void usuario::GeraPesos (TipoPesos p){
//funcao para gerar pesos
     int i;

     srand (time(NULL));//inciando gerador de numeros aleatorios utilizando o tempo
     for (i = 0; i < MAX_LENGTH; i++)
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
		if ( (*(*it)).nome.compare(chave) == 0) return *it;
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
	int num_triplas = seguidores.size() + segue.size() - interseccao;

	int num_triangulos = 0;
	for (vector<usuario*>::iterator it = seguidores.begin(); it < seguidores.end(); it++){
		if (contem_usu(*it, segue)) num_triangulos++;
	}
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
