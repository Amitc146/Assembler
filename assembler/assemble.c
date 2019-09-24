/* 
 *  This file includes the functions of the assembling process.
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "assemble.h"
#include "memory.h"


#define MAX_ERROR_LENGTH 100
#define BIN 2
#define HALF 6
#define EXT_LENGTH 3


enum {
    FALSE, TRUE
};

enum {
    LOWER, UPPER
};


static int first_pass(char *file_name, unsigned int address);

static int second_pass(char *file_name, unsigned int address, sptr symbols_table, wptr data_memory);

static void create_ent(char *file_name, sptr symbols_table);

static void create_ext(char *file_name, wptr instruction_memory);

static void create_ob(char *file_name, wptr data_memory, wptr instruction_memory);

static char *create_file_name(char *file_name, char *format);

static char *get_file_name(char *file_name);

static int convert_binary_to_decimal(const short *word, int part);

static void syntax_error(unsigned int line, char *str);

static int openfile_error(char *file_name);

static char *skip_spaces(char *address);

static int allocate_error(char *func);


int main(int argc, char *argv[]) {
    int i, run = 0;

    if (!argc) {
        printf("No input files detected. \n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        if (assemble(argv[i], DEFAULT_ADDRESS))
            run++;
        printf("\n\n");
    }

    printf("Successfully assembled %d files out of %d.\n", run, argc - 1);

    return 0;
}


/* Launching the assembler by calling 'first_pass' */
int assemble(char *file_name, unsigned int address) {
    int run;
    char *name = get_file_name(file_name);

    run = first_pass(name, address);

    return run;
}


/* Creating the data memory and the symbols table, and counting the instructions with 'instruction_counter' for the second pass */
static int first_pass(char *file_name, unsigned int address) {
    FILE *fd;
    char buf[MAX_LINE_LENGTH], *temp = NULL;
    unsigned int line, data_counter, instruction_counter;
    int label_flag, error_flag;
    wptr data, data_memory, instruction;
    sptr symbol, symbols_table;
    StatementType type;

    if (!(fd = fopen(file_name, "r")))
        return openfile_error(file_name);

    printf("Assembling: '%s' \n", file_name);

    line = data_counter = instruction_counter = 0;
    data_memory = NULL;
    symbols_table = NULL;
    while (fgets(buf, MAX_LINE_LENGTH, fd)) {

        type = get_statement_type(buf);
        label_flag = is_label(buf);
        line++;

        if (is_data_statement(type)) {
            data_counter += address;
            data = store_data(buf, &data_counter);
            data_counter -= address;
            if (data) {
                data_memory = add_word(data_memory, data);
                if (label_flag) {
                    symbol = new_symbol(get_label_name(buf), data->address, DATA_SYMBOL);
                    symbols_table = add_symbol(symbols_table, symbol);
                    if (!symbols_table)
                        syntax_error(line, "invalid label name.");
                }
            } else
                syntax_error(line, "invalid data.");
        } else if (type == EXTERN || type == ENTRY) {
            if (type == EXTERN) {
                temp = get_label_operand(buf, type);
                symbol = new_symbol(temp, 0, EXTERN_SYMBOL);
                symbols_table = add_symbol(symbols_table, symbol);
                if (!symbols_table)
                    syntax_error(line, "invalid label name.");
            }
        } else if (is_operation(buf)) {
            instruction_counter += address;
            instruction = store_instruction(buf, &instruction_counter,
                                            NULL);   /* passing NULL since the symbols table isn't full yet. */
            instruction_counter -= address;
            if (instruction) {
                if (label_flag) {
                    symbol = new_symbol(get_label_name(buf), instruction->address, CODE_SYMBOL);
                    symbols_table = add_symbol(symbols_table, symbol);
                    if (!symbols_table)
                        syntax_error(line, "invalid label name.");
                }
            }
            delete_memory(instruction);
        } else if (!is_comment(buf) && !is_empty(buf))
            syntax_error(line, "invalid statement.");
    }

    update_symbols(symbols_table, &instruction_counter);

    printf("First pass: Done. \n");
    error_flag = get_errors();

    fclose(fd);

    if (error_flag) {
        delete_symbols_table(symbols_table);
        delete_memory(data_memory);
        return 0;
    }
    return second_pass(file_name, address, symbols_table, data_memory);
}


