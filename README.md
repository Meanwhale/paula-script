**Paula Script** is a lightweight scripting language written in C++. Features:
 - Minimum build **size about 30 KB**.
 - **No runtime memory allocation.** Everything runs in buffers, that are initialized at start.
 - Built-in parser.
 - Step-by-step, non-blocking execution.
 - Basic data types: 32-bit integer, 64-bit floating point number, text, boolean.
 - Basic arithmetic (+-*/) and comparison operators (<>=)
 - Conditions (if) and loops (while)
 - Callbacks to call your source code from a script.
 - Compiles for Windows (Visual Studio) and Linux (g++)
 - Use with the command line interface (CLI) or as library.
 - Stand-alone: the only external dependencies are for standard input/output.

## Examples
Run Paula Script from your source code. 
```
CharInput input("print("Hello World!"));
Paula::one.run(input, true);
```
Define a callback function and call it from a script:
```

const paula::Error* paula::testDoubler (Paula& p,Args& args)
{
  int value = 0;
  if(args.count() == 1 && args.get(0).getInt(value)) // gets the first argument, assuming it's an integer
  {
    args.returnInt(2 * value); // multiplies the given argument by 2 and returns it
    return &NO_ERROR;
  }
  return &CALLBACK_ERROR;
}

// ...

Paula::one.addCallback("testDoubler", testDoubler);
CharInput input("print(testDoubler(6))"); // prints "12"
Paula::one.run(input, true);
```
## Paula Language

#### Variables
Variable's type is defined when its value is assigned<p>
_variableName **:** value_<p>
Reassigned value must be of same type.
```
name: "Paula"                 | text variable
population: 123456            | integer
height: 2.04                  | floating-point number
fact: true                    | boolean
```
#### Operators
Operator expression (with +-*/<>=) can have two operards:<p>
_operand1 operator operand2_<p>
An operand can be a _literal_ value (eg. _123_), a variable, or an expression in parenthesis.
```
three: 3                      | assign an integer value
six: (three + 3)
sum: (123 + (1 / b))
greater: (six > three)        | assign boolean, true
```
#### Function calls
_functionName (argument1, argument2, ...)_<p>
An arguement can be a _literal_ value (eg. _5_ or _"Hi!"_), a variable, or an expressions in parenthesis.
```
print("Hi!")                  | literal argument
sum(one, two)                 | variable arguments
foo((b < 3), sum(1, 2))       | expression arguments
```
For conditions and loops, code blocks are defined by tab indentation, one tab per depth.
Line can't start with other whitespace characters.
#### Condition
```
if (value)
    print("It's true!")      | define code block by indentation. Execute code block if the 'value' is true
```
#### Loop
```
i: 3
while (i > 0)                 | prints "321"
    print(i)
    i:i-1
```
