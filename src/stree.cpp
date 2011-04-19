/*
 * stree.cpp
 *
 *  Created on: 28/10/2010
 *      Author: luizhenrique
 */

#include "stree.h"

int Edge::numedges = 0;

int Node::Count = 1;

int Edge::primeirotermo;
int Edge::N;
vector<string> Edge::termos;
set<tipo_termo, comp_tipotermo> Edge::conjunto_termos;
set<string> Edge::termos_to_walk;
TipoPesos Edge::p;
Edge Edge::Edges[ HASH_TABLE_SIZE ];

Node Node::Nodes[ NUM_NODOS ];

//
// The default ctor for Edge just sets start_node
// to the invalid value.  This is done to guarantee
// that the hash table is initially filled with unused
// edges.
//

Edge::Edge()
{
    start_node = -1;
}

//
// I create new edges in the program while walking up
// the set of suffixes from the active point to the
// endpoint.  Each time I create a new edge, I also
// add a new node for its end point.  The node entry
// is already present in the Nodes[] array, and its
// suffix node is set to -1 by the default Node() ctor,
// so I don't have to do anything with it at this point.
//

Edge::Edge( int init_first, int init_last, int parent_node )
{
    first_char_index = init_first;
    last_char_index = init_last;
    start_node = parent_node;
    end_node = Node::Count++;
}

//
// Edges are inserted into the hash table using this hashing
// function.
//

void Edge::GeraPesos (TipoPesos p){
//funcao para gerar pesos
     int i;

     srand (time(NULL));//inciando gerador de numeros aleatorios utilizando o tempo
     for (i = 0; i < MAX_LENGTH; i++)
          p[i] = 1 + (int)(10000.0 * rand()/(RAND_MAX+1.0));
}

int Edge::Hash( int node, string c )
{
	int soma = 0;
	for (unsigned int i = 0; i < c.size(); i++){
		if (c.at(i) >= 0)
			soma += (unsigned int) c.at(i) * p[i];//soma os valores de cada letra multiplicados pelo peso
	}
    return ( ( node << 8 ) + soma ) % HASH_TABLE_SIZE;
}

//
// A given edge gets a copy of itself inserted into the table
// with this function.  It uses a linear probe technique, which
// means in the case of a collision, we just step forward through
// the table until we find the first unused slot.
//

void Edge::Insert()
{
	numedges++;
    //int i = Hash( start_node, T[ first_char_index ] );
    int i = Hash (start_node, termos.at(first_char_index));
    while ( Edges[ i ].start_node != -1 ){
    	i++;
        i = i % HASH_TABLE_SIZE;
    }

    Edges[ i ] = *this;
}

//
// Removing an edge from the hash table is a little more tricky.
// You have to worry about creating a gap in the table that will
// make it impossible to find other entries that have been inserted
// using a probe.  Working around this means that after setting
// an edge to be unused, we have to walk ahead in the table,
// filling in gaps until all the elements can be found.
//
// Knuth, Sorting and Searching, Algorithm R, p. 527
//

void Edge::Remove()
{
    //int i = Hash( start_node, T[ first_char_index ] );
    int i = Hash( start_node, termos.at(first_char_index) );
    while ( Edges[ i ].start_node != start_node ||
            Edges[ i ].first_char_index != first_char_index ){
    	i++;
    	i = i % HASH_TABLE_SIZE;
    }
    for ( ; ; ) {
        Edges[ i ].start_node = -1;
        int j = i;
        for ( ; ; ) {
        	i++;
            i = i % HASH_TABLE_SIZE;
            if ( Edges[ i ].start_node == -1 )
                return;
            //int r = Hash( Edges[ i ].start_node, T[ Edges[ i ].first_char_index ] );
            int r = Hash( Edges[ i ].start_node, termos.at( Edges[ i ].first_char_index ) );
            if ( i >= r && r > j )
                continue;
            if ( r > j && j > i )
                continue;
            if ( j > i && i >= r )
                continue;
            break;
        }
        Edges[ j ] = Edges[ i ];
    }
}

