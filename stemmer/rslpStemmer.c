/*! \mainpage Removedor de Sufixos da Língua Portuguesa - RSLP
*
* \section intro_sec Introdução
*
* Modificações feitas sobre a implementação original do RSLP de autoria
* de Viviane Moreira Orengo e Christian Huyck
* \n http://www.inf.ufrgs.br/~vmorengo/
*
* \section down Downloads
*
* O pacote com os arquivos pode ser obtido em 
* http://www.inf.ufrgs.br/~arcoelho/
*
* \author Alexandre Ramos Coelho
* \author Viviane Moreira Orengo 
* \author Luciana Salete Buriol
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "rslpStemmer.h"
#include "rslpTrie.h"

extern rslp_stemmer_main_struct rslpMainStruct;
extern FILE *rslpInfoFile;

/** 	
 	Default step´s application flow, as written in the original RSPL code.
	It differs from the Orengo's paper, shown below:

	{ "Plural"       , "Feminine"     , "Feminine"     }  , 
	{ "Feminine"     , "Augmentative" , "Augmentative" }  , 
	{ "Augmentative" , "Adverb"       , "Adverb"       }  , 
	{ "Adverb"       , "Noun"         , "Noun"         }  , 
	{ "Noun"         , NULL           , "Verb"         }  , 
	{ "Verb"         , NULL           , "Vowel"        }  , 
	{ "Vowel"        , NULL           , NULL           }  , 
  	
	Can be overwritten by an external file - see rslpconfig.txt
*/

stem_step_seq_struct defaultFlow[] = {
	{ "Plural"       , "Adverb"       , "Adverb"       }  , 
	{ "Adverb"       , "Feminine"     , "Feminine"     }  , 
	{ "Feminine"     , "Augmentative" , "Augmentative" }  , 
	{ "Augmentative" , "Noun"         , "Noun"         }  , 
	{ "Noun"         , NULL           , "Verb"         }  , 
	{ "Verb"         , NULL           , "Vowel"        }  , 
	{ "Vowel"        , NULL           , NULL           }  , 
};

/* -------------------------------------------------------------------------- */
/**
 * \brief	Allocate memory for a string in source
 * \param	dest	Pointer to destination memory. Will be internally allocated
 * \param	source	Source string, to be copied to dest. The end string char is
 * 					manually added at the end.
 * \return	void
 */
