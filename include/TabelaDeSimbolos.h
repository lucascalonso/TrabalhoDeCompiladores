#ifndef TABELA_DE_SIMBOLOS_H
#define TABELA_DE_SIMBOLOS_H

#include "AST.h"
#include <string>
#include <vector>
#include <unordered_map>

enum CategoriaSimbolo {
    CAT_VAR,
    CAT_ARRAY,
    CAT_FUN
};

struct SimboloInfo {
    std::string nome;
    std::string escopo;
    CategoriaSimbolo categoria;
    TipoDado tipo;
    std::vector<TipoDado> tipos_parametros;
    int linha_decl;
    std::vector<int> linhas_usos;
};

class TabelaDeSimbolos {
    private:
        std::vector<std::unordered_map<std::string, SimboloInfo>> escopos;
        std::vector<std::string> nomesEscopos;
        std::vector<SimboloInfo> historicoSimbolos; // Guarda símbolos de blocos já encerrados

    public:
        TabelaDeSimbolos();
        void entrarEscopo(const std::string& nomeEscopo = "bloco");
        void sairEscopo();
        int escopoAtual() const;

        bool adicionarSimbolo(SimboloInfo simbolo);
        SimboloInfo* buscarSimbolo(const std::string& nome);
        SimboloInfo* buscarSimboloNoEscopoAtual(const std::string& nome);

        std::vector<SimboloInfo> obterTodosSimbolosVisiveis() const;
        void imprimirTabela() const;
};

#endif // TABELA_DE_SIMBOLOS_H
