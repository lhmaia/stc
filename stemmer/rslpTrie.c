#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "rslpTrie.h"

/******************************************************************************/
void trieInitNode(TRIE_NODE *node)
{
	unsigned short i;

	node->endWord = 0; // Not a complete word
	node->stem = NULL;
	for(i=0; i<RSLP_TRIE_NUM_LETTERS; i++)
		node->nodes[i] = NULL; // Initializes all nodes as NULL

	return;
}

/******************************************************************************/
void trieUnload(TRIE_NODE *rootNode)
{
	int charIdx;
	TRIE_NODE *currentNode = rootNode;

	for(charIdx = 0; charIdx < RSLP_TRIE_NUM_LETTERS; charIdx++) {
		if(currentNode->nodes[charIdx]) { // Node allocated!
			trieUnload(currentNode->nodes[charIdx]);
			free(currentNode->nodes[charIdx]);
		}
	}
	
	if(currentNode->stem)
		free(currentNode->stem);
	
	return;	
}

/******************************************************************************/
TRIE_NODE *trieNewNode(unsigned long *trieNumNodes, unsigned long *trieMemUsed)
{
	TRIE_NODE *newNode = NULL;

	newNode = malloc(sizeof(TRIE_NODE));
	if(newNode) {
		(*trieNumNodes)++;
		trieInitNode(newNode);
		(*trieMemUsed)+=sizeof(TRIE_NODE);
	}

	return newNode;
}

/******************************************************************************/
/**
 * \brief	Initializes a TRIE tree, creating the root node and initializing the
 * 			counters
 * \param	trieRoot		Pointer to the root node
 * \param	trieNumNodes	Pointer to the tree's nodes counter
 * \param	trieNumWords	Pointer to the complete words counter
 * \param	trieMemUsed		Pointer to the used memory counter
 * \return	void
 */
void trieInit(TRIE_NODE *trieRoot, unsigned long *trieNumNodes, unsigned long *trieNumWords,
		unsigned long *trieMemUsed)
{
	trieInitNode(trieRoot);
	*trieNumNodes = 0;
	*trieNumWords = 0;
	*trieMemUsed = 0;

	return;
}

/******************************************************************************/
/**
 * \brief	Receive an character e returns it's index in the letters array
 * \param	c	Char to have it's index calculated
 * \return	The character index, or -1 if not recognized
 */
int getIndexOfChar(char c)
{
	unsigned char currentChar = (unsigned char)c;
	int indice = -1;
	if((currentChar >= 'A') && (currentChar <= 'Z'))
		indice = ((currentChar | 0x20) - 'a');
	else
	if((currentChar >='a') && (currentChar <='z'))
		indice =  (currentChar - 'a');
	else 
	if((currentChar >=192) && (currentChar <=221)) // maiúscula acentuado
		indice = ((currentChar |0x20)-224 + RSLP_TRIE_NUM_ASCII_LETTERS);
	else 
	if(currentChar >=224) // minúscula acentuado
		indice = (currentChar - 224 + RSLP_TRIE_NUM_ASCII_LETTERS);

	if(indice >= RSLP_TRIE_NUM_LETTERS) // Índice fora do array
		return -1;

	return indice;
}

/******************************************************************************/
void trieAddWord(TRIE_NODE *root, unsigned long *trieNumNodes, unsigned long *trieNumWords,
	   	unsigned long *trieMemUsed, char *word, char *stem, unsigned short endWordFlag)
{
	TRIE_NODE *currentNode = root;
	char *currentChar;
	int charIdx;

	// Check if 'word' is not null
	if(word == NULL || *word == '\0') {
		return;
	}

	currentChar = word; // Points to the first char of word
	
	// Check all chars of word
	while(*currentChar != '\0') {
		charIdx = getIndexOfChar(*currentChar);
		if((charIdx < 0) || (charIdx >= RSLP_TRIE_NUM_LETTERS)) { // Index out of boundaries of array
			return;
		}

		// ok, we have the index inside the letters array
		if(currentNode->nodes[charIdx] == NULL) { // Node not allocated yet
			currentNode->nodes[charIdx] = trieNewNode(trieNumNodes, trieMemUsed);
			if(!currentNode->nodes[charIdx])
				return;
		}

		currentNode = currentNode->nodes[charIdx]; // Points to the node of selected char
		currentChar++; // Next char in 'word'
	}

	if(!currentNode->endWord) { // If 'word' not in dictionary yet
		currentNode->endWord = endWordFlag; // Indicates this is a complete word
		(*trieNumWords)++;

		if(stem) { // 'word' have a stemmed form
			int size = sizeof(char)*(strlen(stem)+1);
			currentNode->stem = malloc(size);
			
			if(currentNode->stem) {
				// Put the stem in the dictionary, associated with 'word'
				strcpy(currentNode->stem, stem);
				(*trieMemUsed) += size;
			}
		}
	}
	return;
}

/******************************************************************************/
TRIE_NODE* trieSearchWord(TRIE_NODE *rootNode, char *word)
{
	TRIE_NODE *currentNode = rootNode;	
	char *currentChar = word;
	int charIdx;

	if( (word == NULL) || (*currentChar == '\0'))
		return NULL; // Invalid word

	while(*currentChar != '\0') {
		charIdx = getIndexOfChar(*currentChar);
		if(charIdx >= 0) {
			if(currentNode->nodes[charIdx]) {
				currentNode = currentNode->nodes[charIdx]; // Points to the next node to be checked
				currentChar++; // Next char in 'word'
			}
			else
				return NULL; // Word not found in the tree
		}
		else
			return NULL; // Invalid index: return as word not found in the tree
	}

	if(currentNode->endWord) // Check if is a complete word, not a substring of a bigger word
		return currentNode;
	else 
		return NULL;
}
