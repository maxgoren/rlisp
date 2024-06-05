#include "repl.hpp"


int main() {
    // (define fib (lambda (x) (if (< x 2) 1 (+ (fib (- x 1)) (fib (- x 2))))))
    REPL repl;
    repl.start();
    return 0;
}