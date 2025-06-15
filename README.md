**Paula Script** is a lightweight scripting language written in C++.
 - **Small** but easy to expand. Currently minimum CLI build size is about 50 kB.
 - **No runtime memory allocation:** everything runs in buffers that are initialized at the start.
 - **Stand-alone:** minimal external dependencies. Built-in parser.
 - **Line-by-line, non-blocking execution.**
 - Compiles for Windows (Visual Studio) and Linux (GCC).
 - Command line interface (CLI) and a static library.

**Paula Script** is work-in-progress and currently applicable to:
 - Simple scripts for communication with the host application via callbacks.
 - Store simple variable values, accessed from source code.

**Future plans**
 - Structured data: dictionary and array.
 - Complete set for basic programming: math, string, and bit operations.
 - Save and load entire engine state.
 - C# API.

Read more about the language details, design, and builds below! 👇

🌏 <a href=https://meanwhale.github.io/paula>Paula Script's web page</a><br>
📄 <a href=https://meanwhale.github.io/paula/api/html/namespacepaula.html>C++ API</a>
## Examples

Run Paula Script from your source code. 
```cpp
void main()
{
	paula::runSafe("print(\"Hello World!\")"); // prints "Hello World!"
}
```
Run a script from a file using CLI. Create a file named _fibonacci.paula_:
```
n:20; i:0; a:0; b:1
while(i<n)
	c:a+b
	print(c, " ")
	a:b; b:c
	i:i+1
```
Execute the script from command line:
```
paula -f fibonacci.paula
```
It prints 20 (``n:20``) Fibonacci numbers.


## Language Features

 - Basic data types: 32-bit integer, 64-bit floating point number, text, boolean.
 - Basic arithmetic (+-*/) and comparison operators (<>=).
 - Logical operators: and, or, xor.
 - Conditions (if) and loops (while).
 - Callbacks to call your source code from a script.

#### Statements

A statement can be an assignment
```
variableName: value
```
or a function call
```
functionName ( argument1, argument2)
```

Statements are separated with line breaks. Consecutive statements, except for conditionals (``if`` and ``while``), can be separated by a semicolon:
```
a: 123; b: 456; foo(a, b)
```

#### Variable assignment

Variable's type is defined when its value is assigned. Reassigned value must be of same type.
```
name: "Paula"                 | text variable
population: 123456            | integer
height: 2.04                  | floating-point number
fact: true                    | boolean
```

#### Operators

Operator expression has two operands and the operator (+-*/<>=) between them:<p>
_operand1 operator operand2_<p>
An operand can be a _literal_ value (eg. _123_), a variable, or an expression in parenthesis.
```
three: 3                      | assign an integer value
five: three + 2
sum: 123 + (100 / five)
greater: five > three         | assign boolean, true
```

#### Function calls

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;_functionName (argument1, argument2, ...)_<p>
An arguement can be a _literal_ value (eg. _5_ or _"Hi!"_), a variable, or an expression.
```
print("Hi!")                  | literal argument
sum(one, two)                 | variable arguments
foo(b<3, sum(1, 2))           | expression arguments
```

#### Condition

```
if (value)
    print("value is true!")       | define code block by tab indentation
```

For conditions, loops, procedures, and functions, *code blocks* are defined by tab indentation.
Line can't start with other whitespace characters.

#### Loop

```
i: 3                          | prints "321"
while (i > 0)
    print(i)
    i:i-1
```

#### Logical operators

Logical expressions have a logical operator (_and_, _or_, _xor_) between two boolean operands:<p>
_operand1 logical_operator operand2_<p>

```
if (p and q)
    print("both p and q are true!")
if (p or q)
    print("p or q is true!")
if (p xor q)
    print("p or q is true, but not both!")
```

#### Scripted Procedures and Functions

There's two different ways to make callable scripts:

- *Procedures* don't return a value, and are non-blocking, meaning that you can still execute line-by-line even when jumping into a procedure.
- *Functions* return a value, and are blocking i.e. function script is execute in a busy loop, not line-be-line.

You can pass both of them any number of arguments, and then read the arguments by calling _arg(i)_ where _i_ is the index of the argument, starting from _0_.

For example, define a procedure and call it:

```
proc("printTwo")
    print(arg(0))
    print(arg(1))

printTwo("Hello", "World")       | prints "HelloWorld"
```

Define a function that returns a value, a number multiplied by two:

