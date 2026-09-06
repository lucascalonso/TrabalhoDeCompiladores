#include "Token.h"
#include "scanner.h"
#include <iostream>

int main(){
    
    Scanner &scanner = Scanner::getInstance();
    if(!scanner.loadFile("teste.c")){
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return 1;
    }
    while(true){
        Token token = scanner.nextToken();
        if(token.tipo == TOKEN_EOF) break;
        std::cout << "Token: " << token.lexema << ", Tipo: " << tokenTypeToString(token.tipo) << ", Linha: " << token.linha << std::endl;
    }
    return 0;
}