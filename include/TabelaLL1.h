#ifndef TABELA_LL1_H
#define TABELA_LL1_H

#include "Token.h"

/*
 * Tabela de analise preditiva LL(1) da linguagem C- (Louden), calculada sobre a
 * gramatica reescrita (sem recursao a esquerda e fatorada).
 *
 *   MATRIZ_LL1[nao_terminal][coluna] = numero da producao a aplicar (1..NUM_PRODUCOES)
 *                                      ou, se for >= ERRO_BASE, o codigo de erro E(valor - ERRO_BASE)
 *                                      (a mensagem esta em MENSAGEM_ERRO[codigo])
 *
 * Os numeros das producoes e os codigos de erro sao os mesmos da planilha tabela_ll1_cminus.xlsx.
 *
 * Erros possiveis na analise (ver MENSAGEM_ERRO):
 *   E1..E37   celula de erro da matriz: o token atual nao pertence ao SELECT de nenhuma
 *             producao do nao-terminal que esta no topo da pilha (um codigo por tipo de situacao);
 *   E38      terminal no topo da pilha diferente do token atual (ERRO_TERMINAL);
 *   E39      erro lexico: sequencia invalida ou comentario nao terminado (ERRO_LEXICO).
 * As colunas seguem a ordem de NOME_TERMINAL; colunaLL1() converte um TipoDeToken
 * na coluna correspondente.
 *
 * Conflito LL(1) unico: (selection-stmt', else). As producoes 36 (else statement) e 37
 * (vazio) tem "else" em seu SELECT (dangling else). A celula guarda a 36, o que liga
 * o else ao if mais proximo; a 37 ocupa as demais celulas de FOLLOW(selection-stmt').
 *
 * A regra simple-expression e inalcancavel a partir de program (expression ja inlinou
 * o corpo dela); a linha foi mantida por completude.
 */

enum Simbolo {
    // ---- nao-terminais: valor = indice da linha em MATRIZ_LL1 ----
    NT_PROGRAM                   =  0,   // "program"
    NT_DECLARATION_LIST          =  1,   // "declaration-list"
    NT_DECLARATION_LIST_P        =  2,   // "declaration-list'"
    NT_DECLARATION               =  3,   // "declaration"
    NT_DECLARATION_P             =  4,   // "declaration'"
    NT_VAR_DECLARATION           =  5,   // "var-declaration"
    NT_VAR_DECLARATION_P         =  6,   // "var-declaration'"
    NT_TYPE_SPECIFIER            =  7,   // "type-specifier"
    NT_FUN_DECLARATION_P         =  8,   // "fun-declaration'"
    NT_PARAMS                    =  9,   // "params"
    NT_PARAMS_P                  = 10,   // "params'"
    NT_PARAM_LIST_P              = 11,   // "param-list'"
    NT_PARAM                     = 12,   // "param"
    NT_PARAM_P                   = 13,   // "param'"
    NT_COMPOUND_STMT             = 14,   // "compound-stmt"
    NT_LOCAL_DECLARATIONS        = 15,   // "local-declarations"
    NT_STATEMENT_LIST            = 16,   // "statement-list"
    NT_STATEMENT                 = 17,   // "statement"
    NT_EXPRESSION_STMT           = 18,   // "expression-stmt"
    NT_SELECTION_STMT            = 19,   // "selection-stmt"
    NT_SELECTION_STMT_P          = 20,   // "selection-stmt'"
    NT_ITERATION_STMT            = 21,   // "iteration-stmt"
    NT_RETURN_STMT               = 22,   // "return-stmt"
    NT_RETURN_STMT_P             = 23,   // "return-stmt'"
    NT_EXPRESSION                = 24,   // "expression"
    NT_ID_EXPR                   = 25,   // "id-expr"
    NT_ID_EXPR_P                 = 26,   // "id-expr'"
    NT_VAR_P                     = 27,   // "var'"
    NT_SIMPLE_EXPRESSION         = 28,   // "simple-expression"
    NT_SIMPLE_EXPRESSION_P       = 29,   // "simple-expression'"
    NT_RELOP                     = 30,   // "relop"
    NT_ADDITIVE_EXPRESSION       = 31,   // "additive-expression"
    NT_ADDITIVE_EXPRESSION_P     = 32,   // "additive-expression'"
    NT_ADDOP                     = 33,   // "addop"
    NT_TERM                      = 34,   // "term"
    NT_TERM_P                    = 35,   // "term'"
    NT_MULOP                     = 36,   // "mulop"
    NT_FACTOR                    = 37,   // "factor"
    NT_FACTOR_P                  = 38,   // "factor'"
    NT_ARGS                      = 39,   // "args"
    NT_ARG_LIST                  = 40,   // "arg-list"
    NT_ARG_LIST_P                = 41,   // "arg-list'"
    NUM_NAO_TERMINAIS            = 42,

