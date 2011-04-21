/*
 * usuario.h
 *
 *  Created on: 21/04/2011
 *      Author: luizhenrique
 */



#ifndef USUARIO_H_
#define USUARIO_H_

#include <iostream>
#include <vector>
#include <set>

using namespace std;

class Usuario {

	public:
		string nome;
		Usuario (string Nome);
		string getNome();
		vector <string> following;

};

#endif /* USUARIO_H_ */

struct comp_usuario{
	bool operator() (const Usuario& u1, const Usuario& u2) const{
		if (u1.nome.compare(u2.nome) < 0) return true;
		else return false;
	}
};
