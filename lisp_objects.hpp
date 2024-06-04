#ifndef lisp_objects_hpp
#define lisp_objects_hpp
#include <iostream>
#include <vector>
#include <stack>
#include <cmath>
using namespace std;

class EvalApply;

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

vector<string> typeSty = { "AS_INT", "AS_REAL", "AS_SYMBOL", "AS_BOOL", "AS_BINNDING", "AS_FUNCTION", "AS_LIST", "AS_ERROR"};

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
bool compareObject(Object* lhs, Object* rhs);
string toString(Object*);

Object* makeIntObject(int value) {
    Object* obj = new Object;
    obj->type = AS_INT;
    obj->intVal = value;
    return obj;
}

Object* makeRealObject(double val) {
    Object* obj = new Object;
    if (fmod(val, 1) == 0) {
        cout<<"Saving as Int."<<endl;
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

struct ListNode {
    Object* info;
    ListNode* next;
    ListNode(Object* obj = nullptr, ListNode* n = nullptr) : info(obj), next(n) { }
};

class List {
    private:
        using node = ListNode;
        using link = node*;
        link head;
        link tail;
        int count;
    public:
        List() {
            head = nullptr;
            tail = nullptr;
            count = 0;
        }
        List(const List& list) {
            head = nullptr;
            tail = nullptr;
            count = 0;
            for (link it = list.head; it != nullptr; it = it->next)
                append(it->info);
        }
        ~List() {
            while (head != nullptr) {
                link x = head;
                head = head->next;
                delete x;
            }
        }
        bool empty() {
            return count == 0;
        }
        int size() {
            return count;
        }
        void append(Object* obj) {
            link t = new node(obj);
            if (empty()) {
                head = t;
            } else {
                tail->next = t;
            }
            tail = t;
            count++;
        }
        void push(Object* obj) {
            head = new node(obj, head);
            if (tail == nullptr)
                tail = head;
            count++;
        }
        List& operator=(const List& list) {
            head = nullptr;
            tail = nullptr;
            count = 0;
            for (link it = list.head; it != nullptr; it = it->next)
                append(it->info);
            return *this;
        }
        ListNode* first() {
            return head;   
        }
        List* rest() {
            return copyOmitNth(0);
        }
        int find(Object* obj) {
            int i = 0;
            for (link it = head; it != nullptr; it = it->next, i++)
                if (compareObject(it->info, obj))
                    return i;
            return -1;
        }
        link getNthNode(int N) {
            link it = head;
            for (int k = 0; k < N; k++)
                it = it->next;
            return it;
        }
        void addMissing(List* list) {
            for (link it = list->first(); it != nullptr; it = it->next) {
                if (find(it->info) == -1) {
                    append(it->info);
                }
            }
        }
        void deleteNth(int N) {
            if (N == 0) {
                pop_front();
            } else {
                link it = head;
                link prev;
                int k;
                for (k = 0; k < N; k++) {
                    prev = it;
                    it = it->next;
                }
                prev->next = it->next;
                if (k == count-1)
                    tail = prev;
                delete it;
                count--;
            }
        }
        Object* pop_front() {
            Object* ret = head->info;
            link t = head;
            head = head->next;
            count--;
            delete t;
            return ret;
        }
        void print() {
            cout<<"(";
            for (link it = head; it != nullptr; it = it->next)
                cout<<toString(it->info)<<" ";
            cout<<")"<<endl;
        }
        string asString() {
            string str = "( ";
            for (link it = head; it != nullptr; it = it->next)
                str.append(toString(it->info) + " ");
            str.append(")");
            return str;
        }
        List* copy() {
            List* nl = new List();
            for (link it = head; it != nullptr; it = it->next)
                nl->append(it->info);
            return nl;
        }
        List* copyOmitNth(int N) {
            List* nl = new List();
            int i = 0;
            for (link it = head; it != nullptr; it = it->next, i++)
                if (i != N)
                    nl->append(it->info);
            return nl;
        }
        void clear() {
            while (!empty()) {
                pop_front();
            }
        }
};

struct Binding {
    Object* symbol;
    Object* value;
    Binding(Object* s = nullptr, Object* v = nullptr) : symbol(s), value(v) { }
};

Binding* makeBinding(Object* symbol, Object* value) {
    return new Binding(symbol, value);
}

string toString(Object* obj) {
    switch (obj->type) {
        case AS_INT: return to_string(obj->intVal);
        case AS_REAL: return to_string(obj->realVal);
        case AS_LIST: return obj->listVal->asString();
        case AS_FUNCTION: return "(func)";
        case AS_ERROR:
        case AS_SYMBOL: return *(obj->strVal);
        case AS_BOOL: return obj->boolVal ? "true":"false";
        case AS_BINDING: return toString(obj->bindingVal->symbol);
        default:
            break;
    }
    return "NIL";
}

bool compareObject(Object* lhs, Object* rhs) {
    if (lhs->type != rhs->type)
        return false;
     switch (lhs->type) {
        case AS_INT: return lhs->intVal == rhs->intVal;
        case AS_REAL: return lhs->realVal == rhs->realVal;
        case AS_FUNCTION: return false;
        case AS_SYMBOL: return *lhs->strVal == *rhs->strVal;
        case AS_BOOL: return lhs->boolVal == rhs->boolVal;
        case AS_LIST:
            {
                ListNode* ls = lhs->listVal->first();
                ListNode* rs = rhs->listVal->first();
                while (ls != nullptr && rs != nullptr) {
                    if (!compareObject(ls->info, rs->info))
                        return false;
                    ls = ls->next;
                    rs = rs->next;
                }
                return true;
            }
        case AS_BINDING:
            return compareObject(lhs->bindingVal->symbol, rhs->bindingVal->symbol);
        default:
            break;
     }
     return false;
}


enum funcType { PRIMITIVE, LAMBDA, VARLAMBDA };

const int EVAL = 0;
const int NO_EVAL = 1;


struct Function {
    funcType type;
    List* env;
    List* free_vars;
    Object* (EvalApply::*func)(List*);
    Object* code;
};

Function* makeFunction(Object* (EvalApply::*function)(List*)) {
    Function* p = new Function;
    p->func = function;
    p->free_vars = new List();
    p->env = nullptr;
    p->type = PRIMITIVE;
    return p; 
}

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