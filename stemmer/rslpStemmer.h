#ifndef RSLP_STEMMER_H
#define RSLP_STEMMER_H

#include <stdio.h>
#include <stdlib.h>
#include "rslpStructs.h"

/**
 * \brief	Defines for the config file reader parser
 */
#define RSLP_STATE_READING_VAR	0	//!< Reading a variable name
#define RSLP_STATE_READING_VAL	1	//!< Reading a variable value
#define RSLP_STATE_PROCESS		2	//!< Processing name and value for variable
#define RSLP_STATE_END_READING	3	//!< End reading

/**
 * \brief	Options for the configuration file
 */
#define RSLP_OPTION_STEPS_FILE							0	//!< Name of the file with steps and rules
#define RSLP_OPTION_FLOW_FILE							1	//!< Name of the file with program flow definition
#define RSLP_OPTION_NAMED_ENTITIES_FILE					2	//!< File with all named entities to be loaded
#define RSLP_OPTION_DO_STEMMING							3	//!< Use the stemming function?
#define RSLP_OPTION_REPLACE_ISO							4	//!< Replace all the accentuated?
#define RSLP_OPTION_USE_STEM_DICTIONARY					5	//!< Use a dictionary for stems?
#define RSLP_OPTION_USE_NAMED_ENTITIES_DICTIONARY		6	//!< Use a dictionary for named entities?
#define RSLP_OPTION_STEM_DICTIONARY_MAX_SIZE			7	//!< Maximum memory to use for the dictionary (in megabytes)
#define RSLP_OPTION_NAMED_ENTITIES_DICTIONARY_MAX_SIZE	8	//!< Maximum memory to use for the dictionary (in megabytes)

/**
 * \brief	Strings for the configuration fields in the configuration file
 */
#define RSLP_STRING_OPTION_STEPS_FILE							"STEPS_FILE"			//!< String
#define RSLP_STRING_OPTION_FLOW_FILE							"FLOW_FILE"				//!< String
#define RSLP_STRING_OPTION_NAMED_ENTITIES_FILE					"NAMED_ENTITIES_FILE"	//!< String
#define RSLP_STRING_OPTION_DO_STEMMING							"DO_STEMMING"			//!< YES/NO
#define RSLP_STRING_OPTION_REPLACE_ISO							"REPLACE_ISO_CHARS"		//!< YES/NO
#define RSLP_STRING_OPTION_USE_STEM_DICTIONARY					"USE_STEM_DICTIONARY"	//!< YES/NO
#define RSLP_STRING_OPTION_USE_NAMED_ENTITIES_DICTIONARY		"USE_NAMED_ENTITIES"	//!< YES/NO
#define RSLP_STRING_OPTION_STEM_DICTIONARY_MAX_SIZE				"STEM_DICT_MAX_SIZE"	//!< Integer
#define RSLP_STRING_OPTION_NAMED_ENTITIES_DICTIONARY_MAX_SIZE	"NAMED_ENTITIES_DICT_MAX_SIZE"	//!< Integer
#define RSLP_STRING_OPTION_NO									"NO"
#define RSLP_STRING_OPTION_YES									"YES"

#define RSLP_STEMMER_MIN_WORD_LENGTH		3	//!< Minimun word length to be stemmed

#define RSLP_END_WORD_STEM					1	//!< Indicates this is a stemmed word
#define RSLP_END_WORD_NAMED_ENTITY			2	//!< Indicates this word is a named entity

#define RSLP_MASK_WORD_START_WITH_CAPITAL_LETTER	1	//!< Bit 0 indicates if word start with a capital letter
#define RSLP_MASK_WORD_HAS_DIGITS					2	//!< Bit 1 indicates if word has digits (numbers)
#define RSLP_MASK_WORD_HAS_ISO_CHARS				4	//!< Bit 2 indicates if word has accentuated characters
#define RSLP_MASK_WORD_TOO_SMALL					8	//!< Bit 3 indicates if word is smaller than RSLP_STEMMER_MIN_WORD_LENGTH 

#define RSLP_WORD_START_WITH_CAPITAL_LETTER(_wordFlags)	(((_wordFlags) & RSLP_MASK_WORD_START_WITH_CAPITAL_LETTER)==RSLP_MASK_WORD_START_WITH_CAPITAL_LETTER)
#define RSLP_WORD_HAS_DIGITS(_wordFlags)				(((_wordFlags) & RSLP_MASK_WORD_HAS_DIGITS)==RSLP_MASK_WORD_HAS_DIGITS)
#define RSLP_WORD_HAS_ISO_CHARS(_wordFlags)				(((_wordFlags) & RSLP_MASK_WORD_HAS_ISO_CHARS)==RSLP_MASK_WORD_HAS_ISO_CHARS)
#define RSLP_WORD_TOO_SMALL(_wordFlags)					(((_wordFlags) & RSLP_MASK_WORD_TOO_SMALL)==RSLP_MASK_WORD_TOO_SMALL)

