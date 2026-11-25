#include "../include/AnalisadorSemantico.h"

#include <iostream>

#include <algorithm>

#include <vector>

extern "C" {
  #include "yacc.tab.h"

}

AnalisadorSemantico::AnalisadorSemantico() {
  inicializarFuncoesPredefinidas();
}

void AnalisadorSemantico::reportarErro(int linha,
  const std::string & mensagem,
    const std::string & sugestao) {
  std::cerr << "Erro Semantico (linha " << linha << "): " << mensagem << std::endl;
  if (!sugestao.empty()) {
    std::cerr << "  -> Sugestao: " << sugestao << std::endl;
  }
  errosSemanticos++;
}

void AnalisadorSemantico::reportarAviso(int linha,
  const std::string & mensagem) {
  std::cerr << "Aviso Semantico (linha " << linha << "): " << mensagem << std::endl;
  avisos++;
}

void AnalisadorSemantico::imprimirTabelaDeSimbolos() const {
  tabelaDeSimbolos.imprimirTabela();
}

static int distanciaLevenshtein(const std::string & s1,
  const std::string & s2) {
  size_t m = s1.size(), n = s2.size();
  std::vector < std::vector < int >> dp(m + 1, std::vector < int > (n + 1, 0));
  for (size_t i = 0; i <= m; ++i) dp[i][0] = static_cast < int > (i);
  for (size_t j = 0; j <= n; ++j) dp[0][j] = static_cast < int > (j);
  for (size_t i = 1; i <= m; ++i) {
    for (size_t j = 1; j <= n; ++j) {
      int custo = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
      dp[i][j] = std::min({
        dp[i - 1][j] + 1,
        dp[i][j - 1] + 1,
        dp[i - 1][j - 1] + custo
      });
    }
  }
  return dp[m][n];
}

std::string AnalisadorSemantico::sugerirIdentificadorSimilar(const std::string & nome) const {
  auto visiveis = tabelaDeSimbolos.obterTodosSimbolosVisiveis();
  std::string melhorNome = "";
  int menorDist = 3; // tolera até 2 caracteres de diferença

  for (const auto & sym: visiveis) {
    int d = distanciaLevenshtein(nome, sym.nome);
    if (d < menorDist) {
      menorDist = d;
      melhorNome = sym.nome;
    }
  }
  if (!melhorNome.empty()) {
    return "Voce quis dizer '" + melhorNome + "'? Caso contrario, declare '" + nome + "' antes de usa-lo.";
  }
  return "Declare '" + nome + "' (ex: 'int " + nome + ";') antes de utiliza-lo.";
}

void AnalisadorSemantico::inicializarFuncoesPredefinidas() {
  SimboloInfo inputInfo;
  inputInfo.nome = "input";
  inputInfo.categoria = CAT_FUN;
  inputInfo.tipo = TIPO_INT;
  inputInfo.linha_decl = 0;
  tabelaDeSimbolos.adicionarSimbolo(inputInfo);

  SimboloInfo outputInfo;
  outputInfo.nome = "output";
  outputInfo.categoria = CAT_FUN;
  outputInfo.tipo = TIPO_VOID;
  outputInfo.tipos_parametros = {
    TIPO_INT
  };
  outputInfo.linha_decl = 0;
  tabelaDeSimbolos.adicionarSimbolo(outputInfo);
}

void AnalisadorSemantico::analisar(ASTNode * raiz) {
  if (!raiz) {
    std::cerr << "Erro: AST vazia." << std::endl;
    return;
  }

  ASTNode * atual = raiz;
  ASTNode * ultimaDecl = nullptr;

  while (atual != nullptr) {
    analisarNo(atual);
    ultimaDecl = atual;
    atual = atual -> irmao;
  }

  SimboloInfo * mainFunc = tabelaDeSimbolos.buscarSimbolo("main");
  if (!mainFunc || mainFunc -> categoria != CAT_FUN) {
    reportarErro(ultimaDecl ? ultimaDecl -> linha : 1,
      "Funcao 'main' nao declarada.",
      "Adicione 'void main(void) { ... }' ao final do programa.");
  } else {
    if (mainFunc -> tipo != TIPO_VOID || !mainFunc -> tipos_parametros.empty()) {
      reportarErro(mainFunc -> linha_decl,
        "Assinatura invalida para a funcao 'main'.",
        "Altere a assinatura para 'void main(void)'.");
    }
    if (!ultimaDecl || ultimaDecl -> kind != NODE_FUN_DECL || ultimaDecl -> nome != "main") {
      reportarErro(mainFunc -> linha_decl,
        "A funcao 'main' deve ser a ultima declaracao do programa.",
        "Mova todas as funcoes e variaveis globais declaradas apos 'main' para antes dela.");
    }
  }
}

