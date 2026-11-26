#include "../include/GeradorMIPS.h"

#include "yacc.tab.h"

#include <iostream>

std::string GeradorMIPS::novoLabel(const std::string & prefixo) {
  return prefixo + "_" + std::to_string(labelCount++);
}

void GeradorMIPS::entrarEscopoLocal() {
  escoposLocais.emplace_back();
}

void GeradorMIPS::sairEscopoLocal() {
  if (!escoposLocais.empty()) {
    escoposLocais.pop_back();
  }
}

LocalVarInfo * GeradorMIPS::buscarLocal(const std::string & nome) {
  for (auto it = escoposLocais.rbegin(); it != escoposLocais.rend(); ++it) {
    auto found = it -> find(nome);
    if (found != it -> end()) {
      return & (found -> second);
    }
  }
  return nullptr;
}

int GeradorMIPS::calcularEspacoLocais(ASTNode * no) {
  int total = 0;
  while (no != nullptr) {
    if (no -> kind == NODE_VAR_DECL) {
      int tam = no -> eh_array ? (no -> tamanho_array * 4) : 4;
      total += tam;
    } else if (no -> kind == NODE_COMPOSTO) {
      if (no -> filhos.size() > 0) total += calcularEspacoLocais(no -> filhos.at(0));
      if (no -> filhos.size() > 1) total += calcularEspacoLocais(no -> filhos.at(1));
    } else if (no -> kind == NODE_IF || no -> kind == NODE_WHILE) {
      for (size_t i = 1; i < no -> filhos.size(); ++i) {
        total += calcularEspacoLocais(no -> filhos.at(i));
      }
    }
    no = no -> irmao;
  }
  return total;
}

bool GeradorMIPS::gerar(ASTNode * raiz,
  const std::string & arquivoSaida) {
  out.open(arquivoSaida);
  if (!out.is_open()) {
    std::cerr << "Erro ao criar arquivo de saida MIPS: " << arquivoSaida << std::endl;
    return false;
  }

  out << "# Codigo MIPS gerado pelo compilador C-\n";
  gerarGlobais(raiz);

  out << "\n.text\n";
  out << ".globl main\n";
  out << "    j main\n\n";

  gerarFuncoes(raiz);

  out.close();
  return true;
}

void GeradorMIPS::gerarGlobais(ASTNode * raiz) {
  out << ".data\n";
  out << "    _newline: .asciiz \"\\n\"\n";
  out << "    .align 2\n";

  ASTNode * atual = raiz;
  while (atual != nullptr) {
    if (atual -> kind == NODE_VAR_DECL) {
      globais[atual -> nome] = atual -> eh_array;
      if (atual -> eh_array) {
        out << "    " << atual -> nome << ": .space " << (atual -> tamanho_array * 4) << "\n";
      } else {
        out << "    " << atual -> nome << ": .word 0\n";
      }
    }
    atual = atual -> irmao;
  }
}

void GeradorMIPS::gerarFuncoes(ASTNode * raiz) {
  ASTNode * atual = raiz;
  while (atual != nullptr) {
    if (atual -> kind == NODE_FUN_DECL) {
      gerarFuncao(atual);
    }
    atual = atual -> irmao;
  }
}

void GeradorMIPS::gerarFuncao(ASTNode * funNode) {
  funcaoAtual = funNode -> nome;
  escoposLocais.clear();
  entrarEscopoLocal();
  offsetLocalAtual = 0;

  ASTNode * param = (funNode -> filhos.size() > 0) ? funNode -> filhos.at(0) : nullptr;
  int paramOffset = 8;
  while (param != nullptr) {
    LocalVarInfo info;
    info.offset = paramOffset;
    info.eh_array = param -> eh_array;
    info.eh_param = true;
    escoposLocais.back()[param -> nome] = info;

    paramOffset += 4;
    param = param -> irmao;
  }

  ASTNode * corpo = (funNode -> filhos.size() > 1) ? funNode -> filhos.at(1) : nullptr;
  int espacoLocais = calcularEspacoLocais(corpo);

  out << "# Funcao: " << funcaoAtual << "\n";
  out << funcaoAtual << ":\n";
  out << "    subu $sp, $sp, 8\n";
  out << "    sw $fp, 4($sp)\n";
  out << "    sw $ra, 0($sp)\n";
  out << "    move $fp, $sp\n";
  if (espacoLocais > 0) {
    out << "    subu $sp, $sp, " << espacoLocais << "\n";
  }

  if (corpo) {
    gerarComposto(corpo, false);
  }

  out << funcaoAtual << "_fim:\n";
  if (funcaoAtual == "main") {
    out << "    li $v0, 10\n";
    out << "    syscall\n\n";
  } else {
    out << "    move $sp, $fp\n";
    out << "    lw $ra, 0($sp)\n";
    out << "    lw $fp, 4($sp)\n";
    out << "    addu $sp, $sp, 8\n";
    out << "    jr $ra\n\n";
  }

  sairEscopoLocal();
}

