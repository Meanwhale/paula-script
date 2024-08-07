**Paula Script** is a lightweight scripting language written in C++. Features:
 - Minimum build **size about 35 KB**.
 - **No runtime memory allocation.** Everything runs in buffers, that are initialized at start.
 - Built-in parser.
 - Line-by-line, non-blocking execution. Paula executes the code on-fly, while reading the input.
   It can take input from an endless input stream, and execute the code from a buffer, without memory issues (no runtime memory allocation).
 - Basic data types: 32-bit integer, 64-bit floating point number, text, boolean.
 - Basic arithmetic (+-*/) and comparison operators (<>=).
 - Conditions (if) and loops (while).
 - Callbacks to call your source code from a script.
 - Compiles for Windows (Visual Studio) and Linux (g++).
 - Command line interface (CLI) and a static library.
 - Stand-alone: the only external dependencies are for standard input/output.

## Examples
Run Paula Script from your source code. 
```cpp
int main()
{
	paula::runAndCatch("print(\"Hello World!\")"); // prints "Hello World!"
}
```

### Language Features

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
Operator expression (with +-*/<>=) can have two operards:<p>
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
