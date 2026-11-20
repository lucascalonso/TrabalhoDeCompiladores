#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "TabelaLL1.h"

#include <memory>
#include <ostream>
#include <string>
#include <vector>

/*
 * Parser LL(1) dirigido por tabela para C- (Louden).
 *
 * Trabalha sobre a lista de tokens ja produzida pela analise lexica e usa a matriz de
 * TabelaLL1.h com uma pilha de simbolos (manipulacao explicita de pilha e cadeia de entrada):
 *
 *   pilha inicial = [ simbolo-inicial, $ ]      (topo a esquerda)
 *   - topo e terminal    : precisa ser igual ao token atual; consome o token e desempilha;
 *   - topo e nao-terminal: MATRIZ_LL1[topo][token] e uma producao (desempilha o topo e empilha
 *                          o corpo, de tras para frente) ou um codigo de erro Ek;
 *   - sucesso quando $ casa com o fim de arquivo.
 *
 * Cada passo (pilha, entrada, acao) e registrado, e a arvore de analise e construida junto:
 * ao aplicar uma producao, cada simbolo do corpo vira um No filho do no que estava no topo.
 */

// Valor de No::simbolo para uma producao vazia (folha "epsilon").
const int SIMBOLO_EPSILON = -1;

// No da arvore de analise sintatica.
struct No {
    int simbolo;                              // valor de Simbolo, ou SIMBOLO_EPSILON
    std::string lexema;                       // so em terminais: texto do token casado
    int linha = 0;                            // so em terminais
    std::vector<std::unique_ptr<No>> filhos;  // corpo da producao, da esquerda para a direita

    explicit No(int s) : simbolo(s) {}
    No(const No&) = delete;
    No& operator=(const No&) = delete;
    ~No();                                    // destruicao iterativa (arvores fundas nao estouram a pilha)
};

// Uma linha da tabela de passos do parse.
struct Passo {
    int numero;
    std::string pilha;     // topo a esquerda; o ultimo simbolo e sempre $
    std::string entrada;   // cadeia de entrada restante, terminada por $
    std::string acao;      // producao aplicada, token casado, aceitacao ou erro
};

enum class ModoEntrada {
    Programa,    // simbolo inicial: program
    Expressao    // simbolo inicial: expression; o fim da entrada age como o delimitador ';'
};

// Nome de um simbolo (terminal, nao-terminal ou epsilon), para saidas.
std::string nomeDoSimbolo(int simbolo);

class Parser {
public:
    // 'tokens' deve estar sem comentarios e terminar em TOKEN_EOF (se nao terminar, o parser acrescenta).
    // Com registrarPassos = false o parse nao guarda a tabela de passos (menos memoria).
    Parser(const std::vector<Token>& tokens,
           ModoEntrada modo = ModoEntrada::Programa,
           bool registrarPassos = true);

    // Executa a analise. true = entrada reconhecida (arvore completa em raiz()).
    // false = erro; o motivo esta em codigoErro() / mensagemErro() e os passos vao ate o erro.
    bool parse();

    const std::vector<Passo>& passos() const { return passos_; }
    const No* raiz() const { return raiz_.get(); }
    int codigoErro() const { return codigoErro_; }                 // Ek, ou 0 se nao houve erro
    const std::string& mensagemErro() const { return mensagemErro_; }

    // Saidas
    void imprimirPassos(std::ostream& os) const;                   // tabela Passo | Pilha | Entrada | Acao
    void imprimirArvore(std::ostream& os) const;                   // arvore em ASCII
    void exportarDot(std::ostream& os) const;                      // arvore em Graphviz DOT
    void estatisticasArvore(int& nos, int& folhas, int& altura) const;

private:
    struct Item {              // elemento da pilha: simbolo + no da arvore que ele vai preencher
        int simbolo;
        No* no;                // nullptr para o marcador $
    };

    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
    ModoEntrada modo_;
    bool registrar_;

    std::vector<Item> pilha_;
    std::unique_ptr<No> raiz_;
    std::vector<Passo> passos_;
    int codigoErro_ = 0;
    std::string mensagemErro_;

    void registra(const std::string& acao);
    bool falha(int codigo, const std::string& mensagem);
    std::string encontrado() const;
};

#endif // PARSER_H