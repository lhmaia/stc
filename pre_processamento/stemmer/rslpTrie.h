#ifndef RSLP_TRIE_H 
#define RSLP_TRIE_H

/**
 * Implementation of a TRIE tree to be used as dictionary in the RSLP stemmer.
 * This tree accepts all 26 regular ASCII letters, and 31 accentuated letters
 * from ISO-8859-1 (57 letters overall).
 *
 * 								ASCII characters
 *
 * Index	|  0|  1|  2|  3|  4|  5|  6|  7|  8|  9| 10| 11| 12|
 * Char		|  a|  b|  c|  d|  e|  f|  g|  h|  i|  j|  k|  l|  m|
 * ASCII	| 97| 98| 99|100|101|102|103|104|105|106|107|108|109|
 *
 * Index	| 13| 14| 15| 16| 17| 18| 19| 20| 21| 22| 23| 24| 25|
 * Char		|  n|  o|  p|  q|  r|  s|  t|  u|  v|  w|  x|  y|  z|  
 * ASCII	|110|111|112|113|114|115|116|117|118|119|120|121|122|  
 *
 *
 *							  ISO-8859-1 characters
 *
 * Index	| 26| 27| 28| 29| 30| 31| 32| 33| 34| 35| 36| 37| 38| 39| 40| 41|
 * Char		|  à|  á|  â|  ã|  ä|  å|   |  ç|  è|  é|  ê|  ë|  ì|  í|  î|  ï|  
 * ISO-8859 |224|225|226|227|228|229|230|231|232|233|234|235|236|237|238|239|
 *
 * Index	| 42| 43| 44| 45| 46| 47| 48| 49| 50| 51| 52| 53| 54| 55| 56|
 * Char		|   |  ñ|  ò|  ó|  ô|  õ|  ö|   |  ù|  ú|  û|  ü|  ý|   |  ÿ|  
 * ISO-8859 |240|241|242|243|244|245|246|247|249|250|251|252|253|254|255|
 */

#define RSLP_TRIE_NUM_ASCII_LETTERS	26	//!< Number of ASCII letters: a-z
#define RSLP_TRIE_NUM_ISO_LETTERS	31	//!< Number of accentuated chars, check 'man ISO-8859-1'
#define RSLP_TRIE_NUM_LETTERS		RSLP_TRIE_NUM_ASCII_LETTERS + RSLP_TRIE_NUM_ISO_LETTERS //!< Número total de letras

/**
 * \brief	Typedef for a structure with a node for a TRIE tree
 */
typedef struct trieNode TRIE_NODE;

/**
 * \brief	Structure with a node for a TRIE tree
 */
struct trieNode {
	TRIE_NODE* nodes[RSLP_TRIE_NUM_LETTERS];	//!< Array for possible chars in a word
	unsigned short endWord;			//!< Indicates if this node is the last char of a complete wowrd
	char *stem;						//!< The stem of a word, if exists
};

/******************************************************************************/
/**
 * \brief	Initializes a node of the TRIE tree. The node must be previously
 * 			aloccated
 * \param	node	Pointer to the node to be initialized
 * \return	void
 */
void trieInit(TRIE_NODE *trieRoot, unsigned long *trieNumNodes, unsigned long *trieNumWords,
		unsigned long *trieMemUsed);

/******************************************************************************/
/**
 * \brief	Unload a TRIE tree, freeing it's memory
 * \param	rootNode	Root node of the tree
 * \return	void
 */
void trieUnload(TRIE_NODE *rootNode);

/******************************************************************************/
/**
 * \brief	Adds a new word to the TRIE tree
 * \param	root			Pointer to the root node of the tree
 * \param	trieNumNodes	Pointer to the node counter
 * \param	trieNumWords	Counts complete word in the dictionary
 * \param	trieMemUsed		Counts the mem allocated and used by the tree
 * \param	word			Word to be added to the tree
 * \param	stem			stemmed word
 * \param	endWordFlag		Indicates if word is complete or a substring. If
 * 							complete, indicates it's type
 * \return	void
 */
void trieAddWord(TRIE_NODE *root, unsigned long *trieNumNodes, unsigned long *trieNumWords,
	   	unsigned long *trieMemUsed, char *word, char *stem, unsigned short endWordFlag);

/******************************************************************************/
/**
 * \brief	Searchs for a word in the TRIE tree
 * \param	rootNode	Root node of the TRIE tree
 * \param	word		Word to searh for
 * \return	1 if 'word' is found in the tree, 0 otherwise
 */
TRIE_NODE* trieSearchWord(TRIE_NODE *rootNode, char *word);

/******************************************************************************/
/**
 * \brief	Creates a TRIE node, allocating the need memory. If sucessful,
 * 			the counter trieNumNodes is incremented
 * \param	trieNumNodes	Pointer to the tree's already allocated nodes
 * \param	trieMemUsed		Counts the memory used by the tree
 * \return	Pointer to the node, or NULL in case of error
 */
TRIE_NODE *trieNewNode(unsigned long *trieNumNodes, unsigned long *trieMemUsed);

#endif


