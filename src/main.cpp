#include "../include/Token.h"
#include "../include/scanner.h"
#include "../include/TabelaLL1.h"
#include "../include/Parser.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

/*
 * Analisador de C- (Louden): lexico + sintatico LL(1) dirigido por tabela.
 *
 * Uso: main [opcoes] <arquivo>        (o arquivo e procurado em ../tests/, como antes)
 *      main [opcoes] -s "<texto>"     (analisa o texto da linha de comando)
 *
 *   -e            a entrada e uma expressao (simbolo inicial: expression), nao um programa
 *   -s <texto>    usa <texto> como entrada em vez de um arquivo
 *   -q            so o resultado (sem tabela de tokens, passos e arvore)
 *   -t            so a analise lexica, no formato original ("Token: ... Tipo: ... Linha: ...")
 *   -dot <arq>    grava a arvore de analise em Graphviz DOT (dot -Tpng arq -o arvore.png)
 *   -tabela       imprime as producoes, a tabela LL(1) e a tabela de erros
 *
 * Saida padrao, em quatro fases:
 *   i.   analise lexica   (tabela de tokens)
 *   ii.  analise sintatica por tabela LL(1), com pilha e cadeia de entrada
 *   iii. passos do parse  (pilha, entrada, acao a cada passo)
 *   iv.  arvore de analise sintatica completa
 *
 * Codigo de saida: 0 = entrada valida, 1 = erro lexico ou sintatico, 2 = arquivo nao abriu.
 */

namespace {

void uso(const char* prog) {
    std::cerr << "Uso: " << prog << " [-e] [-q] [-t] [-dot arquivo.dot] [-tabela] <arquivo.c>\n"
              << "     " << prog << " [-e] [-q] [-dot arquivo.dot] -s \"<texto>\"\n";
}

// Lexema para exibicao em tabela: sem quebras de linha e com no maximo 'max' caracteres.
std::string textoCurto(const char* lexema, std::size_t max = 24) {
    std::string s = lexema;
    for (char& c : s) if (c == '\n' || c == '\r' || c == '\t') c = ' ';
    if (s.size() > max) s = s.substr(0, max - 3) + "...";
    return s;
}

void imprimirTabelaLL1() {
    std::cout << "=== TABELA DE ANALISE LL(1) DE C- ===\n\n";

    std::cout << "Producoes:\n";
    for (int p = 1; p <= NUM_PRODUCOES; ++p) {
        std::cout << "  [" << std::setw(2) << p << "] " << NOME_NAO_TERMINAL[PRODUCOES[p].lhs] << " ->";
        if (PRODUCOES[p].tamanho == 0) std::cout << " vazio";
        for (int i = 0; i < PRODUCOES[p].tamanho; ++i) std::cout << ' ' << nomeDoSimbolo(PRODUCOES[p].corpo[i]);
        std::cout << '\n';
    }

    std::cout << "\nMatriz (por nao-terminal: terminal:producao). Toda outra coluna e erro Ek:\n";
    for (int i = 0; i < NUM_NAO_TERMINAIS; ++i) {
        int codigo = 0;
        for (int c = 0; c < NUM_COLUNAS_LL1 && codigo == 0; ++c)
            if (ehErro(MATRIZ_LL1[i][c])) codigo = codigoDoErro(MATRIZ_LL1[i][c]);
        std::cout << "  " << NOME_NAO_TERMINAL[i] << "  [erro E" << codigo << "]:";
        for (int c = 0; c < NUM_COLUNAS_LL1; ++c)
            if (!ehErro(MATRIZ_LL1[i][c])) std::cout << "  " << NOME_TERMINAL[c] << ':' << MATRIZ_LL1[i][c];
        std::cout << '\n';
    }

    std::cout << "\nErros possiveis:\n";
    for (int e = 1; e <= NUM_ERROS; ++e) std::cout << "  E" << std::setw(2) << std::left << e << std::right
                                                    << " " << MENSAGEM_ERRO[e] << '\n';
}

} // namespace

