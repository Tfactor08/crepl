#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libtcc.h"

#define OUT

#define false 0
#define true  1

int is_symbol_requested(char*);
int is_symbol_being_created(char*);
int get_symbol_type(char*, OUT char**);
int derive_type(char*, OUT char[]);
void derive_name(char*, char*, OUT char*);

typedef struct {
    char name[100];
    char type[50];
} Symbol;

char *symbol_types[] = {"int", "char*"}; // pointer types must be declared exactly like this
size_t symbols_cnt = 0;
char program[300];
Symbol symbols[100];

char skel[] =
"#include <tcclib.h>\n"
"\n"
"int main()\n"
"{\n"
"    %s\n"     
"\n"
"    return 0;\n"
"}";

void handle_error(void *opaque, const char *msg)
{
    fprintf(opaque, "%s\n", msg);
}

void prompt()
{
    printf(">>> ");
}

void init_tcc(TCCState **tcc)
{
    *tcc = tcc_new(); // make the provided pointer point to a new tcc object
    if (!*tcc) {
        fprintf(stderr, "Could not create tcc state\n");
        exit(1);
    }
    tcc_set_error_func(*tcc, stderr, handle_error);
    tcc_set_output_type(*tcc, TCC_OUTPUT_MEMORY);
}


// TODO: make this function free from the new symbols creation process
// symbol -- add new symbol to the top of the program, or leave empty;
// stmt   -- statement to be executed.
int compile_program(TCCState *tcc, char *symbol, char *stmt)
{
    if (strcmp(symbol, "")) {
        char tmp[200];
        strcat(symbol, "%s"); // for future symbols
        sprintf(tmp, skel, symbol, "%s");
        strcpy(skel, tmp);
    }

    // maybe "%s" istead of ""?
    sprintf(program, skel, "", stmt);

    if (tcc_compile_string(tcc, program) == -1)
        return 1;
    if (tcc_relocate(tcc) < 0)
        return 1;
    return 0;
}

int main(int argc, char **argv)
{
    TCCState *tcc;
    char input[100];
    int (*inner_main)();

    init_tcc(&tcc);
    compile_program(tcc, "", "");

    while (1) {
        prompt();
        // TODO: factor out
        fgets(input, sizeof(input), stdin);
        if (strcmp(input, "bye\n") == 0)
            break;

        // TODO: refactor so that temporarly printf is embedded to the main func to print the requested variable
        if (is_symbol_requested(input)) {
            input[strlen(input)-1] = '\0'; // exclude newln char
                                           
            // TODO: might there be a better way to do this?
            char *type;
            if (get_symbol_type(input, &type) == -1) {
                printf("ERROR: %s not found\n", input);
                continue;
            }
            if (strcmp(type, "char*") == 0) {
                char **var = tcc_get_symbol(tcc, input);
                printf("%s\n", *var);
            }
            else if (strcmp(type, "int") == 0) {
                int *var = tcc_get_symbol(tcc, input);
                printf("%d\n", *var);
            }
            continue;
        }

        // TODO: refactor so that new variable is stored in the main function
        if (is_symbol_being_created(input)) {
            char type[20];
            char name[50];

            derive_type(input, type);
            derive_name(input, type, name);

            register_symbol(name, type);

            init_tcc(&tcc);
            compile_program(tcc, input, "");

            continue;
        }

        init_tcc(&tcc);
        compile_program(tcc, "", input);

        inner_main = tcc_get_symbol(tcc, "main");
        if (!inner_main)
            return 1;

        inner_main();
    }

    tcc_delete(tcc);

    return 0;
}

/* add new symbol to the global 'symbols' array */
void register_symbol(char name[], char type[])
{
    symbols[symbols_cnt] = (Symbol){0};

    strncpy(symbols[symbols_cnt].name, name, strlen(name));
    symbols[symbols_cnt].name[strlen(name)] = '\0';

    strncpy(symbols[symbols_cnt].type, type, strlen(type));
    symbols[symbols_cnt].type[strlen(type)] = '\0';

    symbols_cnt++;
}

/* return variable's type saved in the 'symbols' array by the provided name */
int get_symbol_type(char *name, OUT char **type)
{
    // TODO
    // we definitely need some sort of a map here
    for (int i = 0; i < symbols_cnt; i++) {
        if (strcmp(symbols[i].name, name) == 0) {
            *type = symbols[i].type;
            return 1;
        }
    }
    return -1;
}

/* request for a symbol is defined as a statement without a terminating semicolon */
int is_symbol_requested(char *stmt)
{
    if (stmt[strlen(stmt)-2] != ';')
        return true;
    return false;
}

/* request for a symbol creation is defined as a statement containing the "=" token */
int is_symbol_being_created(char *stmt)
{
    if (strchr(stmt, '=') != NULL)
        return true;
    return false;
}

/* derive type from the statement containing varialbe definition;
   return 1 if type found, otherwise -1 */
int derive_type(char *stmt, OUT char type[])
{
    int types_cnt = sizeof(symbol_types) / sizeof(symbol_types[0]);

    // TODO
    // obviously, we need some sort of a map here, but C doesn't provide one
    for (int i = 0; i < types_cnt; i++) {
        char *t = symbol_types[i];
        if (strstr(stmt, t) != NULL) {
            strncpy(type, t, strlen(t));
            type[strlen(t)] = '\0';
            return 1;
        }
    }
    return -1;
}

/* derive name from the statement containing varialbe definition;
   variable's type must be provided */
void derive_name(char *stmt, char *type, OUT char *name)
{
    int start, end, i, j;

    start = strstr(stmt, type)-stmt + strlen(type)+1;
    for (end = start; stmt[end+1] != ' '; end++)
        ;

    for (i=start, j=0; i <= end; i++, j++)
        name[j] = stmt[i];
    name[j] = '\0';
}