void AnalisadorSemantico::analisarNo(ASTNode * no) {
  if (!no) return;

  switch (no -> kind) {
  case NODE_VAR_DECL:
    analisarVarDecl(no);
    break;
  case NODE_FUN_DECL:
    analisarFunDecl(no);
    break;
  case NODE_COMPOSTO:
    analisarComposto(no);
    break;
  case NODE_IF:
    analisarIf(no);
    break;
  case NODE_WHILE:
    analisarWhile(no);
    break;
  case NODE_RETURN:
    analisarReturn(no);
    break;
  case NODE_ASSIGN:
    analisarAssign(no);
    break;
  case NODE_OP:
    analisarOp(no);
    break;
  case NODE_VAR:
    analisarVar(no);
    break;
  case NODE_NUM:
    analisarNum(no);
    break;
  case NODE_CALL:
    analisarCall(no);
    break;
  default:
    reportarErro(no -> linha, "Tipo de no desconhecido na analise semantica.");
    break;
  }
}

void AnalisadorSemantico::analisarVarDecl(ASTNode * no) {
  if (no -> tipo_decl == TIPO_VOID) {
    reportarErro(no -> linha,
      "Variavel '" + no -> nome + "' nao pode ser declarada como 'void'.",
      "Altere o tipo da variavel para 'int' (ex: 'int " + no -> nome + ";').");
    no -> tipo = TIPO_ERRO;
    return;
  }

  if (no -> eh_array && no -> tamanho_array <= 0) {
    reportarErro(no -> linha,
      "Tamanho invalido (" + std::to_string(no -> tamanho_array) + ") para o vetor '" + no -> nome + "'.",
      "Informe um tamanho inteiro positivo maior que zero (ex: 'int " + no -> nome + "[10];').");
    no -> tipo = TIPO_ERRO;
    return;
  }

  SimboloInfo simbolo;
  simbolo.nome = no -> nome;
  simbolo.categoria = no -> eh_array ? CAT_ARRAY : CAT_VAR;
  simbolo.tipo = no -> eh_array ? TIPO_INT_ARRAY : TIPO_INT;
  simbolo.linha_decl = no -> linha;

  SimboloInfo * existente = tabelaDeSimbolos.buscarSimboloNoEscopoAtual(no -> nome);
  if (!tabelaDeSimbolos.adicionarSimbolo(simbolo)) {
    std::string sug = "Remova a declaracao duplicada ou renomeie a variavel";
    if (existente) sug += " (ja declarada na linha " + std::to_string(existente -> linha_decl) + ").";
    reportarErro(no -> linha, "Variavel '" + no -> nome + "' ja declarada neste escopo.", sug);
    no -> tipo = TIPO_ERRO;
    return;
  }

  no -> tipo = simbolo.tipo;
}

