# mgclisp

a simple scheme interpreter in C++.

Based on https://github.com/Jaffe-/lispc/


     repl> (define fib (lambda (x) (if (< x 2) 1 (+ (fib (- x 1)) (fib (- x 2))))))
         -> fib
     repl> (fib 6)
       13
