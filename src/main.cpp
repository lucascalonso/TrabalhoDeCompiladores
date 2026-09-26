#include "../include/Token.h"
#include "../include/scanner.h"
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo.c>" << std::endl;
        return 1;
    }

    std::string filename = "../tests/" + std::string(argv[1]);

    Scanner &scanner = Scanner::getInstance();
    if(!scanner.loadFile(filename)){
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return 1;
    }
    while(true){
        Token token = scanner.nextToken();
        if(token.tipo == TOKEN_EOF) break;
        std::cout << "Token: " << token.lexema << " Tipo: " << tokenTypeToString(token.tipo) << " Linha: " << token.linha << std::endl;
    }
    return 0;
}