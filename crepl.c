#include "crepl.h"

extern TCCState *tcc;

// order matters! (unfortunately)
char *symbol_types[] = {"int", "char*", "char"}; // pointer types must be declared exactly like here
size_t symbols_cnt = 0;
Symbol symbols[100];

char program[1024] =
"#include <tcclib.h>\n"
"\n"
"int main()\n"
"{\n"
"\t%s"
"\n"
"\treturn 0;\n"
"}\n";

int main(int argc, char **argv)
{
    char input[100];

    while (1) {
        if (prompt(input, sizeof(input)) == -1)
            break;

        if (is_symbol_requested(input)) {
            input[strlen(input)-1] = '\0'; // exclude newln char

            char temp_program[1024];
            char *var_name = input;
            if (embed_print_stmt(var_name, sizeof(temp_program), temp_program) == -1)
                continue;

            compile_program_and_run(temp_program);
            continue;
        }

        else if (is_symbol_being_created(input)) {
            char type[20];
            char name[50];

            derive_type(input, type);
            derive_name(input, type, name);

            register_symbol(name, type);
            add_var_to_program(input);

            continue;
        }

        // not a symbol creation or reqest for a symbol
        else {
            char temp_program[1024];
            snprintf(temp_program, sizeof(temp_program), program, input);
            compile_program_and_run(temp_program);
        }
    }

    if (tcc)
        tcc_delete(tcc);

    return 0;
}

/* add variable definition to the program's main function so that it can be referred later */
void add_var_to_program(char stmt[])
{
    // append "%s" to the stmt for future variables
    char modified_str[strlen(stmt) + strlen("\t%s")+1];
    strcpy(modified_str, stmt);
    strcat(modified_str, "\t%s");

    char temp[1024];
    snprintf(temp, sizeof(temp), program, modified_str);
    strcpy(program, temp);
}

/* embed printf to the temporarly program to print the existing variable called 'name' */
int embed_print_stmt(char name[], int temp_size, OUT char temp[])
{
    char *type;
    if (get_symbol_type(name, &type) == -1) {
        printf("ERROR: %s not found\n", name);
        return -1;
    }

    char printf_stmt[256];
    char *specifier;

    if (strcmp(type, "int") == 0)
        specifier = "%d";
    else if (strcmp(type, "char*") == 0)
        specifier = "%s";
    else if (strcmp(type, "char") == 0)
        specifier = "%c";

    snprintf(printf_stmt, sizeof(printf_stmt), "printf(\"%s\\n\", %s);\n", specifier, name);
    snprintf(temp, temp_size, program, printf_stmt);

    return 1;
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

// TODO: add EOF as exit indicator
int prompt(char input[], int size)
{
    printf(">>> ");
    fgets(input, size, stdin);
    if (strcmp(input, "bye\n") == 0)
        return -1;
    return 1;
}
