/* This file is implementing the memory in the project.
 * All of the functions in this file are used to implement data and instruction memory. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "memory.h"


#define EXTERN_LENGTH 7
#define ENTRY_LENGTH 6
#define DECIMAL 10
#define BINARY 2
#define NEXT 1
#define DATA_LENGTH 5
#define STRING_LENGTH 7
#define OPERATION_LENGTH 3
#define NUMBER_OF_ARGUMENTS 3
#define DST_ADDRESSING_TYPE 2
#define SRC_ADDRESSING_TYPE 0
#define OPCODE 1
#define ONE_OPERAND 1
#define TWO_OPERANDS 2
#define REGISTER_ENCODING_LENGTH 5
#define LSB 11
#define OPERAND_LENGTH 3
#define OPCODE_LENGTH 4
#define ENCTYPE_LENGTH 2
#define LARGEST_POSSIBLE_NUMBER 2047

enum {
    FALSE, TRUE
};

enum {
    DST, SRC
};


static wptr new_instruction_word(unsigned int *instruction_counter);

static wptr new_data_word(int value, unsigned int *data_counter);

static wptr store_num(char *statement, unsigned int *data_counter);

static wptr store_string(char *statement, unsigned int *data_counter);

static wptr create_first_word(int *args, unsigned int *instruction_counter);

static wptr create_operands(char *src, char *dst, unsigned int *instruction_counter, sptr symbols_table);

static void create_immediate_operand(short *tar, int val);

static void create_binary_code(short *tar, int val);

static void create_binary_reg(short *tar, char *reg, int pos);

static int get_register_number(char *reg);

static unsigned int get_operand_length(const char *str);

static char *get_operand_position(char *instruction);

static int check_zero(char *str);

static int is_number(char c);

static int is_string(char *str);

static void set_encoding_type(wptr word, sptr symbol);

static char *skip_spaces(char *address);

static int allocate_error(char *func);


/* Adds an instruction in memory */
wptr add_word(wptr head, wptr new) {
    wptr current;

    if (!new)
        return head;

    if (!head)
        return new;

    for (current = head; current->next; current = current->next);
    current->next = new;

    return head;
}


/* Stores data in memory */
wptr store_data(char *statement, unsigned int *data_counter) {
    StatementType type = get_statement_type(statement);
    wptr head = NULL;

    statement = skip_spaces(statement);
    if (is_label(statement))
        statement += get_label_length(statement) + NEXT;
    statement = skip_spaces(statement);


    if (type == DATA)
        head = store_num(statement, data_counter);

    else if (type == STRING)
        head = store_string(statement, data_counter);

    return head;
}


/* Stores an instruction in memory */
wptr store_instruction(char *instruction, unsigned int *instruction_counter, sptr symbols_table) {
    wptr head = NULL;
    int args[NUMBER_OF_ARGUMENTS];
    char *src, *dst;
    int required_operands, number_of_operands = 0;

    args[OPCODE] = get_operation(instruction);
    required_operands = get_number_of_operands(args[OPCODE]);

    if (args[OPCODE] == STOP || args[OPCODE] == RTS) {
        args[SRC_ADDRESSING_TYPE] = args[DST_ADDRESSING_TYPE] = 0;
        return create_first_word(args, instruction_counter);
    }

    src = get_operand(instruction, SRC);
    dst = get_operand(instruction, DST);
    number_of_operands += src ? 1 : 0;
    number_of_operands += dst ? 1 : 0;

    args[SRC_ADDRESSING_TYPE] = get_addressing_type(src);
    args[DST_ADDRESSING_TYPE] = get_addressing_type(dst);

    if ((src && !args[SRC_ADDRESSING_TYPE]) || (dst && !args[DST_ADDRESSING_TYPE]) ||
        (required_operands != number_of_operands))
        return NULL;

    if (src && !dst) {
        int temp = args[SRC_ADDRESSING_TYPE];
        args[SRC_ADDRESSING_TYPE] = args[DST_ADDRESSING_TYPE];
        args[DST_ADDRESSING_TYPE] = temp;
    }

    head = create_first_word(args, instruction_counter);
    head = add_word(head, create_operands(src, dst, instruction_counter, symbols_table));

    if (src)
        free(src);
    if (dst)
        free(dst);

    return head;
}


