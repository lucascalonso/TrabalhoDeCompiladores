#include "../include/Parser.h"
#include "../include/TabelaLL1.h"

#include <iostream>
#include <string>
#include <utility>

namespace {

// Nome do terminal para mensagens: "$" aparece como "fim de arquivo".
std::string nomeDoTerminal(int coluna) {
    const std::string nome = NOME_TERMINAL[coluna];
    return nome == "$" ? "fim de arquivo" : "'" + nome + "'";
}

// Simbolo (terminal) -> indice da coluna da matriz.
int colunaDoSimbolo(int simbolo) { return simbolo - NUM_NAO_TERMINAIS; }

// Nome de qualquer simbolo, para o modo -v e nos da arvore.
std::string nomeDoSimbolo(int simbolo) {
    if (ehNaoTerminal(simbolo)) return NOME_NAO_TERMINAL[simbolo];
    return NOME_TERMINAL[colunaDoSimbolo(simbolo)];
}

// Acima deste numero de tokens possiveis, a mensagem deixa de listar o que era esperado.
const int MAX_ESPERADOS_NA_MENSAGEM = 8;

} // namespace

Parser::Parser(Scanner& scanner, bool verbose) : scanner_(scanner), verbose_(verbose) {}

// Proximo token do scanner, descartando comentarios (o scanner os devolve como TOKEN_COMMENT).
void Parser::advance() {
    do {
        cur_ = scanner_.nextToken();
    } while (cur_.tipo == TOKEN_COMMENT);
}

// Desempilha ja foi feito por quem chama; aqui so empilha o corpo, de tras para frente,
// para que o primeiro simbolo do corpo fique no topo. Producao vazia nao empilha nada.
void Parser::empilhaProducao(int numero) {
    const Producao& p = PRODUCOES[numero];
    for (int i = p.tamanho - 1; i >= 0; --i) pilha_.push_back(p.corpo[i]);
}

void Parser::imprimirArvore(const NoArvore* no, const std::string& prefixo, bool ultimo) const {
    if (!no) return;
    std::cout << prefixo << (ultimo ? "`-- " : "|-- ") << no->rotulo << '\n';
    const std::string novoPrefixo = prefixo + (ultimo ? "  " : "| ");
    for (std::size_t i = 0; i < no->filhos.size(); ++i) {
        imprimirArvore(no->filhos[i].get(), novoPrefixo, i + 1 == no->filhos.size());
    }
}

std::string Parser::encontrado() const {
    if (cur_.tipo == TOKEN_EOF) return "fim de arquivo";
    return std::string("'") + cur_.lexema + "'";
}

// Token que o scanner nao reconheceu (TOKEN_ERROR): nao tem coluna na matriz.
void Parser::erroLexico() const {
    const std::string lex = cur_.lexema;
    std::string msg = "linha " + std::to_string(cur_.linha) + ": erro lexico: ";
    if (lex.compare(0, 2, "/*") == 0) msg += "comentario nao terminado";
    else                              msg += "sequencia invalida '" + lex + "'";
    throw ParseError(msg);
}

// Topo da pilha e um terminal diferente do token atual.
void Parser::erroTerminal(int colunaEsperada) const {
    throw ParseError("linha " + std::to_string(cur_.linha) + ": esperado " +
                     nomeDoTerminal(colunaEsperada) + ", encontrado " + encontrado());
}

// Celula vazia da matriz: a propria linha diz quais tokens seriam aceitos agora.
void Parser::erroNaoTerminal(int linhaDaMatriz) const {
    std::vector<int> esperados;
    for (int c = 0; c < NUM_COLUNAS_LL1; ++c)
        if (MATRIZ_LL1[linhaDaMatriz][c] != ERRO_LL1) esperados.push_back(c);

    std::string msg = "linha " + std::to_string(cur_.linha) + ": ";
    if (esperados.empty() || static_cast<int>(esperados.size()) > MAX_ESPERADOS_NA_MENSAGEM) {
        msg += "token inesperado " + encontrado();
    } else {
        msg += "esperado ";
        for (std::size_t i = 0; i < esperados.size(); ++i) {
            if (i > 0) msg += (i + 1 == esperados.size()) ? " ou " : ", ";
            msg += nomeDoTerminal(esperados[i]);
        }
        msg += ", encontrado " + encontrado();
    }
    throw ParseError(msg);
}

void Parser::parse() {
    pilha_.clear();
    pilha_.push_back(T_EOF);           // marcador de fim
    pilha_.push_back(NT_PROGRAM);      // simbolo inicial

    auto raiz = std::make_unique<NoArvore>(NoArvore{"program", {}});
    std::vector<NoArvore*> pilhaNos = { nullptr, raiz.get() }; // paralela a pilha_

    advance();                         // carrega o primeiro token de lookahead

    while (!pilha_.empty()) {
        const int topo = pilha_.back();

        const int coluna = colunaLL1(cur_.tipo);
        if (coluna < 0) erroLexico();  // TOKEN_ERROR

        if (!ehNaoTerminal(topo)) {
            // ---- terminal: casa com o token atual ----
            if (colunaDoSimbolo(topo) != coluna) erroTerminal(colunaDoSimbolo(topo));

            NoArvore* noAtual = pilhaNos.back();
            pilhaNos.pop_back();
            if (noAtual && (cur_.tipo == TOKEN_IDENTIFIER || cur_.tipo == TOKEN_NUMBER)) {
                noAtual->rotulo += " (" + std::string(cur_.lexema) + ")";
            }

            if (verbose_) {
                std::cout << "match " << nomeDoTerminal(coluna);
                if (cur_.tipo == TOKEN_IDENTIFIER || cur_.tipo == TOKEN_NUMBER)
                    std::cout << " (" << cur_.lexema << ")";
                std::cout << '\n';
            }
            pilha_.pop_back();
            if (topo != T_EOF) advance();
        } else {
            // ---- nao-terminal: consulta a matriz ----
            const unsigned int numero = MATRIZ_LL1[topo][coluna];
            if (numero == ERRO_LL1) erroNaoTerminal(topo);

            const Producao& p = PRODUCOES[numero];
            if (verbose_) {
                std::cout << "[" << numero << "] " << NOME_NAO_TERMINAL[topo] << " ->";
                if (p.tamanho == 0) std::cout << " vazio";
                for (int i = 0; i < p.tamanho; ++i) std::cout << ' ' << nomeDoSimbolo(p.corpo[i]);
                std::cout << '\n';
            }

            NoArvore* pai = pilhaNos.back();
            pilhaNos.pop_back();

            if (p.tamanho > 0) {
                std::vector<NoArvore*> novosFilhos(p.tamanho);
                for (int i = 0; i < p.tamanho; ++i) {
                    auto filho = std::make_unique<NoArvore>(NoArvore{nomeDoSimbolo(p.corpo[i]), {}});
                    novosFilhos[i] = filho.get();
                    pai->filhos.push_back(std::move(filho));
                }
                for (int i = p.tamanho - 1; i >= 0; --i) {
                    pilhaNos.push_back(novosFilhos[i]);
                }
            }

            pilha_.pop_back();
            empilhaProducao(static_cast<int>(numero));
        }
    }

    std::cout << "\n--- Arvore de Analise Sintatica ---\n";
    imprimirArvore(raiz.get());
    std::cout << '\n';
}
