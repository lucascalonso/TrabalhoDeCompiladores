#ifndef PARSER_MANAGER_HPP
#define PARSER_MANAGER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <iomanip>
#include "Token.h"
#include "scanner.h"
#include "LL1.hpp"

// ESTRUTURA DO NÓ DA ÁRVORE SINTÁTICA (Item c.iv)
struct TreeNode {
    std::string name;
    std::vector<TreeNode*> children;

    TreeNode(std::string val) : name(val) {}

    // Destrutor para evitar vazamento de memória (Memory Leak)
    ~TreeNode() {
        for (TreeNode* child : children) {
            delete child;
        }
    }
};

// Token usado internamente pelo parser. É uma cópia do Token do scanner
// (que usa char[40]) convertida para std::string, facilitando a manipulação.
struct ParserToken {
    TipoDeToken tipo;
    std::string valor;
    int linha;
};

class ParserManager {
private:
    LL1Table tableManager;
    std::map<std::string, NonTerminal> strToNT = {
        {"E", NT_E}, {"E'", NT_E_LINE}, {"T", NT_T}, {"T'", NT_T_LINE},
        {"F", NT_F}, {"F'", NT_F_LINE}, {"P", NT_P}
    };

    // Converte o tipo de token do scanner para o terminal da tabela LL(1).
    // Retorna false se o token não pertence ao alfabeto da gramática
    // (identificadores, palavras-chave, '<', ';', etc.).
    bool convertTokenToTerminal(TipoDeToken tipo, Terminal& out) {
        switch (tipo) {
            case TOKEN_NUMBER:      // inteiro
            case TOKEN_FLOAT:       out = T_NUM;    return true;  // real
            case TOKEN_PLUS:        out = T_PLUS;   return true;
            case TOKEN_MINUS:       out = T_MINUS;  return true;
            case TOKEN_MULTIPLY:    out = T_MULT;   return true;
            case TOKEN_DIVIDE:      out = T_DIV;    return true;
            case TOKEN_EXPONENT:    out = T_EXP;    return true;
            case TOKEN_PAREN_OPEN:  out = T_LPAREN; return true;
            case TOKEN_PAREN_CLOSE: out = T_RPAREN; return true;
            case TOKEN_EOF:         out = T_DOLLAR; return true;
            default: return false;
        }
    }

    // Nome do terminal como aparece na pilha (RHS das produções)
    std::string terminalName(Terminal t) {
        switch (t) {
            case T_NUM:    return "num";
            case T_PLUS:   return "+";
            case T_MINUS:  return "-";
            case T_MULT:   return "*";
            case T_DIV:    return "/";
            case T_EXP:    return "^";
            case T_LPAREN: return "(";
            case T_RPAREN: return ")";
            case T_DOLLAR: return "$";
        }
        return "?";
    }

    std::string displayValue(const ParserToken& t) {
        return t.tipo == TOKEN_EOF ? "$" : t.valor;
    }

    void printErrorMessage(int code, const std::string& found) {
        std::cout << "\n[ERRO SINTÁTICO DETECTADO]: ";
        if (code == 1) std::cout << "Expressão mal formada. Esperado número ou '(' mas recebeu '" << found << "'.\n";
        else if (code == 2) std::cout << "Operador deslocado ou duplicado. Recebeu '" << found << "'.\n";
        else if (code == 3) std::cout << "Operando faltante após operador matemático. Encontrado '" << found << "'.\n";
        else if (code == 5) std::cout << "Parênteses não balanceado ou fechado incorretamente em '" << found << "'.\n";
    }

    // AUXILIARES DE FORMATAÇÃO (Item c.iii)
    std::string formatarPilha(std::stack<TreeNode*> p) {
        std::string res = "";
        while (!p.empty()) {
            res = p.top()->name + " " + res;
            p.pop();
        }
        return res;
    }

    std::string formatarEntradaRestante(const std::vector<ParserToken>& tokens, size_t atual_idx) {
        std::string res = "";
        for (size_t i = atual_idx; i < tokens.size(); ++i) {
            if (tokens[i].tipo == TOKEN_EOF) {
                res += "$";
            } else {
                res += tokens[i].valor + " ";
            }
        }
        return res;
    }

    // EXIBIÇÃO DA ÁRVORE GRÁFICA (Item c.iv)
    void printTree(TreeNode* root, const std::string& prefix = "", bool isLast = true) {
        if (root == nullptr) return;

        std::cout << prefix;
        std::cout << (isLast ? "└── " : "├── ");
        std::cout << root->name << "\n";

        std::string newPrefix = prefix + (isLast ? "    " : "│   ");

        for (size_t i = 0; i < root->children.size(); ++i) {
            bool lastChild = (i == root->children.size() - 1);
            printTree(root->children[i], newPrefix, lastChild);
        }
    }

public:
    // Códigos de retorno: distinguem o tipo de falha para quem chama
    enum Resultado { OK = 0, ERRO_LEXICO = 1, ERRO_SINTATICO = 2 };

