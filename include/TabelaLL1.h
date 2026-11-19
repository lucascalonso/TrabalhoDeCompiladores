#ifndef TABELA_LL1_H
#define TABELA_LL1_H

#include "Token.h"

constexpr int NUM_NAO_TERMINAIS = 42;
constexpr int NUM_COLUNAS_LL1   = 28;
constexpr int NUM_PRODUCOES     = 80; // indices 1..79 (0 = ERRO_LL1)
constexpr unsigned int ERRO_LL1 = 0;

enum Simbolo {
    // --- Nao-terminais (0..41) ---
    NT_PROGRAM = 0,
    NT_DECLARATION_LIST,
    NT_DECLARATION_LIST_PRIME,
    NT_DECLARATION,
    NT_DECLARATION_PRIME,
    NT_VAR_DECLARATION,
    NT_VAR_DECLARATION_PRIME,
    NT_TYPE_SPECIFIER,
    NT_FUN_DECLARATION_PRIME,
    NT_PARAMS,
    NT_PARAMS_PRIME,
    NT_PARAM_LIST_PRIME,
    NT_PARAM,
    NT_PARAM_PRIME,
    NT_COMPOUND_STMT,
    NT_LOCAL_DECLARATIONS,
    NT_STATEMENT_LIST,
    NT_STATEMENT,
    NT_EXPRESSION_STMT,
    NT_SELECTION_STMT,
    NT_SELECTION_STMT_PRIME,
    NT_ITERATION_STMT,
    NT_RETURN_STMT,
    NT_RETURN_STMT_PRIME,
    NT_EXPRESSION,
    NT_ID_EXPR,
    NT_ID_EXPR_PRIME,
    NT_VAR_PRIME,
    NT_SIMPLE_EXPRESSION,
    NT_SIMPLE_EXPRESSION_PRIME,
    NT_RELOP,
    NT_ADDITIVE_EXPRESSION,
    NT_ADDITIVE_EXPRESSION_PRIME,
    NT_ADDOP,
    NT_TERM,
    NT_TERM_PRIME,
    NT_MULOP,
    NT_FACTOR,
    NT_FACTOR_PRIME,
    NT_ARGS,
    NT_ARG_LIST,
    NT_ARG_LIST_PRIME,

    // --- Terminais (42..69 -> colunas 0..27) ---
    T_ID,       // 0: ID
    T_NUM,      // 1: NUM
    T_INT,      // 2: int
    T_VOID,     // 3: void
    T_IF,       // 4: if
    T_ELSE,     // 5: else
    T_WHILE,    // 6: while
    T_RETURN,   // 7: return
    T_LPAREN,   // 8: (
    T_RPAREN,   // 9: )
    T_LBRACKET, // 10: [
    T_RBRACKET, // 11: ]
    T_LBRACE,   // 12: {
    T_RBRACE,   // 13: }
    T_SEMI,     // 14: ;
    T_COMMA,    // 15: ,
    T_ASSIGN,   // 16: =
    T_EQ,       // 17: ==
    T_NEQ,      // 18: !=
    T_LT,       // 19: <
    T_LE,       // 20: <=
    T_GT,       // 21: >
    T_GE,       // 22: >=
    T_PLUS,     // 23: +
    T_MINUS,    // 24: -
    T_MULT,     // 25: *
    T_DIV,      // 26: /
    T_EOF       // 27: $
};

inline bool ehNaoTerminal(int simbolo) {
    return simbolo >= 0 && simbolo < NUM_NAO_TERMINAIS;
}

// Mapeia TipoDeToken (Token.h) para a coluna [0..27] da MATRIZ_LL1.
inline int colunaLL1(TipoDeToken tipo) {
    switch (tipo) {
        case TOKEN_IDENTIFIER:    return 0;  // ID
        case TOKEN_NUMBER:        return 1;  // NUM
        case TOKEN_INT:           return 2;  // int
        case TOKEN_VOID:          return 3;  // void
        case TOKEN_IF:            return 4;  // if
        case TOKEN_ELSE:          return 5;  // else
        case TOKEN_WHILE:         return 6;  // while
        case TOKEN_RETURN:        return 7;  // return
        case TOKEN_PAREN_OPEN:    return 8;  // (
        case TOKEN_PAREN_CLOSE:   return 9;  // )
        case TOKEN_BRACKET_OPEN:  return 10; // [
        case TOKEN_BRACKET_CLOSE: return 11; // ]
        case TOKEN_BRACE_OPEN:    return 12; // {
        case TOKEN_BRACE_CLOSE:   return 13; // }
        case TOKEN_SEMICOLON:     return 14; // ;
        case TOKEN_COMMA:         return 15; // ,
        case TOKEN_ASSIGN:        return 16; // =
        case TOKEN_EQUAL_EQUAL:   return 17; // ==
        case TOKEN_NOT_EQUAL:     return 18; // !=
        case TOKEN_LESS:          return 19; // <
        case TOKEN_LESS_EQUAL:    return 20; // <=
        case TOKEN_GREATER:       return 21; // >
        case TOKEN_GREATER_EQUAL: return 22; // >=
        case TOKEN_PLUS:          return 23; // +
        case TOKEN_MINUS:         return 24; // -
        case TOKEN_MULTIPLY:      return 25; // *
        case TOKEN_DIVIDE:        return 26; // /
        case TOKEN_EOF:           return 27; // $
        default:                  return -1; // TOKEN_ERROR
    }
}

