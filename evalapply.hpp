#ifndef evalapply_hpp
#define evalapply_hpp
#include <iostream>
#include <vector>
#include <stack>
#include <unordered_map>
#include "objects.hpp"
#include "lex.hpp"
#include "list.hpp"
using namespace std;

class EvalApply {
    private:
        bool loud;
        int d;
        void enter();
        void enter(string s);
        void leave();
        void leave(string s);
        void say(string s);
        unordered_map<string, SpecialForm> specialForms;
        Object* specialDefine(List* args, List* env);
        Object* specialIf(List* args, List* env);
        Object* specialLambda(List* args, List* env);
        Object* specialQuote(List* args, List* env);
        Object* specialSet(List* args, List* env);
        Object* specialDo(List* args, List* env);
        Object* specialCond(List* args, List* env);
        Object* specialLet(List* args, List* env);
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
        Object* apply(Procedure* proc, List* args, List* env);
        Object* evalList(List* list, List* env);
        Object* eval(Object* obj, List* env);

        List* makeNewEnvironment(List* vars, List* vals);
        void addBinding(Binding* binding);
        void addPrimitive(string symbol, Object* (EvalApply::*func)(List*));
        Object* envLookUp(List* env, Object* obj);
        List* environment;
    public:
        EvalApply(bool noisey = false);
        ~EvalApply();
        Object* eval(List* expression);
        void setTrace(bool trace);
};

void EvalApply::setTrace(bool trace) {
    loud = trace;
}

void EvalApply::say(string s) {
    if (loud) {
        for (int i = 0; i < d; i++) cout<<" ";
        cout<<s<<endl;
    }
}

void EvalApply::enter() {
    d++;
}
void EvalApply::leave() {
    d--;
}
void EvalApply::enter(string s) {
    d++;
    say(s);
}
void EvalApply::leave(string s) {
    say(s);
    d--;
}

void EvalApply::addBinding(Binding* binding) {
    environment->append(makeBindingObject(binding));
}
void EvalApply::addPrimitive(string symbol, Object* (EvalApply::*func)(List*)) {
    addBinding(makeBinding(makeSymbolObject(symbol), makeFunctionObject(makeFunction(func))));
}