    // INTEGRAÇÃO COM O SCANNER (arquivo já carregado com scanner.loadFile).
    //  Fase 1 (léxica): o scanner é executado até o EOF. Se houver QUALQUER
    //          TOKEN_ERROR, todos os erros léxicos são listados e o parser
    //          NÃO é executado.
    //  Fase 2 (sintática): só roda se a fase 1 não encontrou erros.
    // O scanner não é alterado; comentários são descartados aqui, pois não
    // fazem parte da gramática.
    Resultado analisar(Scanner& scanner) {
        std::vector<ParserToken> tokens;
        std::vector<ParserToken> errosLexicos;

        std::cout << "=== ANÁLISE LÉXICA ===\n";

        while (true) {
            Token t = scanner.nextToken();

            // Exibição da fase léxica (mesmo formato do main original)
            if (t.tipo != TOKEN_EOF) {
                std::cout << "Token: " << t.lexema << " Tipo: " << tokenTypeToString(t.tipo)
                          << " Linha: " << t.linha << std::endl;
            }

            if (t.tipo == TOKEN_COMMENT) continue;

            ParserToken pt = {t.tipo, std::string(t.lexema), t.linha};
            if (t.tipo == TOKEN_ERROR) errosLexicos.push_back(pt);
            tokens.push_back(pt);

            if (t.tipo == TOKEN_EOF) break;
        }

        std::cout << "\n=== ANÁLISE SINTÁTICA (LL(1)) ===";

        if (!errosLexicos.empty()) {
            std::cout << "\n";
            for (const auto& e : errosLexicos) {
                std::cout << "[ERRO LÉXICO] (linha " << e.linha
                          << "): símbolo/lexema inválido '" << e.valor << "'\n";
            }
            std::cout << "Análise sintática não executada devido a erro(s) léxico(s).\n";
            return ERRO_LEXICO;
        }

        runParser(tokens);
        return sintaticoOk ? OK : ERRO_SINTATICO;
    }

private:
    bool sintaticoOk = true;

public:
    void runParser(const std::vector<ParserToken>& tokens) {
        sintaticoOk = true;
        // Inicializa a pilha de controle apontando para os nós estruturais
        std::stack<TreeNode*> pilha;

        TreeNode* marcadorFim = new TreeNode("$");
        TreeNode* raiz = new TreeNode("E");

        pilha.push(marcadorFim);
        pilha.push(raiz);

        size_t idx = 0;

        std::cout << "\n---------------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(30) << "PILHA"
                  << " | " << std::left << std::setw(25) << "ENTRADA"
                  << " | " << "AÇÃO APLICADA\n";
        std::cout << "---------------------------------------------------------------------------------------\n";

        while (!pilha.empty()) {
            TreeNode* topoNo = pilha.top();
            std::string topoNome = topoNo->name;
            const ParserToken& t_atual = tokens[idx];

            Terminal t;
            if (!convertTokenToTerminal(t_atual.tipo, t)) {
                std::cout << "\n[ERRO SINTÁTICO] (linha " << t_atual.linha
                          << "): Token '" << t_atual.valor << "' ("
                          << tokenTypeToString(t_atual.tipo)
                          << ") não pertence à gramática de expressões.\n";
                delete raiz;
                delete marcadorFim;
                sintaticoOk = false;
                return;
            }

            std::string term_str = terminalName(t);

            // Print de rastreamento do passo atual (Item c.iii)
            std::cout << std::left << std::setw(30) << formatarPilha(pilha)
                      << " | " << std::left << std::setw(25) << formatarEntradaRestante(tokens, idx) << " | ";

            if (topoNome == term_str) {
                std::cout << "Casar terminal '" << topoNome << "'\n";
                pilha.pop();
                idx++;
            }
            else if (strToNT.find(topoNome) != strToNT.end()) {
                NonTerminal nt = strToNT[topoNome];
                Production prod = tableManager.table[nt][t];

                if (prod.error_code > 0) {
                    std::cout << "Erro Encontrado.\n";
                    std::cout << "(linha " << t_atual.linha << ")";
                    printErrorMessage(prod.error_code, displayValue(t_atual));
                    delete raiz;
                    delete marcadorFim;
                    sintaticoOk = false;
                return;
                }

                std::cout << "Aplicar " << topoNome << " -> ";
                if (prod.rhs.empty()) {
                    std::cout << "ε\n";
                } else {
                    for (const auto& s : prod.rhs) std::cout << s << " ";
                    std::cout << "\n";
                }

                pilha.pop();

                if (!prod.rhs.empty()) {
                    std::vector<TreeNode*> filhos_locais(prod.rhs.size());

                    // 1. Cria os nós filhos mantendo a ordem sequencial da gramática
                    for (size_t i = 0; i < prod.rhs.size(); ++i) {
                        filhos_locais[i] = new TreeNode(prod.rhs[i]);
                    }

                    // 2. Anexa os nós ao pai da esquerda para a direita (ordem visual)
                    for (auto* filho : filhos_locais) {
                        topoNo->children.push_back(filho);
                    }

                    // 3. Empilha da direita para a esquerda (ordem de processamento)
                    for (auto it = filhos_locais.rbegin(); it != filhos_locais.rend(); ++it) {
                        pilha.push(*it);
                    }
                } else {
                    // Adiciona ε explicitamente na árvore para visualização
                    topoNo->children.push_back(new TreeNode("ε"));
                }
            }
            else {
                std::cout << "Erro detectado.\n";
                std::cout << "\n[ERRO SINTÁTICO] (linha " << t_atual.linha << "): Esperado '"
                          << topoNome << "' mas encontrou '" << displayValue(t_atual) << "'.\n";
                delete raiz;
                delete marcadorFim;
                sintaticoOk = false;
                return;
            }
        }

        std::cout << "---------------------------------------------------------------------------------------\n";
        std::cout << "\nSucesso! A expressão passou perfeitamente pela análise léxica e sintática.\n";

        // IMPRESSÃO DA ÁRVORE RESULTANTE (Item c.iv)
        std::cout << "\n=== ÁRVORE DE ANÁLISE SINTÁTICA RESULTANTE (1.c.iv) ===\n";
        printTree(raiz);
        std::cout << "=======================================================\n";

        // Limpeza recursiva da memória alocada para os nós da árvore
        delete raiz;
        delete marcadorFim;
    }
};

#endif // PARSER_MANAGER_HPP