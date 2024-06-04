#ifndef evalapply_hpp
#define evalapply_hpp
#include <iostream>
#include <vector>
#include <stack>
#include "lisp_objects.hpp"
#include "lisp_lex.hpp"
using namespace std;

class EvalApply {
    private:
        int num_operators = 3;
        SpecialForm* specialForms;
        Object* envLookUp(List* env, Object* obj);
        
        Object* specialDefine(List* args, List* env);
        Object* specialIf(List* args, List* env);
        Object* specialLambda(List* args, List* env);

        Object* primitivePlus(List* args);
        Object* primitiveMinus(List* args);
        Object* primitiveMultiply(List* args);
        Object* primitiveDivide(List* args);
        Object* primitiveLess(List* args);
        Object* primitiveGreater(List* args);
        Object* primitiveEquals(List* args); 
        Object* primitivePrint(List* args);
        Object* primitiveCar(List* args);
        Object* primitiveCdr(List* args);

        Object* applySpecial(SpecialForm* special, List* args, List* environment);
        Object* applyMathPrimitive(List* args, string op);
        Object* apply(Function* proc, List* args, List* env);
        List* makeNewEnvironment(List* vars, List* vals);
        void addBinding(Binding* binding);
        void addPrimitive(string symbol, int numArgs, Object* (EvalApply::*func)(List*));
        Object* eval(Object* obj, List* env);
        List* environment;
    public:
        EvalApply() {
            environment = new List();
            specialForms = new SpecialForm[3] {
                {"define", 2, {NO_EVAL, EVAL}, &EvalApply::specialDefine},
                {"if", 3, {EVAL, NO_EVAL, NO_EVAL}, &EvalApply::specialIf},
                {"lambda", 2, {NO_EVAL, NO_EVAL}, &EvalApply::specialLambda}
            };
            addPrimitive("+", 0, &EvalApply::primitivePlus);
            addPrimitive("-", 0, &EvalApply::primitiveMinus);
            addPrimitive("/", 0, &EvalApply::primitiveDivide);
            addPrimitive("*", 0, &EvalApply::primitiveMultiply);
            addPrimitive("<", 0, &EvalApply::primitiveLess);
            addPrimitive("eq", 0, &EvalApply::primitiveEquals);
            addPrimitive("print", 0, &EvalApply::primitivePrint);
            addPrimitive(">", 0, &EvalApply::primitiveGreater);
            addPrimitive("car", 0, &EvalApply::primitiveCar);
            addPrimitive("cdr", 0, &EvalApply::primitiveCdr);
        }
        Object* eval(List* expression);
};

void EvalApply::addBinding(Binding* binding) {
    environment->append(makeBindingObject(binding));
}
void EvalApply::addPrimitive(string symbol, int numArgs, Object* (EvalApply::*func)(List*)) {
    addBinding(makeBinding(makeSymbolObject(symbol), makeFunctionObject(makeFunction(func, numArgs))));
}

Object* EvalApply::envLookUp(List* env, Object* obj) {
    for (ListNode* it = env->first(); it != nullptr; it = it->next) {
        if (compareObject(obj, it->info->bindingVal->symbol))
            return it->info->bindingVal->value;
    }
    return makeErrorObject("Not Found");
}

Object* EvalApply::specialDefine(List* args, List* env) {
    Object* label = args->first()->info;
    Object* value = args->first()->next->info;
    env->append(makeBindingObject(makeBinding(label, value)));
    return label;
}

Object* EvalApply::specialIf(List* args, List* env) {
    Object* test = args->first()->info;
    Object* posRes = args->first()->next->info;
    Object* negRes = args->first()->next->next->info;
    if (test->type == AS_BOOL) {
        return test->boolVal ? eval(posRes, env):eval(negRes, env);
    }
    if (test->type == AS_SYMBOL && *test->strVal == "NIL")
        return eval(negRes, env);
    return eval(posRes, env);
}

Object* EvalApply::specialLambda(List* args, List* env) {
    Object* argsList = args->first()->info;
    Object* code = args->first()->next->info;  
    List* argList = argsList->listVal;
    Object elip;
    elip.type = AS_SYMBOL;
    elip.strVal = new string("..");
    int index;
    funcType pt = LAMBDA;
    if ((index = argList->find(&elip)) != -1) {
        if (index == argList->size() - 2) {
            pt = VARLAMBDA;
            argList = argList->copyOmitNth(index);
        } else {
            return makeErrorObject("<error>");
        }
    }
    return makeFunctionObject(allocFunction(argList, code, env, pt));
}

