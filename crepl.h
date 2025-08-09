#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libtcc.h"

#ifndef CREPL_H
#define CREPL_H

#define OUT

#define false 0
#define true  1

void derive_name(char*, char*, OUT char*);
void add_var_to_program(char[]);
void register_symbol(char name[], char type[]);
int  prompt(char[], int);
int  is_symbol_requested(char*);
int  is_symbol_being_created(char*);
int  get_symbol_type(char*, OUT char**);
int  derive_type(char*, OUT char[]);
int  embed_print_stmt(char[], int, char[]);
int  compile_program_and_run(char[]);

typedef struct {
    char name[100];
    char type[50];
} Symbol;

#endif