    // ---- terminais: valor = NUM_NAO_TERMINAIS + indice da coluna em MATRIZ_LL1 ----
    T_IDENTIFIER                 = 42,   // coluna  0: "ID"
    T_NUMBER                     = 43,   // coluna  1: "NUM"
    T_INT                        = 44,   // coluna  2: "int"
    T_VOID                       = 45,   // coluna  3: "void"
    T_IF                         = 46,   // coluna  4: "if"
    T_ELSE                       = 47,   // coluna  5: "else"
    T_WHILE                      = 48,   // coluna  6: "while"
    T_RETURN                     = 49,   // coluna  7: "return"
    T_PAREN_OPEN                 = 50,   // coluna  8: "("
    T_PAREN_CLOSE                = 51,   // coluna  9: ")"
    T_BRACKET_OPEN               = 52,   // coluna 10: "["
    T_BRACKET_CLOSE              = 53,   // coluna 11: "]"
    T_BRACE_OPEN                 = 54,   // coluna 12: "{"
    T_BRACE_CLOSE                = 55,   // coluna 13: "}"
    T_SEMICOLON                  = 56,   // coluna 14: ";"
    T_COMMA                      = 57,   // coluna 15: ","
    T_ASSIGN                     = 58,   // coluna 16: "="
    T_EQUAL_EQUAL                = 59,   // coluna 17: "=="
    T_NOT_EQUAL                  = 60,   // coluna 18: "!="
    T_LESS                       = 61,   // coluna 19: "<"
    T_LESS_EQUAL                 = 62,   // coluna 20: "<="
    T_GREATER                    = 63,   // coluna 21: ">"
    T_GREATER_EQUAL              = 64,   // coluna 22: ">="
    T_PLUS                       = 65,   // coluna 23: "+"
    T_MINUS                      = 66,   // coluna 24: "-"
    T_MULTIPLY                   = 67,   // coluna 25: "*"
    T_DIVIDE                     = 68,   // coluna 26: "/"
    T_EOF                        = 69,   // coluna 27: "$"
    NUM_SIMBOLOS                 = 70
};

const int NUM_COLUNAS_LL1 = 28;
const unsigned int ERRO_BASE = 100;      // celula >= ERRO_BASE e erro; codigo = celula - ERRO_BASE
const int NUM_ERROS = 39;
const int ERRO_TERMINAL = 38;         // terminal do topo da pilha nao casa com o token
const int ERRO_LEXICO = 39;           // token invalido devolvido pelo scanner

inline bool ehErro(unsigned int celula) { return celula >= ERRO_BASE; }
inline int codigoDoErro(unsigned int celula) { return static_cast<int>(celula - ERRO_BASE); }