static void parser_allocateString(char **dest, char *source)
{
	char *temp;
	int stringLen = strlen(source);

	temp = malloc(sizeof(char)*(stringLen+1));

	memcpy(temp, source, stringLen);
	temp[stringLen]='\0';

	*dest = temp;

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Reads a set of rules from a file. Removes all unwanted chars, keeping 
 * 			the	basic rule structure in one single line to be passed to the parser.
 * 			Search for a open bracket. Them, reads string, separated by comma, until
 * 			found a closing bracket. Closing brackets are represented as pipes ("|")
 * 			in the stepString, indicating the end of a group
 * \param	f	Pointer to the rules file
 * \param	stepString	Pointer to the memory to hold the entire rule
 * \return	1 on sucess, 0 otherwise
 */
static int parser_readStemStepFromFile(FILE *f, char **stepString)
{
	char *line = malloc(sizeof(char)*3000); // TODO: melhorar aqui.
	char ch = '\0';
	char previousChar = ch;
	int idx = 0;
	int bracketCount = 0;

	// 1 - Search for a open bracket '{'
	while(ch!='{') {
		if(feof(f)) {
			*stepString = NULL;
			return 0;
		}
		ch = fgetc(f);
	}
	bracketCount++;

	while(bracketCount!=0) {
		// 2 - Open bracket found. Start copying the line
		ch = fgetc(f); // Read next char from file...
		if(ch == '{') // If it is another open bracket, update the bracket counter
			bracketCount++;
		if(ch == '}') // idem for the closing bracket
			bracketCount--;

		// Copy the char read to the string, except puntuaction and control characters
		if(!isspace(ch) && (ch!='{')) {
			if(ch == '}') { 	// Closing bracket: indicates end of a group
				if(line[idx-1]!='|') {
					line[idx++] = ','; // mark the end with a pipe '|'
					line[idx++] = '|';
				}
			}
			else if(ch == '"') {
				if(previousChar == '"') // indicates a empty string: signal it with an underscore
					line[idx++] = '_';
			}
			else
				line[idx++] = ch;
		}
		previousChar = ch;
	}
	line[idx] = '\0';

	parser_allocateString(stepString, line);

	debug("Rule read: '%s'\n\n", *stepString);

	// Frees memory
	free(line);

	return 1;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Parse the next suffix rule from a rule string, and allocates the
 * 			proper suffix structure 
 * \param	ruleString	String with rules
 * \return	a pointer to the a suffix rule structure, filled with the parsed
 * 			data
 */
static stem_rule_struct* parser_stemParseSuffixRule(char *ruleString)
{
	char *temp;
	int tempSize;
	int count = 0;
	stem_rule_struct *tempSuffix = malloc(sizeof(stem_rule_struct));

	tempSuffix->auxiliar.ruleCount = 0;
	tempSuffix->auxiliar.numExceptions = 0;

	// STEP 1 - Get the suffix string
	temp = strtok(NULL, ",");
	if(temp == NULL) {
		free(tempSuffix);
		return NULL;
	}
		
	debug("Suffix: %s.\n", temp);
	
	parser_allocateString(&tempSuffix->suffix, temp);

	// STEP 2 - Check for minimum stem size
	temp = strtok(NULL, ",");
	if(temp && (strcmp(temp, "|")!=0)) {
		tempSize = atoi(temp);
		
		debug("\tMin. Stem Size: %d.\n", tempSize);
		
		tempSuffix->minStemSize = tempSize;

		// STEP 3 - Check if there is a replacement. Replacements are optional
		temp = strtok(NULL, ",");
		if(temp!=NULL) {
			if(temp[0]=='|') {	// Indicates end of group
				tempSuffix->replacement = NULL;  
				tempSuffix->auxiliar.numExceptions = 0;
				return tempSuffix;
			} 
			else if(temp[0]=='_') {
				tempSuffix->replacement = NULL;  
			}
			else {
				parser_allocateString(&tempSuffix->replacement, temp);
			}
			
			debug("\tReplacement: %s.\n", 
					(tempSuffix->replacement) ? tempSuffix->replacement : "none");
		}
		else
			tempSuffix->replacement = NULL;  

		// STEP 4 - Check for stemmer_exceptions, if any
		do {
			temp = strtok(NULL, ",");
			if(temp) {
				if(strcmp(temp, "|")==0)
					break;
				else {
					strncpy(tempSuffix->stemmer_exceptions[count++], temp, 15);
					debug("\t\tException [%d]: %s.\n", count, temp);
				}
			}
		}while(temp!=NULL);
		tempSuffix->auxiliar.numExceptions = count;
	}
	else {
		tempSuffix->minStemSize = 0;
		tempSuffix->replacement = NULL;  
		tempSuffix->auxiliar.numExceptions = 0;
	}

	return tempSuffix;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Receives a rule set in a string, parses it and create a 
 * 			rule structure with it.
 * \param	infoRuleDumpFile	Pointer to a file where all rules details 
 * 								will be written (for debugging purposes).
 * \param	ruleString	Entire rule set in on string
 *
 * \return	A rule struct filled with the data parsed from rule string.
 * 			The rule struct is allocated, MUST be deallocated by another 
 * 			function.
 */
static stem_step_struct* parser_stemParseRule(FILE *infoRuleDumpFile, char *ruleString)
{
	char *temp;
	int tempSize;
	int tempComp;
	int count = 0;

	// Allocates memory for a new rule structure
	stem_step_struct *tempStem = malloc(sizeof(stem_step_struct));
	stem_rule_struct *tempSuffix = NULL;

	// STEP 1 - Get the step name
	temp = strtok(ruleString, ",");
	if(temp == NULL) {
		free(tempStem);
		return NULL;
	}

	parser_allocateString(&tempStem->stepName, temp);

	// STEP 2 - Get word minimum size to perform a step
	tempSize = atoi(strtok(NULL, ","));
	tempStem->minWordLen = tempSize;
	
	// STEP 3 - Check if should compare all word to found an exception, or should
	// look only for suffixes
	tempComp = atoi(strtok(NULL, ","));
	tempStem->compEntireWord = tempComp;
	
	//  Prints to all steps and rules to a file
	if(infoRuleDumpFile) {
		fprintf(infoRuleDumpFile, "---------------------------------------------------------------------------------\n");
		fprintf(infoRuleDumpFile, "Structure allocate at %p, with %d bytes\n",
				tempStem, sizeof(stem_step_struct));
		fprintf(infoRuleDumpFile, "Rule String: '%s'\n", ruleString);
		
		fprintf(infoRuleDumpFile, "Rule name: %s.\n", temp);
		fprintf(infoRuleDumpFile, "Min. Word Size: %d.\n", tempSize);
		fprintf(infoRuleDumpFile, "Exceptions compare: %s.\n", 
				(tempComp == 1) ? "all word" : "suffix only");
	}

	// STEP 4 - Get end word conditions to perform a step
	do {
		temp = strtok(NULL, ",");
		if(temp) {
			if((strcmp(temp, "|")==0)) {
				break;
			}
			else if(temp[0]=='_') {
				temp = strtok(NULL, ",");
				break;
			}
			else {
				strncpy(tempStem->endWords[count++], temp, 5);
				if(infoRuleDumpFile)	
					fprintf(infoRuleDumpFile, "\tEnd Word Condition [%d]: %s.\n", count, temp);
			}
		}
	}while(temp!=NULL);
	tempStem->numEndWords = count;

	tempStem->rulesNumber = 0;

	// Parse a suffix rule
	do {
		tempSuffix = parser_stemParseSuffixRule(ruleString);

		if(tempSuffix) {
			if(tempStem->rulesNumber)
				tempStem->rules = realloc(tempStem->rules, (sizeof(stem_rule_struct)*(tempStem->rulesNumber++)));
			else { 
				tempStem->rules = malloc(sizeof(stem_rule_struct));
				tempStem->rulesNumber++;
			}

			tempStem->rules[tempStem->rulesNumber-1] = tempSuffix;
		}
	}while(tempSuffix);

	if(infoRuleDumpFile)	
		fprintf(infoRuleDumpFile, "Read %d suffixes for this rule\n", tempStem->rulesNumber);

	// Initialize the step counter
	tempStem->stepCount = 0;

	return tempStem;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Process an option from the configuration file
 * \param	val		Value to be processed. Can be 'YES' or 'NO'
 * \return	0, in case of 'NO', 1 in case of 'YES', -1 in error
 */
static int parser_processOptionValue(char *val)
{
	char *optionStrings[] = {
		RSLP_STRING_OPTION_NO,
		RSLP_STRING_OPTION_YES,
	};
	int i;

	// First, check if this name is valid	
	for(i=0; i<ELEMENTS_OF(optionStrings); i++)
		if(!(strcmp(optionStrings[i], val)))
			return i;

	return -1;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Check the option string passed, and return the value associated with it
 * \param	var		String to be checked
 * \return	The value associated with the string, or -1 in case of not found
 */
static int parser_processOption(char *var)
{
	struct {
		int val;
		char *string;
	}optionStrings[] = {
		{ RSLP_OPTION_STEPS_FILE                         , RSLP_STRING_OPTION_STEPS_FILE                         }  , 
		{ RSLP_OPTION_FLOW_FILE                          , RSLP_STRING_OPTION_FLOW_FILE                          }  , 
		{ RSLP_OPTION_NAMED_ENTITIES_FILE                , RSLP_STRING_OPTION_NAMED_ENTITIES_FILE                }  , 
		{ RSLP_OPTION_DO_STEMMING                        , RSLP_STRING_OPTION_DO_STEMMING                        }  , 
		{ RSLP_OPTION_REPLACE_ISO                        , RSLP_STRING_OPTION_REPLACE_ISO                        }  , 
		{ RSLP_OPTION_USE_STEM_DICTIONARY                , RSLP_STRING_OPTION_USE_STEM_DICTIONARY                }  , 
		{ RSLP_OPTION_USE_NAMED_ENTITIES_DICTIONARY      , RSLP_STRING_OPTION_USE_NAMED_ENTITIES_DICTIONARY      }  , 
		{ RSLP_OPTION_STEM_DICTIONARY_MAX_SIZE           , RSLP_STRING_OPTION_STEM_DICTIONARY_MAX_SIZE           }  , 
		{ RSLP_OPTION_NAMED_ENTITIES_DICTIONARY_MAX_SIZE , RSLP_STRING_OPTION_NAMED_ENTITIES_DICTIONARY_MAX_SIZE }  , 
	};
	int i;

	// First, check if this name is valid	
	for(i=0; i<ELEMENTS_OF(optionStrings); i++)
		if(!(strcmp(optionStrings[i].string, var)))
			return optionStrings[i].val;

	return -1;
}

/* -------------------------------------------------------------------------- */
void info_printStatsInfo(FILE *statsDumpFile, double elapsedTime, unsigned long inputFileCount,
		unsigned long wordCount)
{
	fprintf(statsDumpFile, "RSLP running info.\n");
	fprintf(statsDumpFile, "Total files read:;%lu;Total words read:;%lu\n", 
			inputFileCount, wordCount);
	fprintf(statsDumpFile, "Elapsed time (seconds):;%f\n", elapsedTime);

	// Prints the header
	fprintf(statsDumpFile, "Step;Suffix;Count\n");

	return;
}

/* -------------------------------------------------------------------------- */
void info_printStepsStats(FILE *statsDumpFile, stem_step_struct *stepStruct)
{
	unsigned int indRule;
	stem_rule_struct *rulePtr;

	if(stepStruct->rulesNumber) {
		for(indRule=0; indRule < stepStruct->rulesNumber; indRule++) {
			rulePtr = stepStruct->rules[indRule];
			// Step name; suffix string; # times was applied
			fprintf(statsDumpFile, "%s;%s;%lu\n", 
					stepStruct->stepName, rulePtr->suffix, rulePtr->auxiliar.ruleCount);
		}
	}

	return;
}

/* -------------------------------------------------------------------------- */
void info_printStepInfo(FILE *stepInfoDumpFile, stem_step_struct *stepStruct)
{
	int indRule, indException, indEndWords;
	int countSuffix = 0;
	stem_rule_struct *rulePtr;

	fprintf(stepInfoDumpFile, "=============================================================\n");
	fprintf(stepInfoDumpFile, "Structure allocate @ %p\n", stepStruct);
	fprintf(stepInfoDumpFile, "Rule name:\t '%s'\n", stepStruct->stepName);
	fprintf(stepInfoDumpFile, "Min. word size:\t %d\n", stepStruct->minWordLen);

	if(stepStruct->numEndWords) {
		for(indEndWords=0; indEndWords<stepStruct->numEndWords; indEndWords++) {
			fprintf(stepInfoDumpFile, "[%02d] End Word: '%s'\n", 
					indEndWords+1, stepStruct->endWords[indEndWords]);
		}
	}

	if(stepStruct->rulesNumber) {
		for(indRule=0; indRule<stepStruct->rulesNumber; indRule++) {
			rulePtr = stepStruct->rules[indRule];
			fprintf(stepInfoDumpFile, "-------------------------------------------------------------\n");
			fprintf(stepInfoDumpFile, "[%03d] Suffix:\t'%s'\n", ++countSuffix, rulePtr->suffix);
			fprintf(stepInfoDumpFile, "Min. stem size:\t%d\n", rulePtr->minStemSize);
			fprintf(stepInfoDumpFile, "Replacement:\t'%s'\n", 
					((rulePtr->replacement != NULL) ? rulePtr->replacement : "none"));
			if(rulePtr->auxiliar.numExceptions) {
				fprintf(stepInfoDumpFile, "Exceptions:\t%d\n", rulePtr->auxiliar.numExceptions);
				for(indException=0; indException < rulePtr->auxiliar.numExceptions; indException++)
					fprintf(stepInfoDumpFile, "\t\t%02d: '%s'\n",
							indException+1, rulePtr->stemmer_exceptions[indException]);
			}
		}
	}
	fprintf(stepInfoDumpFile, "=============================================================\n");

	return;
}

/* -------------------------------------------------------------------------- */
// FIXME: verificar todas as funções info_
// Ver quais devem ser removidas e quais devem ser documentadas
void info_showUsageHelp(void)
{
	printf("\n\nRemovedor de Sufixos da Língua Portuguesa (Orengo & Huyck, 2001)\n");
	printf("Usage:\n");
	printf("rslp <file_list> [--ignore-tags]\n");
	printf("Where:\t <file_list>:\tAn file with path and name to all files to be processed\n");
	printf("\t--ignore-tags:\tWhen reading a file, ignore everything between '<' and '>'\n");
	printf("\n");

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Free the memory allocated to the flow table
 * \param	readedFlow	Pointer to the flow table
 * \param	numStepsFlow	Number of steps in the flow table
 * \return	void
 */
static void stemmer_freeFlowStructure(stem_step_seq_struct *readedFlow[], int numStepsFlow)
{
	int x;

	for(x=0; x<numStepsFlow; x++) {
		if(readedFlow[x]->stepName)
			free(readedFlow[x]->stepName);
		if(readedFlow[x]->stepTrue)
			free(readedFlow[x]->stepTrue);
		if(readedFlow[x]->stepFalse)
			free(readedFlow[x]->stepFalse);
		if(readedFlow[x])
			free(readedFlow[x]);
	}

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Loads the rules flow from a file. It defines the order of rules
 * \param	readedFlow	Structure to hold the flow
 * \param	numStepsFlow	Number os steps readed in the file, in case of
 * 							sucess	
 * \param	flowFileName	Name of the flow file to be loaded
 * \return	void
 */
static void stemmer_loadFlowFile(stem_step_seq_struct *readedFlow[], int *numStepsFlow, 
		char *flowFileName)
{
	FILE *flowFile = NULL;
	char line[127];	// Line to be read from file
	int i = 0;
	char buf = '\0';
	char *str[3];
	int idxStr = 0;
	int idxFlow = 0;
	int done = 0;
	int idxStepName;
	int rv = 0;


	if( (flowFile = fopen(flowFileName, "rt")) == NULL) {
		infoMsg("Error opening flow file '%s'.\n", flowFileName);
		rv = -1;
	}
	else {
		while(!done) {
			i = 0;
			idxStr = 0;
			buf = '\0';	

			// Read a line from file
			while(!feof(flowFile)) {
				buf = getc(flowFile);
	
				if(buf=='\n')
					break;
				else
				if(buf != '\r' && buf != ' ') // Chars to ignore
					line[i++] = buf;
			}

			line[i] = '\0';
			if(strlen(line) <= 1) // Invalid line. End
				done=1;

			if(!done) {
				str[idxStr++] = strtok(line, ",");
				str[idxStr++] = strtok(NULL, ",");
				str[idxStr++] = strtok(NULL, ",");
				
				// Check readed strings 
				for(idxStr = 0; idxStr < 3 && !rv; idxStr++) {
					if(str[idxStr] == NULL) // String not found in file
						rv = -1; // Error
					else {
						int comp = 0;
						int found = 0;
						
						// Check for 'NULL' rules name
						if(!strcmp("NULL", str[idxStr])) {
							str[idxStr] = NULL;
							found = 1;
						}
						else {
							// Check readed string against step's names. All
							// string from file MUST BE a step's name, or NULL
							for(idxStepName = 0; 
									idxStepName < rslpMainStruct.rslpStemmerTotalSteps; 
									idxStepName++) {

								comp = strcmp(rslpMainStruct.rslpStemmerSteps[idxStepName]->stepName,
										str[idxStr]);

								if(!comp) // Found!
									found = 1;
							}
						}
						
						if(found) 
							rv = 0;
						else 
							rv = -1;
					}
				}
		
				if(rv == 0) {
					readedFlow[idxFlow] = malloc(sizeof(stem_step_seq_struct));
					// Initializes steps
					readedFlow[idxFlow]->stepName = NULL;
					readedFlow[idxFlow]->stepTrue = NULL;
					readedFlow[idxFlow]->stepFalse = NULL;

					if(str[0])
						parser_allocateString(&readedFlow[idxFlow]->stepName, str[0]);
					
					if(str[1])
						parser_allocateString(&readedFlow[idxFlow]->stepTrue, str[1]);
					
					if(str[2])
						parser_allocateString(&readedFlow[idxFlow]->stepFalse, str[2]);

					idxFlow++;
				}
				else { 
					debug("ERROR reading flow structure from file\n");
					done = 1;
				}
			}
		}
		fclose(flowFile);
	}

	if(rv == -1) { // Cannot read from file
		// Free alocated memory
		stemmer_freeFlowStructure(readedFlow, idxFlow);
		*numStepsFlow = ELEMENTS_OF(defaultFlow);

		// Allocates default flow
		for(idxFlow=0; idxFlow<*numStepsFlow; idxFlow++) {
			readedFlow[idxFlow] = malloc(sizeof(stem_step_seq_struct));
			// Initializes steps
			readedFlow[idxFlow]->stepName = NULL;
			readedFlow[idxFlow]->stepTrue = NULL;
			readedFlow[idxFlow]->stepFalse = NULL;

			if(defaultFlow[idxFlow].stepName)
				parser_allocateString(&readedFlow[idxFlow]->stepName, 
						defaultFlow[idxFlow].stepName);

			if(defaultFlow[idxFlow].stepTrue)
				parser_allocateString(&readedFlow[idxFlow]->stepTrue, 
						defaultFlow[idxFlow].stepTrue);

			if(defaultFlow[idxFlow].stepFalse)
				parser_allocateString(&readedFlow[idxFlow]->stepFalse, 
						defaultFlow[idxFlow].stepFalse);

		}
	}
	else {
		*numStepsFlow = idxFlow;
	}

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Load the configuration file, if exists, and sets the processing
 * 			flags and filenames.
 * \param	configFileName	Name of the file to be loaded with the configuration
 * \param	flags			Structure to store the processing flags
 * \param	files			Structure to keep stemmer filenames and paths
 * \return	void
 */
static void stemmer_loadConfigFile(char *configFileName, stem_processing_flags *flags, stem_filenames_struct *files)
{
	FILE *configFile = NULL;
	char linha[255], variavel[255], valor[255];
	char ch;	
	int i = 0;
	int state = RSLP_STATE_READING_VAR;
	int varIdx, valIdx;

	// Load default values
	flags->process_stemming = 1; // Don the stemming (will be disabled in case of error loading the steps file)
	flags->replace_iso_chars = 1; // Yes, replace characters with accents	
	flags->use_stem_dictionary = 0; // Don't use dictionary for stems	
	flags->use_named_entities_dictionary = 0; // No, don't use named entities
	flags->stem_dictionary_max_size = RSLP_MAX_DICTIONARY_MEMORY_USED;
	flags->named_entities_dictionary_max_size = RSLP_MAX_DICTIONARY_MEMORY_USED;

	files->stepsFileName = NULL;
	files->namedEntitiesFileName = NULL;
	files->flowFileName = NULL;

	if((configFile = fopen(configFileName, "rt"))!=NULL) {
		while(!feof(configFile)||state!=RSLP_STATE_END_READING) {
			ch = getc(configFile);

			if(ch=='=') {
				if(state==RSLP_STATE_READING_VAR) {
					linha[i] = '\0';
					strcpy(variavel, linha);
					i=0;
					state = RSLP_STATE_READING_VAL;
					continue;
				}
				else
					break; // Error!
			}

			if( (ch=='\n')|| (ch==-1) || (ch=='\r')) { // End of line or end of file
				if(state==RSLP_STATE_READING_VAL) {
					linha[i]='\0';
					strcpy(valor, linha);
					i=0;
					if(ch==-1)
						state = RSLP_STATE_END_READING;
					else
						state = RSLP_STATE_PROCESS;
				}
				else
					break; // Error!
			}

			if(state==RSLP_STATE_PROCESS) {
				varIdx = parser_processOption(variavel);
				valIdx = parser_processOptionValue(valor);
				
				switch(varIdx) {
					case RSLP_OPTION_STEPS_FILE:
						parser_allocateString(&files->stepsFileName, valor);
						break;
					case RSLP_OPTION_FLOW_FILE:
						parser_allocateString(&files->flowFileName, valor);
						break;
					case RSLP_OPTION_NAMED_ENTITIES_FILE:
						parser_allocateString(&files->namedEntitiesFileName, valor);
						break;
					case RSLP_OPTION_DO_STEMMING:
						if(valIdx>=0)
							flags->process_stemming = valIdx;
						break;
					case RSLP_OPTION_REPLACE_ISO:
						if(valIdx>=0)
							flags->replace_iso_chars = valIdx;
						break;
					case RSLP_OPTION_USE_STEM_DICTIONARY:
						if(valIdx>=0)
							flags->use_stem_dictionary = valIdx;
						break;
					case RSLP_OPTION_USE_NAMED_ENTITIES_DICTIONARY:
						if(valIdx>=0)
							flags->use_named_entities_dictionary = valIdx;
						break;
					case RSLP_OPTION_STEM_DICTIONARY_MAX_SIZE:
						valIdx = atoi(valor);	
						if(valIdx>=0)
							flags->stem_dictionary_max_size = valIdx;
						break;
					case RSLP_OPTION_NAMED_ENTITIES_DICTIONARY_MAX_SIZE:
						valIdx = atoi(valor);	
						if(valIdx>=0)
							flags->named_entities_dictionary_max_size = valIdx;
						break;
				}
				if(state!=RSLP_STATE_END_READING)
					state = RSLP_STATE_READING_VAR;
			}

			if(state== RSLP_STATE_READING_VAR || state==RSLP_STATE_READING_VAL)
				if(ch>=32 && ch<127) // TODO: and other chars?
					linha[i++] = ch;
		}
		fclose(configFile);
	}

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Free the allocated memory for a rule structure.
 * \param	ruleStruct	Pointer to the rule structure to be freed.
 * \return	void
 */
static void stemmer_freeStemStructure(stem_step_struct *ruleStruct)
{
	int indRule;
	stem_rule_struct *rulePtr;

	debug("Freeing allocated memory for rule '%s'\n", ruleStruct->stepName);

	free(ruleStruct->stepName);
	for(indRule=0; indRule < ruleStruct->rulesNumber; indRule++) {
		rulePtr = ruleStruct->rules[indRule];
		free(rulePtr->suffix);

		if(rulePtr->replacement)
			free(rulePtr->replacement);

		// TODO: 'stemmer_exceptions' are not deallocated because they are not
		// dynamically alocated.
	}

	free(ruleStruct->rules);
	free(ruleStruct);

	debug("Memory freed.\n");

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Append a suffix to a word
 * \param	word	Word to be processed
 * \param	replacement	Suffix to be added to word
 * \return	void
 */
static void stemmer_append(char *word, char *replacement)
{
	int i;
	int wordLen = strlen(word);
	int replacementLen = strlen(replacement);

	for(i=0; i<replacementLen; i++) 
		word[wordLen+i]=replacement[i];

	// TODO: cuidar como word é passada para esta função. Se ela foi
	// alocada, posso estar acessando fora dela com estes índices...
	word[wordLen+replacementLen] = '\0';

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Check if word ends in a given suffix
 * \param	word	Word to check
 * \param	endString	Suffix to be searched in word
 * \return 	1 if word has the suffix, 0 if not	 	
 */
static int stemmer_ends(char *word, char *endString)
{
	int wordLen = strlen(word);
	int endStringLen = strlen(endString);

	while(endStringLen > 0) {
		if( word[wordLen-1] != endString[endStringLen-1] ) {
			return 0;
		}
		else {
			wordLen--;
			endStringLen--;
		}
	}
	return 1;
}

/* -------------------------------------------------------------------------- */
/**	\brief	Check if word falls in any of the step´s exceptions.
 *	\param	word	Word to be checked
 *	\param	stemmer_exceptions	Array of exceptions in this step
 *	\param	numExceptions	Number of exceptions in this step
 * \param	compEntireWord	Flag to indicate how to compare word to exceptions: all word or
 * 							searching only for the suffix
 *	\return	1 if word is an exception, 0 if is not
 */
static int stemmer_exceptions(char *word, char stemmer_exceptions[][15], int numExceptions, 
		int compEntireWord)
{
	int i = 0;

	debug("Exceptions: %s for %d stemmer_exceptions, checking %s.\n", 
			word, numExceptions, 
			((compEntireWord == 1) ? "all word" : "suffix only"));

	while(i<numExceptions) {
		if(compEntireWord) {
			// Compare exception with word
			if(!(strcmp(word, stemmer_exceptions[i])))
				return 1;
			else
				i++;
		}
		else {
			// Search for exception as suffix in word
			if(stemmer_ends(word, stemmer_exceptions[i]))
				return 1;
			else
				i++;
		}
	}

	return 0;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Removes a suffix from word, according to step´s rules
 * \param	word	Word to be checked 
 * \param	compEntireWord	Flag to indicate how to compare word to exceptions: all word or
 * 							searching only for the suffix
 * \param	stem	Stemming rule structure		
 * \return	1 if 'word' was stemmed, 0 if not	
 */
static int stemmer_stripSuffix(char *word, unsigned int compEntireWord, stem_rule_struct *stem)
{
	int wordLen = strlen(word);
	int stemLen = strlen(stem->suffix);

	debug("Strip Suffix: word %s (%d), suffix %s (%d)\n", 
			word, wordLen, stem->suffix, stemLen);

	if((wordLen-stemLen >= stem->minStemSize)
			&& (stemmer_ends(word, stem->suffix))
			&& (!stemmer_exceptions(word, stem->stemmer_exceptions, stem->auxiliar.numExceptions, compEntireWord)))
	{
		word[wordLen-stemLen] = '\0';

		if(stem->replacement) {
			stemmer_append(word, stem->replacement);
		}
		return 1;
	}
	else
		return 0;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Changes the case of all characters from a word to lower case, and
 * 			also checks if has numbers on it. If it has, doesn't apply any
 * 			stemming rules in word
 * \param	word	Word to be processed
 * \return	0 if the doesn't contain any number, 1 if it does
 */
static int stemmer_stemToLowerCase(char *word)
{
	int i;
	int rv = 0;
	int wordLen = strlen(word);

	for(i=0; i<wordLen; i++) {
		if( ((word[i]>='A') && (word[i]<='Z')) 
				|| (((unsigned char)word[i]>=192) && ((unsigned char)word[i]<=221))) {
			word[i] = (word[i] |0x20);
			if(i==0) // Word starts with capital letter. Check named entities
				rv |= RSLP_MASK_WORD_START_WITH_CAPITAL_LETTER;
		}
		else
			if(word[i]>='0' && word[i]<='9')
				rv |= RSLP_MASK_WORD_HAS_DIGITS;

		if((unsigned char)word[i]>127)
			rv |= RSLP_MASK_WORD_HAS_ISO_CHARS;
	}

	if(wordLen<RSLP_STEMMER_MIN_WORD_LENGTH)
		rv |= RSLP_MASK_WORD_TOO_SMALL;
	return rv;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Removes all accentuation characters from a word
 * \param	word	Word to be processed
 * \return	void
 */
static void stemmer_stemRemoveAccents(char *word)
{
	int i = strlen(word)-1;
	unsigned char ch;

	while(i>=0){
		ch = word[i];
		// Small letters
		if(ch>=224 && ch<=229)
			word[i] = 'a';
		else
		if(ch==231) // ç
			word[i] = 'c';
		else
		if(ch>=232 && ch<=235)
			word[i] = 'e';
		else
		if(ch>=236 && ch<=239)
			word[i] = 'i';
		else
		if(ch>=242 && ch<=246)
			word[i] = 'o';
		else
		if(ch==241) // ñ 
			word[i] = 'n';
		else
		if(ch>=249 && ch<=252)
			word[i] = 'u';
		if(ch==253 || ch==255) // ý, 
			word[i] = 'y';

		i--;
	}

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Applies a step to a word
 * \param	step	Pointer to the structure of the step to be applied
 * \param	word	Word to be submitted to step
 * \return	0 if the step was not applied, 1 if it is applied
 */
static int stemmer_stemApplyRule(stem_step_struct *step, char *word)
{
	int i=0;
	int done=0;
	int indEndWords;

	debug("Processing word '%s' with step %s [%d suffixes]...\n",
			word, step->stepName, step->rulesNumber);

	// Tests conditions: minimun word length e end strings
	if(step->minWordLen) {
		if(step->minWordLen > strlen(word)) {
			debug("Returning 0 -> doesn´t have the minimun length (%d < %d).\n",
					step->minWordLen, strlen(word));
			return 0; // Word doesn´t have the minimun length
		}
	}

	if(step->numEndWords) {
		int check = 0;
		for(indEndWords=0; indEndWords < step->numEndWords; indEndWords++) {
			check |= stemmer_ends(word, step->endWords[indEndWords]);
		}
		if(!check) {
			debug("Returning 0 -> doesn´t end with the required strings\n");

			return 0; // Doesn´t end in any of strings required
		}
	}

	while(i < step->rulesNumber && !done) {
		if(stemmer_stripSuffix(word, step->compEntireWord, step->rules[i])) {
			// Increases the rule counter
			(step->rules[i])->auxiliar.ruleCount++;
			done = 1;
		}
		i++;
	}

	if(done)
		step->stepCount++;
	return done;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Receives a step name and searchs for it in the sequence table,
 * 			returning the table entry index if found
 * \param	stepName	Name of step to be searched
 * \param	seq			Table of steps sequences
 * \param	numStepsSeq	Number of steps in the table
 * \return	The table entry index for 'stepName', or -1 if not found
 */
static int stemmer_getIndexOfSeqStep(char *stepName, stem_step_seq_struct *seq[], int numStepsSeq)
{
	int idx;

	for(idx=0; idx<numStepsSeq; idx++) {
		if( strcmp(seq[idx]->stepName, stepName) == 0)
			return idx;
	}

	return -1;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Search for the step named 'wantedStepName' in the table of steps and
 * 			rules, and returns the table line index where the step was found.
 * \param	wantedStepName	Name of step to search
 * \param	steps	Pointer to the main stem struct
 * \param	numSteps	Number of total steps (readed from file)
 * \return	The table line index of the wanted step in steps 
 * 			table (loaded from file) if found, or -1 if not found
 */
static int stemmer_getIndexOfStep(char *wantedStepName, stem_step_struct *steps[], int numSteps)
{
	int idx;
	for(idx=0; idx<numSteps; idx++) {
		if(strcmp(wantedStepName, steps[idx]->stepName) == 0) 
			return idx;
	}
	return -1;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Open the steps file and load all steps and rules 
 * \param	stemmerSteps	Pointer to hold all steps structures
 * \param	stepsFileName	Name and path for the steps and rules file
 * \return	Number os steps read
 */
static int stemmer_loadStepsAndRules(stem_step_struct *stemmerSteps[], char *stepsFileName)
{
	FILE *stepsFile;	// File with all steps rules
	FILE *infoRuleDumpFile = fopen("rule_dump.txt", "wt");	// TODO: verificar a necessidade deste arquivo
	char *ruleString;
	int stepIdx = 0;

	// If not defined the filename, tries a default
	if(!stepsFileName)
		stepsFileName = "steprules.txt";

	// Opens a file with rule description
	if((stepsFile = fopen(stepsFileName, "rt"))==NULL) {
		infoMsg("RSLP ERROR - Cannot read file \t'%s'. Aborting.\n", stepsFileName);
		return 0;
	}
	else
		infoMsg("Loading steps and rules from: '%s'\n", stepsFileName);

	// Reads the entire step´s rules file
	while(!feof(stepsFile)) {
		if(parser_readStemStepFromFile(stepsFile, &ruleString)) {
			stemmerSteps[stepIdx] = parser_stemParseRule(infoRuleDumpFile, ruleString);
			
			if(infoRuleDumpFile)
				info_printStepInfo(infoRuleDumpFile, stemmerSteps[stepIdx]);
			debug("Rule %d @ %p. Suffix @ %p\n", stepIdx, stemmerSteps[stepIdx], stemmerSteps[stepIdx]->rules);
			
			stepIdx++;
		}
	}

	debug("Total of %d steps read from file\n", stepIdx);

	fclose(stepsFile);
	if(infoRuleDumpFile)
		fclose(infoRuleDumpFile);
	
	free(ruleString);

	return stepIdx;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Apply a set of stemming rules to a word. The word is modified by each
 * 			step, resulting in the stem
 * \param	word			Word to be stemmed
 * \param	rslpMainStruct	Global structure containing all information needed by 
 * 							the stemmer
 * \return	1 any step was applied, 0 if not
 */	
static int stemmer_applyRulesToWord(char *word, rslp_stemmer_main_struct *rslpMainStruct)
{
	// get the index of the first step in the sequence table
	int flowIdx = 0;
	int stepIdx;	// Indicates the index of the step in the steps array
	int done = 0;
	int rv = 0;

	// Get the step index for this step (step 0)
	// The 'step index' is the index of a step in the steps array in memory,
	// based in its name
	stepIdx = stemmer_getIndexOfStep(rslpMainStruct->rslpStepsFlow[flowIdx]->stepName,
			rslpMainStruct->rslpStemmerSteps, rslpMainStruct->rslpStemmerTotalSteps);
	
	// Just for precaution...
	if(stepIdx < 0) {
		debug("erro!\n");
		return 0;
	}
	
	while(!done) {
		// Apply the step rule to word
		rv = stemmer_stemApplyRule(rslpMainStruct->rslpStemmerSteps[stepIdx], word);

		debug("Aplicação de '%s' para '%s' retornou: %d\n",
				rslpMainStruct->rslpStemmerSteps[stepIdx]->stepName, word, rv);

		if(rv) { // Returned 'true'
			if(rslpMainStruct->rslpStepsFlow[flowIdx]->stepTrue) {
				// Gets the next step to be applied
				stepIdx = stemmer_getIndexOfStep(rslpMainStruct->rslpStepsFlow[flowIdx]->stepTrue,
						rslpMainStruct->rslpStemmerSteps, rslpMainStruct->rslpStemmerTotalSteps);

				flowIdx = stemmer_getIndexOfSeqStep(rslpMainStruct->rslpStepsFlow[flowIdx]->stepTrue, 
						rslpMainStruct->rslpStepsFlow, rslpMainStruct->rslpNumStepsFlow);
				debug("\tTRUE Idx : flow seq. [%d] '%s', step [%d] '%s'\n", 
						flowIdx, rslpMainStruct->rslpStepsFlow[flowIdx]->stepTrue, 
						stepIdx, rslpMainStruct->rslpStemmerSteps[stepIdx]->stepName);
			}
			else // There´s no next step. Signal to end loop
				done = 1;
		}
		else { // Step returned 'false'
			if(rslpMainStruct->rslpStepsFlow[flowIdx]->stepFalse) {
				stepIdx = stemmer_getIndexOfStep(rslpMainStruct->rslpStepsFlow[flowIdx]->stepFalse, 
						rslpMainStruct->rslpStemmerSteps, rslpMainStruct->rslpStemmerTotalSteps);
				// Gets the next step to be applied
				// Caution with collateral effects in flowIdx. Must be calculated after stepIdx
				flowIdx = stemmer_getIndexOfSeqStep(rslpMainStruct->rslpStepsFlow[flowIdx]->stepFalse, 
						rslpMainStruct->rslpStepsFlow, rslpMainStruct->rslpNumStepsFlow);
				debug("\tFALSE Idx : flow seq. [%d] '%s', step [%d] '%s'\n", 
						flowIdx, rslpMainStruct->rslpStepsFlow[flowIdx]->stepFalse, 
						stepIdx, rslpMainStruct->rslpStemmerSteps[stepIdx]->stepName);
			}
			else
				done = 1;
		}

		// FIXME: just for precaution. 
		if(stepIdx<0 || flowIdx<0) {
			debug("Invalid index: sequence %d, step %d\n", flowIdx, stepIdx);
			done = 1;
		}
	}

	return 1;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Initializes the main dictionary. This dictionary will hold
 * 			words processed and their stems. This way, when a word already
 * 			processed is found in the dictionary, we already have it stem.
 * \param	rslpDict			Pointer to the dictionary structure
 * \param	maxDictionarySize	
 * \return	1 if the dictionary was sucessfully created, 0 otherwise
 */
int dict_initDictionary(stem_dict_struct *rslpDict, int maxDictionarySize)
{
	// Create the root node
	rslpDict->dictRoot = trieNewNode(&rslpDict->dictTrieNumNodes, &rslpDict->dictTrieMemUsed);
	rslpDict->dictMaxSize = maxDictionarySize;
	
	if(!rslpDict->dictRoot) {
		// Failed to created the dictionary
		debug("Unable to create the main dictionary node. Dictionary disabled.\n");
		return 0;
	}
	else {
		// Root node created. Initializes the TRIE tree
		trieInit(rslpDict->dictRoot, &rslpDict->dictTrieNumNodes, 
				&rslpDict->dictTrieNumWords, &rslpDict->dictTrieMemUsed);
		return 1;
	}

	// Shouldn't be here...
	return 0;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Search for an word in the dictionary
 * \param	dictRootNode	Root node from dictionary
 * \param	word	Word to be searched in the dictionary
 * \return	0 if the word is not found, endWord flag if word is found
 */
unsigned short dict_searchWord(TRIE_NODE *dictRootNode, char *word)
{
	TRIE_NODE *node;

	node = trieSearchWord(dictRootNode, word);

	if(node) { // Word found in dictionary	
		if(node->stem) { // has a stem associated with this word
			strcpy(word, node->stem); // word become the stem
		}
		return node->endWord;
	}

	return 0;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Add a word in the dictionary. If the word and stem are not equal,
 * 			adds the stem too.
 * \param	stemDict	Stemmer dictionary structure
 * \param	word		Word to be added to the dictionary
 * \param	stem 		Stem to be added to the word node in the dictionary
 * \param	endWordFlag	Flag indicating if is a complete word or a substring of a word	
 * \return
 */
void dict_addWord(stem_dict_struct *stemDict, char *word, char *stem, unsigned short endWordFlag)
{
	// Check if the dictionary size isn't already exceeded
	if( (stemDict->dictTrieMemUsed) &&
			stemDict->dictTrieMemUsed >= (stemDict->dictMaxSize*1024*1024))
		return;

	if(!strcmp(word, stem)) // Word and stem are equal
		trieAddWord(stemDict->dictRoot, &stemDict->dictTrieNumNodes, &stemDict->dictTrieNumWords,
				&stemDict->dictTrieMemUsed, word, NULL, endWordFlag);
	else
		trieAddWord(stemDict->dictRoot, &stemDict->dictTrieNumNodes, &stemDict->dictTrieNumWords,
				&stemDict->dictTrieMemUsed, word, stem, endWordFlag);

	return;
}

/* -------------------------------------------------------------------------- */
/**
 * \brief	Loads a set of named entities from file. This word will be added
 * 			in the dictionary
 * \param	dict		Pointer to the dictionary structure
 * \param	fileName	Name and path of the file with the named entities 
 * 						to be loaded
 * \return	1 if the file is sucessfully opened and loaded, 0 in error
 */
static int dict_loadNamedEntitiesFile(stem_dict_struct *dict, char *fileName)
{
	FILE *namedEntitiesFile;
	char name[40];	// FIXME: change the 40 for a define
	char buf;
	int i = 0;

	if((namedEntitiesFile = fopen(fileName, "rt"))==NULL)	{
		debug("RSLP ERROR - Cannot read named entities file '%s'.\n", fileName);
		return 0;
	}
	else
		infoMsg("Loading named entities from: '%s'\n", fileName);

	while(!feof(namedEntitiesFile)) {
		buf=getc(namedEntitiesFile);
		
		// Check if is a valid char
		if( (buf>='A' && buf<='Z') || ((unsigned char)buf>=192 && (unsigned char)buf<=221)) {
			name[i] = (buf |0x20);
			i++;
		}
		else if( (buf>='a' && buf<='z')	|| ( (unsigned char)buf>=224)) {
			name[i] = buf;
			i++;
		}
		else {
			name[i] = '\0';
			if(name!=NULL && i >= RSLP_STEMMER_MIN_WORD_LENGTH) {
				dict_addWord(dict, name, name, RSLP_END_WORD_NAMED_ENTITY);
			}
			i = 0;
		}
	}

	fclose(namedEntitiesFile);
	return 1;
}
 
/* -------------------------------------------------------------------------- */
void rslpProcessWord(char *word, rslp_stemmer_main_struct *rslpMainStruct)
{
	int wordStatus;

	wordStatus = stemmer_stemToLowerCase(word);
	if(RSLP_WORD_HAS_DIGITS(wordStatus)) // Do the stemming only if the word doesn't contain numbers
		return;

	// word too small to be stemmed
	if(RSLP_WORD_TOO_SMALL(wordStatus)) {
		if(rslpMainStruct->rslpProcFlags.replace_iso_chars && (RSLP_WORD_HAS_ISO_CHARS(wordStatus)))
			stemmer_stemRemoveAccents(word);
		return;
	}

	// If word starts with capital letter and we're using named entities dictionary, check
	// if it wasn't a name
	if(RSLP_WORD_START_WITH_CAPITAL_LETTER(wordStatus) 
			&& (rslpMainStruct->rslpProcFlags.use_named_entities_dictionary)) {
		unsigned short wordFound;
		
		wordFound = dict_searchWord(rslpMainStruct->rslpNamedEntitiesDict.dictRoot, word);
		if(wordFound) {
			if(rslpMainStruct->rslpProcFlags.replace_iso_chars && RSLP_WORD_HAS_ISO_CHARS(wordStatus))
				stemmer_stemRemoveAccents(word);
			return;
		}
	}

	// Before process a word, check if it already exists in dictionary
	if(rslpMainStruct->rslpProcFlags.use_stem_dictionary) {
		unsigned short wordFound;

		wordFound = dict_searchWord(rslpMainStruct->rslpStemDict.dictRoot, word);
		
		if(!wordFound) { // Word not found in dictionary
			char *originalWord = malloc(strlen(word)+1);
			
			// Save the original word
			strcpy(originalWord, word);
		
			// Process word and then add it to the dictionary
			if(rslpMainStruct->rslpProcFlags.process_stemming)
				stemmer_applyRulesToWord(word, rslpMainStruct);

			if(rslpMainStruct->rslpProcFlags.replace_iso_chars && RSLP_WORD_HAS_ISO_CHARS(wordStatus))
				stemmer_stemRemoveAccents(word);
			
			dict_addWord(&rslpMainStruct->rslpStemDict, originalWord, word, RSLP_END_WORD_STEM);

			free(originalWord);
		}
	}
	else {
		// Not using dictionary
		if(rslpMainStruct->rslpProcFlags.process_stemming)
			stemmer_applyRulesToWord(word, rslpMainStruct);
		
		if(rslpMainStruct->rslpProcFlags.replace_iso_chars)
			stemmer_stemRemoveAccents(word);
	}

	return;
}

/* -------------------------------------------------------------------------- */
void rslpLoadStemmer(rslp_stemmer_main_struct *stemmerMainStruct, char *configFileName)
{
	// open file to write info messages
	rslpInfoFile = fopen("rslpinfo.txt","wt");
	infoMsg("RSLP Stemmer loading...\n");

	// Try to open a configuration file
	stemmer_loadConfigFile(configFileName, &stemmerMainStruct->rslpProcFlags, &stemmerMainStruct->rslpFileNames);

	// Load all steps and their rules
	stemmerMainStruct->rslpStemmerTotalSteps = 
		stemmer_loadStepsAndRules(stemmerMainStruct->rslpStemmerSteps, stemmerMainStruct->rslpFileNames.stepsFileName);
	
	if(!stemmerMainStruct->rslpStemmerTotalSteps) // Error reading the steps?
		stemmerMainStruct->rslpProcFlags.process_stemming = 0; // Disables the stemming	

	// Try to load steps flow from a file
	stemmer_loadFlowFile(stemmerMainStruct->rslpStepsFlow, &stemmerMainStruct->rslpNumStepsFlow,
			stemmerMainStruct->rslpFileNames.flowFileName);
	
	infoMsg("Total steps loaded: %d\n", stemmerMainStruct->rslpStemmerTotalSteps);
	
	// Initialize dictionary for stems
	if(stemmerMainStruct->rslpProcFlags.use_stem_dictionary) {
		stemmerMainStruct->rslpProcFlags.use_stem_dictionary = 
			dict_initDictionary(&stemmerMainStruct->rslpStemDict, stemmerMainStruct->rslpProcFlags.stem_dictionary_max_size);
	}

	infoMsg("Stems dictionary is %s\n", 
			(stemmerMainStruct->rslpProcFlags.use_stem_dictionary) ? "on" : "off");

	// Initialize the dictionary for named entities
	if(stemmerMainStruct->rslpProcFlags.use_named_entities_dictionary) {
		stemmerMainStruct->rslpProcFlags.use_named_entities_dictionary = 
			dict_initDictionary(&stemmerMainStruct->rslpNamedEntitiesDict, stemmerMainStruct->rslpProcFlags.named_entities_dictionary_max_size);
	
		// Named Entities dictionary initialized. Try to load names	
		if(stemmerMainStruct->rslpProcFlags.use_named_entities_dictionary) {
			stemmerMainStruct->rslpProcFlags.use_named_entities_dictionary = 
				dict_loadNamedEntitiesFile(&stemmerMainStruct->rslpNamedEntitiesDict, 
						stemmerMainStruct->rslpFileNames.namedEntitiesFileName);
			infoMsg("Named Entities is on. Dictionary loaded with %lu names.\n",
						stemmerMainStruct->rslpNamedEntitiesDict.dictTrieNumWords);
		}
		else {
			// Couldn't load names from file, so the dictionary is turned off
			infoMsg("Named Entities dictionary is off.\n");
		}
	}
	else
		infoMsg("Named Entities dictionary is off.\n");

	return;
}

/* -------------------------------------------------------------------------- */
void rslpUnloadStemmer(rslp_stemmer_main_struct *stemmerMainStruct)	
{
	int idx;

	// Free the stem dictionary memory
	if(stemmerMainStruct->rslpProcFlags.use_stem_dictionary) {
		infoMsg("Unloading stem dictionary with %lu nodes, %lu words, using %lu Mb (max allowed was %d Mb).\n",
				stemmerMainStruct->rslpStemDict.dictTrieNumNodes, 
				stemmerMainStruct->rslpStemDict.dictTrieNumWords,
				stemmerMainStruct->rslpStemDict.dictTrieMemUsed/1024/1024,
				stemmerMainStruct->rslpStemDict.dictMaxSize);
		
		trieUnload(stemmerMainStruct->rslpStemDict.dictRoot);
	}
	
	if(stemmerMainStruct->rslpProcFlags.use_named_entities_dictionary) {
		infoMsg("Unloading Named Entities dictionary with %lu nodes, %lu words, using %lu Mb (max allowed was %d Mb).\n",
				stemmerMainStruct->rslpNamedEntitiesDict.dictTrieNumNodes, 
				stemmerMainStruct->rslpNamedEntitiesDict.dictTrieNumWords,
				stemmerMainStruct->rslpNamedEntitiesDict.dictTrieMemUsed/1024/1024,
				stemmerMainStruct->rslpNamedEntitiesDict.dictMaxSize);
		
		trieUnload(stemmerMainStruct->rslpNamedEntitiesDict.dictRoot);
	}

	// Free files names structure
	if(stemmerMainStruct->rslpFileNames.stepsFileName)
		free(stemmerMainStruct->rslpFileNames.stepsFileName);
	if(stemmerMainStruct->rslpFileNames.namedEntitiesFileName)
		free(stemmerMainStruct->rslpFileNames.namedEntitiesFileName);
	if(stemmerMainStruct->rslpFileNames.flowFileName)
		free(stemmerMainStruct->rslpFileNames.flowFileName);

	// Free memory alocated for steps flow
	stemmer_freeFlowStructure(stemmerMainStruct->rslpStepsFlow, stemmerMainStruct->rslpNumStepsFlow);

	for(idx = 0; idx < stemmerMainStruct->rslpStemmerTotalSteps; idx++)
		stemmer_freeStemStructure(stemmerMainStruct->rslpStemmerSteps[idx]);

	infoMsg("RSLP Stemmer unloaded.\n");
	
	// Closes the debug file
	if(rslpInfoFile != NULL)
		fclose(rslpInfoFile);

	return;
}
