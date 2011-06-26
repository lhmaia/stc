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
		Reader(string Path, string Path_origin);
		void readDocument(int num_docs, bool considera_usuario);
		static string arquivo_original;

	private:
		string path;


};

#endif /* READER_H_ */