// Indice = codigo do erro (o 0 nao e usado).
static const char* const MENSAGEM_ERRO[NUM_ERROS + 1] = {
    "",
    /* E1  */ "esperado declaracao (tipo 'int' ou 'void')",
    /* E2  */ "esperado nova declaracao ('int' ou 'void') ou fim de arquivo",
    /* E3  */ "esperado ';', '[' ou '(' depois do nome declarado",
    /* E4  */ "esperado declaracao de variavel (tipo 'int' ou 'void')",
    /* E5  */ "esperado ';' ou '[' depois do nome da variavel",
    /* E6  */ "esperado tipo ('int' ou 'void')",
    /* E7  */ "esperado '(' depois do nome da funcao",
    /* E8  */ "esperado parametros ('int' ou 'void')",
    /* E9  */ "esperado nome do parametro ou ')'",
    /* E10 */ "esperado ',' ou ')' na lista de parametros",
    /* E11 */ "esperado parametro (tipo 'int' ou 'void')",
    /* E12 */ "esperado '[', ',' ou ')' depois do nome do parametro",
    /* E13 */ "esperado '{' para abrir o bloco",
    /* E14 */ "esperado declaracao local, comando ou '}'",
    /* E15 */ "esperado comando ou '}'",
    /* E16 */ "esperado comando",
    /* E17 */ "esperado expressao ou ';'",
    /* E18 */ "esperado 'if'",
    /* E19 */ "esperado 'else', comando ou '}'",
    /* E20 */ "esperado 'while'",
    /* E21 */ "esperado 'return'",
    /* E22 */ "esperado expressao ou ';' depois de 'return'",
    /* E23 */ "esperado expressao (ID, NUM ou '(')",
    /* E24 */ "esperado '(', '[', '=', operador, ';', ')', ']' ou ',' depois do identificador",
    /* E25 */ "esperado '=', operador, ';', ')', ']' ou ','",
    /* E26 */ "esperado '[', '=', operador, ';', ')', ']' ou ','",
    /* E27 */ "esperado operador relacional, ';', ')', ']' ou ','",
    /* E28 */ "esperado operador relacional (<, <=, >, >=, == ou !=)",
    /* E29 */ "esperado '+', '-', operador relacional, ';', ')', ']' ou ','",
    /* E30 */ "esperado '+' ou '-'",
    /* E31 */ "esperado '*', '/', '+', '-', operador relacional, ';', ')', ']' ou ','",
    /* E32 */ "esperado '*' ou '/'",
    /* E33 */ "esperado fator (ID, NUM ou '(')",
    /* E34 */ "esperado '[', '(', operador, ';', ')', ']' ou ','",
    /* E35 */ "esperado argumento ou ')'",
    /* E36 */ "esperado argumento (expressao)",
    /* E37 */ "esperado ',' ou ')' na lista de argumentos",
    /* E38 */ "esperado o terminal que esta no topo da pilha (nao casa com o token atual)",
    /* E39 */ "sequencia de caracteres invalida ou comentario nao terminado"
};

static const char* const NOME_NAO_TERMINAL[NUM_NAO_TERMINAIS] = {
    "program", "declaration-list", "declaration-list'", "declaration",
    "declaration'", "var-declaration", "var-declaration'", "type-specifier",
    "fun-declaration'", "params", "params'", "param-list'",
    "param", "param'", "compound-stmt", "local-declarations",
    "statement-list", "statement", "expression-stmt", "selection-stmt",
    "selection-stmt'", "iteration-stmt", "return-stmt", "return-stmt'",
    "expression", "id-expr", "id-expr'", "var'",
    "simple-expression", "simple-expression'", "relop", "additive-expression",
    "additive-expression'", "addop", "term", "term'",
    "mulop", "factor", "factor'", "args",
    "arg-list", "arg-list'"
};

// Na ordem das colunas da matriz; "$" e o fim de arquivo (TOKEN_EOF).
static const char* const NOME_TERMINAL[NUM_COLUNAS_LL1] = {
    "ID", "NUM", "int", "void", "if", "else", "while",
    "return", "(", ")", "[", "]", "{", "}",
    ";", ",", "=", "==", "!=", "<", "<=",
    ">", ">=", "+", "-", "*", "/", "$"
};

// Coluna da matriz para cada tipo de token; -1 para tokens que nao fazem parte da gramatica.
inline int colunaLL1(TipoDeToken tipo) {
    switch (tipo) {
        case TOKEN_IDENTIFIER: return  0;
        case TOKEN_NUMBER: return  1;
        case TOKEN_INT: return  2;
        case TOKEN_VOID: return  3;
        case TOKEN_IF: return  4;
        case TOKEN_ELSE: return  5;
        case TOKEN_WHILE: return  6;
        case TOKEN_RETURN: return  7;
        case TOKEN_PAREN_OPEN: return  8;
        case TOKEN_PAREN_CLOSE: return  9;
        case TOKEN_BRACKET_OPEN: return 10;
        case TOKEN_BRACKET_CLOSE: return 11;
        case TOKEN_BRACE_OPEN: return 12;
        case TOKEN_BRACE_CLOSE: return 13;
        case TOKEN_SEMICOLON: return 14;
        case TOKEN_COMMA: return 15;
        case TOKEN_ASSIGN: return 16;
        case TOKEN_EQUAL_EQUAL: return 17;
        case TOKEN_NOT_EQUAL: return 18;
        case TOKEN_LESS: return 19;
        case TOKEN_LESS_EQUAL: return 20;
        case TOKEN_GREATER: return 21;
        case TOKEN_GREATER_EQUAL: return 22;
        case TOKEN_PLUS: return 23;
        case TOKEN_MINUS: return 24;
        case TOKEN_MULTIPLY: return 25;
        case TOKEN_DIVIDE: return 26;
        case TOKEN_EOF: return 27;
        case TOKEN_COMMENT:
        case TOKEN_ERROR:
            return -1;
    }
    return -1;
}

