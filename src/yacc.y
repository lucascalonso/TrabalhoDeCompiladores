%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex();

/* Posicao do ultimo token valido, mantida pela ponte yylex (src/yylex_bridge.cpp) */
extern int linha_do_erro;
extern int coluna_do_erro;

void yyerror(const char *s);

int erros_sintaticos = 0;
%}

%define parse.error verbose
%locations  

/* Tokens baseados fielmente no Louden (sem FLOAT e POT) */
%token ELSE IF INT RETURN VOID WHILE
%token SOMA SUB MULT DIV
%token LEQ LT GEQ GT EQ NEQ ASSIGN
%token SEMI COMMA LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE
%token ID NUM

/* Resolucao da ambiguidade estrutural do 'dangling else' */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/* 1. programa -> declaracao-lista */
programa : declaracao_lista 
         { 
             printf("[Linha %d] Reducao: programa -> declaracao_lista\n", @1.first_line);
             if (erros_sintaticos == 0)
                 printf("\n>>> Analise sintatica concluida sem erros! <<<\n");
             else
                 printf("\n>>> Analise concluida com %d erros sintaticos! <<<\n", erros_sintaticos);
         }
         ;

/* 2. declaracao-lista -> declaracao-lista declaracao | declaracao */
declaracao_lista : declaracao_lista declaracao 
                   { printf("[Linha %d] Reducao: declaracao_lista -> declaracao_lista declaracao\n", @2.first_line); }
                 | declaracao 
                   { printf("[Linha %d] Reducao: declaracao_lista -> declaracao\n", @1.first_line); }
                 | declaracao_lista error { yyclearin; yyerrok; }
                 ;

/* 3. declaracao -> var-declaracao | fun-declaracao */
declaracao : var_declaracao 
             { printf("[Linha %d] Reducao: declaracao -> var_declaracao\n", @1.first_line); }
           | fun_declaracao 
             { printf("[Linha %d] Reducao: declaracao -> fun_declaracao\n", @1.first_line); }
           ;

/* 4. var-declaracao -> tipo-especificador ID ; | tipo-especificador ID [ NUM ] ; */
var_declaracao : tipo_especificador ID SEMI 
                 { printf("[Linha %d] Reducao: var_declaracao -> tipo_especificador ID ;\n", @1.first_line); }
               | tipo_especificador ID LBRACK NUM RBRACK SEMI 
                 { printf("[Linha %d] Reducao: var_declaracao -> tipo_especificador ID [ NUM ] ;\n", @1.first_line); }
               ;

/* 5. tipo-especificador -> int | void */
tipo_especificador : INT 
                     { printf("[Linha %d] Reducao: tipo_especificador -> int\n", @1.first_line); }
                   | VOID 
                     { printf("[Linha %d] Reducao: tipo_especificador -> void\n", @1.first_line); }
                   ;

/* 6. fun-declaracao -> tipo-especificador ID ( params ) composto-decl */
fun_declaracao : tipo_especificador ID LPAREN params RPAREN composto_decl 
                 { printf("[Linha %d] Reducao: fun_declaracao -> tipo_especificador ID ( params ) composto_decl\n", @1.first_line); }
               ;

/* 7. params -> param-lista | void */
params : param_lista 
         { printf("[Linha %d] Reducao: params -> param_lista\n", @1.first_line); }
       | VOID 
         { printf("[Linha %d] Reducao: params -> void\n", @1.first_line); }
       ;

/* 8. param-lista -> param-lista , param | param */
param_lista : param_lista COMMA param 
              { printf("[Linha %d] Reducao: param_lista -> param_lista , param\n", @1.first_line); }
            | param 
              { printf("[Linha %d] Reducao: param_lista -> param\n", @1.first_line); }
            ;

/* 9. param -> tipo-especificador ID | tipo-especificador ID [ ] */
param : tipo_especificador ID 
        { printf("[Linha %d] Reducao: param -> tipo_especificador ID\n", @1.first_line); }
      | tipo_especificador ID LBRACK RBRACK 
        { printf("[Linha %d] Reducao: param -> tipo_especificador ID [ ]\n", @1.first_line); }
      ;

