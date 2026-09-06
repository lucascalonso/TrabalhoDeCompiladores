#include "scanner.h"
#include <iterator>
#include <cstring>
#include <cctype>

Scanner::Scanner() : position(0), line(1) {}

Scanner& Scanner::getInstance(){
    static Scanner instance;
    return instance;
}

bool Scanner::loadFile(const std::string &filename){
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    buffer.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    position = 0;
    return true;
}

char Scanner::peek() const {
    if(eof()) return '\0';
    return buffer[position]; 
}

char Scanner::advance() {
    if(eof()) return '\0';
    return buffer[position++];
}

bool Scanner::eof() const { return position >= buffer.size(); }

void Scanner::skipWhitespace() {
    while(!eof()) {
        char c = peek();

        if (c == ' ' || c == '\t' || c == '\r') {
            advance();

        } else if (c == '\n') {
            advance();
            line++;

        } else break;
    } 
}                                   

Token Scanner::makeToken(TipoDeToken tipo, const std::string &lexema){
    Token token;
    token.tipo = tipo;
    std::strncpy(token.lexema, lexema.c_str(), sizeof(token.lexema) - 1); 
    token.lexema[sizeof(token.lexema) - 1] = '\0';  // não sei se precisa no 
    token.linha = line;
    return token;
}

Token Scanner::makeError(const std::string &lexema){
    return makeToken(TOKEN_ERROR, lexema);
}

Token Scanner::scanIdentifierOrKeyword() {
    std::string lexema;
    
    while(!eof()){
        char c = peek();

        if( c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'){ // ou usa std::isalpha(static_cast<unsigned char>(c) já que não pode ter número em identificador. Se puder, usar std::isdigit(static_cast<unsigned char>(c)
            lexema += advance();

        } else break;
    }

    TipoDeToken tipo = keywordType(lexema); // esse lexema é uma palavra marcada??

    if(tipo != TOKEN_ERROR){
        return makeToken(tipo, lexema);
    }

    return makeToken(TOKEN_IDENTIFIER, lexema);
}

TipoDeToken Scanner::keywordType(const std::string &lexema) const {
    if (lexema == "else") return TOKEN_ELSE;
    if (lexema == "int") return TOKEN_INT;
    if (lexema == "if") return TOKEN_IF;
    if (lexema == "while") return TOKEN_WHILE;
    if (lexema == "void") return TOKEN_VOID;
    if (lexema == "return") return TOKEN_RETURN;
    return TOKEN_ERROR;
}
                                                                                  
Token Scanner::scanNumber() {
    std::string lexema;
    while (!eof() && std::isdigit(static_cast<unsigned char>(peek()))){
        lexema += advance();
    }
    return makeToken(TOKEN_NUMBER, lexema);
}

Token Scanner::scanOperatorOrDelimiter(){
    char current = advance();

    switch(current){
        case '+': return makeToken(TOKEN_PLUS, "+");
        case '-': return makeToken(TOKEN_MINUS, "-");
        case '*': return makeToken(TOKEN_MULTIPLY, "*");
        case '/': 
            if(peek() == '*'){
                advance();
                char anterior;
                while(!eof()){
                    anterior = advance();
                    if(anterior == '\n') line++;
                    if(anterior == '*' && peek() == '/'){
                        advance();
                        break;
                    }
                }
                return makeToken(TOKEN_COMMENT, "/* ... */");
            }
            return makeToken(TOKEN_DIVIDE, "/");

        case '<':
            if(peek() == '='){
                advance();
                return makeToken(TOKEN_LESS_EQUAL, "<=");
            }
            return makeToken(TOKEN_LESS, "<");
        case '>':
            if(peek() == '='){
                advance();
                return makeToken(TOKEN_GREATER_EQUAL, ">=");
            }
            return makeToken(TOKEN_GREATER, ">");
        case '=':
            if(peek() == '='){
                advance();
                return makeToken(TOKEN_EQUAL_EQUAL, "==");
            }
            return makeToken(TOKEN_ASSIGN, "=");
        case '!':
            if(peek() == '='){
                advance();
                return makeToken(TOKEN_NOT_EQUAL, "!=");
            }
            break;

        case ';': return makeToken(TOKEN_SEMICOLON, ";");
        case ',': return makeToken(TOKEN_COMMA, ",");
        case '(': return makeToken(TOKEN_PAREN_OPEN, "(");
        case ')': return makeToken(TOKEN_PAREN_CLOSE, ")");
        case '[': return makeToken(TOKEN_BRACKET_OPEN, "[");
        case ']': return makeToken(TOKEN_BRACKET_CLOSE, "]");
        case '{': return makeToken(TOKEN_BRACE_OPEN, "{");
        case '}': return makeToken(TOKEN_BRACE_CLOSE, "}");
        default: return makeError(std::string(1, current));
    }
}

Token Scanner::nextToken() {
        skipWhitespace();
        if(eof()) return makeToken(TOKEN_EOF, "");
        char current = peek();

        if(std::isalpha(static_cast<unsigned char>(current))){
            return scanIdentifierOrKeyword();
        } else if(std::isdigit(static_cast<unsigned char>(current))){
            return scanNumber();
        } else {
            return scanOperatorOrDelimiter();
        }
}


