#ifndef lex_hpp
#define lex_hpp
#include <iostream>
#include <vector>
#include <stack>
#include "buffer.hpp"
using namespace std;


template <class T>
struct Stack : public stack<T> {
    T pop() {
        T ret = stack<T>::top();
        stack<T>::pop();
        return ret;
    }
    void clear() {
        while (!stack<T>::empty()) stack<T>::pop();
    }
};


enum Token {
    LPAREN, RPAREN, SYMBOL, NUMBER, REALNUM, ERROR
};

vector<string> tokenStr = { "LPAREN", "RPAREN", "SYMBOL", "NUMBER", "REALNUM", "ERROR" };

struct Lexeme {
    Token token;
    string strVal;
    Lexeme(Token t, string s) : token(t), strVal(s) { }
};

class Lexer {
    private:
        Buffer buffer;
        Stack<char> parStack;
        vector<Lexeme> tokens;
        bool is_skip(char c);
        Lexeme extractNumber();
        Lexeme extractWord();
        Lexeme checkSpecials();
    public:
        Lexer();
        vector<Lexeme> lex(string input);
};

Lexer::Lexer() {

}

vector<Lexeme> Lexer::lex(string input) {
    tokens.clear();
    parStack.clear();
    buffer.init(input);
    while (!buffer.isEOF()) {
        if (is_skip(buffer.getChar())) {
            buffer.advance();
            continue;
        } else if (isdigit(buffer.getChar())) {
            tokens.push_back(extractNumber());
        } else if (isalpha(buffer.getChar())) {
            tokens.push_back(extractWord());
        } else {
            tokens.push_back(checkSpecials());
            if (tokens.back().token != NUMBER && tokens.back().token != REALNUM)
                buffer.advance();
        }
    }
    if (!parStack.empty()) {
        cout<<"Error: Mismatched Parentheses!"<<endl;
        tokens.clear();
        tokens.push_back(Lexeme(ERROR, "Mismatched Parentheses"));
    }
    return tokens;
}

bool Lexer::is_skip(char c) {
    switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
        default:
            return false;
    }
    return false;
}

Lexeme Lexer::extractNumber() {
    bool isReal = false;
    string number;
    while (!is_skip(buffer.getChar()) && (isdigit(buffer.getChar()) || buffer.getChar() == '.')) {
        number.push_back(buffer.getChar());
        if (buffer.getChar() == '.') isReal = true;
        buffer.advance();
    }
    return Lexeme((isReal ? REALNUM:NUMBER), number);
}

Lexeme Lexer::extractWord() {
    string word;
    while (!is_skip(buffer.getChar()) && (isalpha(buffer.getChar()) || isdigit(buffer.getChar()) || buffer.getChar() == '-' || buffer.getChar() == '?')) {
        word.push_back(buffer.getChar());
        buffer.advance();
    }
    return Lexeme(SYMBOL, word);
}

Lexeme Lexer::checkSpecials() {
    switch (buffer.getChar()) {
        case '(': 
            parStack.push(buffer.getChar());
            return Lexeme(LPAREN, "(");
        case ')': 
            if (parStack.empty() || parStack.top() != '(') {
                cout<<"Error: Mismatched Parentheses!"<<endl;
                tokens.clear();
                return Lexeme(ERROR, "Mismatched Parentheses");
            } else {
                parStack.pop();
            }
            return Lexeme(RPAREN, ")");
        case '-':
            buffer.advance();
            if (isdigit(buffer.getChar())) {
                Lexeme lexeme = extractNumber();
                lexeme.strVal = string("-").append(lexeme.strVal);
                return lexeme;
            }
            buffer.reverse();
            break;
        default:
            break;
    }
    string sc;
    sc.push_back(buffer.getChar());
    return Lexeme(SYMBOL, sc);
}

#endif