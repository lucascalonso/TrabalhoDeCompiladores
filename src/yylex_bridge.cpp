// Ponte entre o scanner em C++ (tabela de transição) e o parser gerado pelo Bison.
// O Bison chama yylex(); aqui ela pede o próximo token ao Scanner, ignora
// comentários, reporta erros léxicos e converte TipoDeToken -> token do Yacc.

#include <cstdio>
#include <string>

#include "../include/Token.h"
#include "../include/scanner.h"

extern "C" {
#include "yacc.tab.h"   // gerado por: bison -d -o yacc.tab.c yacc.y

// Histórico duplo: yyerror() reporta a posição do token ANTERIOR ao que
// causou o erro (mesma lógica do REGISTRAR_TOKEN_VALIDO do antigo lexer.l).
int linha_do_erro = 1;
int coluna_do_erro = 1;
int erros_lexicos = 0;

static int ultima_linha_token_valido = 1;
static int ultima_coluna_token_valido = 1;
}

static int paraYacc(TipoDeToken t) {
    switch (t) {
        case TOKEN_ELSE:          return ELSE;
        case TOKEN_IF:            return IF;
        case TOKEN_INT:           return INT;
        case TOKEN_RETURN:        return RETURN;
        case TOKEN_VOID:          return VOID;
        case TOKEN_WHILE:         return WHILE;
        case TOKEN_PLUS:          return SOMA;
        case TOKEN_MINUS:         return SUB;
        case TOKEN_MULTIPLY:      return MULT;
        case TOKEN_DIVIDE:        return DIV;
        case TOKEN_LESS_EQUAL:    return LEQ;
        case TOKEN_LESS:          return LT;
        case TOKEN_GREATER_EQUAL: return GEQ;
        case TOKEN_GREATER:       return GT;
        case TOKEN_EQUAL_EQUAL:   return EQ;
        case TOKEN_NOT_EQUAL:     return NEQ;
        case TOKEN_ASSIGN:        return ASSIGN;
        case TOKEN_SEMICOLON:     return SEMI;
        case TOKEN_COMMA:         return COMMA;
        case TOKEN_PAREN_OPEN:    return LPAREN;
        case TOKEN_PAREN_CLOSE:   return RPAREN;
        case TOKEN_BRACKET_OPEN:  return LBRACK;
        case TOKEN_BRACKET_CLOSE: return RBRACK;
        case TOKEN_BRACE_OPEN:    return LBRACE;
        case TOKEN_BRACE_CLOSE:   return RBRACE;
        case TOKEN_IDENTIFIER:    return ID;
        case TOKEN_NUMBER:        return NUM;
        default:                  return 0;
    }
}


extern "C" int yylex(void) {
    Scanner &scanner = Scanner::getInstance();

    for (;;) {
        Token tok = scanner.nextToken();

        if (tok.tipo == TOKEN_EOF)
            return 0;                       // YYEOF

        if (tok.tipo == TOKEN_COMMENT)
            continue;                       // comentários não vão para o parser

        if (tok.tipo == TOKEN_ERROR) {      // erro léxico: reporta e segue
            ++erros_lexicos;
            std::fprintf(stderr,
                "ERRO LEXICO (Linha %d, Coluna %d): lexema invalido '%s'\n",
                tok.linha, tok.coluna, tok.lexema);
            continue;
        }

        yylloc.first_line   = yylloc.last_line   = tok.linha;
        yylloc.first_column = tok.coluna;
        yylloc.last_column  = tok.coluna + (int)std::string(tok.lexema).size() - 1;

        linha_do_erro  = ultima_linha_token_valido;
        coluna_do_erro = ultima_coluna_token_valido;
        ultima_linha_token_valido  = tok.linha;
        ultima_coluna_token_valido = tok.coluna;

        return paraYacc(tok.tipo);
    }
}