inline bool ehNaoTerminal(int simbolo) { return simbolo < NUM_NAO_TERMINAIS; }

const int NUM_PRODUCOES = 79;
const int MAX_CORPO = 6;

struct Producao {
    Simbolo lhs;                 // nao-terminal do lado esquerdo
    int tamanho;                 // numero de simbolos do corpo (0 = producao vazia)
    Simbolo corpo[MAX_CORPO];    // corpo, da esquerda para a direita
};

// Indice = numero da producao (o 0 nao e usado: 0 na matriz significa erro).
static const Producao PRODUCOES[NUM_PRODUCOES + 1] = {
    /*  0 */ { NUM_NAO_TERMINAIS, 0, { } },   // (sem uso)
    /*  1 */ { NT_PROGRAM, 1, { NT_DECLARATION_LIST } },   // program -> declaration-list
    /*  2 */ { NT_DECLARATION_LIST, 2, { NT_DECLARATION, NT_DECLARATION_LIST_P } },   // declaration-list -> declaration declaration-list'
    /*  3 */ { NT_DECLARATION_LIST_P, 2, { NT_DECLARATION, NT_DECLARATION_LIST_P } },   // declaration-list' -> declaration declaration-list'
    /*  4 */ { NT_DECLARATION_LIST_P, 0, { } },   // declaration-list' -> vazio
    /*  5 */ { NT_DECLARATION, 3, { NT_TYPE_SPECIFIER, T_IDENTIFIER, NT_DECLARATION_P } },   // declaration -> type-specifier ID declaration'
    /*  6 */ { NT_DECLARATION_P, 1, { NT_VAR_DECLARATION_P } },   // declaration' -> var-declaration'
    /*  7 */ { NT_DECLARATION_P, 1, { NT_FUN_DECLARATION_P } },   // declaration' -> fun-declaration'
    /*  8 */ { NT_VAR_DECLARATION, 3, { NT_TYPE_SPECIFIER, T_IDENTIFIER, NT_VAR_DECLARATION_P } },   // var-declaration -> type-specifier ID var-declaration'
    /*  9 */ { NT_VAR_DECLARATION_P, 1, { T_SEMICOLON } },   // var-declaration' -> ;
    /* 10 */ { NT_VAR_DECLARATION_P, 4, { T_BRACKET_OPEN, T_NUMBER, T_BRACKET_CLOSE, T_SEMICOLON } },   // var-declaration' -> [ NUM ] ;
    /* 11 */ { NT_TYPE_SPECIFIER, 1, { T_INT } },   // type-specifier -> int
    /* 12 */ { NT_TYPE_SPECIFIER, 1, { T_VOID } },   // type-specifier -> void
    /* 13 */ { NT_FUN_DECLARATION_P, 4, { T_PAREN_OPEN, NT_PARAMS, T_PAREN_CLOSE, NT_COMPOUND_STMT } },   // fun-declaration' -> ( params ) compound-stmt
    /* 14 */ { NT_PARAMS, 4, { T_INT, T_IDENTIFIER, NT_PARAM_P, NT_PARAM_LIST_P } },   // params -> int ID param' param-list'
    /* 15 */ { NT_PARAMS, 2, { T_VOID, NT_PARAMS_P } },   // params -> void params'
    /* 16 */ { NT_PARAMS_P, 3, { T_IDENTIFIER, NT_PARAM_P, NT_PARAM_LIST_P } },   // params' -> ID param' param-list'
    /* 17 */ { NT_PARAMS_P, 0, { } },   // params' -> vazio
    /* 18 */ { NT_PARAM_LIST_P, 3, { T_COMMA, NT_PARAM, NT_PARAM_LIST_P } },   // param-list' -> , param param-list'
    /* 19 */ { NT_PARAM_LIST_P, 0, { } },   // param-list' -> vazio
    /* 20 */ { NT_PARAM, 3, { NT_TYPE_SPECIFIER, T_IDENTIFIER, NT_PARAM_P } },   // param -> type-specifier ID param'
    /* 21 */ { NT_PARAM_P, 2, { T_BRACKET_OPEN, T_BRACKET_CLOSE } },   // param' -> [ ]
    /* 22 */ { NT_PARAM_P, 0, { } },   // param' -> vazio
    /* 23 */ { NT_COMPOUND_STMT, 4, { T_BRACE_OPEN, NT_LOCAL_DECLARATIONS, NT_STATEMENT_LIST, T_BRACE_CLOSE } },   // compound-stmt -> { local-declarations statement-list }
    /* 24 */ { NT_LOCAL_DECLARATIONS, 2, { NT_VAR_DECLARATION, NT_LOCAL_DECLARATIONS } },   // local-declarations -> var-declaration local-declarations
    /* 25 */ { NT_LOCAL_DECLARATIONS, 0, { } },   // local-declarations -> vazio
    /* 26 */ { NT_STATEMENT_LIST, 2, { NT_STATEMENT, NT_STATEMENT_LIST } },   // statement-list -> statement statement-list
    /* 27 */ { NT_STATEMENT_LIST, 0, { } },   // statement-list -> vazio
    /* 28 */ { NT_STATEMENT, 1, { NT_EXPRESSION_STMT } },   // statement -> expression-stmt
    /* 29 */ { NT_STATEMENT, 1, { NT_COMPOUND_STMT } },   // statement -> compound-stmt
    /* 30 */ { NT_STATEMENT, 1, { NT_SELECTION_STMT } },   // statement -> selection-stmt
    /* 31 */ { NT_STATEMENT, 1, { NT_ITERATION_STMT } },   // statement -> iteration-stmt
    /* 32 */ { NT_STATEMENT, 1, { NT_RETURN_STMT } },   // statement -> return-stmt
    /* 33 */ { NT_EXPRESSION_STMT, 2, { NT_EXPRESSION, T_SEMICOLON } },   // expression-stmt -> expression ;
    /* 34 */ { NT_EXPRESSION_STMT, 1, { T_SEMICOLON } },   // expression-stmt -> ;
    /* 35 */ { NT_SELECTION_STMT, 6, { T_IF, T_PAREN_OPEN, NT_EXPRESSION, T_PAREN_CLOSE, NT_STATEMENT, NT_SELECTION_STMT_P } },   // selection-stmt -> if ( expression ) statement selection-stmt'
    /* 36 */ { NT_SELECTION_STMT_P, 2, { T_ELSE, NT_STATEMENT } },   // selection-stmt' -> else statement
    /* 37 */ { NT_SELECTION_STMT_P, 0, { } },   // selection-stmt' -> vazio
    /* 38 */ { NT_ITERATION_STMT, 5, { T_WHILE, T_PAREN_OPEN, NT_EXPRESSION, T_PAREN_CLOSE, NT_STATEMENT } },   // iteration-stmt -> while ( expression ) statement
    /* 39 */ { NT_RETURN_STMT, 2, { T_RETURN, NT_RETURN_STMT_P } },   // return-stmt -> return return-stmt'
    /* 40 */ { NT_RETURN_STMT_P, 1, { T_SEMICOLON } },   // return-stmt' -> ;
    /* 41 */ { NT_RETURN_STMT_P, 2, { NT_EXPRESSION, T_SEMICOLON } },   // return-stmt' -> expression ;
    /* 42 */ { NT_EXPRESSION, 2, { T_IDENTIFIER, NT_ID_EXPR } },   // expression -> ID id-expr
    /* 43 */ { NT_EXPRESSION, 6, { T_PAREN_OPEN, NT_EXPRESSION, T_PAREN_CLOSE, NT_TERM_P, NT_ADDITIVE_EXPRESSION_P, NT_SIMPLE_EXPRESSION_P } },   // expression -> ( expression ) term' additive-expression' simple-expression'
    /* 44 */ { NT_EXPRESSION, 4, { T_NUMBER, NT_TERM_P, NT_ADDITIVE_EXPRESSION_P, NT_SIMPLE_EXPRESSION_P } },   // expression -> NUM term' additive-expression' simple-expression'
    /* 45 */ { NT_ID_EXPR, 6, { T_PAREN_OPEN, NT_ARGS, T_PAREN_CLOSE, NT_TERM_P, NT_ADDITIVE_EXPRESSION_P, NT_SIMPLE_EXPRESSION_P } },   // id-expr -> ( args ) term' additive-expression' simple-expression'
    /* 46 */ { NT_ID_EXPR, 2, { NT_VAR_P, NT_ID_EXPR_P } },   // id-expr -> var' id-expr'
    /* 47 */ { NT_ID_EXPR_P, 2, { T_ASSIGN, NT_EXPRESSION } },   // id-expr' -> = expression
    /* 48 */ { NT_ID_EXPR_P, 3, { NT_TERM_P, NT_ADDITIVE_EXPRESSION_P, NT_SIMPLE_EXPRESSION_P } },   // id-expr' -> term' additive-expression' simple-expression'
    /* 49 */ { NT_VAR_P, 3, { T_BRACKET_OPEN, NT_EXPRESSION, T_BRACKET_CLOSE } },   // var' -> [ expression ]
    /* 50 */ { NT_VAR_P, 0, { } },   // var' -> vazio
    /* 51 */ { NT_SIMPLE_EXPRESSION, 2, { NT_ADDITIVE_EXPRESSION, NT_SIMPLE_EXPRESSION_P } },   // simple-expression -> additive-expression simple-expression'
    /* 52 */ { NT_SIMPLE_EXPRESSION_P, 2, { NT_RELOP, NT_ADDITIVE_EXPRESSION } },   // simple-expression' -> relop additive-expression
    /* 53 */ { NT_SIMPLE_EXPRESSION_P, 0, { } },   // simple-expression' -> vazio
    /* 54 */ { NT_RELOP, 1, { T_LESS_EQUAL } },   // relop -> <=
    /* 55 */ { NT_RELOP, 1, { T_LESS } },   // relop -> <
    /* 56 */ { NT_RELOP, 1, { T_GREATER } },   // relop -> >
    /* 57 */ { NT_RELOP, 1, { T_GREATER_EQUAL } },   // relop -> >=
    /* 58 */ { NT_RELOP, 1, { T_EQUAL_EQUAL } },   // relop -> ==
    /* 59 */ { NT_RELOP, 1, { T_NOT_EQUAL } },   // relop -> !=
    /* 60 */ { NT_ADDITIVE_EXPRESSION, 2, { NT_TERM, NT_ADDITIVE_EXPRESSION_P } },   // additive-expression -> term additive-expression'
    /* 61 */ { NT_ADDITIVE_EXPRESSION_P, 3, { NT_ADDOP, NT_TERM, NT_ADDITIVE_EXPRESSION_P } },   // additive-expression' -> addop term additive-expression'
    /* 62 */ { NT_ADDITIVE_EXPRESSION_P, 0, { } },   // additive-expression' -> vazio
    /* 63 */ { NT_ADDOP, 1, { T_PLUS } },   // addop -> +
    /* 64 */ { NT_ADDOP, 1, { T_MINUS } },   // addop -> -
    /* 65 */ { NT_TERM, 2, { NT_FACTOR, NT_TERM_P } },   // term -> factor term'
    /* 66 */ { NT_TERM_P, 3, { NT_MULOP, NT_FACTOR, NT_TERM_P } },   // term' -> mulop factor term'
    /* 67 */ { NT_TERM_P, 0, { } },   // term' -> vazio
    /* 68 */ { NT_MULOP, 1, { T_MULTIPLY } },   // mulop -> *
    /* 69 */ { NT_MULOP, 1, { T_DIVIDE } },   // mulop -> /
    /* 70 */ { NT_FACTOR, 3, { T_PAREN_OPEN, NT_EXPRESSION, T_PAREN_CLOSE } },   // factor -> ( expression )
    /* 71 */ { NT_FACTOR, 2, { T_IDENTIFIER, NT_FACTOR_P } },   // factor -> ID factor'
    /* 72 */ { NT_FACTOR, 1, { T_NUMBER } },   // factor -> NUM
    /* 73 */ { NT_FACTOR_P, 1, { NT_VAR_P } },   // factor' -> var'
    /* 74 */ { NT_FACTOR_P, 3, { T_PAREN_OPEN, NT_ARGS, T_PAREN_CLOSE } },   // factor' -> ( args )
    /* 75 */ { NT_ARGS, 1, { NT_ARG_LIST } },   // args -> arg-list
    /* 76 */ { NT_ARGS, 0, { } },   // args -> vazio
    /* 77 */ { NT_ARG_LIST, 2, { NT_EXPRESSION, NT_ARG_LIST_P } },   // arg-list -> expression arg-list'
    /* 78 */ { NT_ARG_LIST_P, 3, { T_COMMA, NT_EXPRESSION, NT_ARG_LIST_P } },   // arg-list' -> , expression arg-list'
    /* 79 */ { NT_ARG_LIST_P, 0, { } }   // arg-list' -> vazio
};

