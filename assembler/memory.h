#ifndef PROJECT_INSTRUCTION_MEMORY_H
#define PROJECT_INSTRUCTION_MEMORY_H

#include "symbols.h"

#define WORD_LENGTH 12              /* Number of bits in a memory word */
#define TOTAL_OPERATIONS 16         /* Number of supported operations */
#define REGISTERS 8                 /* Number of available registers */
#define TOTAL_KEYWORDS 28           /* Number of supported keywords */


/* Supported non-operations statements */
typedef enum statement_type {
    DATA = 1, STRING, ENTRY, EXTERN
} StatementType;


/* Supported operations */
typedef enum operation {
    MOV, CMP, ADD, SUB, NOT, CLR, LEA, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, STOP
} Operation;


/* Addressing type of an operand */
typedef enum addressing_type {
    IMMEDIATE = 1, DIRECT = 3, REGISTER_DIRECT = 5
} AddressingType;


/* Memory word - used as a linked list */
typedef struct word *wptr;
typedef struct word {
    short binary_code[WORD_LENGTH];
    unsigned int address;
    char *ext;
    wptr next;
} Word;


/* Add a word to the memory */
wptr add_word(wptr head, wptr new);


/* Stores data in memory */
wptr store_data(char *statement, unsigned int *data_counter);


/* Stores an instruction in memory */
wptr store_instruction(char *instruction, unsigned int *instruction_counter, sptr symbols_table);


/* Checks if a given statement is an operation */
int is_operation(char *statement);


/* Returns the code of a given operation */
Operation get_operation(char *statement);


/* Returns the label operand for 'entry' or 'extern' statement */
char *get_label_operand(char *statement, StatementType type);


/* Indicates if a given instruction is '.data' or '.string' */
int is_data_statement(StatementType type);


/* Returns an operand as a string. if pos = 0 returns destination, if pos = 1 returns source. */
char *get_operand(char *instruction, int pos);


/* Returns the type of a non-operation instruction */
StatementType get_statement_type(char *statement);


/* Returns the addressing type of a given operand */
AddressingType get_addressing_type(char *operand);


/* Returns the number of required operands for a given operation */
int get_number_of_operands(Operation op);


/* Delete memory and free all of its components */
void delete_memory(wptr head);



#endif
