#include "Token.h"

const char* tokenTypeToString(TipoDeToken tipo) {
    switch (tipo) {
        case TOKEN_ELSE:             return "TOKEN_ELSE";
        case TOKEN_INT:              return "TOKEN_INT";
        case TOKEN_IF:               return "TOKEN_IF";
        case TOKEN_WHILE:            return "TOKEN_WHILE";
        case TOKEN_VOID:             return "TOKEN_VOID";
        case TOKEN_RETURN:           return "TOKEN_RETURN";

        case TOKEN_IDENTIFIER:       return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER:           return "TOKEN_NUMBER";

        case TOKEN_PLUS:             return "TOKEN_PLUS";
        case TOKEN_MINUS:            return "TOKEN_MINUS";
        case TOKEN_MULTIPLY:         return "TOKEN_MULTIPLY";
        case TOKEN_DIVIDE:           return "TOKEN_DIVIDE";
        case TOKEN_LESS:             return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL:       return "TOKEN_LESS_EQUAL";
        case TOKEN_GREATER:          return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL:    return "TOKEN_GREATER_EQUAL";
        case TOKEN_EQUAL_EQUAL:      return "TOKEN_EQUAL_EQUAL";
        case TOKEN_NOT_EQUAL:        return "TOKEN_NOT_EQUAL";
        case TOKEN_ASSIGN:           return "TOKEN_ASSIGN";

        case TOKEN_SEMICOLON:        return "TOKEN_SEMICOLON";
        case TOKEN_COMMA:            return "TOKEN_COMMA";
        case TOKEN_PAREN_OPEN:       return "TOKEN_PAREN_OPEN";
        case TOKEN_PAREN_CLOSE:      return "TOKEN_PAREN_CLOSE";
        case TOKEN_BRACKET_OPEN:     return "TOKEN_BRACKET_OPEN";
        case TOKEN_BRACKET_CLOSE:    return "TOKEN_BRACKET_CLOSE";
        case TOKEN_BRACE_OPEN:       return "TOKEN_BRACE_OPEN";
        case TOKEN_BRACE_CLOSE:      return "TOKEN_BRACE_CLOSE";

        case TOKEN_COMMENT:          return "TOKEN_COMMENT";

        case TOKEN_EOF:              return "TOKEN_EOF";
        case TOKEN_ERROR:            return "TOKEN_ERROR";
    }

    return "UNKNOWN_TOKEN";
}