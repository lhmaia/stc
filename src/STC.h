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
#include <math.h>
#include "stree.h"
#include "usuario.h"
#include "reader.h"

using namespace std;


class clust {

public:

	vector<int> nodo;   //conjunto de nodos deste cluster (cluster basico tem apenas um nodo, mas
						// apos a fase de merge sao adicionados novos nodos)
	vector<int> first_char_index;
	vector<int> tam_sufixo;

	set<int> documentos;
	int score;

	//armazena o rotulo do cluster
	string label;


	clust (int first, int tam, int Nodo);

	int CalculaScore ();
	int tamanho_sufixo();
	int numero_documentos();
	/*
	 * a funcao atualiza documentos, copia os documentos de cada nodo para o conjunto de documentos do cluster
	 */
	void atualiza_documentos();

	/*
	 * compara duas frases com base na cobertura (numero de documentos do cluster que as contem) e
	 * no número de palavras na frase
	 */
	int compara_frase(string frase1, string frase2, int tam1, int tam2, float cobertura1, float cobertura2);

	void QuickSort (vector<string> &frases, vector<float> &cobertura);

	void Ordena (int Esq, int Dir, vector<string> &frases, vector<int> &tmp_sufixo, vector<float> &cobertura);

	void Particao (int Esq, int Dir, int &i, int &j, vector<string> &frases, vector<int> &tmp_sufixo, vector<float> &cobertura);

	//informacoes para a visualizacao de dados
	static int define_width;
	int posicaoX;
	int posicaoY;
	float raio;

	static float calcula_distancia_euclidiana (clust c1, clust c2);
	//determina a distância para os primeiros n agrupamentos básicos
	static void determina_distancia(int n);

	/*****************************************************************************/

	void processa_label();

	static vector<clust> baseclusters;
	static vector<string> documents;
	static vector<usuario*> lista_usu_doc;
	static float threshold;

	static int compara_cluster(clust c1, clust c2);
	static void ordena_clusters(vector<clust> &clusters);
	static void insere_basecluster(int first, int tam, int Nodo);
	static void merge_cluster (bool considera_usuario, int externo, int interno);
	static void Calculo_Score (vector<clust> &clusters);
	static void imprime_clusters (int n);
	static void processa_clusters(float Threshold, bool considera_usuario, int numtoprint);

	//funcao auxiliar
	string strtolower(string str);

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
	static bool similaridade (clust c1, clust c2, bool calcula_inter_usu);
	static int conta_intervencao_metrica;

	/*retorna o numero de elementos na intersecao
	 * entre o conjunto de documentos de dois clusters
	 */
	static int intersecao_doc (clust c1, clust c2);

	static float intersecao_usu (clust c1, clust c2);


};

#endif /* STC_H_ */
