/* This file is implementing the symbol table.
 * All of the functions in this file are used
 * to create and manage the table.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "symbols.h"


enum {
    FALSE, TRUE
};

static int allocate_error(char *func);

static char *skip_spaces(char *address);


/* Checks if there's a label for a specific instruction */
int is_label(char *instruction) {
    int i, length;

    instruction = skip_spaces(instruction);

    for (i = 0, length = 0; instruction[i] != ':' && instruction[i] != '\0'; i++) {
        if (!isalnum(instruction[i]) || isspace(instruction[i]))
            return FALSE;
        length++;
    }
    if (instruction[i] == ':' && length <= MAX_LABEL_LENGTH + 1) {
        return TRUE;
    }

    return FALSE;
}


/* Returns the name of the label as a string. Assuming the instruction has a valid label. */
char *get_label_name(char *instruction) {
    unsigned int i;
    char *label;

    instruction = skip_spaces(instruction);

    for (i = 0; instruction[i] != ':'; i++);

    if (!(label = (char *) malloc((i * sizeof(char)) + 1)))
        exit(allocate_error("get_label_name"));

    strncpy(label, instruction, i);
    label[i] = '\0';

    return label;
}


/* Returns the length of the label. Assuming there is a valid label. */
int get_label_length(const char *instruction) {
    int i, length;

    for (i = 0, length = 0; instruction[i] != ':'; i++, length++);

    return length;
}


/* Create a new symbol for the symbol table */
sptr new_symbol(char *name, unsigned int value, SymbolType type) {
    sptr new;
    if (!name || is_keyword(name)) {
        if (is_keyword(name))
            printf("Failed to create symbol - label name is a keyword.\n");
        return NULL;
    }

    if (!(new = (sptr) malloc(sizeof(Symbol))))
        exit(allocate_error("new_symbol"));

    new->name = name;
    new->value = value;
    new->type = type;
    new->next = NULL;
    new->prev = NULL;

    return new;
}


/* Adds a new symbol to the symbol table. Returns 1 on success, else returns 0. */
sptr add_symbol(sptr head, sptr new) {
    sptr current;

    if (!new)
        return NULL;

    if (!head)
        return new;

    if (search_symbol(head, new->name)) {
        free(new);
        printf("Failed to add symbol - label name already exists.\n");
        delete_symbols_table(head);
        return NULL;
    }

    for (current = head; current->next; current = current->next);

    current->next = new;
    new->prev = current;

    return head;
}


/* Delete all the symbols from the table */
void delete_symbols_table(sptr head) {
    sptr temp;

    if (head) {
        for (temp = head->next; temp; temp = temp->next) {
            free(head->name);
            free(head);
            head = temp;
        }
        free(head->name);
        free(head);
    }
}


/* Update all data symbols by adding the instruction counter to their values. */
void update_symbols(sptr head, const unsigned int *instruction_counter) {
    while (head) {
        if (head->type == DATA_SYMBOL)
            head->value += *instruction_counter;
        head = head->next;
    }
}


/* Searching for a symbol with a specific name in the symbols table. */
sptr search_symbol(sptr head, char *name) {
    if (name) {
        while (head) {
            if (!strcmp(name, head->name))
                return head;
            head = head->next;
        }
    }

    return NULL;
}


/* Set the symbol type of a given symbol to 'entry'. */
void set_entry(sptr head, char *name) {
    if (name) {
        sptr temp = search_symbol(head, name);
        if (temp)
            temp->type = ENTRY_SYMBOL;
    }
}


/* Checks is a given string is an assembly keyword. */
int is_keyword(char *str) {
    int i;
    char *keywords[] = {"data", "string", "entry", "extern", "mov", "cmp", "add", "sub", "not", "clr", "lea", "inc",
                        "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", "r7", "r6", "r5", "r4", "r3", "r2",
                        "r1", "r0"};
    for (i = 0; i < TOTAL_KEYWORDS; i++) {
        if (!strcmp(str, keywords[i]))
            return TRUE;
    }
    return FALSE;
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

