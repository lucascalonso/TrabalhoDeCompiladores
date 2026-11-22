#include <iostream>
#include <fstream>
#include "lexer.hpp"
#include "ParserManager.hpp"
#include <vector>


int main() {
    std::string nomeArquivo = "entrada2.txt";
    std::ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        std::cerr << "Erro crítico: Não foi possível abrir o arquivo '" << nomeArquivo << "'. Certifique-se de que ele foi criado na mesma pasta do executável.\n";
        return 1;
    }

    std::string expressao;
    std::getline(arquivo, expressao);
    arquivo.close();

    std::cout << "=== ANÁLISE DO ARQUIVO: " << nomeArquivo << " ===\n";
    std::cout << "Conteúdo Lido: " << expressao << "\n\n";

    // Executa a Tarefa 1.a (Análise Léxica)
    std::vector<Token> tokens = tokenize(expressao);
    
    std::cout << "Tokens Identificados (1.a):\n";
    for (const auto& t : tokens) {
        std::cout << "[" << t.value << "] ";
    }
    std::cout << "\n";

    // Executa a Tarefa 1.b (Uso e Validação da Matriz Sintática)
    ParserManager parser;
    parser.runParser(tokens);

    return 0;
}