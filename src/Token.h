#ifndef TOKEN_H
#define TOKEN_H


enum TipoDeToken {
    // Palavras-chave
    TOKEN_ELSE,
    TOKEN_INT,
    TOKEN_IF,
    TOKEN_WHILE,
    TOKEN_VOID,
    TOKEN_RETURN,

    // Identificadores e números
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,

    // Símbolos especiais
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_PAREN_OPEN,
    TOKEN_PAREN_CLOSE,
    TOKEN_BRACKET_OPEN,
    TOKEN_BRACKET_CLOSE,
    TOKEN_BRACE_OPEN,
    TOKEN_BRACE_CLOSE,
    TOKEN_COMMENT,

    // Token para indicar o fim do arquivo
    TOKEN_EOF,
    TOKEN_ERROR
};

typedef struct {
    char lexema[40];
    enum TipoDeToken tipo;
    int linha;
} Token;

const char* tokenTypeToString(TipoDeToken tipo);

#endif // TOKEN_H