//
// The whole reason for storing edges in a hash table is that it
// makes this function fairly efficient.  When I want to find a
// particular edge leading out of a particular node, I call this
// function.  It locates the edge in the hash table, and returns
// a copy of it.  If the edge isn't found, the edge that is returned
// to the caller will have start_node set to -1, which is the value
// used in the hash table to flag an unused entry.
//

Edge Edge::Find( int node, string c )
{
    int i = Hash( node, c );
    for ( ; ; ) {
        if ( Edges[ i ].start_node == node )
            //if ( c == T[ Edges[ i ].first_char_index ] )
            if (! (c.compare(termos.at( Edges[ i ].first_char_index )) ))
                return Edges[ i ];
        if ( Edges[ i ].start_node == -1 )
            return Edges[ i ];
        i++;
        i = i % HASH_TABLE_SIZE;
    }
}

//
// When a suffix ends on an implicit node, adding a new character
// means I have to split an existing edge.  This function is called
// to split an edge at the point defined by the Suffix argument.
// The existing edge loses its parent, as well as some of its leading
// characters.  The newly created edge descends from the original
// parent, and now has the existing edge as a child.
//
// Since the existing edge is getting a new parent and starting
// character, its hash table entry will no longer be valid.  That's
// why it gets removed at the start of the function.  After the parent
// and start char have been recalculated, it is re-inserted.
//
// The number of characters stolen from the original node and given
// to the new node is equal to the number of characters in the suffix
// argument, which is last - first + 1;
//

int Edge::SplitEdge( Suffix &s, int start_suffix, int &tam_sufixo, int doc, int nodoanterior )
{
    Remove();
    Edge *new_edge =
      new Edge( first_char_index,
                first_char_index + s.last_char_index - s.first_char_index,
                s.origin_node );
    new_edge->Insert();
    Node::Nodes[ new_edge->end_node ].suffix_node = s.origin_node;
    first_char_index += s.last_char_index - s.first_char_index + 1;

    //copiando documentos do nodo antigo para o novo
    for (set<int>::iterator it = Node::Nodes[nodoanterior].docs.begin(); it != Node::Nodes[nodoanterior].docs.end(); it++ )
    	Node::Nodes[new_edge->end_node].docs.insert(*it);

    Node::Nodes[new_edge->end_node].docs.insert(doc);

    int tamanho = (new_edge->last_char_index - new_edge->first_char_index + 1) + tam_sufixo;
    tam_sufixo = tamanho;

    if (Node::Nodes[new_edge->end_node].docs.size() >= MIN_DOCS_CLUSTER){
		//Suffix::imprime_sufixo(start_suffix, start_suffix + tamanho - 1);
		clust::insere_basecluster(start_suffix, tamanho, new_edge->end_node);
	}


    start_node = new_edge->end_node;
    Insert();
    return new_edge->end_node;
}

//
// This routine prints out the contents of the suffix tree
// at the end of the program by walking through the
// hash table and printing out all used edges.  It
// would be really great if I had some code that will
// print out the tree in a graphical fashion, but I don't!
//

void dump_edges( int current_n )
{
    cout << " Start  End  Suf  First Last  Documents String\n";
    for ( int j = 0 ; j < HASH_TABLE_SIZE ; j++ ) {
        Edge *s = Edge::Edges + j;
        if ( s->start_node == -1 )
            continue;
        cout << setw( 5 ) << s->start_node << " "
             << setw( 5 ) << s->end_node << " "
             << setw( 3 ) << Node::Nodes[ s->end_node ].suffix_node << " "
             << setw( 5 ) << s->first_char_index << " "
             << setw( 6 ) << s->last_char_index << "  "
			 << setw( 5 );
			 //for (unsigned int d = 0; d < Node::Nodes[s->end_node].docs.size(); d++)
               //	cout << Node::Nodes[ s->end_node ].docs.at(d) << " ";
			//cout << setw(5);
        int top;
        if ( current_n > s->last_char_index )
            top = s->last_char_index;
        else
            top = current_n;
        Suffix::imprime_sufixo(s->first_char_index, top);
        /*for ( int l = s->first_char_index ;
                  l <= top;
                  l++ )
            //cout << T[ l ];
            cout << Edge::termos.at(l);*/
        cout << "\n";
    }
}

