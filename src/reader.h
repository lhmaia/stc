/*
 * reader.h
 *
 *  Created on: 24/10/2010
 *      Author: luizhenrique
 */

#ifndef READER_H_
#define READER_H_

#include <fstream>
#include <ios>
#include <iostream>
#include <string.h>

#include "STC.h"
#include "stree.h"
#include "usuario.h"



using namespace std;

class Reader{
	public:
		Reader(string Path);
		void readDocument();
		static string arquivo_original;

		static vector<pair<set<Usuario, comp_usuario>::iterator, bool> > lista_usu_doc;
		static set <Usuario, comp_usuario> lista_usuarios;

	private:
		string path;


};

#endif /* READER_H_ */
