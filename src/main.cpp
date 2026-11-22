#include <iostream>
#include <string>
#include <cstring>

#include "../include/Token.h"
#include "../include/scanner.h"

extern "C" {
    int yyparse(void);
    extern int erros_sintaticos;   // definido em yacc.y
    extern int erros_lexicos;      // definido em yylex_bridge.cpp
}

// Uso:
//   cminus <arquivo.cm>       analise lexica + sintatica
//   cminus -t <arquivo.cm>    apenas lista os tokens (so o scanner)
int main(int argc, char *argv[]) {
    bool somenteTokens = (argc == 3 && std::strcmp(argv[1], "-t") == 0);

    if (argc != 2 && !somenteTokens) {
        std::cerr << "Uso: " << argv[0] << " [-t] <arquivo.cm>" << std::endl;
        return 1;
    }

    const std::string filename = somenteTokens ? argv[2] : argv[1];

    Scanner &scanner = Scanner::getInstance();
    if (!scanner.loadFile(filename)) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return 1;
    }

    if (somenteTokens) {
        while (true) {
            Token token = scanner.nextToken();
            if (token.tipo == TOKEN_EOF) break;
            std::cout << "Token: " << token.lexema
                      << " Tipo: " << tokenTypeToString(token.tipo)
                      << " Linha: " << token.linha
                      << " Coluna: " << token.coluna << std::endl;
        }
        return 0;
    }

    int r = yyparse();
    if (erros_lexicos > 0)
        std::cout << ">>> " << erros_lexicos << " erro(s) lexico(s) encontrado(s) <<<" << std::endl;

    return (r != 0 || erros_sintaticos > 0 || erros_lexicos > 0) ? 1 : 0;
}
