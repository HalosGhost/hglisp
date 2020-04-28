#include <stdio.h>
#include <stdlib.h>
#include <pcq.h>

#include <readline/readline.h>
#include <readline/history.h>

#define FOR_EACH_PARSER \
    X(Num, "number", "/-?[0-9]+/") \
    X(Oper, "operator", "'+' | '-' | '*' | '/'") \
    X(Expr, "expression", "<number> | '(' <operator> <expression>+ ')'") \
    X(Program, "program", "/^/ <operator> <expression>+ /$/")

#define X(id, name, def) + 1
static const int PARSER_COUNT = FOR_EACH_PARSER;
#undef X

long
eval (pcq_ast_t *);

long
eval_op (long, char *, long);

signed
main (void) {

    #define X(id, name, def) pcq_parser_t *id = pcq_new(name);
    FOR_EACH_PARSER
    #undef X

    pcq_err_t * failed_to_parse = pcqa_lang(PCQA_LANG_DEFAULT,
    #define X(id, name, def) name ": " def ";\n"
        FOR_EACH_PARSER
    #undef X
    #define X(id, name, def) , id
        FOR_EACH_PARSER,
    #undef X
        NULL
    );

    if ( failed_to_parse ) {
        #define X(id, name, def) name ": " def ";\n"
        printf("%s\n",
            FOR_EACH_PARSER
        );
        #undef X

        pcq_err_print(failed_to_parse);
        return EXIT_FAILURE;
    }

    char * input;
    pcq_result_t r;
    while ( (input = readline("> ")) ) {
        add_history(input);

        if ( pcq_parse("<stdin>", input, Program, &r) ) {
            printf("%ld\n", eval(r.output));
            pcq_ast_delete(r.output);
        } else {
            pcq_err_print(r.error);
            pcq_err_delete(r.error);
        }

        free(input);
    }

    #define X(id, name, def) , id
    pcq_cleanup(PARSER_COUNT
        FOR_EACH_PARSER
    );
    #undef X

    return EXIT_SUCCESS;
}

long
eval (pcq_ast_t * ast) {

    if ( strstr(ast->tag, "number") ) {
        return strtol(ast->contents, NULL, 10);
    }

    char * op = ast->children[1]->contents;
    long x = eval(ast->children[2]);

    for ( size_t i = 3; strstr(ast->children[i]->tag, "expr"); ++ i ) {
        x = eval_op(x, op, eval(ast->children[i]));
    }

    return x;
}

long
eval_op (long x, char * op, long y) {

    switch ( *op ) {
        case '+': return x + y;
        case '-': return x - y;
        case '*': return x * y;
        case '/': return x / y;
        default:  return 0;
    }
}

