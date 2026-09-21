#include "../include/scanner.h"
#include "../include/TabelaDeTransicao.h"

#include <iterator>
#include <cstring>
#include <cctype>
#include <string>

Scanner::Scanner() : position(0), line(1) {}

Scanner& Scanner::getInstance() {
    static Scanner instance;
    return instance;
}

bool Scanner::loadFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    buffer.assign(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );

    position = 0;
    line = 1;

    return true;
}

void Scanner::reset() {
    position = 0;
    line = 1;
}

char Scanner::peek() const {
    if (eof()) {
        return '\0';
    }

    return buffer[position];
}

char Scanner::advance() {
    if (eof()) {
        return '\0';
    }

    return buffer[position++];
}

bool Scanner::eof() const {
    return position >= buffer.size();
}

void Scanner::skipWhitespace() {
    while (!eof()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            advance();
            ++line;
        } else {
            break;
        }
    }
}

Token Scanner::makeToken(TipoDeToken tipo, const std::string& lexema) {
    Token token;
    token.tipo = tipo;

    std::strncpy(
        token.lexema,
        lexema.c_str(),
        sizeof(token.lexema) - 1
    );

    token.lexema[sizeof(token.lexema) - 1] = '\0';
    token.linha = line;

    return token;
}

Token Scanner::makeError(const std::string& lexema) {
    return makeToken(TOKEN_ERROR, lexema);
}

Token Scanner::nextToken() {
    skipWhitespace();

    if (eof()) {
        return makeToken(TOKEN_EOF, "");
    }

    const std::size_t startPosition = position;
    const std::size_t startLine = line;

    Estado estado = Q_0;
    Estado ultimoEstadoAceitacao = Q_ERRO;

    // Posição imediatamente depois do último estado de aceitação
    std::size_t ultimaPosicaoAceitacao = startPosition;
    std::string lexema;
    std::string ultimoLexemaAceito;

    while (!eof()) {
        const char c = peek();

        /*
         * A matriz não possui colunas para whitespace. Dentro de um
         * comentário, espaços e quebras de linha fazem parte
         * do lexema e precisam ser consumidos
         */
        const int indiceColuna = coluna(c);

        if (isCommentState(estado) && indiceColuna < 0) {
            lexema += advance();
            if (c == '\n') {
                ++line;
            }
            // q_/** + caractere diferente de '*' e '/'
            // volta para q_/*
            estado = Q_BARRA_ASTERISCO;
            continue;
        }

        // Caractere fora do alfabeto da matriz
        if (indiceColuna < 0) {
            break;
        }

        const Estado proximoEstado =
            static_cast<Estado>(MATRIZ[estado][indiceColuna]);

        // Não existe transição válida para esse caractere
        if (proximoEstado == Q_ERRO) {
            break;
        }

        // Consome somente depois de confirmar que existe transição
        lexema += advance();
        estado = proximoEstado;

        if (isAcceptingState(estado)) {
            ultimoEstadoAceitacao = estado;
            ultimaPosicaoAceitacao = position;
            ultimoLexemaAceito = lexema;
        }
    }

    /*
     * Não houve nenhum estado final.
     *
     * Exemplo: "!" sozinho. A matriz leva q0 -> q_!, mas q_! não é
     * estado de aceitação porque "!" não é token válido em C-
     */
    if (ultimoEstadoAceitacao == Q_ERRO) {
        // Caso nada tenha sido consumido, o caractere atual é o erro
        if (position == startPosition) {
            const char erro = advance();
            return makeError(std::string(1, erro));
        }

        /*
         * Já consumimos um prefixo inválido, como "!" ou um comentário
         * sem fechamento. O token de erro inclui o que foi lido
         */
        Token token = makeError(lexema);
        token.linha = static_cast<int>(startLine);
        return token;
    }

    /*
     * A matriz é utilizada com maximal munch:
     * guardamos o último estado final alcançado e seu lexema
     *
     * Normalmente o laço já para antes de consumir o caractere que
     * causaria erro, mas este recorte também mantém a lógica correta
     * caso a tabela contenha estados intermediários adicionais
     */
    if (position != ultimaPosicaoAceitacao) {
        position = ultimaPosicaoAceitacao;
        lexema = ultimoLexemaAceito;

        // A matriz atual só aceita quebras de linha dentro de comentários.
        // Se no futuro houver outro token que atravesse linha, o controle
        // de 'line' precisará acompanhar também o recuo
    } else {
        lexema = ultimoLexemaAceito;
    }

    TipoDeToken tipo = tokenTypeFromState(ultimoEstadoAceitacao);

    Token token = makeToken(tipo, lexema);
    token.linha = static_cast<int>(startLine);

    return token;
}

