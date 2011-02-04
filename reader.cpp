/*
 * reader.cpp
 *
 *  Created on: 24/10/2010
 *      Author: luizhenrique
 */

#include "reader.h"

Reader::Reader(string Path){
	path = Path;
}

void Reader::readDocument(){
	ifstream stream_doc(path.c_str(), ios::in);
	string document;
	getline(stream_doc, document);

	//gerando pesos para a funcao hash da arvore de sufixos
	Edge::GeraPesos (Edge::p);
	int doc = 1;
	int conta = 0;

	while(!stream_doc.eof()){
		char docaux [document.size()];
		clust::documents.push_back(document);
		strcpy(docaux, document.c_str());

		set<tipo_termo, comp_tipotermo> termos_doc;

		char *frase = strtok(docaux, ".;?!");



		while(frase != NULL){
			if (frase != NULL) inserir_frase(frase, doc, termos_doc);
			//cout << frase << endl;
			frase = strtok(NULL, ".;?!");
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
	}
	//dump_edges(Edge::N);
	//walk_tree(0, 0, 0);

	return;

	cout << endl << "****** CONJUNTO DE TERMOS *******" << endl;
	for(set<tipo_termo>::iterator itermo = Edge::conjunto_termos.begin(); itermo != Edge::conjunto_termos.end(); itermo++){
		cout << (*itermo).termo << " - " << (*itermo).numdocs << endl;
	}
	cout << "-----------------------------------" << endl;

	clust::Calculo_Score(clust::baseclusters);

	//ordenando os clusters por valor do score
	clust::ordena_clusters(clust::baseclusters);

	cout << endl << "***** Imprimindo Base Clusters *****" << endl;

	cout << "Numero de base clusters: " << clust::baseclusters.size() << endl;

	clust::imprime_clusters(50);


	cout << endl << "--------------------------------------------------------" << endl;
	cout << "SIMILARIDADE" << endl;
	clust::merge_cluster();
	cout << endl;

	clust::Calculo_Score(clust::baseclusters);

	//ordenando os clusters por valor do score
	clust::ordena_clusters(clust::baseclusters);


	cout << "Numero de clusters finais: " << clust::baseclusters.size() << endl;

	clust::imprime_clusters(50);

}