void AnalisadorSemantico::analisarFunDecl(ASTNode * no) {
  if (no -> tipo_decl != TIPO_VOID && no -> tipo_decl != TIPO_INT) {
    reportarErro(no -> linha,
      "Tipo de retorno invalido para funcao '" + no -> nome + "'.",
      "Use 'int' ou 'void' como tipo de retorno da funcao.");
    no -> tipo = TIPO_ERRO;
    return;
  }

  SimboloInfo simbolo;
  simbolo.nome = no -> nome;
  simbolo.categoria = CAT_FUN;
  simbolo.tipo = no -> tipo_decl;
  simbolo.linha_decl = no -> linha;

  ASTNode * param = (!no -> filhos.empty()) ? no -> filhos[0] : nullptr;
  while (param != nullptr) {
    if (param -> tipo_decl == TIPO_VOID) {
      reportarErro(param -> linha,
        "Parametro '" + param -> nome + "' nao pode ser do tipo 'void'.",
        "Altere o tipo do parametro para 'int " + param -> nome + "' ou 'int " + param -> nome + "[]'.");
      param -> tipo = TIPO_ERRO;
    } else {
      param -> tipo = param -> eh_array ? TIPO_INT_ARRAY : TIPO_INT;
    }
    simbolo.tipos_parametros.push_back(param -> tipo);
    param = param -> irmao;
  }

  SimboloInfo * existente = tabelaDeSimbolos.buscarSimboloNoEscopoAtual(no -> nome);
  if (!tabelaDeSimbolos.adicionarSimbolo(simbolo)) {
    std::string sug = "Renomeie a funcao ou remova a duplicata";
    if (existente) sug += " (ja declarada na linha " + std::to_string(existente -> linha_decl) + ").";
    reportarErro(no -> linha, "Funcao '" + no -> nome + "' ja declarada.", sug);
    no -> tipo = TIPO_ERRO;
  } else {
    no -> tipo = simbolo.tipo;
  }

  tabelaDeSimbolos.entrarEscopo(no -> nome);

  param = (!no -> filhos.empty()) ? no -> filhos[0] : nullptr;
  while (param != nullptr) {
    if (param -> tipo != TIPO_ERRO) {
      SimboloInfo paramSym;
      paramSym.nome = param -> nome;
      paramSym.categoria = param -> eh_array ? CAT_ARRAY : CAT_VAR;
      paramSym.tipo = param -> tipo;
      paramSym.linha_decl = param -> linha;

      if (!tabelaDeSimbolos.adicionarSimbolo(paramSym)) {
        reportarErro(param -> linha,
          "Parametro duplicado '" + param -> nome + "' na funcao '" + no -> nome + "'.",
          "De nomes distintos a cada parametro da funcao.");
      }
    }
    param = param -> irmao;
  }

  TipoDado retornoAnterior = tipoRetornoAtual;
  std::string funcaoAnterior = nomeFuncaoAtual;
  tipoRetornoAtual = no -> tipo_decl;
  nomeFuncaoAtual = no -> nome;

  if (no -> filhos.size() > 1 && no -> filhos[1] != nullptr) {
    ignorarProximoEscopoComposto = true;
    analisarNo(no -> filhos[1]);
  }

  tipoRetornoAtual = retornoAnterior;
  nomeFuncaoAtual = funcaoAnterior;
  tabelaDeSimbolos.sairEscopo();
}

void AnalisadorSemantico::analisarComposto(ASTNode * no) {
  bool abriuEscopo = false;
  if (ignorarProximoEscopoComposto) {
    ignorarProximoEscopoComposto = false;
  } else {
    tabelaDeSimbolos.entrarEscopo(nomeFuncaoAtual + ":bloco");
    abriuEscopo = true;
  }

  ASTNode * decl = (!no -> filhos.empty()) ? no -> filhos[0] : nullptr;
  while (decl != nullptr) {
    analisarNo(decl);
    decl = decl -> irmao;
  }

  ASTNode * stmt = (no -> filhos.size() > 1) ? no -> filhos[1] : nullptr;
  while (stmt != nullptr) {
    analisarNo(stmt);
    stmt = stmt -> irmao;
  }

  if (abriuEscopo) {
    tabelaDeSimbolos.sairEscopo();
  }
}

