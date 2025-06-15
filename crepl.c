#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libtcc.h"

int is_request_for_symbol(char*);

void handle_error(void *opaque, const char *msg)
{
    fprintf(opaque, "%s\n", msg);
}

void prompt()
{
    printf(">>> ");
}

const char hello[] = "Hello World!";

char skel[] =
"#include <tcclib.h>\n"
"\n"
"char msg[] = \"ur mom\";\n"
"\n"
"int main()\n"
"{\n"
"    %s\n"     
""
"    return 0;\n"
"}";

int main(int argc, char **argv)
{
    TCCState *tcc;
    char input[100];
    int (*inner_main)();
    char curr_prgrm[300];
    int init = 0;

    while (1) {
        prompt();
        fgets(input, sizeof(input), stdin);
        if (strcmp(input, "bye\n") == 0)
            break;

        // check if user asks for an existing symbol
        // let's define input without a terminating semicolon as a request for a symbol
        if (is_request_for_symbol(input)) {
            // TODO
            //if (!initialized)
            //  initialize

            // exclude newln char
            input[strlen(input)-1] = '\0';
            // TODO
            // here, we assume the symbol is of type char*, but that is not always the case
            char *var = tcc_get_symbol(tcc, input);
            printf("%s\n", var);
            continue;
        }

        tcc = tcc_new();
        if (!tcc) {
            fprintf(stderr, "Could not create tcc state\n");
            exit(1);
        }
        tcc_set_error_func(tcc, stderr, handle_error);
        tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
        
        sprintf(curr_prgrm, skel, input);

        //printf(curr_prgrm);

        if (tcc_compile_string(tcc, curr_prgrm) == -1)
            return 1;

        init = 1;

        if (tcc_relocate(tcc) < 0)
            return 1;

        inner_main = tcc_get_symbol(tcc, "main");
        if (!inner_main)
            return 1;

        inner_main();
    }

    tcc_delete(tcc);

    return 0;
}

int is_request_for_symbol(char *input)
{
    if (input[strlen(input)-2] != ';')
        return 1;
    return 0;
}
