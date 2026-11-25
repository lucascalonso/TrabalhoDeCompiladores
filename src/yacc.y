%code requires {
    #include "../include/AST.h"
}

%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../include/AST.h"

#ifdef __cplusplus
extern "C" {
#endif
    int yylex(void);
    extern int linha_do_erro;
    extern int coluna_do_erro;
#ifdef __cplusplus
}
#endif

void yyerror(const char *s);
int erros_sintaticos = 0;
ASTNode* raiz_ast = nullptr;

static ASTNode* encadearIrmao(ASTNode* lista, ASTNode* novoNo) {
    if (!lista) return novoNo;
    ASTNode* aux = lista;
    while (aux->irmao != nullptr) {
        aux = aux->irmao;
    }
    aux->irmao = novoNo;
    return lista;
}
%}

%define parse.error verbose
%locations

%union {
    int num_val;
    char* id_name;
    TipoDado tipo_val;
    int op_val;
    ASTNode* node;
}

%token ELSE IF INT RETURN VOID WHILE
%token SOMA SUB MULT DIV
%token LEQ LT GEQ GT EQ NEQ ASSIGN
%token SEMI COMMA LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE
%token <id_name> ID
%token <num_val> NUM

%type <tipo_val> tipo_especificador
%type <op_val> relacional soma mult
%type <node> programa declaracao_lista declaracao var_declaracao fun_declaracao
%type <node> params param_lista param composto_decl local_declaracoes statement_lista
%type <node> statement expressao_decl selecao_decl iteracao_decl retorno_decl
%type <node> expressao var simples_expressao soma_expressao termo fator ativacao
%type <node> args arg_lista

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

programa : declaracao_lista
         {
             printf("[Linha %d] Reducao: programa -> declaracao_lista\n", @1.first_line);
             raiz_ast = $1;
             $$ = $1;
         }
         ;

declaracao_lista : declaracao_lista declaracao
                   {
                       printf("[Linha %d] Reducao: declaracao_lista -> declaracao_lista declaracao\n", @2.first_line);
                       $$ = encadearIrmao($1, $2);
                   }
                 | declaracao
                   {
                       printf("[Linha %d] Reducao: declaracao_lista -> declaracao\n", @1.first_line);
                       $$ = $1;
                   }
                 | declaracao_lista error
                   { yyclearin; yyerrok; $$ = $1; }
                 ;

declaracao : var_declaracao
             {
                 printf("[Linha %d] Reducao: declaracao -> var_declaracao\n", @1.first_line);
                 $$ = $1;
             }
           | fun_declaracao
             {
                 printf("[Linha %d] Reducao: declaracao -> fun_declaracao\n", @1.first_line);
                 $$ = $1;
             }
           ;

var_declaracao : tipo_especificador ID SEMI
                 {
                     printf("[Linha %d] Reducao: var_declaracao -> tipo_especificador ID ;\n", @1.first_line);
                     ASTNode* no = new ASTNode(NODE_VAR_DECL, @1.first_line);
                     no->tipo_decl = $1;
                     no->nome = $2;
                     no->eh_array = false;
                     free($2);
                     $$ = no;
                 }
               | tipo_especificador ID LBRACK NUM RBRACK SEMI
                 {
                     printf("[Linha %d] Reducao: var_declaracao -> tipo_especificador ID [ NUM ] ;\n", @1.first_line);
                     ASTNode* no = new ASTNode(NODE_VAR_DECL, @1.first_line);
                     no->tipo_decl = $1;
                     no->nome = $2;
                     no->eh_array = true;
                     no->tamanho_array = $4;
                     free($2);
                     $$ = no;
                 }
               ;

tipo_especificador : INT
                     {
                         printf("[Linha %d] Reducao: tipo_especificador -> int\n", @1.first_line);
                         $$ = TIPO_INT;
                     }
                   | VOID
                     {
                         printf("[Linha %d] Reducao: tipo_especificador -> void\n", @1.first_line);
                         $$ = TIPO_VOID;
                     }
                   ;

fun_declaracao : tipo_especificador ID LPAREN params RPAREN composto_decl
                 {
                     printf("[Linha %d] Reducao: fun_declaracao -> tipo_especificador ID ( params ) composto_decl\n", @1.first_line);
                     ASTNode* no = new ASTNode(NODE_FUN_DECL, @1.first_line);
                     no->tipo_decl = $1;
                     no->nome = $2;
                     no->filhos.push_back($4);
                     no->filhos.push_back($6);
                     free($2);
                     $$ = no;
                 }
               ;

params : param_lista
         {
             printf("[Linha %d] Reducao: params -> param_lista\n", @1.first_line);
             $$ = $1;
         }
       | VOID
         {
             printf("[Linha %d] Reducao: params -> void\n", @1.first_line);
             $$ = nullptr;
         }
       ;

param_lista : param_lista COMMA param
              {
                  printf("[Linha %d] Reducao: param_lista -> param_lista , param\n", @1.first_line);
                  $$ = encadearIrmao($1, $3);
              }
            | param
              {
                  printf("[Linha %d] Reducao: param_lista -> param\n", @1.first_line);
                  $$ = $1;
              }
            ;

