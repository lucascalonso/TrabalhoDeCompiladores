#include "../include/AST.h"
#include "yacc.tab.h"

const char* tipoDadoToString(TipoDado t) {
    switch (t) {
        case TIPO_VOID:      return "void";
        case TIPO_INT:       return "int";
        case TIPO_INT_ARRAY: return "int[]";
        case TIPO_ERRO:      return "erro";
    }
    return "desconhecido";
}

static const char* operadorToString(int op) {
    switch (op) {
        case SOMA: return "+";
        case SUB:  return "-";
        case MULT: return "*";
        case DIV:  return "/";
        case LEQ:  return "<=";
        case LT:   return "<";
        case GEQ:  return ">=";
        case GT:   return ">";
        case EQ:   return "==";
        case NEQ:  return "!=";
        default:   return "?";
    }
}

void imprimirAST(ASTNode* no, int nivel) {
    while (no != nullptr) {
        for (int i = 0; i < nivel; ++i) std::cout << "  ";

        if (no->kind == NODE_COMPOSTO) {
            std::cout << "[Bloco] (Linha " << no->linha << ") {\n";
            for (ASTNode* filho : no->filhos) {
                imprimirAST(filho, nivel + 1);
            }
            for (int i = 0; i < nivel; ++i) std::cout << "  ";
            std::cout << "}\n";
            no = no->irmao;
            continue;
        }

        switch (no->kind) {
            case NODE_VAR_DECL:
                std::cout << "[VarDecl] " << no->nome << " : " << tipoDadoToString(no->tipo);
                if (no->eh_array) std::cout << " (tam=" << no->tamanho_array << ")";
                break;
            case NODE_FUN_DECL:
                std::cout << "[FunDecl] " << no->nome << " : " << tipoDadoToString(no->tipo);
                break;
            case NODE_PARAM:
                std::cout << "[Param] " << no->nome << " : " << tipoDadoToString(no->tipo);
                break;
            case NODE_IF:
                std::cout << "[If]";
                break;
            case NODE_WHILE:
                std::cout << "[While]";
                break;
            case NODE_RETURN:
                std::cout << "[Return] tipo=" << tipoDadoToString(no->tipo);
                break;
            case NODE_ASSIGN:
                std::cout << "[Assign] tipo=" << tipoDadoToString(no->tipo);
                break;
            case NODE_OP:
                std::cout << "[Op(" << operadorToString(no->op) << ")] tipo=" << tipoDadoToString(no->tipo);
                break;
            case NODE_VAR:
                std::cout << "[Var] " << no->nome << " (tipo=" << tipoDadoToString(no->tipo)
                          << ", lvalue=" << (no->eh_lvalue ? "true" : "false") << ")";
                break;
            case NODE_NUM:
                std::cout << "[Num] " << no->valor << " (tipo=" << tipoDadoToString(no->tipo)
                          << ", const=true" << (no->simplificado ? ", SIMPLIFICADO" : "") << ")";
                break;
            case NODE_CALL:
                std::cout << "[Call] " << no->nome << " (tipo=" << tipoDadoToString(no->tipo) << ")";
                break;
            default:
                break;
        }
        std::cout << " (Linha " << no->linha << ")\n";

        for (ASTNode* filho : no->filhos) {
            imprimirAST(filho, nivel + 1);
        }
        no = no->irmao;
    }
}

void liberarAST(ASTNode* no) {
    if (!no) return;
    for (ASTNode* filho : no->filhos) {
        liberarAST(filho);
    }
    liberarAST(no->irmao);
    delete no;
}