/* 10. composto-decl -> { local-declaracoes statement-lista } */
composto_decl : LBRACE local_declaracoes statement_lista RBRACE 
                { printf("[Linha %d] Reducao: composto_decl -> { local_declaracoes statement_lista }\n", @1.first_line); }
              ;

/* 11. local-declaracoes -> local-declaracoes var-declaracao | vazio */
local_declaracoes : local_declaracoes var_declaracao 
                    { printf("[Linha %d] Reducao: local_declaracoes -> local_declaracoes var_declaracao\n", @2.first_line); }
                  | %empty
                    { printf("         Reducao: local_declaracoes -> vazio\n"); }
                  ;

/* 12. statement-lista -> statement-lista statement | vazio */
statement_lista : statement_lista statement 
                  { printf("[Linha %d] Reducao: statement_lista -> statement_lista statement\n", @2.first_line); }
                | statement_lista error SEMI { yyclearin; yyerrok; }
                | statement_lista error ID { yyerrok; }
                | %empty
                  { printf("         Reducao: statement_lista -> vazio\n"); }
                ;

/* 13. statement -> expressao-decl | composto-decl | selecao-decl | iteracao-decl | retorno-decl */
statement : expressao_decl 
            { printf("[Linha %d] Reducao: statement -> expressao_decl\n", @1.first_line); }
          | composto_decl 
            { printf("[Linha %d] Reducao: statement -> composto_decl\n", @1.first_line); }
          | selecao_decl 
            { printf("[Linha %d] Reducao: statement -> selecao_decl\n", @1.first_line); }
          | iteracao_decl 
            { printf("[Linha %d] Reducao: statement -> iteracao_decl\n", @1.first_line); }
          | retorno_decl 
            { printf("[Linha %d] Reducao: statement -> retorno_decl\n", @1.first_line); }
          ;

/* 14. expressao-decl -> expressao ; | ; */
expressao_decl : expressao SEMI 
                 { printf("[Linha %d] Reducao: expressao_decl -> expressao ;\n", @1.first_line); }
               | SEMI 
                 { printf("[Linha %d] Reducao: expressao_decl -> ;\n", @1.first_line); }
               ;

/* 15. selecao-decl -> if ( expressao ) statement | if ( expressao ) statement else statement */
selecao_decl : IF LPAREN expressao RPAREN statement %prec LOWER_THAN_ELSE
               { printf("[Linha %d] Reducao: selecao_decl -> if ( expressao ) statement\n", @1.first_line); }
             | IF LPAREN expressao RPAREN statement ELSE statement 
               { printf("[Linha %d] Reducao: selecao_decl -> if ( expressao ) statement else statement\n", @1.first_line); }
             | IF error statement { yyclearin; yyerrok; }
             ;

/* 16. iteracao-decl -> while ( expressao ) statement */
iteracao_decl : WHILE LPAREN expressao RPAREN statement 
                { printf("[Linha %d] Reducao: iteracao_decl -> while ( expressao ) statement\n", @1.first_line); }
              ;

/* 17. retorno-decl -> return ; | return expressao ; */
retorno_decl : RETURN SEMI 
               { printf("[Linha %d] Reducao: retorno_decl -> return ;\n", @1.first_line); }
             | RETURN expressao SEMI 
               { printf("[Linha %d] Reducao: retorno_decl -> return expressao ;\n", @1.first_line); }
             ;

/* 18. expressao -> var = expressao | simples-expressao */
expressao : var ASSIGN expressao 
            { printf("[Linha %d] Reducao: expressao -> var = expressao\n", @1.first_line); }
          | simples_expressao 
            { printf("[Linha %d] Reducao: expressao -> simples_expressao\n", @1.first_line); }
          ;