EvalApply::EvalApply(bool noisey) {
    loud = noisey;
    d = 0;
    environment = new List();
    specialForms["define"] = {"define", 2, {NO_EVAL, EVAL}, &EvalApply::specialDefine};
    specialForms["if"] = {"if", 3, {EVAL, NO_EVAL, NO_EVAL}, &EvalApply::specialIf};
    specialForms["lambda"] = {"lambda", 2, {NO_EVAL, NO_EVAL}, &EvalApply::specialLambda};
    specialForms["\\"] = {"\\", 2, {NO_EVAL, NO_EVAL}, &EvalApply::specialLambda};
    specialForms["'"] = {"'", 2, {NO_EVAL, NO_EVAL}, &EvalApply::specialQuote};
    specialForms["set"] = {"set", 2, {NO_EVAL, EVAL}, &EvalApply::specialSet};
    specialForms["do"] = {"do", 0, {}, &EvalApply::specialDo};
    specialForms["cond"] = {"cond", 0, {}, &EvalApply::specialCond};
    specialForms["let"] = {"let",2, {NO_EVAL, NO_EVAL}, &EvalApply::specialLet};
    
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

EvalApply::~EvalApply() {
    destroyList(environment);
}

Object* EvalApply::envLookUp(List* env, Object* obj) {
    for (Object* it : *env) {
        if (compareObject(obj, it->bindingVal->symbol))
            return it->bindingVal->value;
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
    for (Object* curr : *env) {
        if (curr->type == AS_BINDING) {
            if (compareObject(curr->bindingVal->symbol, symbol)) {
                curr->bindingVal->value = replacement;
                return replacement;
            }
        }
    }
    env->append(makeBindingObject(makeBinding(symbol, replacement)));
    return replacement;
}

Object* EvalApply::specialDo(List* args, List* env) {
    Object* result;
    for (Object* info : *args) {
        result = eval(info, env);
        if (result->type == AS_ERROR) {
            return result;
        }
    }
    return result;
}

Object* EvalApply::specialCond(List* args, List* env) {
    Object* result = makeIntObject(0);
    for (Object* info : *args) {
        if (getObjectType(info) != AS_LIST) {
            return makeErrorObject("Error: cond operates on lists only.");
        }
        List* toEval = info->listVal;
        result = eval(info, env);
        if (result->type == AS_ERROR) {
            return result;
        }
    }
    return result;
}

Object* EvalApply::specialLet(List* args, List* env) {
    List* vars = args->first()->info->listVal;
    List* body = args->first()->next->info->listVal;
    List* var_names = new List();
    List* var_vals = new List();
    for (Object* info : *vars) {
        if (info->type == AS_LIST) {
            var_names->append(makeSymbolObject(*info->listVal->first()->info->strVal));
            var_vals->append(info->listVal->first()->next->info);
        } else {
            return makeErrorObject("Let requires its own association list");
        }
    }
    Procedure* tempfunc = allocFunction(var_names, makeListObject(body), env, LAMBDA);
    List* asList = new List();
    asList->append(makeFunctionObject(tempfunc));
    asList->addMissing(var_vals);
    return eval(makeListObject(asList), env);
}

Object* EvalApply::primitivePlus(List* args) {
    say("primitive plus " + args->asString());
    return applyMathPrimitive(args, "+");
}
Object* EvalApply::primitiveMinus(List* args) {
    say("primitive minus " + args->asString());
    return applyMathPrimitive(args, "-");
}
Object* EvalApply::primitiveMultiply(List* args) {
    say("primitive multiply " + args->asString());
    return applyMathPrimitive(args, "*");
}
Object* EvalApply::primitiveDivide(List* args) {
    say("primitive divide" + args->asString());
    return applyMathPrimitive(args, "/");
}
Object* EvalApply::primitiveLess(List* args) {
    say("primitive less " + args->asString());
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
    say("primitive greager " + args->asString());
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
    say("primitive equals" + args->asString());
    Object* first = args->first()->info;
    Object* second = args->first()->next->info;
    return makeBoolObject(compareObject(first, second));
}
Object* EvalApply::primitivePrint(List* args) {
    List* evaldArgs = new List();
    for (Object* it : *args) {
        Object* ce = eval(it, environment);
        evaldArgs->append(ce);
    }
    if (evaldArgs->size() == 1 && evaldArgs->first()->info->type == AS_LIST) {
        cout<<evaldArgs->first()->info->listVal->asString()<<endl;
    } else {
        cout<<evaldArgs->asString()<<endl;
    }
    return new Object();
}
Object* EvalApply::primitiveCar(List* args) {
    say("primitive car " + toString(args->first()->info));
    if (getObjectType(args->first()->info) != AS_LIST)
        return makeErrorObject("Error: car must be supplied a list");
    return args->first()->info->listVal->first()->info;
}

Object* EvalApply::primitiveCdr(List* args) {
    say("primitive cdr " + toString(args->first()->info));
    if (getObjectType(args->first()->info) != AS_LIST)
        return makeErrorObject("Error: cdr must be supplied a list");
    return makeListObject(args->first()->info->listVal->rest());
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

Object* EvalApply::applySpecial(SpecialForm* special, List* args, List* env) {
    enter();
    ListNode* currArg = args->first();
    List* evaluated_args = new List();
    for (int i = 0; i < args->size(); i++) {
        Object* result = currArg->info;
        if (special->numArgs != 0 && special->flags[i] == EVAL) {
            result = eval(currArg->info, env);
        }
        evaluated_args->append(result);
        currArg = currArg->next;
    }
    auto m = special->func;
    leave();
    return (this->*m)(evaluated_args, env);
}

Object* EvalApply::applyMathPrimitive(List* args, string op) {
    Object* first = args->first()->info;
    double result = first->type == AS_INT ? first->intVal:first->realVal;
    for (Object* curr : *args->rest()) {
        if (curr->type == AS_INT || curr->type == AS_REAL) {
            double t = curr->type == AS_INT ? curr->intVal:curr->realVal;
            if (op == "+") result += t;
            if (op == "-") result -= t;
            if (op == "*") result *= t;
            if (op == "/") result /= t;
        }
    }
    return makeRealObject(result);
}

Object* EvalApply::evalList(List* list, List* env) {
    if (getObjectType(list->first()->info) == AS_SYMBOL) {
        string symbol = *list->first()->info->strVal;
        if (specialForms.find(symbol) != specialForms.end()) {
            List* arguments = list->rest();
            leave("Evaluated as Special Form: " + symbol);
            return applySpecial(&specialForms[symbol], arguments, env);
        }
    }
    List* evaluatedArguments = new List();
    say("Evaluating Arguments");
    for (Object* curr : *list) {
        evaluatedArguments->append(eval(curr, env));
    }
    if (getObjectType(evaluatedArguments->first()->info) == AS_FUNCTION)  {
        Procedure* procedure = evaluatedArguments->first()->info->procedureVal;
        List* arguments = evaluatedArguments->rest();
        leave("Evaluated as function expression");
        return apply(procedure, arguments, env);
    }
    leave("Evaluated As List");
    return makeListObject(evaluatedArguments);
}

Object* EvalApply::apply(Procedure* proc, List* args, List* env) {
    enter("apply");
    if (proc->type == PRIMITIVE) {
        auto m = proc->func;
        leave("Applying primitive.");
        return (this->*m)(args);
    }
    if (proc->type == LAMBDA) {
        List* nenv = makeNewEnvironment(proc->freeVars, args);
        nenv->addMissing(proc->env);
        nenv->addMissing(env);
        Object* result = eval(proc->code, nenv);
        leave();
        return result;
    }
    leave();
    return makeErrorObject("An error in apply occured");
}

Object* EvalApply::eval(Object* obj, List* env) {
    enter("eval");
    switch (getObjectType(obj)) {
        case AS_INT:
            leave("Evaluated " + toString(obj) + " as int");
            return obj;
        case AS_REAL:
            leave("Evaluated " + toString(obj) + " as real");
            return obj;
        case AS_BOOL:
            leave("Evaluated " + toString(obj) + " as Bool");
            return obj;
        case AS_FUNCTION:
            leave("Evaluated " + toString(obj) + " as function");
            return obj;
        case AS_ERROR:
            leave("Evaluated " + toString(obj) + " as Error");
            return obj;
        case AS_SYMBOL: {
            Object* ret =  envLookUp(env, obj);
            leave("Evaluated " + toString(ret) + " From Symbol " + toString(obj));
            return ret;
        }
        case AS_LIST: 
            if (obj->listVal->empty()) {
                leave("evaluated as ()");
                return obj;
            }
            return evalList(obj->listVal, env);
        default:
            break;
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
    Object* exprObj = makeListObject(expr);
    Object* result = eval(exprObj, environment);
    return result;
}

#endif