static const unsigned int MATRIZ_LL1[NUM_NAO_TERMINAIS][NUM_COLUNAS_LL1] = {
    // producao 1..79; 100 + k = erro Ek
    //     ID     NUM     int    void      if    else   while  return       (       )       [       ]       {       }       ;       ,       =      ==      !=       <      <=       >      >=       +       -       *       /       $
    {    101,    101,      1,      1,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101 },   // NT_PROGRAM
    {    101,    101,      2,      2,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101 },   // NT_DECLARATION_LIST
    {    102,    102,      3,      3,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,    102,      4 },   // NT_DECLARATION_LIST_P
    {    101,    101,      5,      5,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101,    101 },   // NT_DECLARATION
    {    103,    103,    103,    103,    103,    103,    103,    103,      7,    103,      6,    103,    103,    103,      6,    103,    103,    103,    103,    103,    103,    103,    103,    103,    103,    103,    103,    103 },   // NT_DECLARATION_P
    {    104,    104,      8,      8,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104,    104 },   // NT_VAR_DECLARATION
    {    105,    105,    105,    105,    105,    105,    105,    105,    105,    105,     10,    105,    105,    105,      9,    105,    105,    105,    105,    105,    105,    105,    105,    105,    105,    105,    105,    105 },   // NT_VAR_DECLARATION_P
    {    106,    106,     11,     12,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106,    106 },   // NT_TYPE_SPECIFIER
    {    107,    107,    107,    107,    107,    107,    107,    107,     13,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107,    107 },   // NT_FUN_DECLARATION_P
    {    108,    108,     14,     15,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108,    108 },   // NT_PARAMS
    {     16,    109,    109,    109,    109,    109,    109,    109,    109,     17,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109,    109 },   // NT_PARAMS_P
    {    110,    110,    110,    110,    110,    110,    110,    110,    110,     19,    110,    110,    110,    110,    110,     18,    110,    110,    110,    110,    110,    110,    110,    110,    110,    110,    110,    110 },   // NT_PARAM_LIST_P
    {    111,    111,     20,     20,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111,    111 },   // NT_PARAM
    {    112,    112,    112,    112,    112,    112,    112,    112,    112,     22,     21,    112,    112,    112,    112,     22,    112,    112,    112,    112,    112,    112,    112,    112,    112,    112,    112,    112 },   // NT_PARAM_P
    {    113,    113,    113,    113,    113,    113,    113,    113,    113,    113,    113,    113,     23,    113,    113,    113,    113,    113,    113,    113,    113,    113,    113,    113,    113,    113,    113,    113 },   // NT_COMPOUND_STMT
    {     25,     25,     24,     24,     25,    114,     25,     25,     25,    114,    114,    114,     25,     25,     25,    114,    114,    114,    114,    114,    114,    114,    114,    114,    114,    114,    114,    114 },   // NT_LOCAL_DECLARATIONS
    {     26,     26,    115,    115,     26,    115,     26,     26,     26,    115,    115,    115,     26,     27,     26,    115,    115,    115,    115,    115,    115,    115,    115,    115,    115,    115,    115,    115 },   // NT_STATEMENT_LIST
    {     28,     28,    116,    116,     30,    116,     31,     32,     28,    116,    116,    116,     29,    116,     28,    116,    116,    116,    116,    116,    116,    116,    116,    116,    116,    116,    116,    116 },   // NT_STATEMENT
    {     33,     33,    117,    117,    117,    117,    117,    117,     33,    117,    117,    117,    117,    117,     34,    117,    117,    117,    117,    117,    117,    117,    117,    117,    117,    117,    117,    117 },   // NT_EXPRESSION_STMT
    {    118,    118,    118,    118,     35,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118,    118 },   // NT_SELECTION_STMT
    {     37,     37,    119,    119,     37,     36,     37,     37,     37,    119,    119,    119,     37,     37,     37,    119,    119,    119,    119,    119,    119,    119,    119,    119,    119,    119,    119,    119 },   // NT_SELECTION_STMT_P
    {    120,    120,    120,    120,    120,    120,     38,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120,    120 },   // NT_ITERATION_STMT
    {    121,    121,    121,    121,    121,    121,    121,     39,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121,    121 },   // NT_RETURN_STMT
    {     41,     41,    122,    122,    122,    122,    122,    122,     41,    122,    122,    122,    122,    122,     40,    122,    122,    122,    122,    122,    122,    122,    122,    122,    122,    122,    122,    122 },   // NT_RETURN_STMT_P
    {     42,     44,    123,    123,    123,    123,    123,    123,     43,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123 },   // NT_EXPRESSION
    {    124,    124,    124,    124,    124,    124,    124,    124,     45,     46,     46,     46,    124,    124,     46,     46,     46,     46,     46,     46,     46,     46,     46,     46,     46,     46,     46,    124 },   // NT_ID_EXPR
    {    125,    125,    125,    125,    125,    125,    125,    125,    125,     48,    125,     48,    125,    125,     48,     48,     47,     48,     48,     48,     48,     48,     48,     48,     48,     48,     48,    125 },   // NT_ID_EXPR_P
    {    126,    126,    126,    126,    126,    126,    126,    126,    126,     50,     49,     50,    126,    126,     50,     50,     50,     50,     50,     50,     50,     50,     50,     50,     50,     50,     50,    126 },   // NT_VAR_P
    {     51,     51,    123,    123,    123,    123,    123,    123,     51,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123 },   // NT_SIMPLE_EXPRESSION
    {    127,    127,    127,    127,    127,    127,    127,    127,    127,     53,    127,     53,    127,    127,     53,     53,    127,     52,     52,     52,     52,     52,     52,    127,    127,    127,    127,    127 },   // NT_SIMPLE_EXPRESSION_P
    {    128,    128,    128,    128,    128,    128,    128,    128,    128,    128,    128,    128,    128,    128,    128,    128,    128,     58,     59,     55,     54,     56,     57,    128,    128,    128,    128,    128 },   // NT_RELOP
    {     60,     60,    123,    123,    123,    123,    123,    123,     60,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123 },   // NT_ADDITIVE_EXPRESSION
    {    129,    129,    129,    129,    129,    129,    129,    129,    129,     62,    129,     62,    129,    129,     62,     62,    129,     62,     62,     62,     62,     62,     62,     61,     61,    129,    129,    129 },   // NT_ADDITIVE_EXPRESSION_P
    {    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,    130,     63,     64,    130,    130,    130 },   // NT_ADDOP
    {     65,     65,    123,    123,    123,    123,    123,    123,     65,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123,    123 },   // NT_TERM
    {    131,    131,    131,    131,    131,    131,    131,    131,    131,     67,    131,     67,    131,    131,     67,     67,    131,     67,     67,     67,     67,     67,     67,     67,     67,     66,     66,    131 },   // NT_TERM_P
    {    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,    132,     68,     69,    132 },   // NT_MULOP
    {     71,     72,    133,    133,    133,    133,    133,    133,     70,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133,    133 },   // NT_FACTOR
    {    134,    134,    134,    134,    134,    134,    134,    134,     74,     73,     73,     73,    134,    134,     73,     73,    134,     73,     73,     73,     73,     73,     73,     73,     73,     73,     73,    134 },   // NT_FACTOR_P
    {     75,     75,    135,    135,    135,    135,    135,    135,     75,     76,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135,    135 },   // NT_ARGS
    {     77,     77,    136,    136,    136,    136,    136,    136,     77,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136,    136 },   // NT_ARG_LIST
    {    137,    137,    137,    137,    137,    137,    137,    137,    137,     79,    137,    137,    137,    137,    137,     78,    137,    137,    137,    137,    137,    137,    137,    137,    137,    137,    137,    137 }   // NT_ARG_LIST_P
};

#endif // TABELA_LL1_H