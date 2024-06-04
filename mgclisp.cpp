#include <iostream>
#include <vector>
#include <stack>
#include "lisp_objects.hpp"
#include "lisp_lex.hpp"
using namespace std;

Object* eval(Object*,List*);

Object* envLookUp(List* env, Object* obj) {
    for (ListNode* it = env->first(); it != nullptr; it = it->next) {
        if (compareObject(obj, it->info->bindingVal->symbol))
            return it->info->bindingVal->value;
    }
    return makeErrorObject("Not Found");
}

Object* specialDefine(List* args, List* env) {
    Object* label = args->first()->info;
    Object* value = args->first()->next->info;
    env->append(makeBindingObject(makeBinding(label, value)));
    return label;
}

Object* specialIf(List* args, List* env) {
    Object* test = args->first()->info;
    Object* posRes = args->first()->next->info;
    Object* negRes = args->first()->next->next->info;
    if (test->type == AS_SYMBOL && *test->strVal == "NIL")
        return eval(negRes, env);
    return eval(posRes, env);
}

Object* specialLambda(List* args, List* env) {
    Object* argsList = args->first()->info;
    Object* code = args->first()->next->info;  
    List* argList = argsList->listVal;
    Object elip;
    elip.type = AS_SYMBOL;
    elip.strVal = new string("..");
    int index;
    procType pt = PROCEDURE_LAMBDA;
    if ((index = argList->find(&elip)) != -1) {
        if (index == argList->size() - 2) {
            pt = PROCEDURE_VARLAMBDA;
            argList = argList->copyOmitNth(index);
        } else {
            return makeErrorObject("<error>");
        }
    }
    return makeFunctionObject(allocProcedure(argList, code, env, pt));
}

int num_operators = 3;
SpecialForm specialForms[] = {
    {"define", 2, {NO_EVAL, EVAL}, &specialDefine},
    {"if", 3, {EVAL, NO_EVAL, NO_EVAL}, &specialIf},
    {"lambda", 2, {NO_EVAL, NO_EVAL}, &specialLambda}
};

Object* applySpecial(SpecialForm* special, List* args, List* environment) {
    ListNode* currArg = args->first();
    List* evaluated_args = new List();
    for (int i = 0; i < args->size(); i++) {
        Object* result = currArg->info;
        if (special->num_args != 0 && special->flags[i] == EVAL) {
            result = eval(currArg->info, environment);
        }
        evaluated_args->append(result);
        currArg = currArg->next;
    }
    return special->function(evaluated_args, environment);
}

Object* applyMathPrimitive(List* args, string op) {
    ListNode* curr = args->first();
    double result = curr->info->type == AS_INT ? curr->info->intVal:curr->info->realVal;
    for (curr = args->first()->next; curr != nullptr; curr = curr->next) {
        if (curr->info->type == AS_INT || curr->info->type == AS_REAL) {
            double t = curr->info->type == AS_INT ? curr->info->intVal:curr->info->realVal;
            cout<<"result "<<result<<endl;
            cout<<"     t "<<t<<endl;
            if (op == "+") result += t;
            if (op == "-") result -= t;
            if (op == "*") result *= t;
            if (op == "/") result /= t;
        }
    }
    return makeRealObject(result);
}
Object* primitivePlus(List* args) {
    cout<<"Applying + to "<<args->asString()<<endl;
    return applyMathPrimitive(args, "+");
}
Object* primitiveMinus(List* args) {
    return applyMathPrimitive(args, "-");
}
Object* primitiveMultiply(List* args) {
    return applyMathPrimitive(args, "*");
}
Object* primitiveDivide(List* args) {
    return applyMathPrimitive(args, "/");
}
Object* primitiveLess(List* args) {
    Object* first = args->first()->info;
    Object* second = args->first()->next->info;
    return makeBoolObject(toString(first) < toString(second));
}
Object* primitiveGreater(List* args) {
    Object* first = args->first()->info;
    Object* second = args->first()->next->info;
    return makeBoolObject(toString(second) < toString(first));
}
Object* primitiveEquals(List* args) {
    Object* first = args->first()->info;
    Object* second = args->first()->next->info;
    return makeBoolObject(toString(first) == toString(second));
}
Object* primitivePrint(List* args) {
    cout<<toString(makeListObject(args))<<endl;
    return new Object();
}
Object* primitiveCar(List* args) {
    return args->first()->info;
}

