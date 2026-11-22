#include "../include/Token.h"
#include "../include/scanner.h"
#include "../include/ParserManager.hpp"
#include <string>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // evita acentos quebrados no terminal do Windows
#endif
    if (argc < 2 || argc > 3) {
        std::cerr << "Uso: " << argv[0] << " <arquivo> [--lex]" << std::endl;
        std::cerr << "  sem opção: análise léxica + sintática (LL(1)) com rastreamento do parser" << std::endl;
        std::cerr << "  --lex    : apenas análise léxica (lista de tokens)" << std::endl;
        return 1;
    }

    std::string filename = "../tests/" + std::string(argv[1]);
    bool modoParser = !(argc == 3 && std::string(argv[2]) == "--lex");

    Scanner &scanner = Scanner::getInstance();
    if(!scanner.loadFile(filename)){
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return 1;
    }

    if (modoParser) {
        ParserManager parser;
        // 0 = ok, 1 = erro léxico, 2 = erro sintático
        return static_cast<int>(parser.analisar(scanner));
    }

    while(true){
        Token token = scanner.nextToken();
        if(token.tipo == TOKEN_EOF) break;
        std::cout << "Token: " << token.lexema << " Tipo: " << tokenTypeToString(token.tipo) << " Linha: " << token.linha << std::endl;
    }
    return 0;
}