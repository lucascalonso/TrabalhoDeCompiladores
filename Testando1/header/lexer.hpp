#include <string>
#include <vector>
#include <cctype>

#ifndef LEXER_HPP
#define LEXER_HPP

enum class TokenType {
    PLUS, MINUS, MULT, DIV, EXP, L_PAREN, R_PAREN, NUM, END_OF_INPUT, ERROR
};

struct Token {
    TokenType type;
    std::string value;
};

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.length()) {
        char ch = input[i];

        if (std::isspace(ch)) {
            i++;
            continue;
        }

        if (ch == '+') { tokens.push_back({TokenType::PLUS, "+"}); i++; }
        else if (ch == '-') { tokens.push_back({TokenType::MINUS, "-"}); i++; }
        else if (ch == '*') { tokens.push_back({TokenType::MULT, "*"}); i++; }
        else if (ch == '/') { tokens.push_back({TokenType::DIV, "/"}); i++; }
        else if (ch == '^') { tokens.push_back({TokenType::EXP, "^"}); i++; }
        else if (ch == '(') { tokens.push_back({TokenType::L_PAREN, "("}); i++; }
        else if (ch == ')') { tokens.push_back({TokenType::R_PAREN, ")"}); i++; }
        else if (std::isdigit(ch)) {
            std::string num_str = "";
            bool has_dot = false;
            
            while (i < input.length() && (std::isdigit(input[i]) || input[i] == '.')) {
                if (input[i] == '.') {
                    if (has_dot) { 
                        // Erro: Segundo ponto decimal encontrado (ex: 2.5.3)
                        num_str += input[i];
                        tokens.push_back({TokenType::ERROR, "Numero malformado: " + num_str});
                        i++;
                        // Em vez de dar return, quebramos ou continuamos para pegar o resto do arquivo
                        break; 
                    }
                    has_dot = true;
                }
                num_str += input[i];
                i++;
            }
            
            // Validação: se terminar com ponto (ex: "2."), é um erro léxico
            if (!num_str.empty() && num_str.back() == '.') {
                tokens.push_back({TokenType::ERROR, "Numero malformado: " + num_str});
            } else if (tokens.empty() || tokens.back().type != TokenType::ERROR) {
                tokens.push_back({TokenType::NUM, num_str});
            }
            
        } else {
            // Caractere inválido na linguagem (ex: @, !, a, b, c)
            tokens.push_back({TokenType::ERROR, "Caractere invalido: " + std::string(1, ch)});
            i++;
        }
    }
    
    tokens.push_back({TokenType::END_OF_INPUT, "$"});
    return tokens;
}

#endif // LEXER_HPP
