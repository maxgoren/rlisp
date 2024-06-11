# mgclisp

a simple scheme interpreter in C++.


     mgclisp(1)> (define fib (lambda (x) (if (< x 2) 1 (+ (fib (- x 1)) (fib (- x 2))))))
      fib
     mgclisp(2)> (fib 6)
      13
     mgclisp(3)> (define fact (lambda (x) (if (eq x 0) 1 (* x (fact (- x 1))))))
      fact
     mgclisp(4)> (fact 5)
      120

Lexical Scoping with 'let'

     mgclisp(4)> (define x 7)
      x
     mgclisp(5)> (define y 9)
      y
     mgclisp(6)> (let ((x 2) (y 3)) (+ x y))
      5
     mgclisp(7)> x
      ( 7 )
     mgclisp(8)> y
      ( 9 )
    mgclisp(9)>


Inspired by https://github.com/Jaffe-/lispc
