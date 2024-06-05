# mgclisp

a simple scheme interpreter in C++.


     repl> (define fib (lambda (x) (if (< x 2) 1 (+ (fib (- x 1)) (fib (- x 2))))))
         -> fib
     repl> (fib 6)
       13
     repl> (define fact (lambda (x) (if (eq x 0) 1 (* x (fact (- x 1))))))
        -> fact
     repl> (fact 5)
       120