```
func("doubler")
    x: arg(0)
    return (x * 2)

print(doubler(7))                | prints "14"
```

## Callbacks

Define a callback function and call it from a script:
```cpp
const paula::Error* doubler (paula::Args&args)
{
	if (args.count() != 1) return &WRONG_NUMBER_OF_ARGUMENTS;
	INT value = -1;
	if(args.get(0).getInt(value))
	{
		args.returnInt(2*value);
		return NO_ERROR;
	}
	return &CALLBACK_ERROR;
}

void main()
{
	auto error = paula::addCallback("doubler", doubler);
	paula::runSafe("six: doubler(3)");
	INT value;
	if (paula::get("six").getInt(value))
	{
		std::cout<<"six = "<<value<<std::endl; // prints "six = 6"
	}
}
```
 - Define a callback ``doubler`` that reads a given argument ``args.get(0).getInt(value)`` and returns the argument multiplied by 2: ``args.returnInt(2*value)``.
 - Register the callback to the Paula engine (``addCallback``).
 - Run a script that calls the callback (``doubler(3)``) and assign the return value (6) to a variable ``six``.
 - Get the value of the variable: ``paula::get("six").getInt(value)`` and print it.

# Design

 Paula script’s design goal is to be small, stand-alone, with no implicit runtime memory allocation, targeted for scripting in memory-constrained environments, and to have minimal overhead.

 #### Script execution process

- The script is read from an input stream (*IInputStream*), which can be a file, standard input, or any other source that implements the input interface.
- The script is parsed using the *ByteAutomata* state machine, based on its own [project](https://github.com/Meanwhale/ByteAutomata).
  *ByteAutomata* outputs a token tree. The tree data (*Tree* class) is stored in a fixed-size *int* array to avoid memory allocations.
- The Paula engine (*Engine* class) then executes the parsed script by iterating over the token tree and interpreting the commands.
- Command-line arguments are saved on the stack and accessed using the *arg(i)* function, where *i* is the index of the argument.
  To support minimalism, the same *arg(i)* function is used to read parameters inside scripted functions—i.e., it reads from the stack within the current script block context.
- Script execution is non-blocking, meaning you can execute the script line by line, doing other things in between, as the Paula engine maintains its execution state.
  The only exception is scripted functions that return a value. Since the return value might be needed within the current line, functions are executed in a busy loop before the Paula engine exits.
  Procedures (which do not return values) are a non-blocking alternative for scripted subroutines. See examples for syntax.
- Paula script functionality is executed via callbacks—C++ functions registered in the Paula engine by name.
  When a callback is invoked from the script, the Paula engine calls the corresponding C++ function.
  Call parameters are passed using an *Args* object, which provides access to the parameter array by index.
  Parameter values are accessed via a *Var* object, which wraps a pointer to the data (*int* pointer) and provides the value’s type and contents (*int*, *float*, text, etc.).
- Typed data in variables and the stack is stored in an *int* array format, which includes the data type and size along with the data itself.
- Paula variables are stored in a fixed-size array containing name-value pairs.


# Build

#### Visual Studio

Open _projects/paula-vs/paula-vs.sln_. Projects:

- _paula-core_: script parser, engine, etc.
- _paula-cli_: command line interface (CLI).
- _paula-test_: unit tests.
- _paula-example_: example project to try out Paula script. Main source file: _projects/paula-example/paula-example.cpp_

Build configurations:

- Debug: asserts and debug prints enabled.
- Realease: asserts and debug prints disabled.
- Mini: experimental, minimal build.

#### Linux

Run _make_ with a target option in project root. Build target is _bin_ folder. Make targets:
```
make release    # -> "bin/paula". CLI release build
make debug      # -> "bin/pauladbg". Debug test build
make mini       # -> "bin/paulamini". Experimental, minimal CLI that can only run bytecode, not compile
make example    # -> "bin/paulaexample". Example project to try out Paula script. Main: projects/paula-example/paula-example.cpp
```

Test CLI by running a script:
```
bin/paula -f projects/test.pa
```

Translate script to a bytecode file, to execute later:
```
bin/paula -c projects/test.pa paula.bytecode
```

Run translated bytecode:
```
bin/paula -b paula.bytecode
```

Mini version can only run bytecode via input redirection:
```
bin/paulamini < paula.bytecode
```

Test debug build to see if there's memory or other issues:
```
bin/pauladbg
```



<hr>
Copyright © Meanwhale