param : tipo_especificador ID
        {
            printf("[Linha %d] Reducao: param -> tipo_especificador ID\n", @1.first_line);
            ASTNode* no = new ASTNode(NODE_PARAM, @1.first_line);
            no->tipo_decl = $1;
            no->nome = $2;
            no->eh_array = false;
            free($2);
            $$ = no;
        }
      | tipo_especificador ID LBRACK RBRACK
        {
            printf("[Linha %d] Reducao: param -> tipo_especificador ID [ ]\n", @1.first_line);
            ASTNode* no = new ASTNode(NODE_PARAM, @1.first_line);
            no->tipo_decl = $1;
            no->nome = $2;
            no->eh_array = true;
            free($2);
            $$ = no;
        }
      ;

composto_decl : LBRACE local_declaracoes statement_lista RBRACE
                {
                    printf("[Linha %d] Reducao: composto_decl -> { local_declaracoes statement_lista }\n", @1.first_line);
                    ASTNode* no = new ASTNode(NODE_COMPOSTO, @1.first_line);
                    no->filhos.push_back($2);
                    no->filhos.push_back($3);
                    $$ = no;
                }
              ;

local_declaracoes : local_declaracoes var_declaracao
                    {
                        printf("[Linha %d] Reducao: local_declaracoes -> local_declaracoes var_declaracao\n", @2.first_line);
                        $$ = encadearIrmao($1, $2);
                    }
                  | %empty
                    {
                        printf("         Reducao: local_declaracoes -> vazio\n");
                        $$ = nullptr;
                    }
                  ;

statement_lista : statement_lista statement
                  {
                      printf("[Linha %d] Reducao: statement_lista -> statement_lista statement\n", @2.first_line);
                      $$ = encadearIrmao($1, $2);
                  }
                | statement_lista error SEMI
                  { yyclearin; yyerrok; $$ = $1; }
                | statement_lista error ID
                  { free($3); yyerrok; $$ = $1; }
                | %empty
                  {
                      printf("         Reducao: statement_lista -> vazio\n");
                      $$ = nullptr;
                  }
                ;

statement : expressao_decl { printf("[Linha %d] Reducao: statement -> expressao_decl\n", @1.first_line); $$ = $1; }
          | composto_decl  { printf("[Linha %d] Reducao: statement -> composto_decl\n", @1.first_line);  $$ = $1; }
          | selecao_decl   { printf("[Linha %d] Reducao: statement -> selecao_decl\n", @1.first_line);   $$ = $1; }
          | iteracao_decl  { printf("[Linha %d] Reducao: statement -> iteracao_decl\n", @1.first_line);  $$ = $1; }
          | retorno_decl   { printf("[Linha %d] Reducao: statement -> retorno_decl\n", @1.first_line);   $$ = $1; }
          ;

expressao_decl : expressao SEMI
                 {
                     printf("[Linha %d] Reducao: expressao_decl -> expressao ;\n", @1.first_line);
                     $$ = $1;
                 }
               | SEMI
                 {
                     printf("[Linha %d] Reducao: expressao_decl -> ;\n", @1.first_line);
                     $$ = nullptr;
                 }
               ;

selecao_decl : IF LPAREN expressao RPAREN statement %prec LOWER_THAN_ELSE
               {
                   printf("[Linha %d] Reducao: selecao_decl -> if ( expressao ) statement\n", @1.first_line);
                   ASTNode* no = new ASTNode(NODE_IF, @1.first_line);
                   no->filhos.push_back($3);
                   no->filhos.push_back($5);
                   $$ = no;
               }
             | IF LPAREN expressao RPAREN statement ELSE statement
               {
                   printf("[Linha %d] Reducao: selecao_decl -> if ( expressao ) statement else statement\n", @1.first_line);
                   ASTNode* no = new ASTNode(NODE_IF, @1.first_line);
                   no->filhos.push_back($3);
                   no->filhos.push_back($5);
                   no->filhos.push_back($7);
                   $$ = no;
               }
             | IF error statement
               { yyclearin; yyerrok; $$ = $3; }
             ;

