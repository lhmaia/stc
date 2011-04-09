#ifndef RSLP_STRUCTS_H
#define RSLP_STRUCTS_H

#include "rslpTrie.h"

/**
 * Estruturas do stemmer
 */

/* -------------------------------------------------------------------------- */
/**
 *	\brief	General Counters	
 */
typedef struct {
	unsigned int numExceptions;	//!< number of found stemmer_exceptions
	unsigned long ruleCount;	//!< Used to count the number of times a rule was applied 
}auxiliar_struct;

/**
 *	\brief	Struct to hold all info from a rule
 */
typedef struct {
	char *suffix; 				//!< Suffix
	unsigned int minStemSize; 	//!< Minimum size to replace a stem. 
	char *replacement;			//!< Replacement string
	char stemmer_exceptions[50][15];	//!< FIXME: use dynamic memory allocation
	auxiliar_struct auxiliar;	//!< Miscelaneous objects: counters
}stem_rule_struct;

/**
 * \brief	Defines the basic 'step' structure
*/
typedef struct {
	char *stepName;	//!< Name of the rule
	unsigned int minWordLen;		//!< Minimun word length
	unsigned int compEntireWord; 	//!< Indicates if is needed to compare only the suffix (0) or the entire word (1)
	char endWords[5][6];			//!< End of word conditions array: check words that end with these strings
	unsigned int numEndWords;		//!< Number of end words for this step
	stem_rule_struct **rules;		//!< Pointer to all rules from this step
	unsigned int rulesNumber;		//!< Number of suffixes rules
	unsigned long stepCount;		//!< Count how many times a step was applied
}stem_step_struct;

/**
 * \brief	Define the sequence of application of steps.
 */
typedef struct {
	char *stepName;		//!< Name of the step to be applied. MUST be equal to 'stepName' in stem_step_struct
	char *stepTrue;		//!< Next step to be applied in case 'stepName' results TRUE (applied)
	char *stepFalse;	//!< Next step to be applied in case 'stepName' results FALSE (not applied)
}stem_step_seq_struct;

/**
 * \brief	Structure with general processing flags
 */
typedef struct {
	int process_stemming; 					//!< Should process stemming rules on words - default yes
	int replace_iso_chars;					//!< Should replace ISO characters through stemmer_stemRemoveAccents?
	int use_stem_dictionary; 				//!< Should use dictionary for words and their stems
	int use_named_entities_dictionary; 		//!< Should use dictionary for named entities
	int stem_dictionary_max_size;			//!< Maximum memory allowed for the stem dictionary
	int named_entities_dictionary_max_size; //!< Maximum memory allowed for the named entities dictionary
}stem_processing_flags;

/**
 * \brief 	Structure to hold a dictionary in a TRIE tree
 */
typedef struct {
	TRIE_NODE *dictRoot;			//!< Root node for the dictionary
	unsigned long dictTrieNumNodes;	//!< Number of nodes in this tree
	unsigned long dictTrieNumWords; //!< Number of complete word in this dictionary
	unsigned long dictTrieMemUsed;  //!< Total memory used by the dictionary
	int dictMaxSize; 				//!< Maximum memory allowed to be used by the dictionary
}stem_dict_struct;

/**
 * \brief 	Structure to hold all stemmer filenames and paths
 */
typedef struct {
	char *stepsFileName;			//!< Steps and rules file
	char *namedEntitiesFileName;	//!< File with named entities
	char *flowFileName;				//!< File with main steps flow
}stem_filenames_struct;

/**
 * \brief	Main RSLP structure. Contains all needed data to stem a word
 */
typedef struct {
	stem_step_struct *rslpStemmerSteps[8];	//!< Array of all steps and their rules
	int	rslpStemmerTotalSteps;				//!< Total steps in rslpStemmerSteps
	stem_step_seq_struct *rslpStepsFlow[8];	//!< Pointer to the steps flow sequence
	int rslpNumStepsFlow;					//!< Number of 'lines' in rslpStepsFlow
	stem_processing_flags rslpProcFlags; 	//!< Flags to determine stemmer behavior
	stem_filenames_struct rslpFileNames; 	//!< All stemmer filenames and paths
	stem_dict_struct rslpStemDict; 			//!< Dictionary of words and stems	
	stem_dict_struct rslpNamedEntitiesDict; //!< Dictionary for named entities
}rslp_stemmer_main_struct;

#endif