void GeradorMIPS::gerarComposto(ASTNode * bloco, bool novoEscopo) {
  if (!bloco) return;
  if (novoEscopo) entrarEscopoLocal();

  ASTNode * decl = (bloco -> filhos.size() > 0) ? bloco -> filhos.at(0) : nullptr;
  while (decl != nullptr) {
    if (decl -> kind == NODE_VAR_DECL) {
      int bytes = decl -> eh_array ? (decl -> tamanho_array * 4) : 4;
      offsetLocalAtual -= bytes;

      LocalVarInfo info;
      info.offset = offsetLocalAtual;
      info.eh_array = decl -> eh_array;
      info.eh_param = false;
      escoposLocais.back()[decl -> nome] = info;
    }
    decl = decl -> irmao;
  }

  ASTNode * stmt = (bloco -> filhos.size() > 1) ? bloco -> filhos.at(1) : nullptr;
  while (stmt != nullptr) {
    gerarStatement(stmt);
    stmt = stmt -> irmao;
  }

  if (novoEscopo) sairEscopoLocal();
}

void GeradorMIPS::gerarStatement(ASTNode * stmt) {
  if (!stmt) return;
  switch (stmt -> kind) {
  case NODE_COMPOSTO:
    gerarComposto(stmt, true);
    break;
  case NODE_IF:
    gerarIf(stmt);
    break;
  case NODE_WHILE:
    gerarWhile(stmt);
    break;
  case NODE_RETURN:
    gerarReturn(stmt);
    break;
  case NODE_ASSIGN:
  case NODE_OP:
  case NODE_VAR:
  case NODE_NUM:
  case NODE_CALL:
    gerarExpressao(stmt);
    break;
  default:
    break;
  }
}

void GeradorMIPS::gerarIf(ASTNode * ifNode) {
  if (ifNode -> filhos.size() < 2) return;

  std::string labelElse = novoLabel("L_else");
  std::string labelFim = novoLabel("L_endif");

  gerarExpressao(ifNode -> filhos.at(0));
  out << "    beq $t0, $zero, " << (ifNode -> filhos.size() == 3 ? labelElse : labelFim) << "\n";

  gerarStatement(ifNode -> filhos.at(1));

  if (ifNode -> filhos.size() == 3) {
    out << "    j " << labelFim << "\n";
    out << labelElse << ":\n";
    gerarStatement(ifNode -> filhos.at(2));
  }
  out << labelFim << ":\n";
}

void GeradorMIPS::gerarWhile(ASTNode * whileNode) {
  if (whileNode -> filhos.size() < 2) return;

  std::string labelIni = novoLabel("L_while");
  std::string labelFim = novoLabel("L_endwhile");

  out << labelIni << ":\n";
  gerarExpressao(whileNode -> filhos.at(0));
  out << "    beq $t0, $zero, " << labelFim << "\n";

  gerarStatement(whileNode -> filhos.at(1));
  out << "    j " << labelIni << "\n";
  out << labelFim << ":\n";
}

void GeradorMIPS::gerarReturn(ASTNode * retNode) {
  if (!retNode -> filhos.empty() && retNode -> filhos.at(0) != nullptr) {
    gerarExpressao(retNode -> filhos.at(0));
    out << "    move $v0, $t0\n";
  }
  out << "    j " << funcaoAtual << "_fim\n";
}

void GeradorMIPS::gerarEnderecoVar(ASTNode * varNode) {
  LocalVarInfo * local = buscarLocal(varNode -> nome);

  if (local != nullptr) {
    if (local -> eh_array && local -> eh_param) {
      out << "    lw $t2, " << local -> offset << "($fp)\n";
    } else {
      out << "    addiu $t2, $fp, " << local -> offset << "\n";
    }
  } else {
    out << "    la $t2, " << varNode -> nome << "\n";
  }

  if (varNode -> eh_array && !varNode -> filhos.empty()) {
    out << "    subu $sp, $sp, 4\n";
    out << "    sw $t2, 0($sp)\n";

    gerarExpressao(varNode -> filhos.at(0));

    out << "    lw $t2, 0($sp)\n";
    out << "    addu $sp, $sp, 4\n";
    out << "    sll $t0, $t0, 2\n";
    out << "    addu $t2, $t2, $t0\n";
  }
}