void AnalisadorSemantico::analisarIf(ASTNode * no) {
  if (no -> filhos.size() < 2) return;

  ASTNode * condicao = no -> filhos[0];
  ASTNode * blocoIf = no -> filhos[1];

  analisarNo(condicao);
  if (condicao -> tipo != TIPO_ERRO && condicao -> tipo != TIPO_INT) {
    reportarErro(condicao -> linha,
      "Condicao do 'if' deve avaliar para 'int' (recebido '" + std::string(tipoDadoToString(condicao -> tipo)) + "').",
      "Use uma expressao inteira ou comparacao relacional (ex: 'x > 0').");
  }
  if (condicao -> eh_constante && condicao -> valor == 0) {
    reportarAviso(no -> linha, "Codigo inalcancavel (dead code): condicao do 'if' e sempre falsa.");
  }
  analisarNo(blocoIf);
  if (no -> filhos.size() == 3) {
    analisarNo(no -> filhos[2]);
  }
}
void AnalisadorSemantico::analisarWhile(ASTNode * no) {
  if (no -> filhos.size() < 2) return;
  ASTNode * condicao = no -> filhos[0];
  ASTNode * blocoWhile = no -> filhos[1];
  analisarNo(condicao);
  if (condicao -> tipo != TIPO_ERRO && condicao -> tipo != TIPO_INT) {
    reportarErro(condicao -> linha,
      "Condicao do 'while' deve avaliar para 'int' (recebido '" + std::string(tipoDadoToString(condicao -> tipo)) + "').",
      "Use uma expressao inteira ou comparacao relacional (ex: 'i < 10').");
  }
  if (condicao -> eh_constante && condicao -> valor == 0) {
    reportarAviso(no -> linha, "Codigo inalcancavel (dead code): condicao do 'while' e sempre falsa.");
  }
  analisarNo(blocoWhile);
}
void AnalisadorSemantico::analisarReturn(ASTNode * no) {
  if (no -> filhos.empty()) {
    if (tipoRetornoAtual != TIPO_VOID) {
      reportarErro(no -> linha,
        "Funcao '" + nomeFuncaoAtual + "' deve retornar um valor do tipo '" + std::string(tipoDadoToString(tipoRetornoAtual)) + "'.",
        "Adicione uma expressao inteira ao retorno (ex: 'return 0;') ou mude o tipo da funcao para 'void'.");
      no -> tipo = TIPO_ERRO;
    } else {
      no -> tipo = TIPO_VOID;
    }
  } else {
    ASTNode * expressao = no -> filhos[0];
    analisarNo(expressao);
    if (expressao -> tipo == TIPO_ERRO) {
      no -> tipo = TIPO_ERRO;
      return;
    }
    if (tipoRetornoAtual == TIPO_VOID) {
      reportarErro(no -> linha,
        "Funcao 'void' ('" + nomeFuncaoAtual + "') nao pode retornar valor.",
        "Use apenas 'return;' ou altere o tipo de retorno de '" + nomeFuncaoAtual + "' para 'int'.");
      no -> tipo = TIPO_ERRO;
    } else if (expressao -> tipo != tipoRetornoAtual) {
      reportarErro(no -> linha,
        "Tipo de retorno incompativel na funcao '" + nomeFuncaoAtual + "' (esperado '" +
        std::string(tipoDadoToString(tipoRetornoAtual)) + "', recebido '" +
        std::string(tipoDadoToString(expressao -> tipo)) + "').",
        "Retorne um valor escalar do tipo 'int'.");
      no -> tipo = TIPO_ERRO;
    } else {
      no -> tipo = expressao -> tipo;
    }
  }
}
void AnalisadorSemantico::analisarAssign(ASTNode * no) {
  if (no -> filhos.size() != 2) return;
  ASTNode *
    var = no -> filhos[0];
  ASTNode * expressao = no -> filhos[1];
  analisarNo(var);
  analisarNo(expressao);
  if (var -> tipo == TIPO_ERRO || expressao -> tipo == TIPO_ERRO) {
    no -> tipo = TIPO_ERRO;
    return;
  }
  if (!
    var -> eh_lvalue) {
    std::string sug = "Atribua o valor a uma variavel 'int' ou a uma posicao indexada de vetor.";
    if (var -> tipo == TIPO_INT_ARRAY) {
      sug = "O identificador '" +
        var -> nome + "' e um vetor. Especifique o indice (ex: '" +
        var -> nome + "[0] = ...').";
    }
    reportarErro(var -> linha, "O lado esquerdo da atribuicao deve ser uma variavel modificavel (lvalue).", sug);
    no -> tipo = TIPO_ERRO;
    return;
  }
  if (var -> tipo != expressao -> tipo) {
    std::string sug = "Garanta que ambos os lados da atribuicao sejam do tipo 'int'.";
    if (expressao -> tipo == TIPO_VOID) {
      sug = "A expressao a direita retorna 'void' e nao pode ser atribuida a '" +
        var -> nome + "'.";
    } else if (expressao -> tipo == TIPO_INT_ARRAY) {
      sug = "Indexe o vetor a direita antes de atribuir (ex: '" +
        var -> nome + " = " + expressao -> nome + "[0];').";
    }
    reportarErro(no -> linha,
      "Tipos incompativeis na atribuicao: '" + std::string(tipoDadoToString(var -> tipo)) +
      "' e '" + std::string(tipoDadoToString(expressao -> tipo)) + "'.",
      sug);
    no -> tipo = TIPO_ERRO;
    return;
  }
  no -> tipo =
    var -> tipo;
  no -> eh_lvalue = false;
}
void AnalisadorSemantico::analisarOp(ASTNode * no) {
  if (no -> filhos.size() != 2) return;
  ASTNode * operando1 = no -> filhos[0];
  ASTNode * operando2 = no -> filhos[1];
  analisarNo(operando1);
  analisarNo(operando2);
  if (operando1 -> tipo == TIPO_ERRO || operando2 -> tipo == TIPO_ERRO) {
    no -> tipo = TIPO_ERRO;
    return;
  }
  if (operando1 -> tipo != TIPO_INT || operando2 -> tipo != TIPO_INT) {
    std::string sug = "Certifique-se de que ambos os operandos avaliem para 'int'.";
    if (operando1 -> tipo == TIPO_VOID || operando2 -> tipo == TIPO_VOID) {
      sug = "Nao use chamadas de funcoes 'void' dentro de expressoes matematicas ou relacionais.";
    } else if (operando1 -> tipo == TIPO_INT_ARRAY || operando2 -> tipo == TIPO_INT_ARRAY) {
      sug = "Indexe o vetor (ex: 'v[i]') antes de realizar operacoes com ele.";
    }
    reportarErro(no -> linha, "Operadores aritmeticos e relacionais so podem ser aplicados a inteiros.", sug);
    no -> tipo = TIPO_ERRO;
    return;
  }
  no -> tipo = TIPO_INT;
  no -> eh_lvalue = false;
  if (operando1 -> eh_constante && operando2 -> eh_constante) {
    if (no -> op == DIV && operando2 -> valor == 0) {
      reportarErro(no -> linha,
        "Divisao por zero em tempo de compilacao.",
        "Altere o divisor para uma expressao diferente de zero.");
      no -> tipo = TIPO_ERRO;
      return;
    }
    int resultado = calcularOperacaoConstante(no -> op, operando1 -> valor, operando2 -> valor);
    liberarAST(operando1);
    liberarAST(operando2);
    no -> filhos.clear();
    no -> kind = NODE_NUM;
    no -> valor = resultado;
    no -> eh_constante = true;
    no -> simplificado = true;
  }
}
int AnalisadorSemantico::calcularOperacaoConstante(int op, int valor1, int valor2) {
  switch (op) {
  case SOMA:
    return valor1 + valor2;
  case SUB:
    return valor1 - valor2;
  case MULT:
    return valor1 * valor2;
  case DIV:
    return (valor2 != 0) ? (valor1 / valor2) : 0;
  case LT:
    return (valor1 < valor2) ? 1 : 0;
  case LEQ:
    return (valor1 <= valor2) ? 1 : 0;
  case GT:
    return (valor1 > valor2) ? 1 : 0;
  case GEQ:
    return (valor1 >= valor2) ? 1 : 0;
  case EQ:
    return (valor1 == valor2) ? 1 : 0;
  case NEQ:
    return (valor1 != valor2) ? 1 : 0;
  default:
    return 0;
  }
}
void AnalisadorSemantico::analisarVar(ASTNode * no) {
  SimboloInfo * simbolo = tabelaDeSimbolos.buscarSimbolo(no -> nome);
  if (!simbolo) {
    reportarErro(no -> linha,
      "Variavel '" + no -> nome + "' nao declarada.",
      sugerirIdentificadorSimilar(no -> nome));
    no -> tipo = TIPO_ERRO;
    return;
  }
  if (simbolo -> categoria == CAT_FUN) {
    reportarErro(no -> linha,
      "Identificador '" + no -> nome + "' e uma funcao, nao uma variavel.",
      "Adicione parenteses para chama-la como funcao (ex: '" + no -> nome + "()').");
    no -> tipo = TIPO_ERRO;
    return;
  }
  simbolo -> linhas_usos.push_back(no -> linha);
  if (no -> eh_array) {
    if (simbolo -> categoria != CAT_ARRAY) {
      reportarErro(no -> linha,
        "Variavel '" + no -> nome + "' nao e um vetor para ser indexada.",
        "Remova os colchetes '[]' ou declare '" + no -> nome + "' como vetor (ex: 'int " + no -> nome + "[10];').");
      no -> tipo = TIPO_ERRO;
      return;
    }
    ASTNode * indice = no -> filhos[0];
    analisarNo(indice);
    if (indice -> tipo != TIPO_ERRO && indice -> tipo != TIPO_INT) {
      reportarErro(no -> linha,
        "Indice do vetor '" + no -> nome + "' deve ser do tipo 'int'.",
        "Use uma variavel ou expressao inteira dentro dos colchetes.");
      no -> tipo = TIPO_ERRO;
      return;
    }
    no -> tipo = TIPO_INT;
    no -> eh_lvalue = true;
  } else {
    no -> tipo = simbolo -> tipo;
    no -> eh_lvalue = (simbolo -> categoria == CAT_VAR);
  }
  no -> eh_constante = false;
}
void AnalisadorSemantico::analisarNum(ASTNode * no) {
  no -> tipo = TIPO_INT;
  no -> eh_constante = true;
  no -> eh_lvalue = false;
}
void AnalisadorSemantico::analisarCall(ASTNode * no) {
  SimboloInfo * simbolo = tabelaDeSimbolos.buscarSimbolo(no -> nome);
  if (!simbolo) {
    reportarErro(no -> linha,
      "Funcao '" + no -> nome + "' nao declarada.",
      sugerirIdentificadorSimilar(no -> nome));
    no -> tipo = TIPO_ERRO;
    return;
  }
  if (simbolo -> categoria != CAT_FUN) {
    reportarErro(no -> linha,
      "Identificador '" + no -> nome + "' e uma variavel, nao uma funcao.",
      "Remova os parenteses '()' para acessa-lo como variavel.");
    no -> tipo = TIPO_ERRO;
    return;
  }
  simbolo -> linhas_usos.push_back(no -> linha);
  std::vector < ASTNode * > argumentos;
  ASTNode * arg = (!no -> filhos.empty()) ? no -> filhos[0] : nullptr;
  while (arg != nullptr) {
    analisarNo(arg);
    argumentos.push_back(arg);
    arg = arg -> irmao;
  }
  if (argumentos.size() != simbolo -> tipos_parametros.size()) {
    std::string assinatura = no -> nome + "(";
    for (size_t i = 0; i < simbolo -> tipos_parametros.size(); ++i) {
      assinatura += tipoDadoToString(simbolo -> tipos_parametros[i]);
      if (i + 1 < simbolo -> tipos_parametros.size()) assinatura += ", ";
    }
    assinatura += ")";
    reportarErro(no -> linha,
      "Numero incorreto de argumentos na chamada de '" + no -> nome + "' (esperado " +
      std::to_string(simbolo -> tipos_parametros.size()) + ", recebido " +
      std::to_string(argumentos.size()) + ").",
      "Ajuste a chamada para corresponder a assinatura '" + assinatura + "'.");
    no -> tipo = TIPO_ERRO;
    return;
  }
  for (size_t i = 0; i < argumentos.size(); ++i) {
    if (argumentos[i] -> tipo != TIPO_ERRO && argumentos[i] -> tipo != simbolo -> tipos_parametros[i]) {
      std::string sug = "Passe uma expressao do tipo '" + std::string(tipoDadoToString(simbolo -> tipos_parametros[i])) + "'.";
      if (simbolo -> tipos_parametros[i] == TIPO_INT && argumentos[i] -> tipo == TIPO_INT_ARRAY) {
        sug = "O parametro espera 'int', mas recebeu vetor 'int[]'. Indexe o vetor (ex: '" + argumentos[i] -> nome + "[0]').";
      } else if (simbolo -> tipos_parametros[i] == TIPO_INT_ARRAY && argumentos[i] -> tipo == TIPO_INT) {
        sug = "O parametro espera um vetor 'int[]'. Passe o nome de um vetor sem colchetes.";
      }
      reportarErro(argumentos[i] -> linha,
        "Tipo incompativel no argumento " + std::to_string(i + 1) +
        " da chamada de '" + no -> nome + "' (esperado '" +
        std::string(tipoDadoToString(simbolo -> tipos_parametros[i])) + "', recebido '" +
        std::string(tipoDadoToString(argumentos[i] -> tipo)) + "').",
        sug);
      no -> tipo = TIPO_ERRO;
      return;
    }
  }
  no -> tipo = simbolo -> tipo;
  no -> eh_constante = false;
  no -> eh_lvalue = false;
}