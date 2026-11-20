#include "../include/Parser.h"

#include <algorithm>
#include <string>
#include <utility>

namespace {

// Terminal (valor de Simbolo) -> coluna da matriz.
int colunaDoSimbolo(int simbolo) { return simbolo - NUM_NAO_TERMINAIS; }

// Terminal entre aspas para mensagens; $ aparece como "fim de arquivo".
std::string terminalEntreAspas(int simbolo) {
    const std::string nome = NOME_TERMINAL[colunaDoSimbolo(simbolo)];
    return nome == "$" ? "fim de arquivo" : "'" + nome + "'";
}

// Texto do corpo de uma producao, para a coluna de acao.
std::string corpoDaProducao(const Producao& p) {
    if (p.tamanho == 0) return "vazio";
    std::string s;
    for (int i = 0; i < p.tamanho; ++i) {
        if (i > 0) s += ' ';
        s += nomeDoSimbolo(p.corpo[i]);
    }
    return s;
}

// Rotulo de um no para a arvore: terminais entre aspas duplas (ID e NUM mostram o lexema).
std::string rotuloDoNo(const No& n) {
    if (n.simbolo == SIMBOLO_EPSILON) return "epsilon";
    if (ehNaoTerminal(n.simbolo)) return NOME_NAO_TERMINAL[n.simbolo];
    const std::string nome = NOME_TERMINAL[colunaDoSimbolo(n.simbolo)];
    if (n.simbolo == T_IDENTIFIER || n.simbolo == T_NUMBER) return nome + " \"" + n.lexema + "\"";
    return "\"" + nome + "\"";
}

std::string escapaDot(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '"' || c == '\\') r += '\\';
        r += c;
    }
    return r;
}

} // namespace

std::string nomeDoSimbolo(int simbolo) {
    if (simbolo == SIMBOLO_EPSILON) return "epsilon";
    if (ehNaoTerminal(simbolo)) return NOME_NAO_TERMINAL[simbolo];
    return NOME_TERMINAL[colunaDoSimbolo(simbolo)];
}

// Destruicao sem recursao: move os filhos para uma lista de pendentes e vai soltando um a um.
No::~No() {
    std::vector<std::unique_ptr<No>> pendentes;
    for (auto& f : filhos) pendentes.push_back(std::move(f));
    filhos.clear();
    while (!pendentes.empty()) {
        std::unique_ptr<No> n = std::move(pendentes.back());
        pendentes.pop_back();
        for (auto& f : n->filhos) pendentes.push_back(std::move(f));
        n->filhos.clear();
    }
}

// ============================================================ analise

Parser::Parser(const std::vector<Token>& tokens, ModoEntrada modo, bool registrarPassos)
    : tokens_(tokens), modo_(modo), registrar_(registrarPassos) {
    if (tokens_.empty() || tokens_.back().tipo != TOKEN_EOF) {
        Token eof{};
        eof.tipo = TOKEN_EOF;
        eof.linha = tokens_.empty() ? 1 : tokens_.back().linha;
        tokens_.push_back(eof);
    }
}

std::string Parser::encontrado() const {
    const Token& t = tokens_[pos_];
    if (t.tipo == TOKEN_EOF) return "fim de arquivo";
    return std::string("'") + t.lexema + "'";
}

// Registra a configuracao atual (pilha, entrada restante) junto com a acao que sera executada.
void Parser::registra(const std::string& acao) {
    if (!registrar_) return;

    Passo p;
    p.numero = static_cast<int>(passos_.size()) + 1;

    for (auto it = pilha_.rbegin(); it != pilha_.rend(); ++it) {     // topo primeiro
        if (!p.pilha.empty()) p.pilha += ' ';
        p.pilha += nomeDoSimbolo(it->simbolo);
    }
    for (std::size_t i = pos_; i < tokens_.size() && tokens_[i].tipo != TOKEN_EOF; ++i) {
        p.entrada += tokens_[i].lexema;
        p.entrada += ' ';
    }
    p.entrada += '$';

    p.acao = acao;
    passos_.push_back(std::move(p));
}

