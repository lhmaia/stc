/*
 * STC.cpp
 *
 *  Created on: 22/10/2010
 *      Author: luizhenrique
 */

#include "STC.h"
#include <sys/time.h>
#include <unistd.h>
#include <sys/resource.h>

bool verbose = false;

vector<clust> clust::baseclusters;
vector<string> clust::documents;
float clust::threshold;

void clust::Calculo_Score (vector<clust> &clusters){
	for(vector<clust>::iterator it = clusters.begin(); it < clusters.end(); it++)
		(*it).CalculaScore();
}

void clust::insere_basecluster(int first, int tam, int Nodo){
	clust c (first, tam, Nodo);
	//c.CalculaScore();
	baseclusters.push_back(c);
}

int clust::compara_cluster(clust c1, clust c2){
	return (c1.score > c2.score);
}

void clust::ordena_clusters(vector<clust> &clusters){
	sort(clusters.begin(), clusters.end(), compara_cluster);
}

/*retorna o numero de elementos na intersecao
 * entre o conjunto de documentos de dois clusters
 */

int clust::intersecao_doc (clust c1, clust c2){
	set<int>::iterator it1 = c1.documentos.begin();
	set<int>::iterator it2 = c2.documentos.begin();

	int intersecao = 0;

	while(it1 != c1.documentos.end() && it2 != c2.documentos.end()){
		if ((*it1) == (*it2)){
			intersecao++;
			it1++;
			it2++;
		}
		else {
			if ((*it1) < (*it2)) it1++;
			else it2++;
		}
	}

	return intersecao;

}

int clust::intersecao_usu (clust c1, clust c2){
	/*
	 * PROXIMO PASSO - TRANSFORMAR INTERSECAO EM JACCARD
	 * JACCARD VARIA DE 0 A 1
	 * NA SIMILARIDADE FAZER FORMULA CONSIDERANDO PESOS
	 * clustering coefficient
	 */

	set<string> conj_c1;
	set<string> conj_c2;

	for(set<int>::iterator it = c1.documentos.begin(); it != c1.documentos.end(); it++){
		conj_c1.insert(Reader::lista_usu_doc.at((*it) - 1).first->nome);
	}

	for(set<int>::iterator it = c2.documentos.begin(); it != c2.documentos.end(); it++){
		conj_c2.insert(Reader::lista_usu_doc.at((*it) - 1).first->nome);
	}

	int intersecao = 0;
	set<string>::iterator it1 = conj_c1.begin();
	set<string>::iterator it2 = conj_c2.begin();

	while(it1 != conj_c1.end() && it2 != conj_c2.end()){
		if ((*it1).compare(*it2) == 0){
			intersecao++;
			it1++;
			it2++;
		}
		else {
			if ((*it1).compare(*it2) < 0) it1++;
			else it2++;
		}
	}

	return intersecao;
}

/*
	 * dois clusters sao similares se
	 * |intersecao| / |docs 1| > thresold e
	 * |intersecao| / |docs 2| > thresold
	 * neste caso retorna true
	 */
bool clust::similaridade (clust c1, clust c2){
	int intersecao = intersecao_doc(c1, c2);
	//cout << " intersecao: " << intersecao << " ";
	bool comp1 = (( (float) intersecao )/( (float) c1.numero_documentos() )) > threshold;
	bool comp2 = (( (float) intersecao )/( (float) c2.numero_documentos() )) > threshold;

	return (comp1 && comp2);
}

clust::clust (int first, int tam, int Nodo){
	first_char_index.push_back(first);
	tam_sufixo.push_back(tam);
	nodo.push_back(Nodo);
}

int clust::CalculaScore (){

	const int mindocs = 3;    //para que um termo seja considerado deve aparecer pelo menos mindocs vezes
	const float max_doc_percent = 0.4; //e no maximo max_doc_percent

	int totalnumdocs = documents.size();
	//funcao que considera a relevancia de cada palavra de acordo com o numero de documentos onde aparece
	int f = 0;

	vector<int>::iterator ifirst =  first_char_index.begin();
	vector<int>::iterator itam = 	tam_sufixo.begin();

	for (;ifirst < first_char_index.end();){

		for (int i = (*ifirst); i < (*ifirst + *itam); i++){
			//cout << Edge::termos.at(i) << endl;
			tipo_termo termoaux (Edge::termos.at(i));
			set<tipo_termo>::iterator it = Edge::conjunto_termos.find(termoaux);
			if ((*it).numdocs > mindocs && ((float)(*it).numdocs/(float)totalnumdocs) <= max_doc_percent)
				f++;
		}

		ifirst++;
		itam++;
	}

	score = numero_documentos() * f;
	//cout << "Score: " << score << endl;
	return score;
}

