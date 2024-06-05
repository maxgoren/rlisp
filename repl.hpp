#ifndef repl_hpp
#define repl_hpp
#include <iostream>
#include <vector>
#include <stack>
#include "objects.hpp"
#include "lex.hpp"
#include "evalapply.hpp"
using namespace std;

class REPL {
    private:
        List* parseToList(vector<Lexeme>& lexemes, int& index);
        Lexer lexer;
        EvalApply evaluator;
    public:
        REPL();
        void start();
};

REPL::REPL() {
    cout<<"[mgclisp2]"<<endl;
}

void REPL::start() {
    string input;
    bool running = true;
    int exprNo = 1;
    bool tracing = false;
    while (running) {
        cout<<"mgclisp("<<exprNo<<")> ";
        getline(cin, input);
        if (input == "quit") {
            running = false;
        } else if (input == ".trace") {
            tracing = !tracing;
            evaluator.setTrace(tracing);
        } else {
            auto tokens = lexer.lex(input);
            int inpos = 0;
            cout<<toString(evaluator.eval(parseToList(tokens, inpos)))<<endl;
        }
        exprNo++;
    }
}

List* REPL::parseToList(vector<Lexeme>& lexemes, int& index) {
    List* result = new List();
    if (lexemes[index].token == LPAREN) index++;
    for (; index < lexemes.size(); index++) {
        switch (lexemes[index].token) {
            case LPAREN: 
                result->append(makeListObject(parseToList(lexemes, index)));
                break;
            case RPAREN:
                return result;
            case SYMBOL:
                result->append(makeSymbolObject(lexemes[index].strVal));
                break;
            case NUMBER:
                result->append(makeIntObject(atoi(lexemes[index].strVal.c_str())));
                break;
            case REALNUM:
                result->append(makeRealObject(stof(lexemes[index].strVal.c_str())));
                break;
        }
    }
    return result;
}

#endif