/* Checks if a given statement is an operation */
int is_operation(char *statement) {
    if (get_operation(statement) != -1)
        return TRUE;
    return FALSE;
}


/* Returns the code of a given operation */
Operation get_operation(char *statement) {
    int i;
    char buf[] = {'\0', '\0', '\0', '\0', '\0'};
    char *operations[] = {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc",
                          "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"};

    statement = skip_spaces(statement);
    if (is_label(statement))
        statement += get_label_length(statement) + NEXT;
    statement = skip_spaces(statement);

    for (i = 0; i < OPERATION_LENGTH; i++) {
        buf[i] = *statement;
        statement++;
    }
    for (i = 0; i < TOTAL_OPERATIONS - 1; i++) {
        if (!strcmp(buf, operations[i])) {
            if ((*statement == ' ') || (isspace(*statement) && i == RTS))
                return i;
        }
    }

    buf[OPERATION_LENGTH] = *statement;
    if (!strcmp(buf, operations[TOTAL_OPERATIONS - 1]))
        return STOP;

    return -1;      /* default: invalid operation. */
}


/* Returns an operand as a string. if pos = 0 returns destination, if pos = 1 returns source */
char *get_operand(char *instruction, int pos) {
    unsigned int length;
    char *operand;

    instruction = get_operand_position(instruction);
    length = get_operand_length(instruction);

    if (pos == DST) {
        instruction += length;
        instruction = skip_spaces(instruction);
        if (*instruction == ',')
            instruction++;
        instruction = skip_spaces(instruction);
        length = get_operand_length(instruction);
    }

    if (*instruction == '\0')
        return NULL;

    if (!(operand = (char *) malloc((length * sizeof(char)) + NEXT)))
        exit(allocate_error("get_operand"));

    strncpy(operand, instruction, length);
    operand[length] = '\0';

    instruction += length;
    instruction = skip_spaces(instruction);

    if (pos == DST && *instruction != '\0')
        return NULL;

    return operand;
}


/* Returns the number of required operands for a given operation */
int get_number_of_operands(Operation op) {
    if (op == MOV || op == CMP || op == ADD || op == SUB || op == LEA)
        return TWO_OPERANDS;
    return ONE_OPERAND;

}


/* Returns the addressing type of a given operand */
AddressingType get_addressing_type(char *operand) {
    int i;
    char *registers[] = {"@r0", "@r1", "@r2", "@r3", "@r4", "@r5", "@r6", "@r7"};

    if (!operand)
        return 0;

    if (is_number(*operand)) {
        while (isspace(*operand)) {
            if (!isdigit(*operand))
                return 0;
            operand++;
        }
        return IMMEDIATE;
    } else if (isalpha(*operand)) {
        while (*operand != '\0' && isspace(*operand)) {
            if (!isalnum(*operand))
                return 0;
            operand++;
        }
        return DIRECT;
    }

    for (i = 0; i < REGISTERS; i++) {
        if (!strcmp(operand, registers[i]))
            return REGISTER_DIRECT;
    }

    return 0;
}