/* Creating the instruction memory, and fixing missing details on the symbols table */
static int second_pass(char *file_name, unsigned int address, sptr symbols_table, wptr data_memory) {
    FILE *fd;
    char buf[MAX_LINE_LENGTH], *temp;
    unsigned int line, instruction_counter;
    wptr instruction, instruction_memory;
    StatementType type;
    int error_flag;

    line = instruction_counter = 0;
    instruction_memory = NULL;

    if (!(fd = fopen(file_name, "r")))
        return openfile_error(file_name);

    while (fgets(buf, MAX_LINE_LENGTH, fd)) {
        type = get_statement_type(buf);
        line++;

        if (!is_data_statement(type)) {
            if (type == ENTRY || type == EXTERN) {
                if (type == ENTRY) {
                    temp = get_label_operand(buf, type);
                    set_entry(symbols_table, temp);
                    free(temp);
                }
            } else if (is_operation(buf)) {
                instruction_counter += address;
                instruction = store_instruction(buf, &instruction_counter,
                                                symbols_table);                 /* Passing the symbol table which was built on first pass */
                instruction_counter -= address;
                if (instruction)
                    instruction_memory = add_word(instruction_memory, instruction);
                else
                    syntax_error(line, "invalid statement.");
            } else if (!is_comment(buf) && !is_empty(buf))
                syntax_error(line, "invalid statement.");
        }
    }

    error_flag = get_errors();
    if (!error_flag)
        printf("Second pass: Done. \n");

    create_ent(file_name, symbols_table);
    create_ext(file_name, instruction_memory);
    create_ob(file_name, data_memory, instruction_memory);

    delete_symbols_table(symbols_table);
    delete_memory(instruction_memory);
    delete_memory(data_memory);

    fclose(fd);

    return !error_flag;
}


/* Check if a line is empty */
int is_empty(const char *instruction) {
    int i;

    for (i = 0; instruction[i] != '\0'; i++) {
        if (!isspace(instruction[i]))
            return FALSE;
    }

    return TRUE;
}


/* Check if a line is a comment */
int is_comment(char *statement) {
    statement = skip_spaces(statement);
    return (*statement == ';');
}


/* Print syntax errors */
int get_errors() {
    FILE *err;
    char buf[MAX_ERROR_LENGTH];
    int count_errors = 0;

    if (!(err = fopen("syntax_errors.txt", "r"))) {
        return 0;
    }
    while (fgets(buf, MAX_ERROR_LENGTH, err)) {
        printf("%s", buf);
        count_errors++;
    }
    fclose(err);

    remove("syntax_errors.txt");
    return count_errors;
}


/* Creates the .ob file by converting every memory word into 2 chars in base-64 representation */
static void create_ob(char *file_name, wptr data_memory, wptr instruction_memory) {
    FILE *fd;
    wptr temp;
    int upper_bits, lower_bits, count;
    char base_64[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
                      'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
                      'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4',
                      '5', '6', '7', '8', '9', '+', '/'};
    char *name = create_file_name(file_name, ".ob");

    if (!(fd = fopen(name, "w"))) {
        openfile_error(name);
        return;
    }

    for (temp = instruction_memory, count = 0; temp; temp = temp->next, count++);
    fprintf(fd, "%d ", count);
    for (temp = data_memory, count = 0; temp; temp = temp->next, count++);
    fprintf(fd, "%d\n", count);

    for (temp = instruction_memory; temp; temp = temp->next) {
        upper_bits = convert_binary_to_decimal(temp->binary_code, UPPER);
        lower_bits = convert_binary_to_decimal(temp->binary_code, LOWER);
        fprintf(fd, "%c%c\n", base_64[upper_bits], base_64[lower_bits]);
    }
    for (temp = data_memory; temp; temp = temp->next) {
        upper_bits = convert_binary_to_decimal(temp->binary_code, UPPER);
        lower_bits = convert_binary_to_decimal(temp->binary_code, LOWER);
        fprintf(fd, "%c%c\n", base_64[upper_bits], base_64[lower_bits]);
    }


    printf("file created: '%s' \n", name);
    free(name);
    fclose(fd);
}


