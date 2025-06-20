#include <stdio.h>

int main()
{
    char *f = "%s %s\n";
    char *s;
    sprintf(s, f, "ur");

    printf(s);
}