/* 19. var -> ID | ID [ expressao ] */
var : ID 
      { printf("[Linha %d] Reducao: var -> ID\n", @1.first_line); }
    | ID LBRACK expressao RBRACK 
      { printf("[Linha %d] Reducao: var -> ID [ expressao ]\n", @1.first_line); }
    ;

/* 20. simples-expressao -> soma-expressao relacional soma-expressao | soma-expressao */
simples_expressao : soma_expressao relacional soma_expressao 
                    { printf("[Linha %d] Reducao: simples_expressao -> soma_expressao relacional soma_expressao\n", @1.first_line); }
                  | soma_expressao 
                    { printf("[Linha %d] Reducao: simples_expressao -> soma_expressao\n", @1.first_line); }
                  ;

/* 21. relacional -> <= | < | > | >= | == | != */
relacional : LEQ  { printf("[Linha %d] Reducao: relacional -> <=\n", @1.first_line); }
           | LT   { printf("[Linha %d] Reducao: relacional -> <\n", @1.first_line); }
           | GT   { printf("[Linha %d] Reducao: relacional -> >\n", @1.first_line); }
           | GEQ  { printf("[Linha %d] Reducao: relacional -> >=\n", @1.first_line); }
           | EQ   { printf("[Linha %d] Reducao: relacional -> ==\n", @1.first_line); }
           | NEQ  { printf("[Linha %d] Reducao: relacional -> !=\n", @1.first_line); }
           ;

/* 22. soma-expressao -> soma-expressao soma termo | termo */
soma_expressao : soma_expressao soma termo 
                 { printf("[Linha %d] Reducao: soma_expressao -> soma_expressao soma termo\n", @1.first_line); }
               | termo 
                 { printf("[Linha %d] Reducao: soma_expressao -> termo\n", @1.first_line); }
               ;

/* 23. soma -> + | - */
soma : SOMA { printf("[Linha %d] Reducao: soma -> +\n", @1.first_line); }
     | SUB  { printf("[Linha %d] Reducao: soma -> -\n", @1.first_line); }
     ;

/* 24. termo -> termo mult fator | fator */
termo : termo mult fator 
        { printf("[Linha %d] Reducao: termo -> termo mult fator\n", @1.first_line); }
      | fator 
        { printf("[Linha %d] Reducao: termo -> fator\n", @1.first_line); }
      ;

/* 25. mult -> * | / */
mult : MULT { printf("[Linha %d] Reducao: mult -> *\n", @1.first_line); }
     | DIV  { printf("[Linha %d] Reducao: mult -> /\n", @1.first_line); }
     ;

/* 26. fator -> ( expressao ) | var | ativacao | NUM */
fator : LPAREN expressao RPAREN 
        { printf("[Linha %d] Reducao: fator -> ( expressao )\n", @1.first_line); }
      | var 
        { printf("[Linha %d] Reducao: fator -> var\n", @1.first_line); }
      | ativacao 
        { printf("[Linha %d] Reducao: fator -> ativacao\n", @1.first_line); }
      | NUM 
{ printf("[Linha %d] Reducao: fator -> NUM\n", @1.first_line); }
;
/* 27. ativacao -> ID ( args ) */
ativacao : ID LPAREN args RPAREN
{ printf("[Linha %d] Reducao: ativacao -> ID ( args )\n", @1.first_line); }
;
/* 28. args -> arg-lista | vazio */
args : arg_lista
{ printf("[Linha %d] Reducao: args -> arg_lista\n", @1.first_line); }
| %empty
{ printf("         Reducao: args -> vazio\n"); }
;
/* 29. arg-lista -> arg-lista , expressao | expressao */
arg_lista : arg_lista COMMA expressao
{ printf("[Linha %d] Reducao: arg_lista -> arg_lista , expressao\n", @1.first_line); }
| expressao
{ printf("[Linha %d] Reducao: arg_lista -> expressao\n", @1.first_line); }
;
%%
void yyerror(const char *s) {
erros_sintaticos++;
fprintf(stderr, "Erro Sintatico na linha %d (coluna %d): %s\n",
linha_do_erro, coluna_do_erro, s);
}