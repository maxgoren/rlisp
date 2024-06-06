#ifndef list_hpp
#define list_hpp
#include "objects.hpp"

bool compareObject(Object* lhs, Object* rhs);

string toString(Object*);

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
        List();
        List(const List& list);
        ~List();
        bool empty();
        int size();
        void append(Object* obj);
        void push(Object* obj);
        ListNode* first();
        List* rest();
        int find(Object* obj);
        link getNthNode(int N);
        void addMissing(List* list);
        void deleteNth(int N);
        Object* pop_front();
        void print();
        string asString();
        List* copy();
        List* copyOmitNth(int N);
        void clear();
        List& operator=(const List& list);
};

List::List() {
    head = nullptr;
    tail = nullptr;
    count = 0;
}

List::List(const List& list) {
    head = nullptr;
    tail = nullptr;
    count = 0;
    for (link it = list.head; it != nullptr; it = it->next)
        append(it->info);
}

List::~List() {
    while (head != nullptr) {
        link x = head;
        head = head->next;
        delete x;
    }
}

bool List::empty() {
    return count == 0;
}

int List::size() {
    return count;
}

void List::append(Object* obj) {
    link t = new node(obj);
    if (empty()) {
        head = t;
    } else {
        tail->next = t;
    }
    tail = t;
    count++;
}

void List::push(Object* obj) {
    head = new node(obj, head);
    if (tail == nullptr)
        tail = head;
    count++;
}

ListNode* List::first() {
    return head;   
}

List* List::rest() {
    return copyOmitNth(0);
}

int List::find(Object* obj) {
    int i = 0;
    for (link it = head; it != nullptr; it = it->next, i++)
        if (compareObject(it->info, obj))
            return i;
    return -1;
}

ListNode* List::getNthNode(int N) {
    link it = head;
    for (int k = 0; k < N; k++)
        it = it->next;
    return it;
}

void List::addMissing(List* list) {
    for (link it = list->first(); it != nullptr; it = it->next) {
        if (find(it->info) == -1) {
            append(it->info);
        }
    }
}

void List::deleteNth(int N) {
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

Object* List::pop_front() {
    Object* ret = head->info;
    link t = head;
    head = head->next;
    count--;
    delete t;
    return ret;
}

void List::print() {
    cout<<"(";
    for (link it = head; it != nullptr; it = it->next)
        cout<<toString(it->info)<<" ";
    cout<<")"<<endl;
}

string List::asString() {
    string str = "( ";
    for (link it = head; it != nullptr; it = it->next)
        str.append(toString(it->info) + " ");
    str.append(")");
    return str;
}

List* List::copy() {
    List* nl = new List();
    for (link it = head; it != nullptr; it = it->next)
        nl->append(it->info);
    return nl;
}

List* List::copyOmitNth(int N) {
    List* nl = new List();
    int i = 0;
    for (link it = head; it != nullptr; it = it->next, i++)
        if (i != N)
            nl->append(it->info);
    return nl;
}

void List::clear() {
    while (!empty()) {
        pop_front();
    }
}

List& List::operator=(const List& list) {
    head = nullptr;
    tail = nullptr;
    count = 0;
    for (link it = list.head; it != nullptr; it = it->next)
        append(it->info);
    return *this;
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
                if (lhs->listVal->size() == 0 && rhs->listVal->size() == 0)
                    return true;
                else if (lhs->listVal->size() == 0 || rhs->listVal->size() == 0)
                    return false;
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

Function* makeFunction(Object* (EvalApply::*function)(List*)) {
    Function* p = new Function;
    p->func = function;
    p->free_vars = new List();
    p->env = nullptr;
    p->type = PRIMITIVE;
    return p; 
}

#endif