/*
 * funcao add_preffix, recebe cada sufixo, caracterizado pela primeira palavra
 * do sufixo e adiciona na arvore
*/
void add_preffix( int start_node, int start_suffix, int tam_sufixo, int start, int doc )
{
	if (start >= Edge::termos.size()) return;
	Edge edge = Edge::Find( start_node, Edge::termos.at(start) );
	if ( edge.start_node != -1 ) {
		//encontrou inicio do sufixo

		unsigned int aux = start + 1;
		//bool split = false;
		int j = edge.first_char_index + 1;
		//verificando ate qual parte o sufixo ja esta na arvore
		while((j <= edge.last_char_index) && (aux < Edge::termos.size()) && !(Edge::termos.at(aux).compare(Edge::termos.at(j)))) {
			j++;
			aux++;
		}

		unsigned int dif = j -(edge.first_char_index + 1);

		if ((dif + start) != (Edge::termos.size() - 1) || j < edge.last_char_index){//se o sufixo nao esta completo, faz split do nodo e insere o sufixo
			if ((j - 1) == edge.last_char_index){
				int tamanho = edge.last_char_index - edge.first_char_index + 1 + tam_sufixo;

				//contando o numero de documentos onde este sufixo aparece
				//e inserindo no nodo a informacao de em qual documento este sufixo esta contido
				int tam_setdocs_antes = Node::Nodes[edge.end_node].docs.size();

				Node::Nodes[edge.end_node].docs.insert(doc);

				int tam_setdocs_depois = Node::Nodes[edge.end_node].docs.size();

				if (tam_setdocs_depois > tam_setdocs_antes && Node::Nodes[edge.end_node].docs.size() == MIN_DOCS_CLUSTER){
					//Suffix::imprime_sufixo(start_suffix, start_suffix + tamanho - 1);
					clust::insere_basecluster(start_suffix, tamanho, edge.end_node);
				}

				//--------------------------


				add_preffix(edge.end_node, start_suffix, tamanho, aux, doc);
			}
			else{
				Suffix s (start_node, 0, j - edge.first_char_index - 1);
				int nodoanterior = edge.end_node;

				int endnode = edge.SplitEdge(s, start_suffix, tam_sufixo, doc, nodoanterior);

				add_preffix(endnode, start_suffix, tam_sufixo, aux, doc);
			}
		}
		else{
			int tamanho = (edge.last_char_index - edge.first_char_index) + 1 + tam_sufixo;

			//contando o numero de documentos onde este sufixo aparece
			//e inserindo no nodo a informacao de em qual documento este sufixo esta contido
			int tam_setdocs_antes = Node::Nodes[edge.end_node].docs.size();

			Node::Nodes[edge.end_node].docs.insert(doc);

			int tam_setdocs_depois = Node::Nodes[edge.end_node].docs.size();

			if (tam_setdocs_depois > tam_setdocs_antes && Node::Nodes[edge.end_node].docs.size() == MIN_DOCS_CLUSTER){
				//Suffix::imprime_sufixo(start_suffix, start_suffix + tamanho - 1);
				clust::insere_basecluster(start_suffix, tamanho, edge.end_node);
			}

			//--------------------------
		}
	}
	else{//insercao caso nao tenha encontrado
		Edge *new_edge = new Edge( start, Edge::termos.size() - 1, start_node );
		new_edge->Insert();

		int tamanho = (new_edge->last_char_index - new_edge->first_char_index) + 1 + tam_sufixo;

		int tam_setdocs_antes = Node::Nodes[new_edge->end_node].docs.size();

		Node::Nodes[new_edge->end_node].docs.insert(doc);

		int tam_setdocs_depois = Node::Nodes[new_edge->end_node].docs.size();

		if (tam_setdocs_depois > tam_setdocs_antes && Node::Nodes[new_edge->end_node].docs.size() == MIN_DOCS_CLUSTER){
			//Suffix::imprime_sufixo(start_suffix, start_suffix + tamanho - 1);
    		clust::insere_basecluster(start_suffix, tamanho, new_edge->end_node);
		}
	}
}

/*
 * Recebe uma frase, divide em palavras e insere na arvore
 * */