bool Parser::falha(int codigo, const std::string& mensagem) {
    codigoErro_ = codigo;
    mensagemErro_ = "linha " + std::to_string(tokens_[pos_].linha) + ": erro E" +
                    std::to_string(codigo) + ": " + mensagem;
    registra("ERRO E" + std::to_string(codigo) + ": " + mensagem);
    return false;
}

bool Parser::parse() {
    pilha_.clear();
    passos_.clear();
    raiz_.reset();
    pos_ = 0;
    codigoErro_ = 0;
    mensagemErro_.clear();

    const int inicial = (modo_ == ModoEntrada::Programa) ? NT_PROGRAM : NT_EXPRESSION;
    raiz_.reset(new No(inicial));
    pilha_.push_back({T_EOF, nullptr});             // marcador de fim
    pilha_.push_back({inicial, raiz_.get()});       // simbolo inicial no topo

    while (!pilha_.empty()) {
        const Item topo = pilha_.back();
        const Token& tok = tokens_[pos_];

        // Token que o scanner nao reconheceu: nao tem coluna na matriz.
        const int coluna = colunaLL1(tok.tipo);
        if (coluna < 0) {
            const std::string lex = tok.lexema;
            return falha(ERRO_LEXICO, lex.compare(0, 2, "/*") == 0
                                          ? std::string("comentario nao terminado")
                                          : "sequencia invalida '" + lex + "'");
        }

        if (!ehNaoTerminal(topo.simbolo)) {
            // ---------- terminal no topo: tem de casar com o token atual ----------
            if (colunaDoSimbolo(topo.simbolo) != coluna)
                return falha(ERRO_TERMINAL, "esperado " + terminalEntreAspas(topo.simbolo) +
                                            ", encontrado " + encontrado());

            if (topo.simbolo == T_EOF) {
                registra("aceitar: entrada reconhecida");
                pilha_.pop_back();
                break;
            }

            std::string acao = "casar " + nomeDoSimbolo(topo.simbolo);
            if (topo.simbolo == T_IDENTIFIER || topo.simbolo == T_NUMBER) acao += std::string(" (") + tok.lexema + ")";
            registra(acao);

            topo.no->lexema = tok.lexema;
            topo.no->linha = tok.linha;
            pilha_.pop_back();
            ++pos_;
        } else {
            // ---------- nao-terminal no topo: consulta a matriz ----------
            // No modo expressao nao ha ';' no fim: o fim da entrada e consultado como ';'.
            const int colunaConsulta = (modo_ == ModoEntrada::Expressao && tok.tipo == TOKEN_EOF)
                                           ? colunaLL1(TOKEN_SEMICOLON)
                                           : coluna;
            const unsigned int celula = MATRIZ_LL1[topo.simbolo][colunaConsulta];

            if (ehErro(celula)) {
                const int codigo = codigoDoErro(celula);
                return falha(codigo, std::string(MENSAGEM_ERRO[codigo]) + ", encontrado " + encontrado());
            }

            const Producao& p = PRODUCOES[celula];
            registra("aplicar [" + std::to_string(celula) + "] " + NOME_NAO_TERMINAL[topo.simbolo] +
                     " -> " + corpoDaProducao(p));

            // Cria os filhos do no (na ordem do corpo) e empilha de tras para frente.
            No* pai = topo.no;
            pilha_.pop_back();
            if (p.tamanho == 0) {
                pai->filhos.emplace_back(new No(SIMBOLO_EPSILON));
            } else {
                for (int i = 0; i < p.tamanho; ++i) pai->filhos.emplace_back(new No(p.corpo[i]));
                for (int i = p.tamanho - 1; i >= 0; --i) pilha_.push_back({p.corpo[i], pai->filhos[i].get()});
            }
        }
    }
    return true;
}

// ============================================================ saidas