int clust::tamanho_sufixo(){
	int tam = 0;

	for (vector<int>::iterator it = tam_sufixo.begin(); it < tam_sufixo.end(); it++)
		tam += (*it);

	return tam;
}

void clust::atualiza_documentos(){
	for (vector<int>::iterator it = nodo.begin(); it < nodo.end(); it++)
			for (set<int>::iterator is = Node::Nodes[(*it)].docs.begin(); is != Node::Nodes[(*it)].docs.end(); is++)
				documentos.insert(*is);
}

int clust::numero_documentos(){

	return documentos.size();
}

void clust::merge_cluster (){
	int externo = N_RELEVANTES;
	int interno = N_RELEVANTES;
	int comp = 0;
	for (vector<clust>::iterator it1 = baseclusters.begin(); it1 < baseclusters.end() && it1 < (baseclusters.begin() + externo); it1++){
		for (vector<clust>::iterator it2 = baseclusters.begin(); it2 < baseclusters.end() && it2 < (baseclusters.begin() + interno) && it1 < baseclusters.end();){
			comp++;
			if (it1 != it2 && similaridade(*it1, *it2)){
				vector<int>::iterator inodo = 	(*it2).nodo.begin();
				vector<int>::iterator ifirst =  (*it2).first_char_index.begin();
				vector<int>::iterator itam = 	(*it2).tam_sufixo.begin();

				for (;inodo < (*it2).nodo.end();){
					//cout << *inodo << " ";
					(*it1).nodo.push_back(*inodo);
					(*it1).first_char_index.push_back(*ifirst);
					(*it1).tam_sufixo.push_back(*itam);

					inodo++;
					ifirst++;
					itam++;
				}
				//cout << " merge com: " << (*it1).nodo.at(0);
				//cout << endl;

				//(*it1).atualiza_documentos();
				for (set<int>::iterator tmp = (*it2).documentos.begin(); tmp != (*it2).documentos.end(); tmp++)
					(*it1).documentos.insert(*tmp);


				baseclusters.erase(it2);
				interno--;
				externo--;
				if (it1 >= it2) {
					it1++;
				}
			}
			else{
				it2++;
			}
			//if (interno >= N_RELEVANTES) break;
		}
		//if (externo >= N_RELEVANTES) break;
	}
	VERB cout << "Numero de comparacoes: " << comp << endl;
}

