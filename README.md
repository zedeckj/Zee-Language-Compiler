# Zee Language Compiler
Zee is a simple procedural language I designed to learn about compiler implementation. There is one data type, no functions, and no scope. The language is intended as a toy rather than for practical aplications. The compiler itself is written in C and tagets x86-64 Assembly. 

I'll note this project is years old now, and since its creation I've learned a lot about compilers and language design. I'm sure there's plenty of mistakes in the implementation. 

# Syntax
The syntax of Zee is composed of two elements, instructions and expressions. Expressions evaluate to a single integer value, and can either be an integer themselves or be composed of parenthesized pre-fixed operations on other expressions. Currently defined operations are:

Unary Operations:  
++, --: Add/Subtract 1 from argument  
!: Logical not  

Examples:  
```
(++ 1) -> 2
(! 0) -> 1
(! 15) -> 0
```
Binary Operations:  
=, !=, <, >, <=, >=: Comparison operations  

Examples:
```
(= 5 5) -> 1
(< 4 2) -> 0
(>= 100 2) -> 1
```
Ternary Operations:
?: C-style ternary  

Examples:
```
(? 1 10 11) -> 10
(? 0 5 50) -> 50
(? (= 10 1) 10 (< 10 1)) -> 0
```

Variable Argument Operations:
+, -, /, *: Basic arithmetic operations  
%: Modulo   
<<, >>: Arithmetic shift operations  
&, |, ^: Bitwise and, or, xor  
&&, ||, ^^, ->: Logical and, or, xor, material conditional   

Examples:
```
(+ 2 2) -> 4
(- 20 (* 5 3) 4) -> 1
(% 7 3) -> 1
(<< 64 1) -> 128
(>> -1 2) -> -1
(& 127 130) -> 2
(| -1 27) -> 27
(&& 53 2) -> 1
(|| 0 0) -> 0
```

# Instructions
The core composition of Zee is instructions, which can modify state but do not return values. Currently defined instructions are:

Decleration Instructions: Creates a new variable
```I64 <variable_name>; 
I64 <variable_name> = (Expression);
```
Assignment Instructions: Assigns or Modifies the value of an existing variable
```
<variable> = (Expression); 
<variable> += (Expression);
<variable> -= (Expression);
```
Print Insturction: Basic printf to stdout. The formatter string is the only required component. Examples:
```
“Hello World!\n”;
“(+ %d %d) = %d\n”, 2, 2, 4;
“The ASCII code for %c is %d\n”, ‘a’, ‘a’;
```
Putn Instruction: Prints a single value to stdout. A shorthand for “%d\n”,
putn (Expression);

Goto Instructions: Run instructions starting at either a label or relative line position. 
```
goto <label_name>;
goto +<number>;
goto -<number>;
```

Conditional Goto Instructions: Only perform goto if the given Expression does not evaluate to 0.
```
goto <label_name> if (Expression);
goto +<number> if (Expression);
goto -<number> if (Expression);
```
Label Instructions: Marks a line which can be used in a goto instruction.
```
label <name>: 
section <name>:
continue;
```
Sections are labels which are skipped over if they are not reached through a goto. The end of a section or seires of sections is marked with the continue instruction. If the continue is excluded, it is implied to be the last line of the code. The following two code blocks are equivalent

```
I64 x = 2;
goto one if (= x 1);
goto two if (= x 2);
section one:
“x is one\n”;
section two:
“x is two\n”;
continue;
“End Program\n”;
```

```
I64 x = 2;
goto one if (= x 1);
goto two if (= x 2);
goto continue;
label one:
“x is one”;
goto continue;
label two:
“x is two”;
label continue:
“End Program\n”;
```


# Building and Use

Upon downloading, set the working directory to the path of the repository. Run the following to build the compiler and test the example program:  
```make```  
```./zeec fib.zee```  
```./fib```   

The -S flag can be added when running ```./zeec``` to save the generated Assembly files.