constexpr const char* NOME_NAO_TERMINAL[NUM_NAO_TERMINAIS] = {
    "program",
    "declaration-list",
    "declaration-list'",
    "declaration",
    "declaration'",
    "var-declaration",
    "var-declaration'",
    "type-specifier",
    "fun-declaration'",
    "params",
    "params'",
    "param-list'",
    "param",
    "param'",
    "compound-stmt",
    "local-declarations",
    "statement-list",
    "statement",
    "expression-stmt",
    "selection-stmt",
    "selection-stmt'",
    "iteration-stmt",
    "return-stmt",
    "return-stmt'",
    "expression",
    "id-expr",
    "id-expr'",
    "var'",
    "simple-expression",
    "simple-expression'",
    "relop",
    "additive-expression",
    "additive-expression'",
    "addop",
    "term",
    "term'",
    "mulop",
    "factor",
    "factor'",
    "args",
    "arg-list",
    "arg-list'"
};

constexpr const char* NOME_TERMINAL[NUM_COLUNAS_LL1] = {
    "ID", "NUM", "int", "void", "if", "else", "while", "return",
    "(", ")", "[", "]", "{", "}", ";", ",",
    "=", "==", "!=", "<", "<=", ">", ">=", "+", "-", "*", "/", "$"
};

struct Producao {
    int tamanho;
    int corpo[6]; // maior producao da gramatica tem 6 simbolos
};

