#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

enum TipoDado {
    TIPO_VOID,
    TIPO_INT,
    TIPO_INT_ARRAY,
    TIPO_ERRO
};

enum NodeKind {
    NODE_VAR_DECL,
    NODE_FUN_DECL,
    NODE_PARAM,
    NODE_COMPOSTO,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_ASSIGN,
    NODE_OP,
    NODE_VAR,
    NODE_NUM,
    NODE_CALL
};

struct ASTNode {
    NodeKind kind;
    int linha;

    // Estrutura da árvore
    std::vector<ASTNode*> filhos;
    ASTNode* irmao = nullptr;

    // Dados léxicos / sintáticos
    std::string nome;
    int op = 0;                 // Token do operador (SOMA, SUB, LT, etc.)
    TipoDado tipo_decl = TIPO_VOID; // Tipo explícito na declaração (int/void)
    bool eh_array = false;      // Indica se declaração/param/var tem []
    int tamanho_array = 0;      // Tamanho declarado em int v[N]

    // Atributos Semânticos (Slide 2 e Slide 9)
    TipoDado tipo = TIPO_VOID;  // Sintetizado: tipo inferido/calculado
    int valor = 0;              // Sintetizado: valor estático (se eh_constante)
    bool eh_constante = false;  // Indica se 'valor' é conhecido em compilação
    bool eh_lvalue = false;     // Sintetizado: se pode ficar à esquerda de '='
    bool simplificado = false;  // Indica se sofreu Constant Folding (Slide 14)

    int escopo = 0;             // Herdado: nivel do escopo ativo

    ASTNode(NodeKind k, int l) : kind(k), linha(l) {}
};

const char* tipoDadoToString(TipoDado t);
void imprimirAST(ASTNode* raiz, int nivel = 0);
void liberarAST(ASTNode* raiz);

#endif // AST_H
