/*
 * usuario.h
 *
 *  Created on: 10/05/2011
 *      Author: luizhenrique
 */

#ifndef USUARIO_H_
#define USUARIO_H_

#define M 				18795429
#define MAX_LENGTH 		50 		//tamanho maximo de nome de usuario no twitter
#define TAM_VEC_USU 	10700000

#include <iostream>
#include <vector>
#include <cstdlib>


typedef int TipoPesos[MAX_LENGTH];

using namespace std;

class usuario {

public:

	usuario();
	usuario(string Nome);

	string nome;
	vector <usuario*> seguidores;
	vector <usuario*> segue;

	double lcoefficient;

	double calcula_coefficient ();

	bool contem_usu (usuario* usu, vector<usuario*> &lista);

	//static vector<usuario> usuarios;
	static usuario usuarios[TAM_VEC_USU];

	static vector<usuario*> hash [M];

	static TipoPesos p;

	static void GeraPesos (TipoPesos p);

	static int hashing (string chave);

	static usuario* pesquisa_hash (string chave);

	static void insere_hash (usuario* usu);

	static int num_triangulos_total;
	static unsigned long num_triplas_total;
};





#endif /* USUARIO_H_ */
