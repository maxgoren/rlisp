#include <iostream>
#include <vector>
#include <stack>
#include "lisp_objects.hpp"
#include "lisp_lex.hpp"
#include "evalapply.hpp"
using namespace std;

class REPL {
    private:
        List* parseToList(vector<Lexeme>& lexemes, int& index) {
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
                        result->append(makeIntObject(stof(lexemes[index].strVal.c_str())));
                        break;
                }
            }
            return result;
        }
        Lexer lexer;
        EvalApply evaluator;
    public:
        REPL() {
            cout<<"[mgclisp2]"<<endl;
        }
        void start() {
            string input;
            while (true) {
                cout<<"repl> ";
                getline(cin, input);
                auto tokens = lexer.lex(input);
                int inpos = 0;
                List* asList = parseToList(tokens, inpos);
                cout<<toString(evaluator.eval(asList))<<endl;
            }
        }
};


int main() {
    // (define fib (lambda (x) (if (< x 2) 1 (+ (fib (- x 1)) (fib (- x 2))))))
    REPL repl;
    repl.start();
    return 0;
}