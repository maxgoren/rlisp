#include "repl.hpp"

int main() {
    // (define fib (lambda (x) (if (< x 2) 1 (+ (fib (- x 1)) (fib (- x 2))))))
    // (define fact (lambda (x) (if (eq x 0) 1 (* x (fact (- x 1))))))
    REPL repl;
    repl.start();
    return 0;
}