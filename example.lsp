
(defun one () "Just return 1." 1)
(defun two () "Just return 2." 2)

(prin1					; print the result
 (* (+ (one) (two) 3) 1.5))

(defmacro unless (cond &rest body)
  "Defining unless macro the opposite of `if`."
  (list (quote if)
	(list (quote not) cond)
	(cons (quote progn) body)))

(unless (equal 5 6)
  (prin1 "5 is not equal to 6")
  (prin1 "5 is equal to 6!"))

(defun print-list (l)
  "Print all items of the list L recursively.."
  (prin1 (car l))			; print head
  (unless (equal (length l) 1)
      (print-list (cdr l))))		; keep going..
(print-list (list 1 2 3))

;;(defun sum (l)
;;  ""
;;  (if l
;;      (+ (car l) (sum (cdr l)))
;;      0))
;;(sum (list 1 2 3))

(defun count-to (i n)
  "Count from I to N recursively."
  (unless (equal i n)
    (progn
      (prin1 i)
      (count-to (+ i 1) n))))
(count-to 0 10)

(defun fibonacci (x)
  "Compute the fibonacci number for X."
  (if (equal x 0) 1
      (if (equal x 1) 1
	  (+ (fibonacci (- x 1))
	     (fibonacci (- x 2))))))

(prin1 (fibonacci 2))
(prin1 (fibonacci 3))
(prin1 (fibonacci 4))
(prin1 (fibonacci 10))