Object* primitive_cdr(List* args) {
    return makeListObject(args->rest());
}

List* makeNewEnvironment(List* vars, List* vals) {
    List* nenv = new List();
    ListNode* currVar = vars->first();
    ListNode* currVal = vals->first();
    for (int i = 0; i < vars->size(); i++) {
        nenv->append(makeBindingObject(makeBinding(currVar->info, currVal->info)));
        currVar = currVar->next;
        currVal = currVal->next;
    }
    return nenv;
}

Object* apply(Procedure* proc, List* args, List* env) {
    if (proc->type == PROCEDURE_PRIMITIVE) {
        cout<<"Primitive"<<endl;
        return proc->function(args);
    }
    if (proc->type == PROCEDURE_LAMBDA) {
        cout<<"Lambda"<<endl;
        List* nenv = makeNewEnvironment(proc->free_vars, args);
        nenv->addMissing(proc->env);
        nenv->addMissing(env);
        return eval(proc->code, nenv);
    }
    return makeErrorObject("An error in apply occured");
}

Object* eval(Object* obj, List* env) {
    if (obj->type == AS_INT) {
        cout<<"Evaluated "<<toString(obj)<<" as int"<<endl;
        return obj;
    }
    if (obj->type == AS_REAL) {
        cout<<"Evaluated "<<toString(obj)<<" as real"<<endl;
        return obj;
    }
    if (obj->type == AS_FUNCTION) {
        cout<<"Evaluated "<<toString(obj)<<" as function"<<endl;
        return obj;
    }
    if (obj->type == AS_ERROR) {
        cout<<"Evaluated "<<toString(obj)<<" as Error"<<endl;
        return obj;
    }
    if (obj->type == AS_SYMBOL) {
        Object* ret =  envLookUp(env, obj);
        cout<<"Evaluated "<<toString(ret)<<" From Symbol "<<toString(obj)<<endl;
        return ret;
    }
    if (obj->type == AS_LIST) {
        List* list = obj->listVal;
        if (list->empty())
            return obj;
        if (list->first()->info->type == AS_SYMBOL) {
            string sym = *list->first()->info->strVal;
            for (int i = 0; i < num_operators; i++) {
                if (sym == specialForms[i].name) {
                    List* arguments = list->rest();
                    return applySpecial(&specialForms[i], arguments, env);
                }
            }
        }
        List* evaluated_args = new List();
        for (ListNode* curr = list->first(); curr != nullptr; curr = curr->next) {
            Object* el = eval(curr->info, env);
            evaluated_args->append(el);
        }
        if (evaluated_args->first()->info->type == AS_FUNCTION)  {
            List* arguments = evaluated_args->rest();
            return apply(evaluated_args->first()->info->procedureVal, arguments, env);
        }
        return makeListObject(evaluated_args);
    }
    return makeErrorObject("Error during eval");
}

class REPL {
    private:
        Lexer lexer;
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
        List* env;
        void addBinding(Binding* binding) {
            env->append(makeBindingObject(binding));
        }
        void addPrimitive(string symbol, int numArgs, Object* (*func)(List*)) {
            addBinding(makeBinding(makeSymbolObject(symbol), makeFunctionObject(makeProcedure(func, numArgs))));
        }
    public:
        REPL() {
            env = new List();
            addPrimitive("+", 0, &primitivePlus);
            addPrimitive("-", 0, &primitiveMinus);
            addPrimitive("/", 0, &primitiveDivide);
            addPrimitive("*", 0, &primitiveMultiply);
            addPrimitive("<", 0, &primitiveLess);
            addPrimitive("eq", 0, &primitiveEquals);
            addPrimitive("print", 0, &primitivePrint);
            addPrimitive(">", 0, &primitiveGreater);
            addPrimitive("car", 0, &primitiveCar);
            addPrimitive("cdr", 0, &primitive_cdr);
        }
        void start() {
            string input;
            while (true) {
                cout<<"repl> ";
                getline(cin, input);
                auto tokens = lexer.lex(input);
                for (auto m : tokens) {
                    cout<<"< "<<tokenStr[m.token]<<", "<<m.strVal<<" > "<<endl;
                }
                int inpos = 0;
                List* asList = parseToList(tokens, inpos);
                cout<<toString(eval(makeListObject(asList), env))<<endl;
            }
        }
};


int main() {
    REPL repl;
    repl.start();
    return 0;
}