constexpr Producao PRODUCOES[NUM_PRODUCOES] = {
    /* 0  */ {0, {}},
    /* 1  */ {1, {NT_DECLARATION_LIST}},
    /* 2  */ {2, {NT_DECLARATION, NT_DECLARATION_LIST_PRIME}},
    /* 3  */ {2, {NT_DECLARATION, NT_DECLARATION_LIST_PRIME}},
    /* 4  */ {0, {}},
    /* 5  */ {3, {NT_TYPE_SPECIFIER, T_ID, NT_DECLARATION_PRIME}},
    /* 6  */ {1, {NT_VAR_DECLARATION_PRIME}},
    /* 7  */ {1, {NT_FUN_DECLARATION_PRIME}},
    /* 8  */ {3, {NT_TYPE_SPECIFIER, T_ID, NT_VAR_DECLARATION_PRIME}},
    /* 9  */ {1, {T_SEMI}},
    /* 10 */ {4, {T_LBRACKET, T_NUM, T_RBRACKET, T_SEMI}},
    /* 11 */ {1, {T_INT}},
    /* 12 */ {1, {T_VOID}},
    /* 13 */ {4, {T_LPAREN, NT_PARAMS, T_RPAREN, NT_COMPOUND_STMT}},
    /* 14 */ {4, {T_INT, T_ID, NT_PARAM_PRIME, NT_PARAM_LIST_PRIME}},
    /* 15 */ {2, {T_VOID, NT_PARAMS_PRIME}},
    /* 16 */ {3, {T_ID, NT_PARAM_PRIME, NT_PARAM_LIST_PRIME}},
    /* 17 */ {0, {}},
    /* 18 */ {3, {T_COMMA, NT_PARAM, NT_PARAM_LIST_PRIME}},
    /* 19 */ {0, {}},
    /* 20 */ {3, {NT_TYPE_SPECIFIER, T_ID, NT_PARAM_PRIME}},
    /* 21 */ {2, {T_LBRACKET, T_RBRACKET}},
    /* 22 */ {0, {}},
    /* 23 */ {4, {T_LBRACE, NT_LOCAL_DECLARATIONS, NT_STATEMENT_LIST, T_RBRACE}},
    /* 24 */ {2, {NT_VAR_DECLARATION, NT_LOCAL_DECLARATIONS}},
    /* 25 */ {0, {}},
    /* 26 */ {2, {NT_STATEMENT, NT_STATEMENT_LIST}},
    /* 27 */ {0, {}},
    /* 28 */ {1, {NT_EXPRESSION_STMT}},
    /* 29 */ {1, {NT_COMPOUND_STMT}},
    /* 30 */ {1, {NT_SELECTION_STMT}},
    /* 31 */ {1, {NT_ITERATION_STMT}},
    /* 32 */ {1, {NT_RETURN_STMT}},
    /* 33 */ {2, {NT_EXPRESSION, T_SEMI}},
    /* 34 */ {1, {T_SEMI}},
    /* 35 */ {6, {T_IF, T_LPAREN, NT_EXPRESSION, T_RPAREN, NT_STATEMENT, NT_SELECTION_STMT_PRIME}},
    /* 36 */ {2, {T_ELSE, NT_STATEMENT}},
    /* 37 */ {0, {}},
    /* 38 */ {5, {T_WHILE, T_LPAREN, NT_EXPRESSION, T_RPAREN, NT_STATEMENT}},
    /* 39 */ {2, {T_RETURN, NT_RETURN_STMT_PRIME}},
    /* 40 */ {1, {T_SEMI}},
    /* 41 */ {2, {NT_EXPRESSION, T_SEMI}},
    /* 42 */ {2, {T_ID, NT_ID_EXPR}},
    /* 43 */ {6, {T_LPAREN, NT_EXPRESSION, T_RPAREN, NT_TERM_PRIME, NT_ADDITIVE_EXPRESSION_PRIME, NT_SIMPLE_EXPRESSION_PRIME}},
    /* 44 */ {4, {T_NUM, NT_TERM_PRIME, NT_ADDITIVE_EXPRESSION_PRIME, NT_SIMPLE_EXPRESSION_PRIME}},
    /* 45 */ {6, {T_LPAREN, NT_ARGS, T_RPAREN, NT_TERM_PRIME, NT_ADDITIVE_EXPRESSION_PRIME, NT_SIMPLE_EXPRESSION_PRIME}},
    /* 46 */ {2, {NT_VAR_PRIME, NT_ID_EXPR_PRIME}},
    /* 47 */ {2, {T_ASSIGN, NT_EXPRESSION}},
    /* 48 */ {3, {NT_TERM_PRIME, NT_ADDITIVE_EXPRESSION_PRIME, NT_SIMPLE_EXPRESSION_PRIME}},
    /* 49 */ {3, {T_LBRACKET, NT_EXPRESSION, T_RBRACKET}},
    /* 50 */ {0, {}},
    /* 51 */ {2, {NT_ADDITIVE_EXPRESSION, NT_SIMPLE_EXPRESSION_PRIME}},
    /* 52 */ {2, {NT_RELOP, NT_ADDITIVE_EXPRESSION}},
    /* 53 */ {0, {}},
    /* 54 */ {1, {T_LE}},
    /* 55 */ {1, {T_LT}},
    /* 56 */ {1, {T_GT}},
    /* 57 */ {1, {T_GE}},
    /* 58 */ {1, {T_EQ}},
    /* 59 */ {1, {T_NEQ}},
    /* 60 */ {2, {NT_TERM, NT_ADDITIVE_EXPRESSION_PRIME}},
    /* 61 */ {3, {NT_ADDOP, NT_TERM, NT_ADDITIVE_EXPRESSION_PRIME}},
    /* 62 */ {0, {}},
    /* 63 */ {1, {T_PLUS}},
    /* 64 */ {1, {T_MINUS}},
    /* 65 */ {2, {NT_FACTOR, NT_TERM_PRIME}},
    /* 66 */ {3, {NT_MULOP, NT_FACTOR, NT_TERM_PRIME}},
    /* 67 */ {0, {}},
    /* 68 */ {1, {T_MULT}},
    /* 69 */ {1, {T_DIV}},
    /* 70 */ {3, {T_LPAREN, NT_EXPRESSION, T_RPAREN}},
    /* 71 */ {2, {T_ID, NT_FACTOR_PRIME}},
    /* 72 */ {1, {T_NUM}},
    /* 73 */ {1, {NT_VAR_PRIME}},
    /* 74 */ {3, {T_LPAREN, NT_ARGS, T_RPAREN}},
    /* 75 */ {1, {NT_ARG_LIST}},
    /* 76 */ {0, {}},
    /* 77 */ {2, {NT_EXPRESSION, NT_ARG_LIST_PRIME}},
    /* 78 */ {3, {T_COMMA, NT_EXPRESSION, NT_ARG_LIST_PRIME}},
    /* 79 */ {0, {}}
};

