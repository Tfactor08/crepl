#include "crepl.h"

TCCState *tcc = NULL;

void handle_error(void *opaque, const char *msg)
{
    fprintf(opaque, "%s\n", msg);
}

void init_tcc()
{
    tcc = tcc_new();
    if (!tcc) {
        fprintf(stderr, "Could not create tcc state\n");
        exit(1);
    }
    tcc_set_error_func(tcc, stderr, handle_error);
    tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
}

int compile_program_and_run(char program[])
{
    int (*inner_main)();
    init_tcc(&tcc);
    if (tcc_compile_string(tcc, program) == -1)
        return -1;
    if (tcc_relocate(tcc) < 0)
        return -1;
    inner_main = tcc_get_symbol(tcc, "main");
    if (!inner_main)
        return -1;
    inner_main();
    return 1;
}
