#ifndef PROJECT_ASSEMBLE_H
#define PROJECT_ASSEMBLE_H


#define MAX_LINE_LENGTH 82          /* Maximum character in a line */
#define DEFAULT_ADDRESS 100         /* Default address for assembling */


/* Launching the assembler by calling 'first_pass' */
int assemble(char *file_name, unsigned int address);


/* Check if a line is empty */
int is_empty(const char *instruction);


/* Check if a line is a comment */
int is_comment(char *statement);


/* Print syntax errors */
int get_errors();


#endif
