#ifndef PROJECT_SYMBOLS_H
#define PROJECT_SYMBOLS_H

#define MAX_LABEL_LENGTH 31         /* Maximum length for a label name */
#define TOTAL_KEYWORDS 28           /* Number of supported keywords */


/* Supported types of symbols */
typedef enum symbol_type {
    CODE_SYMBOL = -99, DATA_SYMBOL, EXTERN_SYMBOL, ENTRY_SYMBOL
} SymbolType;


/* A symbol - used as a linked list */
typedef struct symbol *sptr;
typedef struct symbol {
    char *name;
    unsigned int value;
    SymbolType type;
    sptr next;
    sptr prev;
} Symbol;


/* Create a new symbol for the symbol table */
sptr new_symbol(char *name, unsigned int value, SymbolType type);


/* Adds a new symbol to the symbol table. Returns 1 on success, else returns 0 */
sptr add_symbol(sptr head, sptr new);


/* Checks if there's a label for a specific instruction */
int is_label(char *instruction);


/* Returns the length of the label. Assuming there is a valid label */
int get_label_length(const char *instruction);


/* Returns the name of the label as a string. Assuming the instruction has a valid label */
char *get_label_name(char *instruction);


/* Delete all the symbols from the table */
void delete_symbols_table(sptr head);


/* Update all data symbols by adding the instruction counter to their values */
void update_symbols(sptr head, const unsigned int *instruction_counter);


/* Searching for a symbol with a specific name in the symbols table */
sptr search_symbol(sptr head, char *name);


/* Set the symbol type of a given symbol to 'entry' */
void set_entry(sptr head, char *name);


/* Checks is a given string is an assembly keyword. */
int is_keyword(char *str);


#endif