void clust::processa_label(){
	//parametros do label
	int num_frases = 6;
	int max = 20;
	int min_dif_cobertura = 20;
	int max_overlap = 60;

	//gerando frases
	vector<string> frases;
	vector<float> cobertura;
	string tmp_frase;
	int tmp_numdocs;


	for (unsigned int t = 0; t < first_char_index.size(); t++){
		tmp_frase = "";
		//cout << "Sufixo de " << first_char_index.at(t) << " de tamanho " << tam_sufixo.at(t) << endl;
		for (int i = first_char_index.at(t); i <= (tam_sufixo.at(t) + first_char_index.at(t) - 1); i++){
			//cout << Edge::termos.at(i) << " ";
			tmp_frase.append(Edge::termos.at(i));
			if ((i + 1) <= (tam_sufixo.at(t) + first_char_index.at(t) - 1)) tmp_frase.append(" ");
		}

		size_t found = string::npos;
		tmp_numdocs = 0;
		for (set<int>::iterator it = documentos.begin(); it != documentos.end(); it++){
			//verificando a cobertura
			found = documents.at(*it - 1).find(tmp_frase);
			if (found != string::npos) tmp_numdocs++;

		}
		float cob = ((float) tmp_numdocs / (float) documentos.size()) * 100;

		cobertura.push_back(cob);
		frases.push_back(tmp_frase);
	}
	//cout << compara_frase(frases.at(0), frases.at(2), tam_sufixo.at(0), tam_sufixo.at(2)) << endl;

	//ordenando as frases de acordo com a cobertura e o numero de termos

	//sort(frases.begin(), frases.end(), this.compara);
	QuickSort(frases, cobertura);

	if (frases.size() < max) max = frases.size();
	/*
	 * a matriz abaixo possui uma coluna contendo a
	 * informacao se a frase e most specific, most general e se e ou nao selecionada
	 */
	bool tabela_seleciona [max][3];

	for (int i = 0; i < max; i++){
		bool most_general = true;
		bool most_specific = true;
		bool most_general_low = true;
		for (int j = 0; j < max; j++){
			if(i != j){
				//verificando se frase j e super frase de i
				if (frases.at(j).find(frases.at(i)) != string::npos){
					//tem super frase
					most_specific = false;
					if ((cobertura.at(i) - cobertura.at(j)) >= min_dif_cobertura) most_general_low = false;
				}
				//verificando se frase j e sub frase de i
				if (frases.at(i).find(frases.at(j)) != string::npos){
					//tem sub frase
					most_general = false;
				}

				//aplicando heuristica Word Overlap (se ha overlap de mais de 60% das palavras nao e selecionada)
				if (i < j){
					size_t found = string::npos;
					int tam_overlap = 0;
					int num_palavras = 0;

					char aux[frases.at(i).size()];
					strcpy(aux, frases.at(i).c_str());

					char* palavra = strtok(aux, " ");

					while(palavra != NULL){
						num_palavras++;
						if (palavra != NULL) {
							if(frases.at(j).find(palavra) != string::npos) tam_overlap++;
						}
						palavra = strtok(NULL, " ");
					}
					if (((float)tam_overlap / (float) num_palavras) > max_overlap) {
						most_general = false;
						most_specific = false;
					}
				}
			}
		}
		bool selecionada;
		if (most_specific) selecionada = true;
		else{
			if (most_general && !most_general_low) selecionada = true;
			else selecionada = false;
		}
		tabela_seleciona[i][0] = most_specific;
		tabela_seleciona[i][1] = most_general;
		tabela_seleciona[i][2] = selecionada;
	}

/*
	cout << endl << "Elementos ordenados:" << endl;
		for (unsigned int i = 0; i < frases.size(); i++){
			cout << frases.at(i) << ": "<< cobertura.at(i) << "% - ";
			if (tabela_seleciona[i][0]) cout << 1 << " - "; else cout << 0 << " - ";
			if (tabela_seleciona[i][1]) cout << 1 << " - "; else cout << 0 << " - ";
			if (tabela_seleciona[i][2]) cout << 1; else cout << 0;
			cout << endl;
		}*/

	int conta_frase = 0;
	label = "";
	for (unsigned int i = 0; i < max; i++){
		if (tabela_seleciona[i][2]){
			conta_frase++;
			if (label.compare("") == 0) label = frases.at(i);
			else {
				label.append(", ");
				label.append(frases.at(i));
			}
			if (conta_frase >= num_frases) break;
		}
	}

}



void clust::imprime_clusters(int n){

	if (n <= 0) return;

	ifstream stream_doc_origin (Reader::arquivo_original.c_str(), ios::in);

	for (vector<clust>::iterator it = clust::baseclusters.begin(); it < clust::baseclusters.end() && it < (clust::baseclusters.begin() + n); it++){
		if ((*it).tamanho_sufixo() >= 1){

			cout << (*it).label << endl;
			cout << (*it).documentos.size() << endl;

			/*
			cout << "Nodos: ";

			for (vector<int>::iterator in = (*it).nodo.begin(); in < (*it).nodo.end(); in++)
				cout << (*in) << " ";
			cout << " Score: " << (*it).score << endl;

			cout << "Numero de documentos onde aparece: " << (*it).numero_documentos() << endl;
			*/

			//imprimindo os documentos
			VERB
			{
			//cout << "==============================================================================" << endl;

			for (set<int>::iterator in = (*it).documentos.begin(); in != (*it).documentos.end(); in++){
				int contalinha = 1;
				//cout << *in << endl;
				//cout << documents.at(*in - 1) << endl;

				string documento;
				while (contalinha <= *in){
					getline(stream_doc_origin, documento);
					contalinha++;
				}
				cout << documento << endl;
				stream_doc_origin.seekg(0, ios::beg);

			};
			//cout << "==============================================================================" << endl;
			}

			//imprimindo sufixo
			/*
			for (unsigned int t = 0; t < (*it).first_char_index.size(); t++){
				//cout << "Sufixo de " << (*it).first_char_index.at(t) << " de tamanho " << (*it).tam_sufixo.at(t) << endl;
				for (int i = (*it).first_char_index.at(t); i <= ((*it).tam_sufixo.at(t) + (*it).first_char_index.at(t) - 1); i++)
					cout << Edge::termos.at(i) << " ";
				cout << "; " << endl;
			}
			cout << endl << "-------------------------------" << endl;
			*/
			//cout << "FIMDOC" << endl;
		}
	}
	stream_doc_origin.close();
}

