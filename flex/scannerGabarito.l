%{
#include <stdio.h>
#include <string.h>

int linha_atual = 1;

/* Buffer para acumular o texto do comentário */
char string_comentario[4096];
%}

%x COMENTARIO

%%
"else"          { printf("(\"%s\", ELSE)\n", yytext); }
"if"            { printf("(\"%s\", IF)\n", yytext); }
"int"           { printf("(\"%s\", INT)\n", yytext); }
"return"        { printf("(\"%s\", RETURN)\n", yytext); }
"void"          { printf("(\"%s\", VOID)\n", yytext); }
"while"         { printf("(\"%s\", WHILE)\n", yytext); }

"+"             { printf("(\"%s\", SOMA)\n", yytext); }
"-"             { printf("(\"%s\", SUB)\n", yytext); }
"*"             { printf("(\"%s\", MULT)\n", yytext); }
"/"             { printf("(\"%s\", DIV)\n", yytext); }

"<="            { printf("(\"%s\", LEQ)\n", yytext); }
"<"             { printf("(\"%s\", LT)\n", yytext); }
">="            { printf("(\"%s\", GEQ)\n", yytext); }
">"             { printf("(\"%s\", GT)\n", yytext); }
"=="            { printf("(\"%s\", EQ)\n", yytext); }
"!="            { printf("(\"%s\", NEQ)\n", yytext); }
"="             { printf("(\"%s\", ASSIGN)\n", yytext); }

";"             { printf("(\"%s\", SEMI)\n", yytext); }
","             { printf("(\"%s\", COMMA)\n", yytext); }
"("             { printf("(\"%s\", LPAREN)\n", yytext); }
")"             { printf("(\"%s\", RPAREN)\n", yytext); }
"["             { printf("(\"%s\", LBRACK)\n", yytext); }
"]"             { printf("(\"%s\", RBRACK)\n", yytext); }
"{"             { printf("(\"%s\", LBRACE)\n", yytext); }
"}"             { printf("(\"%s\", RBRACE)\n", yytext); }

[0-9]+          { printf("(\"%s\", NUM)\n", yytext); }
[a-zA-Z]+ { printf("(\"%s\", ID)\n", yytext); }

"/*"            {
                    /* Inicializa o buffer com o inicio do comentario */
                    strcpy(string_comentario, "/*");
                    BEGIN(COMENTARIO);
                }
<COMENTARIO>"*/" {
                    /* Finaliza o buffer, imprime o token e volta ao normal */
                    strcat(string_comentario, "*/");
                    printf("(\"%s\", COMENTARIO)\n", string_comentario);
                    BEGIN(INITIAL);
                }
<COMENTARIO>\n   {
                    linha_atual++;
                    strcat(string_comentario, yytext);
                }
<COMENTARIO>.    {
                    strcat(string_comentario, yytext);
                }

\n              { linha_atual++; }
[ \t\r]+        { /* ignora espacos vazios */ }
.               { printf("ERRO LEXICO (Linha %d): Caractere inesperado '%s'\n", linha_atual, yytext); }
%%

int yywrap() { return 1; }
int main() { yylex(); return 0; }