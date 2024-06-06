#ifndef lisp_objects_hpp
#define lisp_objects_hpp
#include <iostream>
#include <vector>
#include <stack>
#include <cmath>
using namespace std;


enum objType {
    AS_INT,
    AS_REAL,
    AS_SYMBOL,
    AS_BOOL,
    AS_BINDING,
    AS_FUNCTION,
    AS_LIST,
    AS_ERROR
};

inline vector<string> typeStr = { "AS_INT", "AS_REAL", "AS_SYMBOL", "AS_BOOL", "AS_BINNDING", "AS_FUNCTION", "AS_LIST", "AS_ERROR"};

class EvalApply;
class List;
struct Binding;
struct Function;

struct Object {
    objType type;
    union {
        int    intVal;
        double realVal;
        bool boolVal;
        string* strVal;
        List* listVal;
        Binding* bindingVal;
        Function* procedureVal;
    };
};

Object* makeIntObject(int value) {
    Object* obj = new Object;
    obj->type = AS_INT;
    obj->intVal = value;
    return obj;
}

Object* makeRealObject(double val) {
    Object* obj = new Object;
    if (fmod(val, 1) == 0) {
        obj->type = AS_INT;
        obj->intVal = val;
    } else {
        obj->type = AS_REAL;
        obj->realVal = val;
    }
    return obj;
}

Object* makeSymbolObject(string value) {
    Object* obj = new Object;
    obj->type = AS_SYMBOL;
    obj->strVal = new string(value);
    return obj;
}

Object* makeListObject(List* value) {
    Object* obj = new Object;
    obj->type = AS_LIST;
    obj->listVal = value;
    return obj;
}

Object* makeFunctionObject(Function* proc) {
    Object* obj = new Object;
    obj->type = AS_FUNCTION;
    obj->procedureVal = proc;
    return obj;
}

Object* makeBindingObject(Binding* value) {
    Object* obj = new Object;
    obj->type = AS_BINDING;
    obj->bindingVal = value;
    return obj;
}

Object* makeErrorObject(string error) {
    Object* obj = new Object;
    obj->type = AS_ERROR;
    obj->strVal = new string(error);
    return obj;
}

Object* makeBoolObject(bool value) {
    Object* obj = new Object;
    obj->type = AS_BOOL;
    obj->boolVal = value;
    return obj;
}

objType getObjectType(Object* obj) {
    return obj->type;
}

struct Binding {
    Object* symbol;
    Object* value;
    Binding(Object* s = nullptr, Object* v = nullptr) : symbol(s), value(v) { }
};

Binding* makeBinding(Object* symbol, Object* value) {
    return new Binding(symbol, value);
}

enum funcType { PRIMITIVE, LAMBDA };

const int EVAL = 0;
const int NO_EVAL = 1;

struct Function {
    funcType type;
    List* env;
    List* free_vars;
    Object* (EvalApply::*func)(List*);
    Object* code;
};

Function* allocFunction(List* vars, Object* code, List* penv, funcType type) {
    Function* p = new Function;
    p->code = code;
    p->env = penv;
    p->type = type;
    p->free_vars = vars;
    return p;
}

struct SpecialForm {
    string name;
    int num_args;
    char flags[3];
    Object* (EvalApply::*func)(List*, List*);
};

#endif