/* Returns the label operand for 'entry' or 'extern' statement */
char *get_label_operand(char *statement, StatementType type) {
    unsigned int length = 0;
    char *operand;

    statement = skip_spaces(statement);
    if (is_label(statement)) {
        printf("WARNING: label on 'ENTRY' or 'EXTERN' statement has no effect.");
        statement += get_label_length(statement);
    }
    statement = skip_spaces(statement);

    statement += (type == EXTERN) ? EXTERN_LENGTH : ENTRY_LENGTH;
    statement = skip_spaces(statement);

    while (statement[length] != '\n' && statement[length] != '\0') {
        if (!isalnum(*statement) || isspace(*statement) || length > MAX_LABEL_LENGTH)
            return NULL;
        length++;
    }

    if (!(operand = (char *) malloc((length * sizeof(char)) + NEXT)))
        exit(allocate_error("get_extern_operand"));

    strncpy(operand, statement, length);
    operand[length] = '\0';

    if (!length || isspace(operand[0]))
        return NULL;

    return operand;
}


/* Indicates if a given instruction is '.data' or '.string' */
int is_data_statement(StatementType type) {
    return (type == DATA || type == STRING);
}


/* Returns the type of a non-operation instruction */
StatementType get_statement_type(char *statement) {
    char *str;
    int i;
    unsigned int length = 0;
    StatementType type = 0;

    if (is_label(statement))
        statement += get_label_length(statement) + NEXT;
    statement = skip_spaces(statement);

    for (i = 0; !isspace(statement[i]); i++)
        length++;

    if (!(str = (char *) malloc((length * sizeof(char)) + 1)))
        exit(allocate_error("get_data_type"));

    strncpy(str, statement, length);
    str[length] = '\0';

    if (!strcmp(str, ".data"))
        type = DATA;
    else if (!strcmp(str, ".string"))
        type = STRING;
    else if (!strcmp(str, ".entry"))
        type = ENTRY;
    else if (!strcmp(str, ".extern"))
        type = EXTERN;

    free(str);

    return type;
}


/* Delete memory and free all of its components */
void delete_memory(wptr head) {
    wptr temp;

    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}


/* Creates a new instruction word */
static wptr new_instruction_word(unsigned int *instruction_counter) {
    wptr new;

    if (!(new = (wptr) calloc(1, sizeof(Word))))
        exit(allocate_error("new_instruction_word"));

    new->address = (*instruction_counter)++;
    new->binary_code[LSB] = new->binary_code[LSB - 1] = 0;      /* set the encoding type to be A by default */
    new->ext = NULL;
    new->next = NULL;

    return new;
}


/* Creates a new data word */
static wptr new_data_word(int value, unsigned int *data_counter) {
    wptr new;

    if (!(new = (wptr) malloc(sizeof(Word))))
        exit(allocate_error("new_data_word"));


    create_binary_code(new->binary_code, value);
    new->address = (*data_counter)++;
    new->ext = NULL;
    new->next = NULL;

    return new;
}


/* Used by 'store_data' to store a numeric data */
static wptr store_num(char *statement, unsigned int *data_counter) {
    wptr new, head = NULL;
    int count = 0;

    statement += DATA_LENGTH;
    statement = skip_spaces(statement);

    if (!is_number(*statement))
        return NULL;

    while (*statement != '\0') {
        int temp = (int) strtol(statement, &statement, DECIMAL);
        if (temp == 0) {
            if (!check_zero(statement)) {
                *data_counter -= count;
                return NULL;
            }
        }
        temp = temp > LARGEST_POSSIBLE_NUMBER ? LARGEST_POSSIBLE_NUMBER : temp;
        new = new_data_word(temp, data_counter);
        head = add_word(head, new);
        count++;
        statement = skip_spaces(statement);

        if (*statement != ',' && *statement != '\0') {
            *data_counter -= count;
            return NULL;
        }
        if (*statement != '\0') {
            statement++;
            statement = skip_spaces(statement);
        }
    }

    while (isspace(*(--statement)));
    if (!isdigit(*statement))
        return NULL;

    return head;
}


/* Used by 'store_data' to store a string */
static wptr store_string(char *statement, unsigned int *data_counter) {
    wptr new, head = NULL;

    statement += STRING_LENGTH;
    statement = skip_spaces(statement);

    if (!is_string(statement))
        return NULL;
    statement++;

    while (*statement != '"') {
        new = new_data_word(*statement, data_counter);
        head = add_word(head, new);
        statement++;
    }
    new = new_data_word('\0', data_counter);
    head = add_word(head, new);

    return head;
}