void GeradorMIPS::gerarExpressao(ASTNode * expr) {
  if (!expr) return;

  switch (expr -> kind) {
  case NODE_NUM:
    out << "    li $t0, " << expr -> valor << "\n";
    break;

  case NODE_VAR:
    gerarEnderecoVar(expr);
    if (expr -> tipo == TIPO_INT_ARRAY) {
      out << "    move $t0, $t2\n";
    } else {
      out << "    lw $t0, 0($t2)\n";
    }
    break;

  case NODE_ASSIGN:
    gerarAssign(expr);
    break;

  case NODE_OP:
    gerarOp(expr);
    break;

  case NODE_CALL:
    gerarCall(expr);
    break;

  default:
    break;
  }
}

void GeradorMIPS::gerarAssign(ASTNode * assignNode) {
  ASTNode * varNode = assignNode -> filhos.at(0);
  ASTNode * rhsNode = assignNode -> filhos.at(1);

  gerarExpressao(rhsNode);
  out << "    subu $sp, $sp, 4\n";
  out << "    sw $t0, 0($sp)\n";

  gerarEnderecoVar(varNode);

  out << "    lw $t0, 0($sp)\n";
  out << "    addu $sp, $sp, 4\n";
  out << "    sw $t0, 0($t2)\n";
}

void GeradorMIPS::gerarOp(ASTNode * opNode) {
  gerarExpressao(opNode -> filhos.at(0));
  out << "    subu $sp, $sp, 4\n";
  out << "    sw $t0, 0($sp)\n";

  gerarExpressao(opNode -> filhos.at(1));

  out << "    lw $t1, 0($sp)\n";
  out << "    addu $sp, $sp, 4\n";

  switch (opNode -> op) {
  case SOMA:
    out << "    add $t0, $t1, $t0\n";
    break;
  case SUB:
    out << "    sub $t0, $t1, $t0\n";
    break;
  case MULT:
    out << "    mul $t0, $t1, $t0\n";
    break;
  case DIV:
    out << "    div $t1, $t0\n";
    out << "    mflo $t0\n";
    break;
  case LT:
    out << "    slt $t0, $t1, $t0\n";
    break;
  case LEQ:
    out << "    sle $t0, $t1, $t0\n";
    break;
  case GT:
    out << "    sgt $t0, $t1, $t0\n";
    break;
  case GEQ:
    out << "    sge $t0, $t1, $t0\n";
    break;
  case EQ:
    out << "    seq $t0, $t1, $t0\n";
    break;
  case NEQ:
    out << "    sne $t0, $t1, $t0\n";
    break;
  default:
    break;
  }
}

void GeradorMIPS::gerarCall(ASTNode * callNode) {
  if (callNode -> nome == "input") {
    out << "    li $v0, 5\n";
    out << "    syscall\n";
    out << "    move $t0, $v0\n";
    return;
  }

  if (callNode -> nome == "output") {
    if (!callNode -> filhos.empty() && callNode -> filhos.at(0) != nullptr) {
      gerarExpressao(callNode -> filhos.at(0));
      out << "    move $a0, $t0\n";
      out << "    li $v0, 1\n";
      out << "    syscall\n";
      out << "    la $a0, _newline\n";
      out << "    li $v0, 4\n";
      out << "    syscall\n";
    }
    return;
  }
  std::vector < ASTNode * > args;
  ASTNode * arg = (!callNode -> filhos.empty()) ? callNode -> filhos.at(0) : nullptr;
  while (arg != nullptr) {
    args.push_back(arg);
    arg = arg -> irmao;
  }
  int qtdArgs = (int) args.size();
  for (int i = qtdArgs - 1; i >= 0; --i) {
    gerarExpressao(args.at(i));
    out << "    subu $sp, $sp, 4\n";
    out << "    sw $t0, 0($sp)\n";
  }
  out << "    jal " << callNode -> nome << "\n";
  if (!args.empty()) {
    out << "    addu $sp, $sp, " << (args.size() * 4) << "\n";
  }
  out << "    move $t0, $v0\n";
}