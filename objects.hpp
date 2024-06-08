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

enum funcType { PRIMITIVE, LAMBDA };
const int EVAL = 0;
const int NO_EVAL = 1;

class EvalApply;
class List;
struct Binding;
struct Procedure;

struct Object {
    objType type;
    union {
        int    intVal;
        double realVal;
        bool boolVal;
        string* strVal;
        List* listVal;
        Binding* bindingVal;
        Procedure* procedureVal;
    };
};

struct Binding {
    Object* symbol;
    Object* value;
    Binding(Object* s = nullptr, Object* v = nullptr) : symbol(s), value(v) { }
};

struct Procedure {
    funcType type;
    List* env;
    List* freeVars;
    Object* (EvalApply::*func)(List*);
    Object* code;
};

struct SpecialForm {
    string name;
    int numArgs;
    char flags[3];
    Object* (EvalApply::*func)(List*, List*);
};

objType getObjectType(Object* obj) {
    return obj->type;
}

Binding* makeBinding(Object* symbol, Object* value) {
    return new Binding(symbol, value);
}

void destroyObject(Object* obj);

void destoryBinding(Binding* binding) {
    if (binding != nullptr) {
        destroyObject(binding->symbol);
        destroyObject(binding->value);
    }
}

Procedure* allocFunction(List* vars, Object* code, List* penv, funcType type) {
    Procedure* p = new Procedure;
    p->code = code;
    p->env = penv;
    p->type = type;
    p->freeVars = vars;
    return p;
}

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

Object* makeBoolObject(bool value) {
    Object* obj = new Object;
    obj->type = AS_BOOL;
    obj->boolVal = value;
    return obj;
}

Object* makeSymbolObject(string value) {
    Object* obj = new Object;
    if (value == "true" || value == "false")
        return makeBoolObject(value == "true");
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

Object* makeFunctionObject(Procedure* proc) {
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

void destroyList(List* list);
int c = 0;
void destroyObject(Object* obj) {
    if (obj == nullptr)
        return;
    switch (getObjectType(obj)) {
        case AS_BINDING:  
            destoryBinding(obj->bindingVal);
            break;
        case AS_FUNCTION: 
            if (obj->procedureVal != nullptr)
                delete obj->procedureVal;
            break;
        case AS_LIST:     
            destroyList(obj->listVal);
            break;
        case AS_ERROR:
        case AS_SYMBOL:   
            if (obj->strVal != nullptr)
                delete obj->strVal;
        case AS_INT: 
        case AS_REAL: 
        case AS_BOOL: 
        default:
            break;
    }
    c++;
    delete obj;
}

#endif