/* Creates the first word of an instruction */
static wptr create_first_word(int *args, unsigned int *instruction_counter) {
    int i, j;
    short temp[WORD_LENGTH];
    wptr new = new_instruction_word(instruction_counter);

    j = LSB - ENCTYPE_LENGTH;   /* j starts at the end of the array and goes down */

    create_binary_code(temp, args[DST_ADDRESSING_TYPE]);
    for (i = 0; i < OPERAND_LENGTH; i++)
        new->binary_code[j--] = temp[LSB - i];

    create_binary_code(temp, args[OPCODE]);
    for (i = 0; i < OPCODE_LENGTH; i++)
        new->binary_code[j--] = temp[LSB - i];

    create_binary_code(temp, args[SRC_ADDRESSING_TYPE]);
    for (i = 0; i < OPERAND_LENGTH; i++)
        new->binary_code[j--] = temp[LSB - i];

    return new;
}


/* Creates the machine code for the operands of an instruction */
static wptr create_operands(char *src, char *dst, unsigned int *instruction_counter, sptr symbols_table) {
    wptr head = NULL;
    AddressingType src_type, dst_type;
    sptr src_symbol = NULL, dst_symbol = NULL;

    src_type = get_addressing_type(src);
    dst_type = get_addressing_type(dst);
    head = new_instruction_word(instruction_counter);

    src_symbol = search_symbol(symbols_table, src);
    dst_symbol = search_symbol(symbols_table, dst);


    if (src_type == IMMEDIATE) {
        int temp = (int) strtol(src, &src, DECIMAL);
        if (temp == 0 && *(src - 1) != '0')
            return NULL;
        create_immediate_operand(head->binary_code, temp);
    } else if (src_type == DIRECT) {
        if (src_symbol) {
            create_immediate_operand(head->binary_code, src_symbol->value);
            set_encoding_type(head, src_symbol);
            if (src_symbol->type == EXTERN_SYMBOL)
                head->ext = src_symbol->name;
        }

    } else if (src_type == REGISTER_DIRECT)
        create_binary_reg(head->binary_code, src, SRC);

    if (dst_type == DIRECT) {
        if (src_type) {
            head = add_word(head, new_instruction_word(instruction_counter));
            if (dst_symbol) {
                create_immediate_operand(head->next->binary_code, dst_symbol->value);
                set_encoding_type(head->next, dst_symbol);
                if (dst_symbol->type == EXTERN_SYMBOL)
                    head->ext = dst_symbol->name;
            }
        } else {
            if (dst_symbol) {
                create_immediate_operand(head->binary_code, dst_symbol->value);
                set_encoding_type(head, dst_symbol);
                if (dst_symbol->type == EXTERN_SYMBOL)
                    head->ext = dst_symbol->name;
            }
        }

    } else if (dst_type == REGISTER_DIRECT) {
        if (src_type == REGISTER_DIRECT)
            create_binary_reg(head->binary_code, dst, DST);
        else {
            head = add_word(head, new_instruction_word(instruction_counter));
            create_binary_reg(head->next->binary_code, dst, DST);
        }

    } else if (dst_type == IMMEDIATE) {
        if (!src_type)
            create_immediate_operand(head->binary_code, (int) strtol(dst, &dst, DECIMAL));
    }

    return head;
}


/* Creates the machine code for an immediate operand */
static void create_immediate_operand(short *tar, int val) {
    int i;

    create_binary_code(tar, val);

    for (i = 0; i < WORD_LENGTH - ENCTYPE_LENGTH; i++)
        tar[i] = tar[i + ENCTYPE_LENGTH];
    tar[LSB] = tar[LSB - 1] = 0;
}


