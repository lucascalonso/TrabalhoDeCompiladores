#ifndef PARSER_MANAGER_HPP
#define PARSER_MANAGER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <iomanip>
#include "lexer.hpp"
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

class ParserManager {
private:
    LL1Table tableManager;
    std::map<std::string, NonTerminal> strToNT = {
        {"E", NT_E}, {"E'", NT_E_LINE}, {"T", NT_T}, {"T'", NT_T_LINE},
        {"F", NT_F}, {"F'", NT_F_LINE}, {"P", NT_P}
    };

    Terminal convertTokenToTerminal(TokenType type) {
        switch (type) {
            case TokenType::NUM: return T_NUM;
            case TokenType::PLUS: return T_PLUS;
            case TokenType::MINUS: return T_MINUS;
            case TokenType::MULT: return T_MULT;
            case TokenType::DIV: return T_DIV;
            case TokenType::EXP: return T_EXP;
            case TokenType::L_PAREN: return T_LPAREN;
            case TokenType::R_PAREN: return T_RPAREN;
            case TokenType::END_OF_INPUT: return T_DOLLAR;
            default: return T_DOLLAR;
        }
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

    std::string formatarEntradaRestante(const std::vector<Token>& tokens, size_t atual_idx) {
        std::string res = "";
        for (size_t i = atual_idx; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::END_OF_INPUT) {
                res += "$";
            } else {
                res += tokens[i].value + " ";
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
    void runParser(const std::vector<Token>& tokens) {
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
            Token t_atual = tokens[idx];

            if (t_atual.type == TokenType::ERROR) {
                std::cout << "\n[ERRO LÉXICO]: Caractere inválido ou número mal formatado: " << t_atual.value << "\n";
                delete raiz;
                delete marcadorFim;
                return;
            }

            std::string term_str = (t_atual.type == TokenType::END_OF_INPUT) ? "$" : t_atual.value;
            if (t_atual.type == TokenType::NUM) term_str = "num";

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
                Terminal t = convertTokenToTerminal(t_atual.type);
                Production prod = tableManager.table[nt][t];

                if (prod.error_code > 0) {
                    std::cout << "Erro Encontrado.\n";
                    printErrorMessage(prod.error_code, t_atual.value);
                    delete raiz;
                    delete marcadorFim;
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

                // CORREÇÃO: Alocação e empilhamento controlados para respeitar a ordem top-down na árvore
                if (!prod.rhs.empty()) {
                    std::vector<TreeNode*> filhos_locais(prod.rhs.size());
                    
                    // 1. Cria os nós filhos mantendo a ordem sequencial da gramática
                    for (size_t i = 0; i < prod.rhs.size(); ++i) {
                        filhos_locais[i] = new TreeNode(prod.rhs[i]);
                    }

                    // 2. Anexa os nós criados ao nó pai da esquerda para a direita (Visual correto)
                    for (auto* filho : filhos_locais) {
                        topoNo->children.push_back(filho);
                    }

                    // 3. Empilha os nós localmente da direita para a esquerda (Ordem correta de processamento na pilha)
                    for (auto it = filhos_locais.rbegin(); it != filhos_locais.rend(); ++it) {
                        pilha.push(*it);
                    }
                } else {
                    // Adiciona o símbolo de épsilon explicitamente na árvore para visualização
                    topoNo->children.push_back(new TreeNode("ε"));
                }
            } 
            else {
                std::cout << "Erro detectado.\n";
                std::cout << "\n[ERRO SINTÁTICO]: Esperado '" << topoNome << "' mas encontrou '" << t_atual.value << "'.\n";
                delete raiz;
                delete marcadorFim;
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