int main(int argc, char *argv[]) {
    bool soTokens = false, quieto = false, mostrarTabela = false;
    ModoEntrada modo = ModoEntrada::Programa;
    std::string nome, texto, arquivoDot;
    bool temTexto = false;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "-t") soTokens = true;
        else if (a == "-q") quieto = true;
        else if (a == "-e") modo = ModoEntrada::Expressao;
        else if (a == "-tabela") mostrarTabela = true;
        else if (a == "-s" && i + 1 < argc) { texto = argv[++i]; temTexto = true; }
        else if (a == "-dot" && i + 1 < argc) arquivoDot = argv[++i];
        else nome = a;
    }

    if (mostrarTabela) {
        imprimirTabelaLL1();
        if (nome.empty() && !temTexto) return 0;
        std::cout << '\n';
    }
    if (nome.empty() && !temTexto) {
        uso(argv[0]);
        return 1;
    }

    Scanner &scanner = Scanner::getInstance();
    if (temTexto) {
        scanner.loadString(texto);
    } else if (!scanner.loadFile("../tests/" + nome)) {
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return 2;
    }

    // ---------------------------------------------------------------- -t (formato original)
    if (soTokens) {
        while (true) {
            Token token = scanner.nextToken();
            if (token.tipo == TOKEN_EOF) break;
            std::cout << "Token: " << token.lexema << " Tipo: " << tokenTypeToString(token.tipo) << " Linha: " << token.linha << std::endl;
        }
        return 0;
    }

    // ---------------------------------------------------------------- i. analise lexica
    std::vector<Token> todos;            // tudo o que o scanner devolveu (inclui comentarios e erros)
    while (true) {
        Token token = scanner.nextToken();
        todos.push_back(token);
        if (token.tipo == TOKEN_EOF) break;
    }

    if (!quieto) {
        std::cout << "=== i. ANALISE LEXICA ===\n";
        std::cout << std::right << std::setw(4) << "#" << "  " << std::left << std::setw(26) << "Lexema"
                  << std::setw(22) << "Token" << std::setw(12) << "Terminal" << "Linha\n";
        int n = 0;
        for (const Token& t : todos) {
            if (t.tipo == TOKEN_EOF) continue;
            const int col = colunaLL1(t.tipo);
            std::string terminal = col >= 0 ? NOME_TERMINAL[col] : (t.tipo == TOKEN_COMMENT ? "(ignorado)" : "(invalido)");
            std::cout << std::right << std::setw(4) << ++n << "  " << std::left << std::setw(26) << textoCurto(t.lexema)
                      << std::setw(22) << tokenTypeToString(t.tipo) << std::setw(12) << terminal << t.linha << '\n';
        }
        std::cout << std::right << n << " token(s) lidos (incluindo comentarios e simbolos invalidos); fim de arquivo = $.\n\n";
    }

    std::vector<Token> tokens;           // o que o parser consome: sem comentarios, terminado em EOF
    int errosLexicos = 0;
    for (const Token& t : todos) {
        if (t.tipo == TOKEN_ERROR) {
            ++errosLexicos;
            const std::string lex = t.lexema;
            std::cerr << "linha " << t.linha << ": erro E" << ERRO_LEXICO << ": "
                      << (lex.compare(0, 2, "/*") == 0 ? std::string("comentario nao terminado")
                                                        : "sequencia invalida '" + lex + "'") << '\n';
        } else if (t.tipo != TOKEN_COMMENT) {
            tokens.push_back(t);
        }
    }
    if (errosLexicos > 0) {
        std::cerr << "Erro: " << errosLexicos << " erro(s) lexico(s); analise sintatica nao executada." << std::endl;
        return 1;
    }

    // ---------------------------------------------------------------- ii/iii. parse por tabela LL(1)
    Parser parser(tokens, modo, !quieto);
    const bool ok = parser.parse();

    if (!quieto) {
        std::cout << "=== ii/iii. ANALISE SINTATICA LL(1) POR TABELA: PASSOS DO PARSE ===\n";
        std::cout << "(topo da pilha a esquerda; $ marca o fundo da pilha e o fim da entrada; simbolo inicial: "
                  << (modo == ModoEntrada::Programa ? "program" : "expression") << ")\n";
        if (modo == ModoEntrada::Expressao)
            std::cout << "(modo expressao: sem ';' no fim, o fim da entrada e consultado na tabela como ';')\n";
        std::cout << '\n';
        parser.imprimirPassos(std::cout);
        std::cout << '\n';
    }

    if (!ok) {
        std::cerr << "Erro: " << parser.mensagemErro() << std::endl;
        return 1;
    }

    // ---------------------------------------------------------------- iv. arvore de analise
    if (!quieto) {
        std::cout << "=== iv. ARVORE DE ANALISE SINTATICA ===\n";
        parser.imprimirArvore(std::cout);
        int nos, folhas, altura;
        parser.estatisticasArvore(nos, folhas, altura);
        std::cout << "\nArvore: " << nos << " nos, " << folhas << " folhas, altura " << altura << ".\n\n";
    }

    if (!arquivoDot.empty()) {
        std::ofstream dot(arquivoDot);
        if (!dot) {
            std::cerr << "Nao foi possivel gravar " << arquivoDot << std::endl;
            return 2;
        }
        parser.exportarDot(dot);
        std::cout << "Arvore gravada em " << arquivoDot << " (Graphviz DOT).\n";
    }

    std::cout << "Analise sintatica concluida: "
              << (modo == ModoEntrada::Programa ? "programa valido." : "expressao valida.") << std::endl;
    return 0;
}