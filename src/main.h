#ifndef MAIN_H
#define MAIN_H

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pcq.h>

#include <readline/readline.h>
#include <readline/history.h>

#define FOR_EACH_PARSER \
    X(Int, "integer", "/-?[0-9]+/") \
    X(Flt, "float", "/-?[0-9]+\\.[0-9]+/") \
    X(Num, "number", "<float> | <integer>") \
    X(Oper, "operator", "'+' | '-' | '*' | '/' | '%'") \
    X(Expr, "expression", "<number> | '(' <operator> <expression>+ ')'") \
    X(Program, "program", "/^/ <operator> <expression>+ /$/")

#define X(id, name, def) + 1
static const int PARSER_COUNT = FOR_EACH_PARSER;
#undef X

static signed errsv;

enum lvtype {
    Integer,
    Floating,
    Error
};

#define FOR_EACH_ERROR_TYPE \
    X(Division_By_Zero, "Division By Zero") \
    X(Invalid_Operator, "Invalid Operator") \
    X(Invalid_Number, "Invalid Number")

#define X(c, s) c,
enum letype {
    FOR_EACH_ERROR_TYPE
};
#undef X

#define X(c, s) [c] = s,
static const char * errors[] = {
    FOR_EACH_ERROR_TYPE
};
#undef X

struct lvalue {
    enum lvtype type;
    union {
        long integer;
        double floating;
        enum letype error;
    };
};

#define LERROR(num) \
    ((struct lvalue){ .type = Error, .error = (num) })

#define LINT(num) \
    ((struct lvalue){ .type = Integer, .integer = (num) })

#define LFLOAT(num) \
    ((struct lvalue){ .type = Floating, .floating = (num) })

struct lvalue
eval (pcq_ast_t *);

struct lvalue
eval_op (struct lvalue, char *, struct lvalue);

void
print_value (struct lvalue);

void
println_value (struct lvalue);

#endif