void inserir_frase (string frase, int doc, set <tipo_termo, comp_tipotermo> &set_termos_doc){

	if (frase.find_first_not_of(' ') == string::npos) return;

	//cout << "INSERINDO FRASE: " << frase << endl;

	//dividindo a frase em palavras
	size_t found = 0;
	size_t found_origin = 0;
	size_t pos = 0;
	size_t pos_origin = 0;

	Edge::primeirotermo = Edge::termos.size();

	while(found != string::npos){
		found = frase.find(" ", found + 1);
		string tmp = frase.substr(pos, found - pos);

		size_t n = tmp.find_first_of(' ');
		while(n != string::npos){
			tmp.erase(n, 1);
			n = tmp.find_first_of(' ');
		}

		if (tmp.size() > 0){
			Edge::termos.push_back(tmp);
			Edge::termos_to_walk.insert(tmp);

			tipo_termo termotmp(tmp);
			set_termos_doc.insert(termotmp);
			//cout << frase.substr(pos, found - pos) << endl;
		}
		pos = found + 1;
	}
	/*
	//dividindo frase original em palavras
	while(found != string::npos){
			found = frase.find(" ", found + 1);
			string tmp = frase.substr(pos, found - pos);

			size_t n = tmp.find_first_of(' ');
			while(n != string::npos){
				tmp.erase(n, 1);
				n = tmp.find_first_of(' ');
			}

			if (tmp.size() > 0){
				Edge::termos.push_back(tmp);
				Edge::termos_to_walk.insert(tmp);

				tipo_termo termotmp(tmp);
				set_termos_doc.insert(termotmp);
				//cout << frase.substr(pos, found - pos) << endl;
			}
			pos = found + 1;
		}
	*/
	//inserindo termo em um conjunto de termos deste documento

	Edge::N = Edge::termos.size() - 1;

	Suffix active( 0, 0, -1 );  //prefixo inicial
	//cout << "Numero de termos ate agora: " << Edge::termos.size() << endl;
	for ( int i = Edge::primeirotermo ; i <= Edge::N ; i++ )
		add_preffix(0, i, 0, i, doc);

}

void Suffix::imprime_sufixo (int first_word, int last_word){
	for (int i = first_word; i <= last_word; i++)
		cout << Edge::termos.at(i) << " ";
}

vector<string> CurrentString;

int walk_tree( int start_node, int last_char_so_far, int num_to_desempilha )
{

    int edges = 0;
    //cout << "EMPILHA" << endl;
    set<string>::iterator it;
    int i = -1;
    for ( it = Edge::termos_to_walk.begin(); it != Edge::termos_to_walk.end() ; it++ ) {
    	i++;
        Edge edge = Edge::Find( start_node, *it );
        if ( edge.start_node != -1 ) {

            edges++;
            unsigned int l = last_char_so_far;
            for ( int j = edge.first_char_index ; j <= edge.last_char_index ; j++ ){
				//cout << "incrementando termo corrente: " << Edge::termos.at(j) << endl;
				CurrentString.push_back(Edge::termos.at(j));
				l++;
			}

            walk_tree( edge.end_node, l, l - last_char_so_far );
        }
    }
//
// If this node didn't have any child edges, it means we
// are at a leaf node, and can check on this suffix.

    if ( edges == 0 ) {
    	/* imprime o sufixo*/
        cout << "Suffix : ";
        for ( int m = 0 ; m < last_char_so_far ; m++ )
            cout << CurrentString.at(m);
        cout << "\n";


		for(int d = num_to_desempilha; d > 0; d-- ){
			//cout << "decrementando corrente: " << CurrentString.at(CurrentString.size()-1) << endl;
			CurrentString.pop_back();
		}
		//cout << "DESEMPILHA" << endl;
        return 1;
    } else{
		for(int d = num_to_desempilha; d > 0; d-- ){
			//cout << "decrementando corrente: " << CurrentString.at(CurrentString.size()-1) << endl;
			CurrentString.pop_back();
		}
		//cout << "DESEMPILHA" << endl;
    	return 0;
    }

}

/*tipo termo*/

tipo_termo::tipo_termo(string Termo){
	numdocs = 1;
	termo = Termo;
}
void tipo_termo::incrementa(){
	numdocs++;
}
