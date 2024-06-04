#ifndef lisp_lex_hpp
#define lisp_lex_hpp

#include <iostream>
#include <vector>
#include <stack>
#include "lisp_objects.hpp"
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

const char eofChar = '$';

class Buffer {
    private:
        string buffer;
        int inPos;
    public:
        Buffer() {

        }
        void init(string& str) {
            buffer = str;
            inPos = 0;
        }
         char getChar() {
            return inPos < buffer.size() ? buffer[inPos]:eofChar;
        }
        char advance() {
            if (inPos+1 < buffer.size()) {
                inPos++;
                return buffer[inPos];
            }
            inPos++;
            return eofChar;
        }
        char reverse() {
            if (inPos-1 > 0) {
                inPos--;
            }
            return buffer[inPos];
        }
        bool isEOF() {
            return inPos >= buffer.size();
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
        bool is_skip(char c) {
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
        Lexeme extractNumber() {
            bool isReal = false;
            string number;
            while (!is_skip(buffer.getChar()) && (isdigit(buffer.getChar()) || buffer.getChar() == '.')) {
                number.push_back(buffer.getChar());
                if (buffer.getChar() == '.') isReal = true;
                buffer.advance();
            }
            return Lexeme((isReal ? REALNUM:NUMBER), number);
        }
        Lexeme extractWord() {
            string word;
            while (!is_skip(buffer.getChar()) && (isalpha(buffer.getChar()) || isdigit(buffer.getChar()) || buffer.getChar() == '-')) {
                word.push_back(buffer.getChar());
                buffer.advance();
            }
            return Lexeme(SYMBOL, word);
        }
        Lexeme checkSpecials() {
            switch (buffer.getChar()) {
                case '(': 
                    parStack.push(buffer.getChar());
                    return Lexeme(LPAREN, "(");
                case ')': 
                    if (parStack.empty() || parStack.top() != '(') {
                        cout<<"Error: Mismatched Parentheses!"<<endl;
                        return Lexeme(ERROR, "Mismatched Parentheses");
                    } else {
                        parStack.pop();
                    }
                    return Lexeme(RPAREN, ")");
                default:
                    break;
            }
            string sc;
            sc.push_back(buffer.getChar());
            return Lexeme(SYMBOL, sc);
        }
    public:
        Lexer() {

        }
        vector<Lexeme> lex(string input) {
            vector<Lexeme> tokens;
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
                    buffer.advance();
                }
            }
            return tokens;
        }
};

#endif