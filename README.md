**Paula Script** is a lightweight scripting language written in C++. Features:
 - Bare-bones but fully operational build size about 30 KB.
 - No runtime memory allocation.
 - Basic data types: int, double, text, boolean.
 - Operators: +-*/<>=
 - Conditions (if) and loops (while)
 - Callbacks to call your source code from a script.
 - Built-in parser.
 - Step-by-step, non-blocking execution.
 - Compiles for Windows (Visual Studio) and Linux (g++)

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
  if(args.get(0).getInt(value)) // gets the first argument, assuming it's an integer
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
