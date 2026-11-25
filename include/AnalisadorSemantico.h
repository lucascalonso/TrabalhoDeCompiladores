#ifndef ANALISADOR_SEMANTICO_H
#define ANALISADOR_SEMANTICO_H

#include "AST.h"
#include "TabelaDeSimbolos.h"
#include <string>

class AnalisadorSemantico {
    private:
        TabelaDeSimbolos tabelaDeSimbolos;
        int errosSemanticos = 0;
        int avisos = 0;

        TipoDado tipoRetornoAtual = TIPO_VOID;
        std::string nomeFuncaoAtual = "";
        bool ignorarProximoEscopoComposto = false;

        void reportarErro(int linha, const std::string& mensagem, const std::string& sugestao = "");
        void reportarAviso(int linha, const std::string& mensagem);
        std::string sugerirIdentificadorSimilar(const std::string& nome) const;

        void inicializarFuncoesPredefinidas();
        void analisarNo(ASTNode* no);

        void analisarVarDecl(ASTNode* no);
        void analisarFunDecl(ASTNode* no);
        void analisarComposto(ASTNode* no);
        void analisarIf(ASTNode* no);
        void analisarWhile(ASTNode* no);
        void analisarReturn(ASTNode* no);
        void analisarAssign(ASTNode* no);
        void analisarOp(ASTNode* no);
        void analisarVar(ASTNode* no);
        void analisarNum(ASTNode* no);
        void analisarCall(ASTNode* no);

        int calcularOperacaoConstante(int op, int valor1, int valor2);

    public:
        AnalisadorSemantico();
        void analisar(ASTNode* raiz);
        void imprimirTabelaDeSimbolos() const;
        int getErrosSemanticos() const { return errosSemanticos; }
        int getAvisos() const { return avisos; }
};

#endif // ANALISADOR_SEMANTICO_H
