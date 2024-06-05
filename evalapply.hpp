#ifndef evalapply_hpp
#define evalapply_hpp
#include <iostream>
#include <vector>
#include <stack>
#include "objects.hpp"
#include "lex.hpp"
#include "list.hpp"
using namespace std;

class EvalApply {
    private:
        bool loud;
        void say(string s);
        int num_operators;
        SpecialForm* specialForms;

        Object* specialDefine(List* args, List* env);
        Object* specialIf(List* args, List* env);
        Object* specialLambda(List* args, List* env);
        Object* specialQuote(List* args, List* env);
        Object* specialSet(List* args, List* env);
        Object* specialDo(List* args, List* env);

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
        Object* primitivePush(List* args);

        Object* applySpecial(SpecialForm* special, List* args, List* environment);
        Object* applyMathPrimitive(List* args, string op);
        Object* apply(Function* proc, List* args, List* env);
        Object* eval(Object* obj, List* env);

        List* makeNewEnvironment(List* vars, List* vals);
        void addBinding(Binding* binding);
        void addPrimitive(string symbol, Object* (EvalApply::*func)(List*));
        Object* envLookUp(List* env, Object* obj);
        List* environment;
    public:
        EvalApply(bool noisey = false);
        Object* eval(List* expression);
        void setTrace(bool trace);
};

void EvalApply::setTrace(bool trace) {
    loud = trace;
}

void EvalApply::say(string s) {
    if (loud)
        cout<<s<<endl;
}

void EvalApply::addBinding(Binding* binding) {
    environment->append(makeBindingObject(binding));
}
void EvalApply::addPrimitive(string symbol, Object* (EvalApply::*func)(List*)) {
    addBinding(makeBinding(makeSymbolObject(symbol), makeFunctionObject(makeFunction(func))));
}

EvalApply::EvalApply(bool noisey) {
    loud = noisey;
    environment = new List();
    num_operators = 6;
    specialForms = new SpecialForm[num_operators] {
        {"define", 2, {NO_EVAL, EVAL}, &EvalApply::specialDefine},
        {"if", 3, {EVAL, NO_EVAL, NO_EVAL}, &EvalApply::specialIf},
        {"lambda", 2, {NO_EVAL, NO_EVAL}, &EvalApply::specialLambda},
        {"'", 2, {NO_EVAL, NO_EVAL}, &EvalApply::specialQuote},
        {"set", 2, {NO_EVAL, EVAL}, &EvalApply::specialSet},
        {"do", 0, {}, &EvalApply::specialDo}
    };

    addPrimitive("+", &EvalApply::primitivePlus);
    addPrimitive("-", &EvalApply::primitiveMinus);
    addPrimitive("/", &EvalApply::primitiveDivide);
    addPrimitive("*", &EvalApply::primitiveMultiply);
    addPrimitive("<", &EvalApply::primitiveLess);
    addPrimitive("eq", &EvalApply::primitiveEquals);
    addPrimitive("print", &EvalApply::primitivePrint);
    addPrimitive(">", &EvalApply::primitiveGreater);
    addPrimitive("car", &EvalApply::primitiveCar);
    addPrimitive("cdr", &EvalApply::primitiveCdr);
    addPrimitive("push", &EvalApply::primitivePush);
}

Object* EvalApply::envLookUp(List* env, Object* obj) {
    for (ListNode* it = env->first(); it != nullptr; it = it->next) {
        if (compareObject(obj, it->info->bindingVal->symbol))
            return it->info->bindingVal->value;
    }
    return makeErrorObject("<Error: " + toString(obj) + " Not Found>");
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
    return makeFunctionObject(allocFunction(argList, code, env, LAMBDA));
}

Object* EvalApply::specialQuote(List* args, List* env) {
    return args->first()->info;
}

Object* EvalApply::specialSet(List* args, List* env) {
    Object* symbol = args->first()->info;
    Object* replacement = args->first()->next->info;
    for (ListNode* it = env->first(); it != nullptr; it = it->next) {
        if (compareObject(it->info->bindingVal->symbol, symbol)) {
            it->info->bindingVal->value = replacement;
            return replacement;
        }
    }
    env->append(makeBindingObject(makeBinding(symbol, replacement)));
    return replacement;
}

Object* EvalApply::specialDo(List* args, List* env) {
    Object* result;
    for (ListNode* it = args->first(); it != nullptr; it = it->next) {
        result = eval(it->info, env);
        if (result->type == AS_ERROR) {
            return result;
        }
    }
    return result;
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

Object* EvalApply::primitivePush(List* args) {
    if (args->first()->next->info->type != AS_LIST) {
        return makeErrorObject("<Error: Can only push to a list!>");
    }
    Object* toPush = args->first()->info;
    List* addTo = args->first()->next->info->listVal->copy();
    addTo->push(toPush);
    return makeListObject(addTo);
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
        say("Evaluated " + toString(obj) + " as int");
        return obj;
    }
    if (obj->type == AS_REAL) {
        say("Evaluated " + toString(obj) + " as real");
        return obj;
    }
    if (obj->type == AS_FUNCTION) {
        say("Evaluated " + toString(obj) + " as function");
        return obj;
    }
    if (obj->type == AS_ERROR) {
        say("Evaluated " + toString(obj) + " as Error");
        return obj;
    }
    if (obj->type == AS_SYMBOL) {
        Object* ret =  envLookUp(env, obj);
        say("Evaluated " + toString(ret) + " From Symbol " + toString(obj));
        return ret;
    }
    if (obj->type == AS_LIST) {
        say("Evaluated " + toString(obj) + " as List");
        List* list = obj->listVal;
        if (list->empty())
            return obj;
        if (list->first()->info->type == AS_SYMBOL) {
            string sym = *list->first()->info->strVal;
            for (int i = 0; i < num_operators; i++) {
                if (sym == specialForms[i].name) {
                    List* arguments = list->rest();
                    say("Applying Special Form: " + specialForms[i].name);
                    return applySpecial(&specialForms[i], arguments, env);
                }
            }
        }
        List* evaluated_args = new List();
        say("Evaluating Arguments");
        for (ListNode* curr = list->first(); curr != nullptr; curr = curr->next) {
            Object* el = eval(curr->info, env);
            evaluated_args->append(el);
        }
        if (evaluated_args->first()->info->type == AS_FUNCTION)  {
            List* arguments = evaluated_args->rest();
            say("Applying function");
            return apply(evaluated_args->first()->info->procedureVal, arguments, env);
        }
        return makeListObject(evaluated_args);
    }
    return makeErrorObject("Error during eval");
}

/*
 * I met a traveller from an antique land,
 * Who said—“Two vast and trunkless legs of stone
 * Stand in the desert. . . . Near them, on the sand,
 * Half sunk a shattered visage lies, whose frown,
 * And wrinkled lip, and sneer of cold command,
 * Tell that its sculptor well those passions read
 * Which yet survive, stamped on these lifeless things,
 * The hand that mocked them, and the heart that fed;
 * And on the pedestal, these words appear:
 * My name is Ozymandias, King of Kings;
 * Look on my Works, ye Mighty, and despair!
 * Nothing beside remains. Round the decay
 * Of that colossal Wreck, boundless and bare
 * The lone and level sands stretch far away.”
*/

Object* EvalApply::eval(List* expr) {
    return eval(makeListObject(expr), environment);
}

#endif