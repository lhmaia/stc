/*
 * usuario.h
 *
 *  Created on: 21/04/2011
 *      Author: luizhenrique
 */


#ifndef USUARIO_H_
#define USUARIO_H_

#define M 				18795429
#define MAX_LENGTH_USU 		50 		//tamanho maximo de nome de usuario no twitter
#define TAM_VEC_USU 	10700000

#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <string.h>


typedef int TipoPesosUsu[MAX_LENGTH_USU];

using namespace std;

class usuario {

public:

	usuario();
	usuario(string Nome);

	string nome;
	vector <usuario*> seguidores;
	vector <usuario*> segue;

	string getNome();

	double lcoefficient;

	double calcula_coefficient ();

	bool contem_usu (usuario* usu, vector<usuario*> &lista);

	//static vector<usuario> usuarios;
	static usuario usuarios[TAM_VEC_USU];

	static vector<usuario*> hash [M];

	static TipoPesosUsu p;

	static void GeraPesos (TipoPesosUsu p);

	static int hashing (string chave);

	static usuario* pesquisa_hash (string chave);

	static void insere_hash (usuario* usu);

	static int num_triangulos_total;
	static unsigned long num_triplas_total;

	static void carregar_usuarios ();
};

struct comp_usuario{
	bool operator() (const usuario& u1, const usuario& u2) const{
		if (u1.nome.compare(u2.nome) < 0) return true;
		else return false;
	}
};


#endif /* USUARIO_H_ */


