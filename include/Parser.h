#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "scanner.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class ParseError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct NoArvore {
    std::string rotulo;
    std::vector<std::unique_ptr<NoArvore>> filhos;
};

class Parser {
public:
    explicit Parser(Scanner& scanner, bool verbose = false);

    // Analisa o arquivo carregado no scanner. Lanca ParseError no primeiro erro.
    void parse();

private:
    Scanner& scanner_;
    Token cur_{};                 // token de lookahead
    bool verbose_;
    std::vector<int> pilha_;      // valores de Simbolo (terminais e nao-terminais)

    void advance();
    void empilhaProducao(int numero);
    void imprimirArvore(const NoArvore* no, const std::string& prefixo = "", bool ultimo = true) const;

    [[noreturn]] void erroLexico() const;
    [[noreturn]] void erroTerminal(int colunaEsperada) const;
    [[noreturn]] void erroNaoTerminal(int linhaDaMatriz) const;
    std::string encontrado() const;
};

#endif // PARSER_H