Object* EvalApply::primitivePlus(List* args) {
    return applyMathPrimitive(args, "+");
}
Object* EvalApply::primitiveMinus(List* args) {
    return applyMathPrimitive(args, "-");
}
Object* EvalApply::primitiveMultiply(List* args) {
    return applyMathPrimitive(args, "*");
}
Object* EvalApply::primitiveDivide(List* args) {
    return applyMathPrimitive(args, "/");
}
Object* EvalApply::primitiveLess(List* args) {
    Object* first = args->first()->info;
    Object* second = args->first()->next->info;
    bool result;
    if ((first->type == AS_INT || first->type == AS_REAL) && (second->type == AS_INT || second->type == AS_REAL)) {
        double lhs = first->type == AS_INT ? first->intVal:first->realVal;
        double rhs = second->type == AS_INT ? second->intVal:second->realVal;
        result = lhs < rhs;
    } else {
        result = toString(first) < toString(second);
    }
    return makeBoolObject(result);
}
Object* EvalApply::primitiveGreater(List* args) {
    Object* first = args->first()->info;
    Object* second = args->first()->next->info;
    bool result;
    if ((first->type == AS_INT || first->type == AS_REAL) && (second->type == AS_INT || second->type == AS_REAL)) {
        double lhs = first->type == AS_INT ? first->intVal:first->realVal;
        double rhs = second->type == AS_INT ? second->intVal:second->realVal;
        result = rhs < lhs;
    } else {
        result = toString(second) < toString(first);
    }
    return makeBoolObject(result);
}
Object* EvalApply::primitiveEquals(List* args) {
    Object* first = args->first()->info;
    Object* second = args->first()->next->info;
    return makeBoolObject(compareObject(first, second));
}
Object* EvalApply::primitivePrint(List* args) {
    List* evaldArgs = new List();
    for (ListNode* it = args->first(); it != nullptr; it = it->next) {
        Object* ce = eval(it->info, environment);
        evaldArgs->append(ce);
    }
    cout<<evaldArgs->asString()<<endl;
    return new Object();
}
Object* EvalApply::primitiveCar(List* args) {
    return args->first()->info;
}

Object* EvalApply::primitiveCdr(List* args) {
    return makeListObject(args->rest());
}

List* EvalApply::makeNewEnvironment(List* vars, List* vals) {
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

Object* EvalApply::applySpecial(SpecialForm* special, List* args, List* environment) {
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
    auto m = special->func;
    return (this->*m)(evaluated_args, environment);
}

Object* EvalApply::applyMathPrimitive(List* args, string op) {
    ListNode* curr = args->first();
    double result = curr->info->type == AS_INT ? curr->info->intVal:curr->info->realVal;
    for (curr = args->first()->next; curr != nullptr; curr = curr->next) {
        if (curr->info->type == AS_INT || curr->info->type == AS_REAL) {
            double t = curr->info->type == AS_INT ? curr->info->intVal:curr->info->realVal;
            if (op == "+") result += t;
            if (op == "-") result -= t;
            if (op == "*") result *= t;
            if (op == "/") result /= t;
        }
    }
    return makeRealObject(result);
}

Object* EvalApply::apply(Function* proc, List* args, List* env) {
    if (proc->type == PRIMITIVE) {
        auto m = proc->func;
        return (this->*m)(args);
    }
    if (proc->type == LAMBDA) {
        List* nenv = makeNewEnvironment(proc->free_vars, args);
        nenv->addMissing(proc->env);
        nenv->addMissing(env);
        return eval(proc->code, nenv);
    }
    return makeErrorObject("An error in apply occured");
}

Object* EvalApply::eval(Object* obj, List* env) {
    if (obj->type == AS_INT) {
        //cout<<"Evaluated "<<toString(obj)<<" as int"<<endl;
        return obj;
    }
    if (obj->type == AS_REAL) {
        //cout<<"Evaluated "<<toString(obj)<<" as real"<<endl;
        return obj;
    }
    if (obj->type == AS_FUNCTION) {
        //cout<<"Evaluated "<<toString(obj)<<" as function"<<endl;
        return obj;
    }
    if (obj->type == AS_ERROR) {
        //cout<<"Evaluated "<<toString(obj)<<" as Error"<<endl;
        return obj;
    }
    if (obj->type == AS_SYMBOL) {
        Object* ret =  envLookUp(env, obj);
       // cout<<"Evaluated "<<toString(ret)<<" From Symbol "<<toString(obj)<<endl;
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

Object* EvalApply::eval(List* expr) {
    return eval(makeListObject(expr), environment);
}

#endif