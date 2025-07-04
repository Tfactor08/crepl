#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libtcc.h"

#define false 0
#define true  1

int is_symbol_requested(char*);
int is_symbol_being_created(char*);

char skel[] =
"#include <tcclib.h>\n"
"\n"
"%s"
"\n"
"int main()\n"
"{\n"
"    %s\n"     
""
"    return 0;\n"
"}";
char program[300];

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

int compile_program(TCCState *tcc, char *symbols, char *stmt)
{
    // TODO
    // not sure how to properly check for an empty string
    if (strcmp(symbols, "") != 0) {
        char tmp[200];
        strcat(symbols, "%s"); // for future symbols
        sprintf(tmp, skel, symbols, "%s");
        strcpy(skel, tmp);

        puts(skel);
    }

    sprintf(program, skel, "", stmt);

    if (tcc_compile_string(tcc, program) == -1)
        return 1;
    if (tcc_relocate(tcc) < 0)
        return 1;
    return 0;
}

// TODO
// started implemntation of new symbols definition feature:
// for now, only string symbols can be handled properly.
// also, error containing code can be included into 'skel',
// so that also needs to be handled
int main(int argc, char **argv)
{
    TCCState *tcc;
    char input[100];
    int (*inner_main)();

    init_tcc(&tcc);
    compile_program(tcc, "", "");

    while (1) {
        prompt();
        // TODO
        // factor out
        fgets(input, sizeof(input), stdin);
        if (strcmp(input, "bye\n") == 0)
            break;

        // check if user asks for an existing symbol
        if (is_symbol_requested(input)) {
            // exclude newln char
            input[strlen(input)-1] = '\0';
            // TODO
            // here, we assume the symbol is of type char*, but that is not always the case
            // (the symbol may even be a function)
            char *var = tcc_get_symbol(tcc, input);
            if (!var) {
                printf("ERROR: %s not found\n", input);
                continue;
            }
            printf("%s\n", var);
            continue;
        }

        else if (is_symbol_being_created(input)) {
            init_tcc(&tcc);
            compile_program(tcc, input, "");

            puts(program);

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

// define input without a terminating semicolon as a request for a symbol
int is_symbol_requested(char *input)
{
    if (input[strlen(input)-2] != ';')
        return true;
    return false;
}

// for now, define an input containing the "=" token as a symbol creation request
int is_symbol_being_created(char *input)
{
    if (strchr(input, '=') != NULL)
        return true;
    return false;
}