void clust::processa_clusters(float Threshold){
	//definindo o threshold
	threshold = Threshold;

	VERB cout << endl;

	VERB cout << "* Atualizando o conjunto de documentos de cada cluster" << endl;
	for (vector<clust>::iterator it = baseclusters.begin(); it < baseclusters.end(); it++)
		(*it).atualiza_documentos();

	VERB cout << "* Calculando score para clusters basicos ... " << endl;
	Calculo_Score(baseclusters);

	VERB cout << "* Ordenando clusters por ordem decrescente de score ..." << endl;
	ordena_clusters(baseclusters);

	/*
		 * ********** Medida do tempo de execucao ************
		 * variaveis para medida do tempo de execucao
		 */
			double tusuario;
			double tsistema;
			double texecucao;
			struct timeval tinicio;
			struct timeval tfim;
			struct rusage recursos;
			struct timeval usuario;
			struct timeval sistema;

			gettimeofday(&tinicio, NULL);


		 //* ****************************************************


	VERB cout << "* Fazendo merge de clusters similares ..." << endl;
	merge_cluster();

	//calculo do tempo
			gettimeofday(&tfim, NULL);

			getrusage(RUSAGE_SELF, &recursos);
			usuario = recursos.ru_utime;
			sistema = recursos.ru_stime;

			tusuario = (double) recursos.ru_utime.tv_sec + 1.e-6 * (double) recursos.ru_utime.tv_usec;
			tsistema = (double) recursos.ru_stime.tv_sec + 1.e-6 * (double) recursos.ru_stime.tv_usec;

			texecucao = (tfim.tv_sec-tinicio.tv_sec)*1.e6;
			texecucao = (texecucao+(tfim.tv_usec-tinicio.tv_usec))*1.e-6;
			/*
			printf("tempo de execucao: %5f\n", texecucao);
			printf("tempo de usuario: %5f\n", tusuario);
			printf("tempo de sistema: %5f\n", tsistema);
			printf("--------------------------------------------------\n");
			*/
			cout << clust::documents.size() << " " << texecucao << endl;


		 //* ****************************************************

	VERB cout << "* Recalculando score ..." << endl;
 	Calculo_Score(baseclusters);

 	VERB cout << "* Reordenando ..." << endl;
	ordena_clusters(baseclusters);

	VERB cout << "* Processamento dos labels ..." << endl;
	for (int cluster = 0; cluster < N_RELEVANTES && cluster < baseclusters.size(); cluster++)
		baseclusters.at(cluster).processa_label();

	VERB cout << "* Processamento terminado!!!" << endl << endl;
}

//funcoes para estatisticas
	/*
	 * gerando graficos de sufixo x numero de ocorrencia por documento no cluster
	 * sera gerado um arquivo no formato do gnuplot para cada um dos n primeiros clusters
	 */

