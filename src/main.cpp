#include "../include/Token.h"
#include "../include/scanner.h"
#include "../include/Parser.h"
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
    bool soTokens = false;
    bool verbose = false;
    std::string nome;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-t") soTokens = true;
        else if (a == "-v") verbose = true;
        else nome = a;
    }

    if (nome.empty()) {
        std::cerr << "Uso: " << argv[0] << " [-t] [-v] <arquivo.c>" << std::endl;
        return 1;
    }

    std::string filename = "../tests/" + nome;

    Scanner &scanner = Scanner::getInstance();
    if(!scanner.loadFile(filename)){
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return 2;
    }

    if (soTokens) {
        while(true){
            Token token = scanner.nextToken();
            if(token.tipo == TOKEN_EOF) break;
            std::cout << "Token: " << token.lexema << " Tipo: " << tokenTypeToString(token.tipo) << " Linha: " << token.linha << std::endl;
        }
        return 0;
    }

    try {
        Parser parser(scanner, verbose);
        parser.parse();
        std::cout << "Analise sintatica concluida: programa valido." << std::endl;
        return 0;
    } catch (const ParseError &e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }
}