iteracao_decl : WHILE LPAREN expressao RPAREN statement
                {
                    printf("[Linha %d] Reducao: iteracao_decl -> while ( expressao ) statement\n", @1.first_line);
                    ASTNode* no = new ASTNode(NODE_WHILE, @1.first_line);
                    no->filhos.push_back($3);
                    no->filhos.push_back($5);
                    $$ = no;
}
;
retorno_decl : RETURN SEMI
{
printf("[Linha %d] Reducao: retorno_decl -> return ;\n", @1.first_line);
$$ = new ASTNode(NODE_RETURN, @1.first_line);
}
| RETURN expressao SEMI
{
printf("[Linha %d] Reducao: retorno_decl -> return expressao ;\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_RETURN, @1.first_line);
no->filhos.push_back($2);
$$ = no;
}
;
expressao : var ASSIGN expressao
{
printf("[Linha %d] Reducao: expressao -> var = expressao\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_ASSIGN, @1.first_line);
no->filhos.push_back($1);
no->filhos.push_back($3);
$$ = no;
}
| simples_expressao
{
printf("[Linha %d] Reducao: expressao -> simples_expressao\n", @1.first_line);
$$ = $1;
}
;
var : ID
{
printf("[Linha %d] Reducao: var -> ID\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_VAR, @1.first_line);
no->nome = $1;
no->eh_array = false;
free($1);
$$ = no;
}
| ID LBRACK expressao RBRACK
{
printf("[Linha %d] Reducao: var -> ID [ expressao ]\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_VAR, @1.first_line);
no->nome = $1;
no->eh_array = true;
no->filhos.push_back($3);
free($1);
$$ = no;
}
;
simples_expressao : soma_expressao relacional soma_expressao
{
printf("[Linha %d] Reducao: simples_expressao -> soma_expressao relacional soma_expressao\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_OP, @1.first_line);
no->op = $2;
no->filhos.push_back($1);
no->filhos.push_back($3);
$$ = no;
}
| soma_expressao
{
printf("[Linha %d] Reducao: simples_expressao -> soma_expressao\n", @1.first_line);
$$ = $1;
}
;
relacional : LEQ { printf("[Linha %d] Reducao: relacional -> <=\n", @1.first_line); $$ = LEQ; }
| LT  { printf("[Linha %d] Reducao: relacional -> <\n", @1.first_line);  $$ = LT; }
| GT  { printf("[Linha %d] Reducao: relacional -> >\n", @1.first_line);  $$ = GT; }
| GEQ { printf("[Linha %d] Reducao: relacional -> >=\n", @1.first_line); $$ = GEQ; }
| EQ  { printf("[Linha %d] Reducao: relacional -> ==\n", @1.first_line); $$ = EQ; }
| NEQ { printf("[Linha %d] Reducao: relacional -> !=\n", @1.first_line); $$ = NEQ; }
;
soma_expressao : soma_expressao soma termo
{
printf("[Linha %d] Reducao: soma_expressao -> soma_expressao soma termo\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_OP, @1.first_line);
no->op = $2;
no->filhos.push_back($1);
no->filhos.push_back($3);
$$ = no;
}
| termo
{
printf("[Linha %d] Reducao: soma_expressao -> termo\n", @1.first_line);
$$ = $1;
}
;
soma : SOMA { printf("[Linha %d] Reducao: soma -> +\n", @1.first_line); $$ = SOMA; }
| SUB  { printf("[Linha %d] Reducao: soma -> -\n", @1.first_line); $$ = SUB; }
;
termo : termo mult fator
{
printf("[Linha %d] Reducao: termo -> termo mult fator\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_OP, @1.first_line);
no->op = $2;
no->filhos.push_back($1);
no->filhos.push_back($3);
$$ = no;
}
| fator
{
printf("[Linha %d] Reducao: termo -> fator\n", @1.first_line);
$$ = $1;
}
;
mult : MULT { printf("[Linha %d] Reducao: mult -> *\n", @1.first_line); $$ = MULT; }
| DIV  { printf("[Linha %d] Reducao: mult -> /\n", @1.first_line); $$ = DIV; }
;
fator : LPAREN expressao RPAREN
{
printf("[Linha %d] Reducao: fator -> ( expressao )\n", @1.first_line);
$$ = $2;
}
| var
{
printf("[Linha %d] Reducao: fator -> var\n", @1.first_line);
$$ = $1;
}
| ativacao
{
printf("[Linha %d] Reducao: fator -> ativacao\n", @1.first_line);
$$ = $1;
}
| NUM
{
printf("[Linha %d] Reducao: fator -> NUM\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_NUM, @1.first_line);
no->valor = $1;
$$ = no;
}
;
ativacao : ID LPAREN args RPAREN
{
printf("[Linha %d] Reducao: ativacao -> ID ( args )\n", @1.first_line);
ASTNode* no = new ASTNode(NODE_CALL, @1.first_line);
no->nome = $1;
no->filhos.push_back($3);
free($1);
$$ = no;
}
;
args : arg_lista
{
printf("[Linha %d] Reducao: args -> arg_lista\n", @1.first_line);
$$ = $1;
}
| %empty
{
printf("         Reducao: args -> vazio\n");
$$ = nullptr;
}
;
arg_lista : arg_lista COMMA expressao
{
printf("[Linha %d] Reducao: arg_lista -> arg_lista , expressao\n", @1.first_line);
$$ = encadearIrmao($1, $3);
}
| expressao
{
printf("[Linha %d] Reducao: arg_lista -> expressao\n", @1.first_line);
$$ = $1;
}
;
%%
void yyerror(const char *s) {
erros_sintaticos++;
fprintf(stderr, "Erro Sintatico na linha %d (coluna %d): %s\n",
linha_do_erro, coluna_do_erro, s);
}