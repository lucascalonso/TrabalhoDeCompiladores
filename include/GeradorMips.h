#ifndef GERADOR_MIPS_H
#define GERADOR_MIPS_H

#include "AST.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

struct LocalVarInfo {
    int offset;         // Deslocamento relativo a $fp (positivo = param, negativo = local)
    bool eh_array;      // true se for vetor
    bool eh_param;      // true se for parametro (vetor passado por referencia)
};

class GeradorMIPS {
private:
    std::ofstream out;
    int labelCount = 0;
    int offsetLocalAtual = 0;
    std::string funcaoAtual;

    // Pilha de escopos locais (escopo 0 da funcao = params + locals de nivel 1, etc.)
    std::vector<std::unordered_map<std::string, LocalVarInfo>> escoposLocais;
    // Conjunto de variaveis/vetores globais declarados
    std::unordered_map<std::string, bool> globais; // nome -> eh_array

    std::string novoLabel(const std::string& prefixo = "L");
    void entrarEscopoLocal();
    void sairEscopoLocal();
    LocalVarInfo* buscarLocal(const std::string& nome);

    int calcularEspacoLocais(ASTNode* no);
    void gerarGlobais(ASTNode* raiz);
    void gerarFuncoes(ASTNode* raiz);
    void gerarFuncao(ASTNode* funNode);

    void gerarStatement(ASTNode* stmt);
    void gerarComposto(ASTNode* bloco, bool novoEscopo = true);
    void gerarIf(ASTNode* ifNode);
    void gerarWhile(ASTNode* whileNode);
    void gerarReturn(ASTNode* retNode);

    void gerarExpressao(ASTNode* expr);
    void gerarEnderecoVar(ASTNode* varNode);
    void gerarAssign(ASTNode* assignNode);
    void gerarOp(ASTNode* opNode);
    void gerarCall(ASTNode* callNode);

public:
    bool gerar(ASTNode* raiz, const std::string& arquivoSaida);
};

#endif // GERADOR_MIPS_H
