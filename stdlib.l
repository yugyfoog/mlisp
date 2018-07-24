(set 't 't)
(set 'null (lambda (x) (eq x ())))
(set 'caar (lambda (x) (car (car (x)))))
(set 'cadr (lambda (x) (car (cdr (x)))))
