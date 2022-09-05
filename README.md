<div id="top"></div>

# A C Compiler

*****
*The credit for this readme goes mostly to [Fardin Anam Aungon](https://github.com/fardinanam). Thanks to him for his amazing writeup.*
*****

## Introduction

This is a C compiler that performs some lexical, syntax and semantic error checking with the help of Flex (lexer) and Bison (YACC parser) and then converts the C code to 8086 assembly language code. It is not an advance C compiler but covers most of the basic features of the language. For more details, see <a href="#syntax-analyser">here</a>.

## Techs

<img src="https://img.shields.io/badge/Languages-151515?style=for-the-badge&logo=plex&logoColor=FFFFFF">![badge-yacc](https://img.shields.io/badge/c%2B%2B-151515?style=for-the-badge&logo=c%2B%2B&logoColor=00599C&labelColor=151515)![badge-python](https://img.shields.io/badge/LEX-151515?style=for-the-badge&logo=&logoColor=f4ff19&labelColor=151515)![badge-java](https://img.shields.io/badge/BISON-151515?style=for-the-badge&logo=&logoColor=c93618&labelColor=151515) <br/>

## Table of Contents

<div>
<ul>
        <li><a href="#lexical-analyser">Lexical Analyser</a></li>
        <li><a href="#syntax-analyser">Syntax Analyser</a></li>
        <li><a href="#semantic-analyser">Semantic Analyser</a></li>
        <li><a href="#intermediate-code-generation">Intermediate Code Generation</a>
        <ul>
        <li><a href="#the-algorithm">The Algorithm</a></li>
        <li><a href="#evaluating-for-loop-is-somewhat-tricky">Evaluating for loop is somewhat tricky</a></li>
        <li><a href="#evaluating-functions">Evaluating functions</a></li>
        <li><a href="#declaring-variables">Declaring variables</a></li>
        <li><a href="#accessing-variables">Accessing variables</a></li>
        <li><a href="#conclusion">Conclusion</a></li>
        <li><a href="#optimizing-assembly-code">Optimizing assembly code</a></li>
        </ul>
        </li>
</ul>
</div>

## Lexical Analyser

For lexical analyzer we will use Flex (Fast Lexical Analyzer Generator), which is basicallly a computer program that generates lexical analyzers(scanners or lexers).

`Note:` For more details about Flex, see [here](/Lexical%20Analyzer/Lexical%20Analysis%20Preview.pdf).

### Installation

***Installing Flex on linux:*** 

    sudo apt-get update
    sudo apt-get install flex

### Lexer returns the following tokens to the parser:

- Keywords

        Matched lexeme  :       Returned Token

        if              :       IF
        else            :       ELSE
        for             :       FOR
        while           :       WHILE
        do              :       DO
        break           :       BREAK
        int             :       INT
        char            :       CHAR
        float           :       FLOAT
        double          :       DOUBLE
        void            :       VOID
        return          :       RETURN
        switch          :       SWITCH
        case            :       CASE
        default         :       DEFAULT
        continue        :       CONTINUE

- Constants

        Returned Tokens

        CONST_INT
        CONST_FLOAT
        CONST_CHAR

- Operators and punctuators

        Matched Lexeme          :       Returned Token

        +, -                    :       ADDOP           *
        *, /, %                 :       MULOP           *
        ++, --                  :       INCOP           *
        <, <=, >, >=, ==, !=    :       RELOP           *
        =                       :       ASSIGNOP
        &&, ||                  :       LOGICOP         *
        !                       :       NOT
        (                       :       LPAREN
        )                       :       RPAREN
        {                       :       LCURL
        }                       :       RCURL
        [                       :       LTHIRD
        ]                       :       RTHIRD
        ,                       :       COMMA
        ;                       :       SEMICOLON

- Identifiers

        ID                                              *

- Strings

        STRING

- **Whitespaces and comments are identified by the lexer but these are not passed to the parser.**
- **Lexer also counts the line numbers when it finds a newline.**

**Tokens with \* are passed as SymbolInfo objects to the parser. The SymbolInfo contains <matched lexeme, returned token> of the lexeme**

### Lexical Errors:

Detect lexical errors in the source program and reports it along with corresponding line no. Detects the following type of errors:

- Too many decimal point error for character sequence like `1.2.345`
- Ill formed number such as `1E10.7`
- Invalid Suffix on numeric constant or invalid prefix on identifier for character sequence like `12abcd`
- Multi character constant error for character sequence like `‘ab’`
- Unfinished character such as `‘a` , `‘\n` or `‘\’`
- Empty character constant error `''`
- Unfinished string like `"this is an unfinished string `
- Unfinished comment like `/* This is an unfinished comment `
- Unrecognized character (Any character that does not match any defined regular expressions)
- **Also counts the total number of errors.**

<p align="right"><a href="#top">back to top</a></p>

## Syntax Analyser

For syntax & symentic analyzer we will use YACC (Yet Another Compiler Compiler), which is basicallly a program that takes as input a specification of a syntax and produces as output a procedure for recognizing that language. An updated version of YACC is Bison. Bison is basically a general-purpose parser generator that converts an annotated context-free grammar into a deterministic LR or generalized LR (GLR) parser.

`Note:` For more details about YACC or Bison, see [here](/Syntax%20and%20Semantic%20Analyzer/Necessary%20Resources/YACC_Bison.pdf).

### Installation

***Installing Bison on linux:*** 

    sudo apt-get update
    sudo apt-get install bison

***You can see the grammers given for our C-Compiler [here](/Syntax%20and%20Semantic%20Analyzer/BisonAssignmentGrammar.pdf)***

### Our chosen subset of the C language has the following characteristics:

- There can be multiple functions. No two functions will have the same name. A function needs
  to be defined or declared before it is called. Also, a function and a global variable cannot have the same symbol.
- There will be no pre-processing directives like `#include` or `#define`.
- Variables can be declared at suitable places inside a function. Variables can also be declared in
  the global scope.
- Precedence and associativity rules are as per standard. Although we will ignore consecutive logical operators or consecutive
  relational operators like, `a && b && c`, `a < b < c`.
- No `break` statement and `switch-case` statement will be used.
- `println(int n)` is used instead of `printf(“%d\n”, n)` to simplify the analysis.

### Error recovery:

Some common syntax errors are handled and recovered so that the parser does not stop parsing immediately after recongnizing an error.

<p align="right"><a href="#top">back to top</a></p>

## Semantic Analyser

### Following semantics are checked in the compiler:

<div>
<details>
<summary>
        Type Checking 
</summary>
<ol>
<li>
        Generates error message if operands of an assignment operator are not consistent with each other. The second operand of the assignment operator will be an expression that may contain numbers, variables, function calls, etc.
</li>
<li> 
        Generates an error message if the index of an array is not an integer.
</li>
<li> 
        Both the operands of the modulus operator should be integers.
</li>
        During a function call all the arguments should be consistent with the function definition.
<li>
        A void function cannot be called as a part of an expression.
</li>
</ol>
</details>
<details>
<summary>
        Type Conversion 
</summary>
        Conversion from float to integer in any expression generates an error. Also, the result of RELOP and LOGICOP operations are integers.
</details>
<details>
<summary>
        Uniqueness Checking
</summary>
        Checks whether a variable used in an expression is declared or not. Also, checks whether there are multiple declarations of variables with the same ID in the same scope.
</details>
<details>
<summary>
        Array Index
</summary>
        Checks whether there is an index used with array and vice versa.
</details>
<details>
<summary>
        Function Parameters
</summary>
        Check whether a function is called with appropriate number of parameters with appropriate types. Function definitions should also be consistent with declaration if there is any. Besides that, a function call cannot be made with non-function type identifier.
</details>

### Non terminal data types used:

Used to check the consistancy of different terms and expressions with variable types.

- Trivial data types:

        CONST_INT
        CONST_FLOAT
        CONST_CHAR

- Other data types:
  
        VARIABLE (Any type of ID)
        FUNCTION
        ARRAY

<p align="right"><a href="#top">back to top</a></p>
        
## Intermediate Code Generation
After the syntax analyser and the semantic analyser confirms that the source program is correct, the compiler generates the intermediate code.  Ideally a three address code is generated in real life compilers. But we have used `8086 Assembly Language` as our intermediate code so that we can run it in `emu 8086` and justify that our compilation is correct.

### On the fly intermediate code generation strategy:

#### Table of Contents:

- [The Algorithm](#the-algorithm)
- [Evaluating for loop](#evaluating-for-loop-is-somewhat-tricky)
- [Evaluating functions](#evaluating-functions)
- [Declaring variables](#declaring-variables)
- [Accessing variables](#accessing-variables)
- [Conclusion](#conclusion)
- [Optimization](#optimizing-assembly-code)

#### The Algorithm:

We have generated the intermediate code on the fly. Which means that, instead of using any data structure and passing the whole code one after another to the production rules of the grammar, we have generated the intermediate code as soon as we match a rule and write it in the `code.asm` file. To do that, we have to use the `PUSH` and `POP` instructions in the assembly code which utilize the stack.

- Let's understand the algorithm with an example.
  Let's say we have a grammar like this:

          E -> E + T
          E -> T
          T -> T * F
          T -> F
          F -> id
          F -> digit

- While bison evaluates any string like `a + 2 * c` the lexer will first read the string and generate the tokens like `id, +, digit, *, id`. So the order of the matched rules will be as follows:

        a found : F -> id reduced
        + found : nothing reduced (no rules matched yet)
        2 found : F -> digit reduced
        * found : nothing reduced (no rules matched yet)
        c found : F -> id reduced

- At first only the production rules of `F` matched. So, we are going to push the `id` and `digit` to the stack of 8086.

```asm
        PUSH a
        PUSH 2
        PUSH c
```

The stack will look like this now:

        SP -> c
              2
              a

        * here SP points to the top of the stack

- Now that all the lexemes are matched, the parser will match the production rules of `E` and `T` as follows:

- As '2' and 'a' reduced to 'F' previously,
  now, for the F of '2', ` T -> F` is reduced. Now, it's finally the time to match the rule corresponding to '\*'

          2 * c found : T -> T * F reduced

- As this rule has been matched, we are going to `pop` the ID c and digit 2 from the stack, evaluate the multiplication operation and push the result to the stack.

```asm
        POP BX          ;this will pop c from the stack
        POP AX          ;this will pop 2 from the stack
        MUL BX          ;this will multiply 2 with c and store the result in AX
        PUSH AX         ;this will push the result of 2 * c to the stack
```

- Now the stack will look like this:

        SP -> 2*c
              a

- Similarly `E -> E + T` will now be reduced. So, we can pop the last two values from the stack and simply add them.

```asm
        POP BX          ;this will pop 2*c from the stack
        POP AX          ;this will pop a from the stack
        ADD AX, BX      ;this will add a and 2*c and store the result in AX
        PUSH AX         ;this will push the result of a+2*c  to the stack
```

- `Note 1`: After the whole expression is evaluated, the value of the expression is already pushed in the stack. So, we can use it in our next expression if needed or we can just pop it. For example, if we had `d = a + 2 * c`, we could use the value of `a + 2 * c` in `d` by popping it out. The stack will look like this:

        SP -> a + 2*c
              d

        * here 'd' was pushed because it matched F -> id first. But this value of 'd' is useless. So, we can pop it out.

- Here is the required asm code to evaluate the expression

```asm
        POP BX          ;this will pop a+2*c from the stack
        POP AX          ;this will pop the (useless) value of d from the stack
        MOV d, BX       ;this will store the value of a+2*c in d
        PUSH d         ;this will push the value of d to the stack
```

- The stack will look like this now:

        SP -> d

- `Note2`: With the above algorithm, we can evaluate any expression. But the catch is that, there is always an extra `PUSH` at the end of every expression. So, when we don't need that value, we can just pop it out. i.e; when we get a SEMICOLON `;` after the expression above, that will mean that the expression is over. For the example above, if it ends with a semicolon then we don't need the value of d anymore. So, we can pop it out like this:

```asm
        POP BX          ;this will pop d from the stack
```

- The stack is empty again.
- So the complete code for evaluating `d = a + 2 * c;` looks like this:

```asm
        PUSH d
        PUSH a
        PUSH 2
        PUSH c

        POP BX          ;this will pop c from the stack
        POP AX          ;this will pop 2 from the stack
        MUL BX          ;this will multiply 2 with c and store the result in AX
        PUSH AX         ;this will push the result of 2 * c to the stack

        POP BX          ;this will pop 2*c from the stack
        POP AX          ;this will pop a from the stack
        ADD AX, BX      ;this will add a and 2*c and store the result in AX
        PUSH AX         ;this will push the result of a+2*c  to the stack

        POP BX          ;this will pop a+2*c from the stack
        POP AX          ;this will pop the (useless) value of d from the stack
        MOV d, BX       ;this will store the value of a+2*c in d
        PUSH d         ;this will push the value of d to the stack

        POP BX          ;this will pop d (useless) from the stack. This was pushed at the start of the expression.
```

- If there was no semicolon in the expression, then we would not pop out the last value of d. This helps in passing parameters to functions or evaluating if statements.

<p align="right"><a href="#top">back to top</a></p>

#### Evaluating `for` loop is somewhat tricky:

- Though the above algorithm works for every expression used in the code, it requires a small trick to evaluate for loops. A for loop's grammar looks like this:

```bison
expression -> FOR LPAREN expression_statement expression_statement expression RPAREN LCURL statements RCURL
expression_statement -> expression SEMICOLON
```

- The problem in evaluating `for` loops is that we need to evaluate the third expression after the statements in the curly braces are executed. Here, we have done this by saving the line number of the starting of third expression and then inserting the codes corresponding the statements from that line. The following pseudo code might help:

```pseudo
expression -> FOR LPAREN expression_statement expression_statement
        {
                isInForLoop = true;
                lineNo = currentLineNo; // Saves the line number before third expression
        }
        expression RPAREN LCURL statements RCURL

statements -> ... {
                if(isInForLoop) {
                        insertCode(lineNo);
                } else {
                        insertCode(currentLineNo);
                }
        }

/* You might need to write a function to insert text in the middle of a file. */
```

#### Evaluating `functions`:

- Please read pages 303-305 (14.5.3 Using the stack for procedures) of the book [Assembly Language Programming and Organization of the IBM PC by Ytha Yu, Charles Marut](https://drive.google.com/file/d/1Gt-PvcimLN0oiuXbkhZ6KVM2X6POcqcM/view?usp=sharing)

<p align="right"><a href="#top">back to top</a></p>

#### Declaring variables:

- **Global variables**: We have inserted the global variables in the data segment. This is done by pushing it in the stack with it's name & corresponding offset.

- **Global arrays**: We have done this the same way as we have done for global variables. But the syntax is a little different. Follow the example below.

- **Local variables**: Whenever a local variable is declared inside a function, we have **PUSHed** a dummy value to the stack. Then we just saved the offset of that stack address with respect to BP (bottom pointer referenced in the book [here](#evaluating-functions)) in the `SymbolInfo` of that id.

- **Local Arrays**: Here we have followed the same technique as local variables but the stack is pushed "size of the array" times. Offset of the first element along with the size of the array is saved in the `SymbolInfo` of the array id.
        
    - `Note`: A better approach is to subtract the byte size of the array from SP. It will just move the SP at the end of the array.
- **Example**:

```c
        int a, b[3];
        int main() {
            int c, d[3], e;
        }
```

```asm
        ...
        .DATA
            a DW ?
            b DW 3 DUP(?)
        .CODE
        main PROC
            PUSH AX         ;A garbage value pushed for c
                            ;Offset is -2
            ;PUSH AX
            ;PUSH AX
            ;PUSH AX        ;3 garbage values pushed for d[3]
                            ;offset is -4
            ;better way of allocating space for an array
            SUB SP, 6       ;SP is moved to the end of d[3]

            PUSH AX         ;A garbage value pushed for e
                            ;offset is -10
        main ENDP
        END MAIN
```

#### Accessing variables:

- **Global variables**: We just check if it's a global variable or not. If yes then we just used the name of the variable.

- **Global arrays**: We have to use the name of the array and the index of the array. See the example below.

- **Local variables**: We have to use the offset of the stack address with respect to BP.

- **Local Arrays**: We have to use the offset of the stack address with respect to BP and then add the array index times 2 with it.

- **Example**: Continued from the previous example.

```asm
        a               ;global variable a is accessed
        b[0]            ;global array b at index 0 is accessed
        b[2]            ;global array b at index 1 is accessed

        [BP + -2]       ;local variable c is accessed
        [BP + -4]       ;local array d at index 0 is accessed
        [BP + -8]       ;local array d at index 2 is accessed
        [BP + -10]      ;local variable e is accessed
```

#### Conclusion

- It requires a lot of push and pop instructions in this approach. So, sometimes the same value is pushed and popped consecutively. For that reason, an optimization is required. This is called peephole optimization. We have done it in the second pass (after all the expressions are evaluated).

<p align="right"><a href="#top">back to top</a></p>

### Optimizing assembly code

As the above algorithm can generate some redundant instructions, we have to optimize the code. The following optimizations are done:

* **Remove redundant push and pop instructions.**
    * If the first instruction is push and the second is pop and those contains the same address or register then we can remove both the instructions. For exammple,
        ```asm
                *code.asm                               *optimized_code.asm
                PUSH AX                 ->              ;PUSH AX
                POP AX                                  ;POP AX
        ```
    * If the first is push and the second is a pop containing a register or an address then we can replace the two instructions with one MOV instruction. For example,
        ```asm
                *code.asm                               *optimized_code.asm
                PUSH [BP + -2]          ->              MOV AX, [BP + -2]
                POP AX
        ```
* **Remove redundant move instructions**
    * If a move instruction has the same source and destination then we can remove the instruction. For example,
        ```asm
                *code.asm                               *optimized_code.asm
                MOV AX, AX              ->              ;MOV AX, AX
        ```
    * If consecutive two instructions are move and the first instruction contains the same register or address as the second instruction then we can remove the first instruction. For example,
        ```asm
                *code.asm                              *optimized_code.asm
                MOV AX, BX              ->             ;MOV AX, BX
                MOV AX, CX                             MOV AX, CX
        ```
    * If consecutive two instructions are move and the source of the first instruction is the destination of the second instruction and the source of the second one is the destination of the first then we can remove the second instruction. For example,
        ```asm
                *code.asm                              *optimized_code.asm
                MOV AX, BX              ->             MOV AX, BX
                MOV BX, AX                             ;MOV BX, AX
        ```
* **Remove jump instructions followed by a label that is used by that jump**
    ```asm
                *code.asm                              *optimized_code.asm
                CMP AX, BX              ->              CMP AX, BX
                JE L1                                   ;JE L1
                L1:                                     L1:
    ```
* **Remove compare instructions that are not followed by any jump instruction**
    * It is not likely to occur in the first pass of optimization. But it might occur in the next passes if the jump instructions that followed the compare instructions were removed in a previous pass. For example, in the first pass,
        ```asm
                *code.asm                              *optimized_code.asm
                CMP AX, BX              ->              CMP AX, BX
                JE L1                                   ;JE L1
                L1:                                     L1:
        ```
    * Now, the compare instruction is useless.
        ```asm
                *optimized_code.asm                     *final_optimized_code.asm
                CMP AX, BX              ->              ;CMP AX, BX
                ;JE L1                                  ;JE L1
                L1:                                     L1:
        ```
<p align="right"><a href="#top">back to top</a></p>