#include <stdio.h>
#include <stdlib.h>
#include <pcq.h>

#include <readline/readline.h>
#include <readline/history.h>

#define PARSERS \
    X(Num, "number", "/-?[0-9]+/") \
    X(Oper, "operator", "'+' | '-' | '*' | '/'") \
    X(Expr, "expression", "<number> | '(' <operator> <expression>+ ')'") \
    X(Program, "program", "/^/ <operator> <expression>+ /$/")

#define X(id, name, def) + 1
static const int PARSER_COUNT = PARSERS;
#undef X

signed
main (void) {

    #define X(id, name, def) pcq_parser_t *id = pcq_new(name);
    PARSERS
    #undef X

    pcqa_lang(PCQA_LANG_DEFAULT,
    #define X(id, name, def) name " : " def " ; "
        PARSERS
    #undef X
    #define X(id, name, def) , id
        PARSERS
    #undef X
    );

    char * input;
    pcq_result_t r;
    while ( (input = readline("> ")) ) {
        add_history(input);

        if (pcq_parse("<stdin>", input, Program, &r)) {
            pcq_ast_print(r.output);
            pcq_ast_delete(r.output);
        } else {
            pcq_err_print(r.error);
            pcq_err_delete(r.error);
        }

        free(input);
    }

    #define X(id, name, def) , id
    pcq_cleanup(PARSER_COUNT
        PARSERS
    );
    #undef X

    return EXIT_SUCCESS;
}
