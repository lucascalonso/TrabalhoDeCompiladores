#include "../include/TabelaDeSimbolos.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

TabelaDeSimbolos::TabelaDeSimbolos() {
    entrarEscopo("global");
}

void TabelaDeSimbolos::entrarEscopo(const std::string& nomeEscopo) {
    escopos.emplace_back();
    nomesEscopos.push_back(nomeEscopo);
}

void TabelaDeSimbolos::sairEscopo() {
    if (!escopos.empty()) {
        // Salva no histórico antes de destruir o escopo (preservando linhas_usos)
        for (const auto& par : escopos.back()) {
            historicoSimbolos.push_back(par.second);
        }
        escopos.pop_back();
        nomesEscopos.pop_back();
    }
}

int TabelaDeSimbolos::escopoAtual() const {
    return static_cast<int>(escopos.size()) - 1;
}

bool TabelaDeSimbolos::adicionarSimbolo(SimboloInfo simbolo) {
    auto& topo = escopos.back();
    if (topo.find(simbolo.nome) != topo.end()) {
        return false;
    }
    simbolo.escopo = nomesEscopos.back();
    topo[simbolo.nome] = simbolo;
    return true;
}

SimboloInfo* TabelaDeSimbolos::buscarSimbolo(const std::string& nome) {
    for (auto it = escopos.rbegin(); it != escopos.rend(); ++it) {
        auto found = it->find(nome);
        if (found != it->end()) {
            return &(found->second);
        }
    }
    return nullptr;
}

SimboloInfo* TabelaDeSimbolos::buscarSimboloNoEscopoAtual(const std::string& nome) {
    auto& topo = escopos.back();
    auto found = topo.find(nome);
    if (found != topo.end()) {
        return &(found->second);
    }
    return nullptr;
}

std::vector<SimboloInfo> TabelaDeSimbolos::obterTodosSimbolosVisiveis() const {
    std::vector<SimboloInfo> visiveis;
    for (const auto& escopo : escopos) {
        for (const auto& par : escopo) {
            visiveis.push_back(par.second);
        }
    }
    return visiveis;
}

static const char* categoriaToString(CategoriaSimbolo cat) {
    switch (cat) {
        case CAT_VAR:   return "Variavel";
        case CAT_ARRAY: return "Vetor";
        case CAT_FUN:   return "Funcao";
    }
    return "-";
}

void TabelaDeSimbolos::imprimirTabela() const {
    std::vector<SimboloInfo> todos = historicoSimbolos;
    for (const auto& escopo : escopos) {
        for (const auto& par : escopo) {
            todos.push_back(par.second);
        }
    }

    std::sort(todos.begin(), todos.end(), [](const SimboloInfo& a, const SimboloInfo& b) {
        if (a.linha_decl != b.linha_decl) return a.linha_decl < b.linha_decl;
        return a.nome < b.nome;
    });

    std::cout << "\n--- TABELA DE SIMBOLOS ---\n";
    std::cout << std::left
              << std::setw(16) << "Nome"
              << std::setw(15) << "Escopo"
              << std::setw(12) << "Categoria"
              << std::setw(10) << "Tipo"
              << std::setw(12) << "Decl."
              << "Linhas de Uso\n";
    std::cout << std::string(78, '-') << "\n";

    for (const auto& s : todos) {
        std::cout << std::left
                  << std::setw(16) << s.nome
                  << std::setw(15) << s.escopo
                  << std::setw(12) << categoriaToString(s.categoria)
                  << std::setw(10) << tipoDadoToString(s.tipo)
                  << std::setw(12) << (s.linha_decl == 0 ? "nativa" : std::to_string(s.linha_decl));

        if (s.linhas_usos.empty()) {
            std::cout << "-";
        } else {
            for (size_t i = 0; i < s.linhas_usos.size(); ++i) {
                std::cout << s.linhas_usos[i] << (i + 1 < s.linhas_usos.size() ? ", " : "");
            }
        }
        std::cout << "\n";
    }
}