#define RSLP_MAX_DICTIONARY_MEMORY_USED		256		//!< Maximum memory allocated for a dictionary (in Mb)

#define ELEMENTS_OF(_v)	( (sizeof(_v))/(sizeof(*_v))) //!< Count the number of elements from an array

/**
 * \brief	Shows debugs messages across the code. Must have DEBUG defined
 */
//#define DEBUG
#ifdef DEBUG
	#define debug(fmt, args...)	(void)( printf(fmt, ##args))
#else
	#define debug(fmt, args...) 	{ }		
#endif

/**
 * \brief	Shows informational messages across the code. Must have INFO_MESSAGES
 * 			defined
 */
#define INFO_MESSAGES
#ifdef INFO_MESSAGES
	#define infoMsg(fmt, args...)	(void)(fprintf(rslpInfoFile, fmt, ##args))
#else
	#define infoMsg(fmt, args...) 	{ }		
#endif


/* -------------------------------------------------------------------------- */
/**
 * \brief	Prints overall info in the stats file
 * \param	statsDumpFile	File where all statistics will be printed out
 * \param	elapsedTime	The stemming processing elapsed time
 * \param	inputFileCount	Number of input files processed
 * \param	wordCount	Total words read from input files
 * \return	void
 */
void info_printStatsInfo(FILE *statsDumpFile, double elapsedTime, unsigned long inputFileCount,
		unsigned long wordCount);

/* -------------------------------------------------------------------------- */
/**
 * \brief	Prints step's rules statistics 
 * \param	statsDumpFile	File where all statistics will be printed out
 * \param	stepStruct	Pointer to a step struct
 * \return	void
 */
void info_printStepsStats(FILE *statsDumpFile, stem_step_struct *stepStruct);

/* -------------------------------------------------------------------------- */
/**
 * \brief	Prints all objects from a step structure
 * \param	stepInfoDumpFile File where all step info will be printed
 * \param	stepStruct	Pointer to a step struct
 * \return	void
 */
void info_printStepInfo(FILE *stepInfoDumpFile, stem_step_struct *stepStruct);

/* -------------------------------------------------------------------------- */
/**
 * \brief	Shows a brief usage help
 * \return	void
 */
void info_showUsageHelp(void);

/* -------------------------------------------------------------------------- */
/**
 * \brief	Process a word, according to process flags. If defined, try to stem
 * 			a word, and replace all ISO chars
 *
 * This is the function that will actually try to stem a word. As word is a 
 * pointer, it will be modified during the stemming process, ending in the
 * stem. If none of the steps can be applied (or the stemmer is configured
 * to not stem the word), the end word will be the original. Also, if it's
 * configured, will replace all accentuaded characters for equivalent
 * non-accentuated characters.
 *
 * \param	word			Word to be stemmed
 * \param	rslpMainStruct	Global structure containing all information needed by 
 * 							the stemmer
 * \return	none
 */	
void rslpProcessWord(char *word, rslp_stemmer_main_struct *rslpMainStruct);

/* -------------------------------------------------------------------------- */
/**
 * \brief	Loads the RSLP stemmer
 *
 * This is the main RSLP Stemmer function. MUST be called before any other
 * stemmer functions. It's will load all structures, read all steps and
 * rules from file and such.
 *
 * \param	stemmerMainStruct	Pointer to the main stemmer structure
 * \param	configFileName		Name of the configuration file to be opened
 * \return	void
 */
void rslpLoadStemmer(rslp_stemmer_main_struct *stemmerMainStruct, char *configFileName);

/* -------------------------------------------------------------------------- */
/**
 * \brief	Unload all the structures used by the RSLP stemmer
 *
 * This function will unload all structures loaded by rslpLoadStemmer, freeing
 * the memory used. Must be called on the end of the program
 *
 * \param	stemmerMainStruct	Pointer to the main stemmer structure
 * \return	void
 */
void rslpUnloadStemmer(rslp_stemmer_main_struct *stemmerMainStruct);

rslp_stemmer_main_struct rslpMainStruct;	//!< Stemmer main structure
FILE *rslpInfoFile;	//!< Stemmer file for write program info

#endif