void clust::grafico_termo_por_doc(int n_primeiros){
	int contacluster = 1;
	vector<float> percentuais;

	for (vector<clust>::iterator it = clust::baseclusters.begin(); it < clust::baseclusters.end() && it < (clust::baseclusters.begin() + n_primeiros); it++){

		int numdoc = (*it).numero_documentos();
		int contatermo = 1;

		//stringstream nome_arquivo;
		//nome_arquivo << "graficos/termo_por_doc_" << contacluster << ".dat";
		//ofstream arquivo_saida ("graficos/termo_por_doc.dat");

		vector<int>::iterator ifirst =  (*it).first_char_index.begin();
		vector<int>::iterator itam = 	(*it).tam_sufixo.begin();

		for (;ifirst < (*it).first_char_index.end();){

			for (int i = (*ifirst); i < (*ifirst + *itam); i++){
				int ocorrencias = 0;
				//cout << Edge::termos.at(i) << endl;
				for (set<int>::iterator idoc = (*it).documentos.begin(); idoc != (*it).documentos.end(); idoc++){
					size_t found = documents.at(*idoc - 1).find(Edge::termos.at(i));
					if (found != string::npos) ocorrencias++;
				}
				float percentual = ((float)ocorrencias) / ((float)numdoc);
				percentuais.push_back(percentual);

				//cout << contatermo << " " << percentual << endl;
				//arquivo_saida << contatermo << " " << percentual << endl;
				contatermo++;
			}

			ifirst++;
			itam++;
		}
		contacluster++;
		//arquivo_saida.close();
	}
	cout << "=====================================================" << endl;

	cout << endl << "Estatísticas gerais:" << endl;

	ofstream arquivo_saida ("graficos/termo_por_doc.dat");

	sort(percentuais.begin(), percentuais.end());

	vector<float>::iterator ipercent = percentuais.begin();
	float f = *ipercent;
	int conta = 1;
	ipercent++;
	for (;ipercent < percentuais.end(); ipercent++){
		if ((*ipercent) == f) conta++;
		else{
			cout << f * 100 << " " << conta << endl;
			arquivo_saida << f * 100 << " " << conta << endl;
			f = *ipercent;
			conta = 1;
		}
	}
	cout << f * 100 << " " << conta << endl;
	arquivo_saida << f * 100 << " " << conta << endl;
	arquivo_saida.close();


	cout << "========================================" << endl;
}

/*
 * funcoes para ordenacao das frases segundo os criterios para determinacao do label
 */

int clust::compara_frase(string frase1, string frase2, int tam1, int tam2, float cobertura1, float cobertura2){

	//se as frase sao iguais retorna 0
	if (frase1.compare(frase2) == 0) return 0;

	if (cobertura1 == cobertura2) return tam1 > tam2;
	else return cobertura1 > cobertura2;
}

//funcao particao
void clust::Particao (int Esq, int Dir, int &i, int &j, vector<string> &frases, vector<int> &tmp_sufixo, vector<float> &cobertura){

	string pivo, aux;
	int int_pivo, tam_pivo, tam_aux;
	float cob_pivo, cob_aux;
	i = Esq;
	j = Dir;
	int_pivo = (i + j)/2;
	pivo = frases.at(int_pivo);
	tam_pivo = tmp_sufixo.at(int_pivo);
	cob_pivo = cobertura.at(int_pivo);

	do{
		while (compara_frase(frases.at(i), pivo, tmp_sufixo.at(i), tam_pivo, cobertura.at(i), cob_pivo)){
			i++;
		}
		while (compara_frase(pivo, frases.at(j), tam_pivo, tmp_sufixo.at(j), cob_pivo, cobertura.at(j))){
			j--;
		}
		if (i <= j){
			aux = frases.at(i);
			tam_aux = tmp_sufixo.at(i);
			cob_aux = cobertura.at(i);

			frases.at(i) = frases.at(j);
			frases.at(j) = aux;

			tmp_sufixo.at(i) = tmp_sufixo.at(j);
			tmp_sufixo.at(j) = tam_aux;

			cobertura.at(i) = cobertura.at(j);
			cobertura.at(j) = cob_aux;

			i++;
			j--;
		}
	} while (i <= j);

}
//funcao ordena
void clust::Ordena (int Esq, int Dir, vector<string> &frases, vector<int> &tmp_sufixo, vector<float> &cobertura){

	int i, j;
	Particao (Esq, Dir, i, j, frases, tmp_sufixo, cobertura);
	if (Esq < j) Ordena (Esq, j, frases, tmp_sufixo, cobertura);
	if (i < Dir) Ordena (i, Dir, frases, tmp_sufixo, cobertura);

}
//QuickSort Recursivo
void clust::QuickSort (vector<string> &frases, vector<float> &cobertura){

	vector<int>tmp_sufixo = tam_sufixo;
	Ordena (0, frases.size() - 1, frases, tmp_sufixo, cobertura);

}