/* Creates the .ent file by scanning the symbols table for symbols with 'entry' type */
static void create_ent(char *file_name, sptr symbols_table) {
    FILE *fd;
    sptr temp;
    int check = 0;
    char *name = create_file_name(file_name, ".ent");

    if (!(fd = fopen(name, "w"))) {
        openfile_error(name);
        return;
    }

    for (temp = symbols_table; temp; temp = temp->next) {
        if (temp->type == ENTRY_SYMBOL) {
            fprintf(fd, "%-10s %d\n", temp->name, temp->value);
            check++;
        }
    }

    fclose(fd);

    if (!check) {
        remove(name);
        free(name);
        return;
    }

    printf("file created: '%s' \n", name);
    free(name);
}


/* Creates the .ext file by scanning the memory for words with 'ext' field */
static void create_ext(char *file_name, wptr instruction_memory) {
    FILE *fd;
    wptr temp;
    int check = 0;
    char *name = create_file_name(file_name, ".ext");

    if (!(fd = fopen(name, "w"))) {
        openfile_error(name);
        return;
    }

    for (temp = instruction_memory; temp; temp = temp->next) {
        if (temp->ext) {
            fprintf(fd, "%-10s %d\n", temp->ext, temp->address);
            check++;
        }
    }

    fclose(fd);

    if (!check) {
        remove(name);
        free(name);
        return;
    }

    printf("file created: '%s' \n", name);
    free(name);
}


/* Returns a string of a file name with a specified extension */
static char *create_file_name(char *file_name, char *extension) {
    unsigned int i;
    char *name;

    for (i = 0; file_name[i] != '.'; i++);

    if (!(name = (char *) malloc(((i + strlen(extension)) * sizeof(char)) + 1)))
        exit(allocate_error("create_file_name"));

    strncpy(name, file_name, i);
    name[i] = '\0';
    strcat(name, extension);

    return name;
}


/* Adding '.as' to the file name */
static char *get_file_name(char *file_name) {
    unsigned int i;
    char *name;

    for (i = 0; file_name[i] != '\0'; i++);
    i += EXT_LENGTH;   /* adding 3 for the '.as' */

    if (!(name = (char *) malloc((i * sizeof(char))) + 1))
        exit(allocate_error("get_file_name"));

    strncpy(name, file_name, i);
    name[i] = '\0';
    strcat(name, ".as");

    return name;
}


/* Converts a specific half of a given data word into a decimal number */
static int convert_binary_to_decimal(const short *word, int part) {
    int i, res = 0;

    if (part == UPPER) {
        for (i = 0; i < HALF; i++)
            res += (int) (word[i] * pow(BIN, (HALF - 1 - i)));
    } else if (part == LOWER) {
        for (i = 0; i < HALF; i++)
            res += (int) (word[HALF + i] * pow(2, (HALF - 1 - i)));
    }

    return res;
}


/* Returns a pointer to the next non-space character */
static char *skip_spaces(char *address) {
    while (isspace(*address))
        address++;
    return address;
}


/* Assembling errors (syntax error) */
static void syntax_error(unsigned int line, char *str) {
    FILE *err;

    if (!(err = fopen("syntax_errors.txt", "a+"))) {
        openfile_error("syntax_error");
        return;
    }

    fprintf(err, "ERROR: in line %d - %s\n", line, str);

    fclose(err);
}


/* Printing a message when fails to open a file */
static int openfile_error(char *file_name) {
    fprintf(stderr, "*** ERROR: failed to open '%s' *** \n", file_name);
    return 0;
}


/* Memory allocation fail */
static int allocate_error(char *func) {
    fprintf(stderr, "*** ERROR: In function '%s' - failed to allocate memory. *** \n", func);
    return 1;
}


