**Paula Script** is a lightweight scripting language written in C++.
 - **Small** but easy to expand. Currently minimum CLI build size is about 35 KB.
 - **No runtime memory allocation:** everything runs in buffers, that are initialized at start.
 - **Stand-alone:** the only external dependencies are for standard input/output. Built-in parser.
 - **Line-by-line, non-blocking execution:** Paula executes the code on-fly, while reading the input.
   It can take input from an endless input stream, and execute the code from a buffer, without memory issues (no runtime memory allocation).
 - Compiles for Windows (Visual Studio) and Linux (g++).
 - Command line interface (CLI) and a static library.

**Project status:** work-in-progress. 

## Examples

Run Paula Script from your source code. 
```cpp
int main()
{
	paula::runAndCatch("print(\"Hello World!\")"); // prints "Hello World!"
}
```

### Language Features

 - Basic data types: 32-bit integer, 64-bit floating point number, text, boolean.
 - Basic arithmetic (+-*/) and comparison operators (<>=).
 - Conditions (if) and loops (while).
 - Callbacks to call your source code from a script.

#### Variable assignment

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;_variableName **:** value_<p>
Variable's type is defined when its value is assigned. Reassigned value must be of same type.
```
name: "Paula"                 | text variable
population: 123456            | integer
height: 2.04                  | floating-point number
fact: true                    | boolean
```

#### Operators

Operator expression can have two operards and the operator (+-*/<>=) between them:<p>
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
An arguement can be a _literal_ value (eg. _5_ or _"Hi!"_), a variable, or an expression in parenthesis.
```
print("Hi!")                  | literal argument
sum(one, two)                 | variable arguments
foo(b<3, sum(1, 2))           | expression arguments
```

#### Condition

```
if (value)
    print("It's true!")       | define code block by indentation. Execute code block if the 'value' is true
```

#### Loop

```
i: 3                          | prints "321"
while (i > 0)
    print(i)
    i:i-1
```
For conditions and loops, code blocks are defined by tab indentation, one tab per depth.
Line can't start with other whitespace characters.

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

int main()
{
	auto error = paula::addCallback("doubler", doubler);
	paula::runAndCatch("six: doubler(3)");
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

# C++ API

See <a href="https://github.com/Meanwhale/paula-script/blob/main/src/paula.h">paula.h header file</a> for complete, public API.

<!--
Global functions in ``paula`` namespace:
```cpp
const Error * run(const char*);      // run a script and return error (NO_ERROR if everything's OK)
const Error * run(IInputStream&);    // run a script from a stream (see stream.h)
void runAndCatch(const char*);       // run script and print error if any
void runAndCatch(IInputStream&);
Var get(const char * varName);       // get a variable that is assigned in the script. See about Var below.
const Error * addCallback(const char* name, const Error* (*_action)(Args&)); // register a callback to call from a script
```
``Var`` class is basically a pointer (int*) to a Paula variable or empty data by default.
Getters assign the variable's value to reference argument.
They return true of the variable is found, or false if not.
```cpp
bool getInt(INT& out) const;
bool getDouble(DOUBLE& out) const;
bool getBool(bool& out) const;
bool getOp(char& out) const;
bool getChars(char*&out) const;
```
For example:
```cpp
paula::runAndCatch("six: 6");
INT value;
Var v = paula::get("six");
if (v.getInt(value)) std::cout<<"six = "<<value<<std::endl;
else std::cout<<"variable not found..."<<std::endl;
```
-->

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
make release    # CLI release build
make debug      # Debug test build
make mini       # experimental, minimal  CLI
make example    # example project to try out Paula script. Main source file: projects/paula-example/paula-example.cpp
```