/* Creates binary code for a given value */
static void create_binary_code(short *tar, int val) {
    int i, temp = 0;
    tar[0] = 0;

    if (val < 0) {
        temp = val;
        val = -val;
        tar[0] = 1;
    }

    for (i = WORD_LENGTH - 1; i > 0; i--) {
        tar[i] = (short) (val % BINARY);
        val /= BINARY;
    }

    if (temp < 0) {
        for (i = WORD_LENGTH - BINARY; i > 0; i--)
            tar[i] = tar[i] == 0 ? (short) 1 : (short) 0;
    }
}


/* Creates binary code for registers operands */
static void create_binary_reg(short *tar, char *reg, int pos) {
    int i, j, k, value;
    short temp[WORD_LENGTH];

    value = get_register_number(reg);
    create_binary_code(temp, value);

    k = LSB - ENCTYPE_LENGTH;
    if (pos == DST) {
        j = LSB;
        for (i = k; i >= (k - REGISTER_ENCODING_LENGTH + 1); i--)
            tar[i] = temp[j--];
    } else if (pos == SRC) {
        j = WORD_LENGTH - REGISTER_ENCODING_LENGTH;
        for (i = 0; i < REGISTER_ENCODING_LENGTH; i++)
            tar[i] = temp[j++];
    }
}


/* used by 'create_binary_reg' */
static int get_register_number(char *reg) {
    int i;
    char *registers[] = {"@r0", "@r1", "@r2", "@r3", "@r4", "@r5", "@r6", "@r7"};

    for (i = 0; i < REGISTERS; i++) {
        if (!strcmp(reg, registers[i]))
            return i;
    }

    return -1;
}


/* used by 'get_operand' */
static unsigned int get_operand_length(const char *str) {
    unsigned int length = 0;

    while (*str != ',' && !isspace(*str) && *str != '\0') {
        length++;
        str++;
    }

    return length;
}


/* used by 'get_operand' */
static char *get_operand_position(char *instruction) {
    char *temp;

    instruction = skip_spaces(instruction);
    if (is_label(instruction))
        instruction += get_label_length(instruction) + NEXT;
    instruction = skip_spaces(instruction);
    instruction += OPERATION_LENGTH;
    instruction = skip_spaces(instruction);
    temp = instruction;

    return temp;
}


/* used by 'store_num' */
static int check_zero(char *str) {
    if (is_number(*str)) {
        while (*str != ',' && *str != '\0' && *str != '\n') {
            if (*str != '0')
                return FALSE;
            str++;
        }
        if (*str == ',' && (*(str + 1) == '\n' || *(str + 1) == '\0'))
            return FALSE;
        return TRUE;
    }
    return FALSE;
}


/* Checks if a character could be a start of a number. */
static int is_number(char c) {
    return (c == '+' || c == '-' || isdigit(c));
}


/* Checks if a given string is a valid string statement. */
static int is_string(char *str) {
    char *temp;

    if (*str == '"') {
        temp = str;
        while (*str != '\0' && *str != '\n')
            str++;
        if ((*str == '\0' || *str == '\n') && *(str - 1) == '"')
            return ((str - temp) > 1);
    }
    return FALSE;
}


/* Set the encoding type (a,r,e bits) of a given word to the type of its symbol's type */
static void set_encoding_type(wptr word, sptr symbol) {
    if (symbol->type == EXTERN_SYMBOL) {        /* extern encoding type */
        word->binary_code[LSB] = 1;
        word->binary_code[LSB - 1] = 0;
    } else {                            /* relocatable encoding type */
        word->binary_code[LSB] = 0;
        word->binary_code[LSB - 1] = 1;

    }
}


/* Returns a pointer to the next non-space character */
static char *skip_spaces(char *address) {
    while (isspace(*address))
        address++;
    return address;
}


/* Memory allocation fail */
static int allocate_error(char *func) {
    fprintf(stderr, "*** ERROR: In function '%s' - failed to allocate memory. *** \n", func);
    return 1;
}




