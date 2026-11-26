#include <iostream>
#include <string>
#include <cstring>

#include "../include/Token.h"
#include "../include/scanner.h"
#include "../include/AST.h"
#include "../include/AnalisadorSemantico.h"
#include "../include/GeradorMIPS.h"

extern "C" {
    extern int erros_lexicos;
}

int yyparse(void);
extern int erros_sintaticos;
extern ASTNode* raiz_ast;

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

    // ==========================================
    // FASE 1: ANALISE LEXICA
    // ==========================================
    std::cout << "==========================================" << std::endl;
    std::cout << "           1. ANALISE LEXICA              " << std::endl;
    std::cout << "==========================================" << std::endl;

    while (true) {
        Token token = scanner.nextToken();
        if (token.tipo == TOKEN_EOF) break;
        std::cout << "Token: " << token.lexema
                  << " | Tipo: " << tokenTypeToString(token.tipo)
                  << " | Linha: " << token.linha
                  << " | Coluna: " << token.coluna << std::endl;
    }

    if (somenteTokens) {
        return 0;
    }

    // Reinicia o scanner para o parser consumir os tokens desde o inicio
    scanner.reset();

    // ==========================================
    // FASE 2: ANALISE SINTATICA
    // ==========================================
    std::cout << "\n==========================================" << std::endl;
    std::cout << "          2. ANALISE SINTATICA            " << std::endl;
    std::cout << "==========================================" << std::endl;

    int r = yyparse();

    if (erros_lexicos > 0)
        std::cout << ">>> " << erros_lexicos << " erro(s) lexico(s) encontrado(s) <<<" << std::endl;

    if (r == 0 && erros_sintaticos == 0 && erros_lexicos == 0) {
        std::cout << ">>> Analise sintatica concluida sem erros! <<<" << std::endl;
    } else {
        std::cout << ">>> Analise sintatica concluida com " << erros_sintaticos << " erro(s) sintatico(s)! <<<" << std::endl;
        liberarAST(raiz_ast);
        return 1;
    }

    // ==========================================
    // FASE 3: ANALISE SEMANTICA
    // ==========================================
    std::cout << "\n==========================================" << std::endl;
    std::cout << "          3. ANALISE SEMANTICA            " << std::endl;
    std::cout << "==========================================" << std::endl;

    AnalisadorSemantico analisador;
    analisador.analisar(raiz_ast);

    std::cout << "\n--- AST DECORADA (POS-ANALISE SEMANTICA) ---" << std::endl;
    imprimirAST(raiz_ast);

    int errosSem = analisador.getErrosSemanticos();
    int avisosSem = analisador.getAvisos();

    if (avisosSem > 0)
        std::cout << "\n>>> " << avisosSem << " aviso(s) semantico(s) emitido(s) <<<" << std::endl;

    if (errosSem > 0) {
        std::cout << ">>> " << errosSem << " erro(s) semantico(s) encontrado(s) <<<" << std::endl;
        liberarAST(raiz_ast);
        return 1;
    }

    std::cout << "\n>>> Analise semantica concluida sem erros! <<<" << std::endl;

    // ==========================================
    // FASE 4: GERACAO DE CODIGO MIPS
    // ==========================================
    std::cout << "\n==========================================" << std::endl;
    std::cout << "         4. GERACAO DE CODIGO MIPS        " << std::endl;
    std::cout << "==========================================" << std::endl;

    GeradorMIPS gerador;
    const std::string arquivoMips = "saida_mips.txt";
    if (gerador.gerar(raiz_ast, arquivoMips)) {
        std::cout << ">>> Codigo MIPS gerado com sucesso em: " << arquivoMips << " <<<" << std::endl;
    }

    liberarAST(raiz_ast);
    return 0;
}