bool Scanner::isAcceptingState(Estado estado) {
    switch (estado) {
        case Q_I:
        case Q_IN:
        case Q_W:
        case Q_WH:
        case Q_WHI:
        case Q_WHIL:
        case Q_E:
        case Q_EL:
        case Q_ELS:
        case Q_V:
        case Q_VO:
        case Q_VOI:
        case Q_R:
        case Q_RE:
        case Q_RET:
        case Q_RETU:
        case Q_RETUR:
        case Q_ID:
            return true;

        case Q_IF:
        case Q_INT:
        case Q_WHILE:
        case Q_ELSE:
        case Q_VOID:
        case Q_RETURN:
            return true;

        case Q_NUM:
        case Q_MAIS:
        case Q_MENOS:
        case Q_ASTERISCO:
        case Q_BARRA:
        case Q_COMMENT:
        case Q_ASSIGN:
        case Q_EQUAL:
        case Q_NOTEQUAL:
        case Q_MAIOR:
        case Q_MAIORIGUAL:
        case Q_MENOR:
        case Q_MENORIGUAL:
        case Q_VIRGULA:
        case Q_PONTO_VIRGULA:
        case Q_FECHA_PAREN:
        case Q_ABRE_PAREN:
        case Q_ABRE_COLCHETE:
        case Q_FECHA_COLCHETE:
        case Q_ABRE_CHAVE:
        case Q_FECHA_CHAVE:
            return true;

        case Q_0:
        case Q_BARRA_ASTERISCO:
        case Q_BARRA_ASTERISCO_ASTERISCO:
        case Q_EXCLAMACAO:
        case Q_ERRO:
            return false;

        default:
            return false;
    }
}

TipoDeToken Scanner::tokenTypeFromState(Estado estado) {
    switch (estado) {
        case Q_IF:         return TOKEN_IF;
        case Q_INT:        return TOKEN_INT;
        case Q_WHILE:      return TOKEN_WHILE;
        case Q_ELSE:       return TOKEN_ELSE;
        case Q_VOID:       return TOKEN_VOID;
        case Q_RETURN:     return TOKEN_RETURN;

        case Q_I:
        case Q_IN:
        case Q_W:
        case Q_WH:
        case Q_WHI:
        case Q_WHIL:
        case Q_E:
        case Q_EL:
        case Q_ELS:
        case Q_V:
        case Q_VO:
        case Q_VOI:
        case Q_R:
        case Q_RE:
        case Q_RET:
        case Q_RETU:
        case Q_RETUR:
        case Q_ID:
            return TOKEN_IDENTIFIER;

        case Q_NUM:        return TOKEN_NUMBER;
        case Q_MAIS:       return TOKEN_PLUS;
        case Q_MENOS:      return TOKEN_MINUS;
        case Q_ASTERISCO:  return TOKEN_MULTIPLY;
        case Q_BARRA:      return TOKEN_DIVIDE;
        case Q_COMMENT:    return TOKEN_COMMENT;
        case Q_ASSIGN:     return TOKEN_ASSIGN;
        case Q_EQUAL:      return TOKEN_EQUAL_EQUAL;
        case Q_NOTEQUAL:   return TOKEN_NOT_EQUAL;
        case Q_MAIOR:      return TOKEN_GREATER;
        case Q_MAIORIGUAL: return TOKEN_GREATER_EQUAL;
        case Q_MENOR:      return TOKEN_LESS;
        case Q_MENORIGUAL: return TOKEN_LESS_EQUAL;
        case Q_VIRGULA:    return TOKEN_COMMA;
        case Q_PONTO_VIRGULA: return TOKEN_SEMICOLON;
        case Q_FECHA_PAREN:   return TOKEN_PAREN_CLOSE;
        case Q_ABRE_PAREN:    return TOKEN_PAREN_OPEN;
        case Q_ABRE_COLCHETE: return TOKEN_BRACKET_OPEN;
        case Q_FECHA_COLCHETE: return TOKEN_BRACKET_CLOSE;
        case Q_ABRE_CHAVE:    return TOKEN_BRACE_OPEN;
        case Q_FECHA_CHAVE:   return TOKEN_BRACE_CLOSE;

        default:
            return TOKEN_ERROR;
    }
}

bool Scanner::isCommentState(Estado estado) {
    return estado == Q_BARRA_ASTERISCO ||
           estado == Q_BARRA_ASTERISCO_ASTERISCO;
}

bool Scanner::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}