void Parser::imprimirPassos(std::ostream& os) const {
    std::size_t wPilha = 5, wEntrada = 7;     // largura minima: "Pilha", "Entrada"
    for (const Passo& p : passos_) {
        wPilha = std::max(wPilha, p.pilha.size());
        wEntrada = std::max(wEntrada, p.entrada.size());
    }
    const std::size_t wNum = std::max<std::size_t>(5, std::to_string(passos_.size()).size());

    auto linha = [&](const std::string& num, const std::string& pilha, const std::string& entrada,
                     const std::string& acao) {
        os << std::string(wNum - std::min(wNum, num.size()), ' ') << num << " | "
           << pilha << std::string(wPilha - std::min(wPilha, pilha.size()), ' ') << " | "
           << entrada << std::string(wEntrada - std::min(wEntrada, entrada.size()), ' ') << " | "
           << acao << '\n';
    };
    linha("Passo", "Pilha", "Entrada", "Acao");
    os << std::string(wNum, '-') << "-+-" << std::string(wPilha, '-') << "-+-"
       << std::string(wEntrada, '-') << "-+-" << std::string(20, '-') << '\n';
    for (const Passo& p : passos_) linha(std::to_string(p.numero), p.pilha, p.entrada, p.acao);
}

// Arvore em ASCII, percorrida com pilha propria (sem recursao).
void Parser::imprimirArvore(std::ostream& os) const {
    if (!raiz_) return;
    struct Quadro {
        const No* no;
        std::string prefixo;     // texto que vem antes do conector
        bool ultimo;             // e o ultimo filho do pai?
        bool ehRaiz;
    };
    std::vector<Quadro> pilha;
    pilha.push_back({raiz_.get(), "", true, true});
    while (!pilha.empty()) {
        Quadro q = std::move(pilha.back());
        pilha.pop_back();

        std::string filhosPrefixo;
        if (q.ehRaiz) {
            os << rotuloDoNo(*q.no) << '\n';
        } else {
            os << q.prefixo << (q.ultimo ? "`-- " : "|-- ") << rotuloDoNo(*q.no) << '\n';
            filhosPrefixo = q.prefixo + (q.ultimo ? "    " : "|   ");
        }
        const auto& f = q.no->filhos;
        for (std::size_t i = f.size(); i-- > 0;)
            pilha.push_back({f[i].get(), filhosPrefixo, i + 1 == f.size(), false});
    }
}

void Parser::exportarDot(std::ostream& os) const {
    os << "digraph arvore {\n"
       << "    ordering=out;\n"
       << "    node [fontname=\"Helvetica\"];\n";
    if (!raiz_) { os << "}\n"; return; }

    int proximoId = 0;
    std::vector<std::pair<const No*, int>> pilha;
    pilha.push_back({raiz_.get(), proximoId++});
    std::string arestas;
    while (!pilha.empty()) {
        const auto [no, id] = pilha.back();
        pilha.pop_back();

        std::string rotulo = rotuloDoNo(*no);
        std::string estilo;
        if (no->simbolo == SIMBOLO_EPSILON) estilo = "shape=plaintext";
        else if (ehNaoTerminal(no->simbolo)) estilo = "shape=ellipse";
        else estilo = "shape=box, style=filled, fillcolor=\"#DDEBF7\"";
        os << "    n" << id << " [label=\"" << escapaDot(rotulo) << "\", " << estilo << "];\n";

        std::vector<std::pair<const No*, int>> filhos;
        for (const auto& f : no->filhos) {
            const int fid = proximoId++;
            arestas += "    n" + std::to_string(id) + " -> n" + std::to_string(fid) + ";\n";
            filhos.push_back({f.get(), fid});
        }
        for (std::size_t i = filhos.size(); i-- > 0;) pilha.push_back(filhos[i]);
    }
    os << arestas << "}\n";
}

void Parser::estatisticasArvore(int& nos, int& folhas, int& altura) const {
    nos = folhas = altura = 0;
    if (!raiz_) return;
    std::vector<std::pair<const No*, int>> pilha;
    pilha.push_back({raiz_.get(), 1});
    while (!pilha.empty()) {
        const auto [no, nivel] = pilha.back();
        pilha.pop_back();
        ++nos;
        altura = std::max(altura, nivel);
        if (no->filhos.empty()) ++folhas;
        for (const auto& f : no->filhos) pilha.push_back({f.get(), nivel + 1});
    }
}