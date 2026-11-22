#ifndef LL1_HPP
#define LL1_HPP

#include <vector>
#include <string>

enum NonTerminal { NT_E, NT_E_LINE, NT_T, NT_T_LINE, NT_F, NT_F_LINE, NT_P };
enum Terminal { T_NUM, T_PLUS, T_MINUS, T_MULT, T_DIV, T_EXP, T_LPAREN, T_RPAREN, T_DOLLAR };

struct Production {
    int error_code = 0;              // 0 significa válido; > 0 representa um erro específico (E1 a E5)
    std::vector<std::string> rhs;    // Símbolos do lado direito da produção
};

class LL1Table {
public:
    Production table[7][9];

    LL1Table() {
        // Inicializa todas as células com erro genérico E1 por padrão
        for(int i=0; i<7; i++) {
            for(int j=0; j<9; j++) {
                table[i][j].error_code = 1; // E1 padrão
            }
        }
        configureTable();
    }
    
    void configureTable() {
        // --- Linha do E (Erros padrão: E1) --- 
        table[NT_E][T_NUM]    = { 0, {"T", "E'"} };
        table[NT_E][T_LPAREN] = { 0, {"T", "E'"} };

        // --- Linha do E' (Erros padrão: E2) ---
        for(int j=0; j<9; j++) table[NT_E_LINE][j].error_code = 2;
        table[NT_E_LINE][T_PLUS]   = { 0, {"+", "T", "E'"} };
        table[NT_E_LINE][T_MINUS]  = { 0, {"-", "T", "E'"} };
        table[NT_E_LINE][T_RPAREN] = { 0, {} }; 
        table[NT_E_LINE][T_DOLLAR] = { 0, {} };

        // --- Linha do T (Erros padrão: E3) ---
        for(int j=0; j<9; j++) table[NT_T][j].error_code = 3;
        table[NT_T][T_NUM]    = { 0, {"F", "T'"} };
        table[NT_T][T_LPAREN] = { 0, {"F", "T'"} };

        // --- Linha do T' (Erros padrão: E2) ---
        for(int j=0; j<9; j++) table[NT_T_LINE][j].error_code = 2;
        table[NT_T_LINE][T_PLUS]   = { 0, {} };
        table[NT_T_LINE][T_MINUS]  = { 0, {} };
        table[NT_T_LINE][T_MULT]   = { 0, {"*", "F", "T'"} };
        table[NT_T_LINE][T_DIV]    = { 0, {"/", "F", "T'"} };
        table[NT_T_LINE][T_RPAREN] = { 0, {} };
        table[NT_T_LINE][T_DOLLAR] = { 0, {} };

        // --- Linha do F (Erros padrão: E3) ---
        for(int j=0; j<9; j++) table[NT_F][j].error_code = 3;
        table[NT_F][T_NUM]    = { 0, {"P", "F'"} };
        table[NT_F][T_LPAREN] = { 0, {"P", "F'"} };

        // --- Linha do F' (Erros padrão: E2) ---
        for(int j=0; j<9; j++) table[NT_F_LINE][j].error_code = 2;
        table[NT_F_LINE][T_PLUS]   = { 0, {} };
        table[NT_F_LINE][T_MINUS]  = { 0, {} };
        table[NT_F_LINE][T_MULT]   = { 0, {} };
        table[NT_F_LINE][T_DIV]    = { 0, {} };
        table[NT_F_LINE][T_EXP]    = { 0, {"^", "F"} };
        table[NT_F_LINE][T_RPAREN] = { 0, {} };
        table[NT_F_LINE][T_DOLLAR] = { 0, {} };

        // --- Linha do P (Erros mesclados) ---
        table[NT_P][T_NUM]    = { 0, {"num"} };
        table[NT_P][T_PLUS]   = { 1, {} }; 
        table[NT_P][T_MINUS]  = { 1, {} };
        table[NT_P][T_MULT]   = { 1, {} };
        table[NT_P][T_DIV]    = { 1, {} };
        table[NT_P][T_EXP]    = { 1, {} };
        table[NT_P][T_LPAREN] = { 0, {"(", "E", ")"} };
        table[NT_P][T_RPAREN] = { 5, {} }; // E5: Parênteses não balanceado
        table[NT_P][T_DOLLAR] = { 1, {} };
    }
};

#endif