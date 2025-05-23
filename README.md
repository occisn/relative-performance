# relative-performance

Hobby project to compare execution speed between several languages, namely C, Common Lisp, Emacs Lisp and... Excel.

**WORK IN PROGRESS**

One of the objectives is to see to which extent Common Lisp could be as fast as C.

On the two examples (Leibniz formula and butterfly), Common Lisp SBCL is as fast as C, or, at least, in the same order of magnitude.

More precisely, compared languages will be:  
- C: single-threaded, with parallelism  
- Common Lisp: single-threaded, with parallelism, calling C with CFFI  
- Emacs Lisp: interpreted, byte-compiled, native-compiled, calling C with FFI  
- Excel: VBA, recursion, array formulas, calling C  
- (for fun) Gnu Emacs Calc

## Table of contents

**[1. Leibniz formula](#1-leibniz-formula)** :  
- [1.1. Synthesis](#11-synthesis)  
- [1.2. C](#12-c)  
- [1.3. Common Lisp SBCL](#13-common-lisp-sbcl)  
- [1.4. Emacs Lisp](#14-emacs-lisp)  
- [1.5. Excel](#15-excel): [VBA](#151-excel-by-vba), [spreadsheet](#152-excel-by-spreadsheet), [recursion](#153-excel-by-recursion), [array formulas](#154-excel-by-array-formulas)  
- [1.6. GNU Emacs Calc](#16-gnu-emacs-calc)

**[2. Butterfly](#2-butterfly)** 


## 1. Leibniz formula

As Niklas Heer dit it for its [speed comparison](https://github.com/niklas-heer/speed-comparison) of programming language, we will use [Leibniz formula](https://en.wikipedia.org/wiki/Leibniz_formula_for_%CF%80) n times to calculate an approximate value of pi.

### 1.1. Synthesis

Actual digits of pi are: 3.14159265358979323846264338327950288419...

For n = 10,000,000,000 (10 zeros) with no SIMD or parallelization at this stage, **Common Lisp SBCL is as quick as C**:

| Language                                    | Results                        | Execution duration | Function name |
|---------------------------------------------|--------------------------------|--------------------|---------------|
| **C**, -O3, basic                           | **3.141592653**48834582099     | **10.1 s**         | leibniz 3b    |
| **C** with pragma parallelism               | **3.141592653**48820548880     | **2.8 s**          | leibniz 5     |
| **C** with pragma parallelism + SIMD        | **3.141592653**49174532389     | **1.5 s**          | leibniz 9     |
| **SBCL**, basic                             | **3.14159265**258805040000 [4] | 177 s [4]          | leibniz 2     |
| **SBCL**, typed and (speed 3)               | **3.141592653**48834600000     | **10.1 s**         | leibniz 5ter  |
| **SBCL** with parallelism (lparallel)       | **3.141592653**48898300000     | **3.5 s**          | leibniz 9     |
| **SBCL** with parallelism (sb-threads)      | **3.141592653**48898300000     | **3.6 s**          | leibniz 10     |
| **SBCL** calling C                          | **3.141592653**48898300000                            | ???                |               |
| **Emacs Lisp**, interpreted                 | **3.141592**55358979150330 [2] | 4300 s [2]         | leibniz A 3   |
| **Emacs Lisp**, byte-compiled               | **3.141592**55358979150330 [2] | 2920 s [2]         | leibniz B 3   |
| **Emacs Lisp**, native-compiled             | **3.141592**55358979150330 [2] | 2870 s [2]         | leibniz C 3   |
| **Emacs Lisp** calling C                    | ???                            | ???                |               |
| **Excel**, VBA                              | **3,1415926**4457277000000 [4] | 125 s [4]          | VBA 3         |
| **Excel**, recursion (all cores)            | **3,1415926**4053770000000 [3] | 1100 s [3]         | recursion 2   |
| **Excel**, arrays formulas (all cores)      | **3.141592**55822236000000 [3] | 2200 s [3]         | version 3     |
| **Excel** calling C                         | ???                            | ???                |               |
| **GNU Emacs Calc** on stack                 | **3.141**49267357 [0]          | 10,000,000 s [0]   |               |
| **GNU Emacs Calc** with algbraic expression | **3.14159**165356 [1]          | 210,000 s [1]      |               |


[0] extrapolated from n = 10,000 (4 zeros)  
[1] extrapolated from n = 1,000,000 (6 zeros)  
[2] extrapolated from n = 10,000,000 (7 zeros)  
[3] extrapolated from n = 100,000,000 (8 zeros)  
[4] extrapolated from n = 1,000,000,000 (9 zeros)  

### 1.2. C

gcc version: 13.2.0

Basic function is:
``` C
int leibniz_3() {
  uint64_t n9 = 1000000000; // 9 zeros
  double tmp = 0.0;
  double sign = 1.0;
  for (uint64_t i = 0; i <= n9; i++) {
    tmp = tmp + sign / (2.0 * i + 1.0);
    sign = -sign;
  }
  tmp = 4 * tmp;
  printf("Result: %.20f\n", tmp);
  return EXIT_SUCCESS;
}
```

Execution time: 10.2 s

2-loop unrolling to avoid explicit change of sign = leibiniz_3b : same duration, 10.1 s

4-loop unrolling = leibiniz_4 : same duration, 10.5 s

basic pragma parallelization = leibniz_5 : 2.87 s

basic pragma parallelization with 16-loop unrolling = leibniz_6 : same duration, 2.83 s

pragma parallelization with chunks = leibniz_6b : same duration, 2.8-2,9 s

SIMD vectorization with 8-array for float precision (but no parallelization) = leibniz_7 : 7.3 s but bad precision

SIMD vectorization with 4-array for double precision (but no parallelization) = leibniz_8 : 5.1 s
 
SIMD vectorization with 4-array for double precision, and basic pragma parallelization = leibniz_9 : 1.5 s

SIMD vectorization with 4-array for double precision, and pragma parallelization with chunks = leibniz_9b : same duration, 1.5 s


### 1.3. Common Lisp SBCL

SBCL version: 2.4.10

Basic function is:

``` lisp
(defun leibniz ()
  "Calculate an approximation of pi using Leibniz formula."
  (let ((tmp 0.0d0)
        (sign 1.0d0))
    (dotimes (i *n8*)
      (setq tmp (+ tmp (/ sign (+ (* 2 i) 1))))
      (setq sign (- sign)))
    (* 4 tmp)))
```

It is accelerated within leibniz-5 function by type declarations, (speed 3) and the use of `evenp`.

leibniz-5-bis: play on integer calculation, but not quicker

leibniz-6: introduction of 4-loop unrolling

8- and 16-loop unrolling do not yield quicker results

### 1.4. Emacs Lisp

Basic function is:

``` elisp
(defun leibniz (n)
  "Calculate an approximation of pi using Leibniz formula with N terms."
  (let ((tmp 0.0)
        (sign 1.0))
    (dotimes (i n)
      (setq tmp (+ tmp (/ sign (float (+ (* 2 i) 1)))))
      (setq sign (- sign)))
    (setq tmp (* 4 tmp))
    (message "Result: %.20f" tmp))
```

It is slightly accelerated by the use of `cl-evenp` function:
```elisp
(setq tmp (+ tmp (/ (if (cl-evenp i) 1.0 -1.0) (float (+ (* 2 i) 1)))))
```

And even more by grouping terms by 2:
``` elisp
(defun leibniz (n)
  "Calculate an approximation of pi using Leibniz formula with N terms."
  (let ((tmp 0.0)
        (i 0))
    (while (<= i (- n 1))
      (setq tmp (+ tmp (/ 1.0 (float (+ (* 2 i) 1))))
            tmp (- tmp (/ 1.0 (float (+ (* 2 i) 3))))
            i (+ i 2)))
    (setq tmp (* 4 tmp))
    (message "Result: %.20f" tmp)))
```

Then by byte-compilation and native-compilation.

To be noted: native compilation does not bring a significant speed increase.

Native compilation: libccjit.dll provided by msys2 version 3.5.7-4, containing gcc 13.20 (within msys64)

### 1.5. Excel

#### 1.5.1. Excel by VBA

Basic function is in VBA 1 file:
``` VBA
Function Leibniz(n As Long) As Double
    Dim tmp As Double
    Dim sign As Double
    Dim i As Long
    tmp = 0
    sign = 1
    For i = 0 To n
        tmp = tmp + sign / (2 * i + 1)
        sign = -sign
    Next i
    Leibniz = 4 * tmp
End Function
```

It can be accelerated by almost a factor 2 by grouping terms (see VBA 2):
``` VBA
Function Leibniz(n As Long) As Double
    Dim tmp As Double
    Dim i As Long
    tmp = 0
    For i = 0 To (n - 1) Step 2
        tmp = tmp + 1 / (2 * i + 1) - 1 / (2 * i + 3)
    Next i
    Leibniz = 4 * tmp
End Function
```

Let's use # to indicate Double literals, avoiding implicit type conversions. See VBA 3:
``` VBA
Function Leibniz(n As Long) As Double
    Dim tmp As Double
    Dim i As Long
    tmp = 0#
    For i = 0 To (n - 1) Step 2
        tmp = tmp + 1# / (2# * i + 1#) - 1 / (2# * i + 3#)
    Next i
    Leibniz = 4# * tmp
End Function
```

In VBA 4, we try to divide 1-1000000000 (9 zeros) range into chunks, but this does not increase speed (no parallelism in such situation).

#### 1.5.2. Excel by spreadsheet

File B proposes a 100 x 100 = 10,000 (4 zeros) table in a spreadsheet.

The same file with a 1,000 x 1,000 = 1,000,000 (6 zeros) table would weight around 16 Mo.

#### 1.5.3. Excel by recursion

File C, version 1, divises 0 - 10,000,000 (7 zeros) in 2443 chunks of 4094 range (Excel recursion maximum depth with 1 parameter), then uses recursion on each chunk.

Typical formula is:
``` Excel
=LET(NMIN; B10;
     NMAX; NMIN+4094-1;
     SUB;LAMBDA(ME;N;IF(N<NMIN;0;(-1)^N/(2*N+1)+ME(ME;N-1)));
     SUB(SUB;NMAX))
```

Version 2 performs the same, but chunks are 32 times larger, since each formula computes 32 terms of the sum:
``` Excel
=LET(NMIN; B10;
     NMAX; NMIN+4094*32;
     SUB; LAMBDA(ME;N;IF(N+31>NMAX;0;1/(2*N+1)-1/(2*N+3)+1/(2*N+5)-1/(2*N+7)+1/(2*N+9)-1/(2*N+11)+1/(2*N+13)-1/(2*N+15)+1/(2*N+17)-1/(2*N+19)+1/(2*N+21)-1/(2*N+23)+1/(2*N+25)-1/(2*N+27)+1/(2*N+29)-1/(2*N+31)+1/(2*N+33)-1/(2*N+35)+1/(2*N+37)-1/(2*N+39)+1/(2*N+41)-1/(2*N+43)+1/(2*N+45)-1/(2*N+47)+1/(2*N+49)-1/(2*N+51)+1/(2*N+53)-1/(2*N+55)+1/(2*N+57)-1/(2*N+59)+1/(2*N+61)-1/(2*N+63)+ME(ME;N+32)));
     SUB(SUB;NMIN))
```
So, only ~ 76 chunks are needed.  
Actually ~ 764, since the increase of speed allows calculating for n = 100,000,000 (8 zeros).

If we hard-code the chunk limits, there is apparently a small increase of speed, but nothing significant.

In version 3, we try an one-liner:
```
=4*SOMME(MAP(
SEQUENCE(764;1;0;32*4094);
LAMBDA(NMIN;
LET(NMAX;NMIN+(4094-1)*32;
SUB ; LAMBDA(ME;N;IF(N+31>NMAX;0;1/(2*N+1)-1/(2*N+3)+1/(2*N+5)-1/(2*N+7)+1/(2*N+9)-1/(2*N+11)+1/(2*N+13)-1/(2*N+15)+1/(2*N+17)-1/(2*N+19)+1/(2*N+21)-1/(2*N+23)+1/(2*N+25)-1/(2*N+27)+1/(2*N+29)-1/(2*N+31)+1/(2*N+33)-1/(2*N+35)+1/(2*N+37)-1/(2*N+39)+1/(2*N+41)-1/(2*N+43)+1/(2*N+45)-1/(2*N+47)+1/(2*N+49)-1/(2*N+51)+1/(2*N+53)-1/(2*N+55)+1/(2*N+57)-1/(2*N+59)+1/(2*N+61)-1/(2*N+63)+ME(ME;N+32)));
SUB(SUB;NMIN)))))
```

#### 1.5.4. Excel by array formulas

File D, version 1, divises 0 - 100,000,000 (8 zeros) range in 96 chunks of 1,048,576 range (Excel sequence max size), then uses the following array formulas on each chunk:
```
=SUM(MAP(SEQUENCE(C11-B11+1;1;B11); LAMBDA(M;(-1)^M/(2*M+1))))
```

Version 2 replaces `(-1)^M` by a conditional on `M` evenness:
```
=SUM(MAP(SEQUENCE(C11-B11+1;1;B11); LAMBDA(M;IF(MOD(M;2)=0;1/(2*M+1);-1/(2*M+1)))))
```

It does not really increase speed.

Version 3 takes advantage of Excel built-in functions working on sequences:
```
= LET(
  M; SEQUENCE(C11-B11+1;1;B11);
  SUM( (-1)^M / (2*M + 1)))
```

It slightly increases speed.

Note: hard-coding chunks limits does *not* improve speed.


Version 4 proposes an one-liner:
```
= 4 * SOMME( MAP( SEQUENCE(96;1;0;1048576);
          LAMBDA(NMIN; LET(NMAX;NMIN+1048576-1;
                 M; SEQUENCE(NMAX-NMIN+1;1;NMIN);
                    SOMME( (-1)^M / (2*M + 1))))))
```

The execution is a bit longer.

### 1.6. GNU Emacs Calc

In each version, terms are grouped by two.

Stack version:
```
0 SPC 0 SPC 10000 Z( RET 2 * 1 + & TAB 2 * 3 + & - + 2 Z) 4 *
```

Algebraic versions:

```
'(1/((2*(2*k))+1)-1/((2*(2*k+1))+1)) RET 'k RET 0 SPC 1000000 SPC 2 / a+ RET 4 *
```

```
'4*sum(1/((2*(2*k))+1)-1/((2*(2*k+1))+1), k, 0, 1000000/2) RET
```

GNU Emacs version: 29.4

## 2. Butterfly

| Language                                              | Execution duration | function name |
|-------------------------------------------------------|--------------------|---------------|
| **C**, -O3, basic                                     | **172 s**          | populate 1    |
| **C**, -O3, pragma parallelization with chunks        | **42 s**           | populate 2    |
| **Common Lisp SBCL**, typed and (speed 3)             | **138 s**          | butterfly 1   |
| **Common Lisp SBCL**, typed, (speed 3) and lparallel  | **35 s**           | butterfly 2   |
| **Common Lisp SBCL**, typed, (speed 3) and sb-threads | **34 s**           | butterfly 3   |

(end of README)