// Colunas:
//  0:ID   1:NUM  2:int  3:void 4:if   5:else 6:whil 7:ret  8:(    9:)
// 10:[   11:]   12:{   13:}   14:;   15:,   16:=   17:==  18:!=  19:<
// 20:<=  21:>   22:>=  23:+   24:-   25:*   26:/   27:$
constexpr unsigned int MATRIZ_LL1[NUM_NAO_TERMINAIS][NUM_COLUNAS_LL1] = {
    /* 0  program              */ { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 1  declaration-list     */ { 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 2  declaration-list'    */ { 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    /* 3  declaration          */ { 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 4  declaration'         */ { 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 5  var-declaration      */ { 0, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 6  var-declaration'     */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,10, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 7  type-specifier       */ { 0, 0,11,12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 8  fun-declaration'     */ { 0, 0, 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 9  params               */ { 0, 0,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 10 params'              */ {16, 0, 0, 0, 0, 0, 0, 0, 0,17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 11 param-list'          */ { 0, 0, 0, 0, 0, 0, 0, 0, 0,19, 0, 0, 0, 0, 0,18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 12 param                */ { 0, 0,20,20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 13 param'               */ { 0, 0, 0, 0, 0, 0, 0, 0, 0,22,21, 0, 0, 0, 0,22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 14 compound-stmt        */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 15 local-declarations   */ {25,25,24,24,25, 0,25,25,25, 0, 0, 0,25,25,25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 16 statement-list       */ {26,26, 0, 0,26, 0,26,26,26, 0, 0, 0,26,27,26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 17 statement            */ {28,28, 0, 0,30, 0,31,32,28, 0, 0, 0,29, 0,28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 18 expression-stmt      */ {33,33, 0, 0, 0, 0, 0, 0,33, 0, 0, 0, 0, 0,34, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 19 selection-stmt       */ { 0, 0, 0, 0,35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 20 selection-stmt'      */ {37,37, 0, 0,37,36,37,37,37, 0, 0, 0,37,37,37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 21 iteration-stmt       */ { 0, 0, 0, 0, 0, 0,38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 22 return-stmt          */ { 0, 0, 0, 0, 0, 0, 0,39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 23 return-stmt'         */ {41,41, 0, 0, 0, 0, 0, 0,41, 0, 0, 0, 0, 0,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 24 expression           */ {42,44, 0, 0, 0, 0, 0, 0,43, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 25 id-expr              */ { 0, 0, 0, 0, 0, 0, 0, 0,45,46,46,46, 0, 0,46,46,46,46,46,46,46,46,46,46,46,46,46, 0},
    /* 26 id-expr'             */ { 0, 0, 0, 0, 0, 0, 0, 0, 0,48, 0,48, 0, 0,48,48,47,48,48,48,48,48,48,48,48,48,48, 0},
    /* 27 var'                 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0,50,49,50, 0, 0,50,50,50,50,50,50,50,50,50,50,50,50,50, 0},
    /* 28 simple-expression    */ {51,51, 0, 0, 0, 0, 0, 0,51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 29 simple-expression'   */ { 0, 0, 0, 0, 0, 0, 0, 0, 0,53, 0,53, 0, 0,53,53, 0,52,52,52,52,52,52, 0, 0, 0, 0, 0},
    /* 30 relop                */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,58,59,55,54,56,57, 0, 0, 0, 0, 0},
    /* 31 additive-expression  */ {60,60, 0, 0, 0, 0, 0, 0,60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 32 additive-expression' */ { 0, 0, 0, 0, 0, 0, 0, 0, 0,62, 0,62, 0, 0,62,62, 0,62,62,62,62,62,62,61,61, 0, 0, 0},
    /* 33 addop                */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,63,64, 0, 0, 0},
    /* 34 term                 */ {65,65, 0, 0, 0, 0, 0, 0,65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 35 term'                */ { 0, 0, 0, 0, 0, 0, 0, 0, 0,67, 0,67, 0, 0,67,67, 0,67,67,67,67,67,67,67,67,66,66, 0},
    /* 36 mulop                */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,68,69, 0},
    /* 37 factor               */ {71,72, 0, 0, 0, 0, 0, 0,70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 38 factor'              */ { 0, 0, 0, 0, 0, 0, 0, 0,74,73,73,73, 0, 0,73,73, 0,73,73,73,73,73,73,73,73,73,73, 0},
    /* 39 args                 */ {75,75, 0, 0, 0, 0, 0, 0,75,76, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 40 arg-list             */ {77,77, 0, 0, 0, 0, 0, 0,77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* 41 arg-list'            */ { 0, 0, 0, 0, 0, 0, 0, 0, 0,79, 0, 0, 0, 0, 0,78, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
#endif // TABELA_LL1_H