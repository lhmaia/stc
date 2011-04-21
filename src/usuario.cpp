/*
 * usuario.cpp
 *
 *  Created on: 21/04/2011
 *      Author: luizhenrique
 */

#include "usuario.h"

Usuario::Usuario (string Nome){
	nome = Nome;
}

string Usuario::getNome(){
	return nome;
}
