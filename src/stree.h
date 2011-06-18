
#ifndef STREE_H_
#define STREE_H_

//
// STREE2006.CPP - Suffix tree creation
//
// Mark Nelson, updated December, 2006
//
// This program asks you for a line of input, then
// creates the suffix tree corresponding to the given
// text. Additional code is provided to validate the
// resulting tree after creation.
//
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string.h>
#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <limits.h>

#include "STC.h"


using namespace std;

//using std::cout;
using std::cin;
using std::cerr;
using std::setw;
using std::flush;
using std::endl;


/*class tipo termo*/

class tipo_termo {
public:
	tipo_termo(string Termo);
	void incrementa();

	string termo;
	int numdocs;
};

struct comp_tipotermo{
	bool operator() (const tipo_termo& t1, const tipo_termo& t2) const{
		if (t1.termo.compare(t2.termo) < 0) return true;
		else return false;
	}
};

//
// The maximum input string length this program
// will handle is defined here.  A suffix tree
// can have as many as 2N edges/nodes.  The edges
// are stored in a hash table, whose size is also
// defined here.
//
const int MAX_LENGTH = 1000;
const int NUM_NODOS = 2000000;
const int HASH_TABLE_SIZE = 3503777;  //A prime roughly 10% larger

//functions to hash function

typedef int TipoPesos[MAX_LENGTH];

//
// When a new tree is added to the table, we step
// through all the currently defined suffixes from
// the active point to the end point.  This structure
// defines a Suffix by its final character.
// In the canonical representation, we define that last
// character by starting at a node in the tree, and
// following a string of characters, represented by
// first_char_index and last_char_index.  The two indices
// point into the input string.  Note that if a suffix
// ends at a node, there are no additional characters
// needed to characterize its last character position.
// When this is the case, we say the node is Explicit,
// and set first_char_index > last_char_index to flag
// that.
//

class Suffix {
    public :
        int origin_node;
        int first_char_index;
        int last_char_index;
        Suffix( int node, int start, int stop )
            : origin_node( node ),
              first_char_index( start ),
              last_char_index( stop ){};

        static void imprime_sufixo (int first_word, int last_word);
};

//
// The suffix tree is made up of edges connecting nodes.
// Each edge represents a string of characters starting
// at first_char_index and ending at last_char_index.
// Edges can be inserted and removed from a hash table,
// based on the Hash() function defined here.  The hash
// table indicates an unused slot by setting the
// start_node value to -1.
//

class Edge {
    public :
        int first_char_index;
        int last_char_index;
        int end_node;
        int start_node;
        void Insert();
        void Remove();
        Edge();
        Edge( int init_first_char_index,
              int init_last_char_index,
              int parent_node );
        int SplitEdge( Suffix &s, int start_suffix, int &tam_sufixo, int doc, int nodoanterior );
        static Edge Find( int node, string c );
        static int Hash( int node, string c );
        static void GeraPesos (TipoPesos p);

        static int primeirotermo;
        static int N;

        static vector<string> termos;
        static vector<int> doc_por_termo;

    	static set<tipo_termo, comp_tipotermo> conjunto_termos;
        static set<string> termos_to_walk;


        static TipoPesos p;
        static Edge Edges[ HASH_TABLE_SIZE ];

        static int numedges;
};

//
//  The only information contained in a node is the
//  suffix link. Each suffix in the tree that ends
//  at a particular node can find the next smaller suffix
//  by following the suffix_node link to a new node.  Nodes
//  are stored in a simple array.
//
class Node {
    public :
        int suffix_node;
        Node() { suffix_node = -1; }
        static int Count;
        set<int> docs;

        static Node Nodes[ NUM_NODOS ];
};


int walk_tree( int start_node, int last_char_so_far, int num_to_desempilha );

void dump_edges( int current_n );

void add_preffix( int start_node, int start_suffix, int start, int doc );

void inserir_frase (string frase, int doc, set <tipo_termo, comp_tipotermo> &set_termos_doc);

#endif /* STREE_H_ */
