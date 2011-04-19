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



using namespace std;

class Reader{
	public:
		Reader(string Path);
		void readDocument();
	private:
		string path;
		string arquivo_original;
};

#endif /* READER_H_ */
