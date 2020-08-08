#include "main.h"

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
        FOR_EACH_PARSER
    #undef X
    #ifndef NDEBUG
        , NULL
    #endif
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
            println_value(eval(r.output));
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

struct lvalue
eval (pcq_ast_t * ast) {

    if ( strstr(ast->tag, "number") ) {
        if ( strstr(ast->tag, "integer") ) {
            errno = 0;
            long x = 0;
            sscanf(ast->contents, "%ld", &x);
            errsv = errno;

            if ( errsv ) {
                return LERROR(Invalid_Number);
            }

            return LINT(x);
        } else if ( strstr(ast->tag, "float") ) {
            errno = 0;
            double x = 0;
            sscanf(ast->contents, "%lf", &x);
            errsv = errno;

            if ( errsv ) {
                return LERROR(Invalid_Number);
            }

            return LFLOAT(x);
        }
    }

    char * op = ast->children[1]->contents;
    struct lvalue x = eval(ast->children[2]);

    for ( size_t i = 3; strstr(ast->children[i]->tag, "expr"); ++ i ) {
        x = eval_op(x, op, eval(ast->children[i]));
    }

    return x;
}

struct lvalue
eval_op (struct lvalue x, char * op, struct lvalue y) {

    if ( x.type == Error ) { return x; }
    if ( y.type == Error ) { return y; }

    if ( x.type == Floating || y.type == Floating ) {
        switch ( *op ) {
            case '+': return LFLOAT((x.type == Floating ? x.floating : x.integer) + (y.type == Floating ? y.floating : y.integer));
            case '-': return LFLOAT((x.type == Floating ? x.floating : x.integer) - (y.type == Floating ? y.floating : y.integer));
            case '*': return LFLOAT((x.type == Floating ? x.floating : x.integer) * (y.type == Floating ? y.floating : y.integer));
            case '%': return LERROR(Invalid_Operator);
            case '/':
                return !y.floating ? LERROR(Division_By_Zero)
                                 : LFLOAT((x.type == Floating ? x.floating : x.integer) / (y.type == Floating ? y.floating : y.integer));
            default:  return LERROR(Invalid_Operator);
        }
    }

    switch ( *op ) {
        case '+': return LINT(x.integer + y.integer);
        case '-': return LINT(x.integer - y.integer);
        case '*': return LINT(x.integer * y.integer);
        case '%': return LINT(x.integer % y.integer);
        case '/':
            return !y.integer ? LERROR(Division_By_Zero)
                             : LINT(x.integer / y.integer);
        default:  return LERROR(Invalid_Operator);
    }
}

void
print_value (struct lvalue val) {

    switch ( val.type ) {
        case Integer:
            printf("%ld", val.integer);
            break;

        case Floating:
            printf("%g", val.floating);
            break;

        case Error:
            printf("(E%d): %s", val.error, errors[val.error]);
            break;
    }
}

void
println_value (struct lvalue val) {

    print_value(val);
    putchar('\n');
}

