#ifndef SCANNER_H
#define SCANNER_H

#include "Token.h"
#include <fstream>
#include <string>

class Scanner{
    private:
        std::string buffer;
        std::size_t position;
        std::size_t line;

        Scanner();
        Scanner(const Scanner&) = delete;
        Scanner& operator=(const Scanner&) = delete;

        char peek() const;
        char advance();
        bool eof() const;
        void skipWhitespace();

        Token scanIdentifierOrKeyword();
        Token scanNumber();
        Token scanOperatorOrDelimiter();
        Token scanComment();
        Token makeToken(TipoDeToken tipo, const std::string &lexema);
        Token makeError(const std::string &lexema);

        TipoDeToken keywordType(const std::string &lexema) const;
    
    public:
        static Scanner &getInstance();
        bool loadFile(const std::string &filename);
        Token nextToken();
        void reset();

    };
#endif