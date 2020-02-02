#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>

signed
main (void) {

    for ( ; ; ) {
        char * input = readline("> ");

        add_history(input);

        puts(input);

        free(input);
    }

    return EXIT_SUCCESS;
}
