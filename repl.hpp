#ifndef repl_hpp
#define repl_hpp
#include <iostream>
#include <vector>
#include "objects.hpp"
#include "lex.hpp"
#include "evalapply.hpp"
#include "readline/readline.h"
using namespace std;

class REPL {
    private:
        Lexer lexer;
        EvalApply evaluator;
        List* parseToList(vector<Lexeme>& lexemes, int& index);
    public:
        REPL();
        void start();
};

REPL::REPL() {
    cout<<"[mgclisp repl]"<<endl;
}

void REPL::start() {
    string input;
    bool running = true;
    int exprNo = 1;
    bool tracing = false;
     while (running) {
        string prompt = "mgclisp(" + to_string(exprNo) + ")> ";
        //If you dont want to use GNU readline, replace the following line
        //with getline(cin, input);
        input = readline(prompt.c_str());
        if (input.empty())
            continue;
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
    bool shouldQuote = false;
    if (lexemes[index].token == LPAREN) index++;
    else shouldQuote = true;
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
    if (shouldQuote) {
        List* nr = new List();
        nr->append(makeSymbolObject("'"));
        nr->append(makeListObject(result));
        result = nr;
    }
    return result;
}

#endif