/*
 * STC.h
 *
 *  Created on: 22/10/2010
 *      Author: luizhenrique
 */

#ifndef STC_H_
#define STC_H_

#define MIN_DOCS_CLUSTER 2
#define N_RELEVANTES 500

#define VERB if(verbose)

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "stree.h"

using namespace std;


class clust {

public:

	vector<int> nodo;
	vector<int> first_char_index;
	vector<int> tam_sufixo;

	set<int> documentos;
	int score;


	clust (int first, int tam, int Nodo);

	int CalculaScore ();
	int tamanho_sufixo();
	int numero_documentos();
	/*
	 * a funcao atualiza documentos, copia os documentos de cada nodo para o conjunto de documentos do cluster
	 */
	void atualiza_documentos();

	static vector<clust> baseclusters;
	static vector<string> documents;
	static float threshold;

	static int compara_cluster(clust c1, clust c2);
	static void ordena_clusters(vector<clust> &clusters);
	static void insere_basecluster(int first, int tam, int Nodo);
	static void merge_cluster ();
	static void Calculo_Score (vector<clust> &clusters);
	static void imprime_clusters (int n);
	static void processa_clusters(float Threshold);

	//funcoes para estatisticas
	/*
	 * gerando graficos de sufixo x numero de ocorrencia por documento no cluster
	 * sera gerado um arquivo no formato do gnuplot para cada um dos n primeiros clusters
	 */
	static void grafico_termo_por_doc(int n_primeiros);

	/*
	 * dois clusters sao similares se
	 * |intersecao| / |docs 1| > thresold e
	 * |intersecao| / |docs 2| > thresold
	 * neste caso retorna true
	 */
	static bool similaridade (clust c1, clust c2);

	/*retorna o numero de elementos na intersecao
	 * entre o conjunto de documentos de dois clusters
	 */
	static int intersecao_doc (clust c1, clust c2);


};

#endif /* STC_H_ */
