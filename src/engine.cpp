#include "engine.h"
#include "stream.h"
#include "args.h"

using namespace paula;
using namespace paula::core;

// PAULA

ERROR_STATUS printAction (Engine&,Args&args)
{
	LOG.println("-------- PRINT ACTION --------");
	for(INT i=0; i<args.count(); i++)
	{
		pout.print(args.get(i));
	}
	LOG.endl().println("------------------------------");
	return NO_ERROR;
}
ERROR_STATUS notAction (Engine&p,Args&args)
{
	LOG.println("-------- NOT ACTION --------");
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	bool value = false;
	if(args.get(0).getBool(value))
	{
		args.returnBool(!value);
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS getArgAction (Engine&p,Args&args)
{
	LOG.println("-------- GET ARG ACTION --------");
	VRB(p.stack.printData());
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	INT argIndex = false;
	if(args.get(0).getInt(argIndex))
	{
		INT numArgs = p.globalArgs.count();
		CHECK (argIndex >= 0 && argIndex < numArgs, ARRAY_OUT_OF_RANGE);
		args.returnData(p.globalArgs.get(numArgs - argIndex - 1)); // reverse order
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS numArgsAction (Engine&p,Args&args)
{
	LOG.println("-------- NUM ARGS ACTION --------");
	CHECK(args.count() == 0, WRONG_NUMBER_OF_ARGUMENTS);
	args.returnInt(p.globalArgs.count());
	return NO_ERROR;
}
ERROR_STATUS whileAction (Engine&p,Args&args)
{
	LOG.println("-------- WHILE ACTION --------");
	CHECK(p.oneLiner, CONDITION_LINE_WITH_SEMICOLON);
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	bool value = false;
	if(args.get(0).getBool(value))
	{
		if (value) p.startLoop();
		else p.skipBlock();
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS ifAction (Engine&p,Args&args)
{
	LOG.println("-------- IF ACTION --------");
	CHECK(p.oneLiner, CONDITION_LINE_WITH_SEMICOLON);
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	bool value = false;
	if(args.get(0).getBool(value))
	{
		if (value) p.startIf();
		else p.skipBlock();
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS createProcedureOrFunctionAction (Engine&p,Args&args, bool function)
{
	LOG.println("-------- CREATE PROCEDURE/FUNCTION ACTION --------");
	CHECK(p.currentIndentation == 0, INDENTATION_ERROR);
	CHECK(p.oneLiner, CONDITION_LINE_WITH_SEMICOLON);
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	char* procedureName;
	if(args.get(0).getChars(procedureName))
	{
		// create _proc_ variable that points to procedure bytecode

		LOG.print("add ").print(function?"FUNCTION":"PROCEDURE").print(": ").print(procedureName).print(", address ").print(p.bytecodeIndex).endl();

		p.skipBlock(); // skip the procedure code after creating it
		return p.addProcedure(procedureName, p.bytecodeIndex, function);
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS createProcedureAction (Engine&p,Args&args)
{
	return createProcedureOrFunctionAction(p,args,false);
}

ERROR_STATUS createScriptFunctionAction (Engine&p,Args&args)
{
	return createProcedureOrFunctionAction(p,args,true);
}

ERROR_STATUS procedureBackAction (Engine&p,Args&args)
{
	LOG.println("-------- PROCEDURE BACK ACTION --------");
	CHECK(args.count() == 0, WRONG_NUMBER_OF_ARGUMENTS);
	return p.backFromProcedureOrFunction(false);
}


ERROR_STATUS scriptFunctionReturnAction (Engine&p,Args&args)
{
	LOG.println("-------- FUNCTION RETURN ACTION --------");
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	p.stack.pushData(args.get(0));
	// args.returnData(args.get(0));
	return p.backFromProcedureOrFunction(true);
}


Engine Engine::one = Engine();


Engine::Engine() : // NOTE: unnecessary warning about blockStack initialization
	vars(VARS_SIZE),
	oneLiner(true),
	skipNextAfterJump(false),
	returnCalled(false),
	currentIndentation(0),
	skipIndentation(-1),
	blockStackSize(0),
	bytecodeIndex(0),
	numCallbacks(0),
	numProcedures(0),
	jumpIndex(-1),
#ifndef PAULA_MINI
	automata(*this),
#endif
	stack(ARG_STACK_SIZE),
	constants(CONSTANTS_SIZE),
	bytecode(BYTECODE_SIZE),
	globalArgs(),
	returnValue(MAX_RETURN_VALUE_SIZE),
	commands
	{
		Command("print", printAction),
		Command("not", notAction),
		Command("while", whileAction),
		Command("if", ifAction),
		Command("arg", getArgAction),
		Command("argc", numArgsAction),
		Command("proc", createProcedureAction),
		Command("func", createScriptFunctionAction),
		Command("back", procedureBackAction),
		Command("return", scriptFunctionReturnAction)
	}
{
	constants.init(NODE_SUBTREE);

	LOG.println("---------------- NEW PAULA ----------------");

	INT kvIndex = constants.addSubtree(0, NODE_KV_TREE);
	constants.addText(kvIndex, "true");
	constants.addBool(kvIndex, true);
	
	kvIndex = constants.addSubtree(0, NODE_KV_TREE);
	constants.addText(kvIndex, "false");
	constants.addBool(kvIndex, false);

	/*
			INT andKeywordData[KEYWORD_DATA_SIZE],
				 orKeywordData[KEYWORD_DATA_SIZE],
				xprKeywordData[KEYWORD_DATA_SIZE];*/

	Array<INT> andNameData (andKeywordData, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData("and", andNameData);
	Array<INT> orNameData (orKeywordData, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData("or", orNameData);
	Array<INT> xorNameData (xorKeywordData, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData("xor", xorNameData);
}
void Engine::reset()
{
	vars.init(NODE_SUBTREE);
	bytecode.init(NODE_SUBTREE);

	oneLiner = true;
	skipNextAfterJump = false;
	returnCalled = false;
	currentIndentation = 0;
	skipIndentation = -1;
	blockStackSize = 0;
	// numCallbacks: won't reset
	numProcedures = 0;
	jumpIndex = -1;
	returnValue.clear();
	const Error* error = nullptr; // NO_ERROR
}


#ifndef PAULA_MINI

ERROR_STATUS Engine::compile(IInputStream&input, BinaryOutputStream&output)
{
	reset();
	CHECK_CALL(parse(input)); // read script from stream
	bytecode.write(output); // save parsed bytecode to stream
	return NO_ERROR;
}

void paula::core::Engine::runSafe(IInputStream& input)
{
	runSafe(input, nullptr, 0);
}

void paula::core::Engine::runSafe(IInputStream& input, const char** args, int numArgs)
{
	auto error = runScript(input, args, numArgs);
	if (error != NO_ERROR)
	{
		ERR.print("RUN SAFE Caught an exception: ").print(error->name).print(" (id=").print(error->id).print(")").endl();
		ERR.flush();

		// MINI
		// log.endl().print("ERROR: L").print(error->id).endl(); // L = line

	}
	log.flush();
}
ERROR_STATUS Engine::parse(IInputStream& input)
{
	automata.init(&input);
	bool running = true;
	while(running)
	{
		running = automata.parseLine(&input);
		const Error* error = automata.getError();
		if (error != nullptr) return error;
		CHECK_CALL(addParsedLine());	// add command to bytecode list
		automata.resetCommand();		// prepare to another command

		// reset line here so that addParsedLine has correct values
		if (automata.currentState == automata.stateNewLine) automata.resetNewLine();
	}
	return NO_ERROR;
}

ERROR_STATUS Engine::addParsedLine()
{
	if (automata.commandType == LINE_UNDEFINED) return NO_ERROR; // empty line

	// add line info and parsed tree
	INT nodeIndex = bytecode.addSubtree(0, NODE_SUBTREE);
	bytecode.addInt(nodeIndex, automata.lineNumber);
	bytecode.addInt(nodeIndex, automata.indentation);
	bytecode.addInt(nodeIndex, automata.commandType);
	bytecode.addBool(nodeIndex, automata.oneLiner);
	bytecode.addRawTree(nodeIndex, automata.tree);
	//VRB(LOG.print("PARSED TREE ADDED:\n"));
	//VRB(automata.tree.print());
	//VRB(LOG.endl());
	LOG.print("PARSED TREE ADDED:\n");
	automata.tree.print();
	LOG.endl();
	return NO_ERROR;
}

ERROR_STATUS Engine::runScript(IInputStream& input)
{
	return runScript(input, nullptr, 0);
}
ERROR_STATUS paula::core::Engine::runScript(IInputStream& input, const char** args, int numArgs)
{
	LOG.println("Paula::run");

	reset();

	// command-line args

	stack.clear();
	for (INT i=0; i<numArgs; i++)
	{
		stack.pushText(args[i]);
	}
	stack.pushInt(numArgs); // TODO: tee oma datatyyppi
	globalArgs = Args(this, stack.topPtr());

	// parse lines and add them to the bytecode list
	CHECK_CALL(parse(input));

	// all lines parsed --> execute bytecode

	VRB(bytecode.print());

	CHECK_CALL(runBytecode(-1));

	return NO_ERROR;
}
#endif

ERROR_STATUS Engine::runBytecode(IInputStream& input, const char **args, int numArgs)
{
	// read bytecode from input stream and run it
	reset();
	CHECK_CALL(bytecode.read(input));
	CHECK_CALL(runBytecode(-1));
	return NO_ERROR;
}

ERROR_STATUS Engine::runBytecode(INT startIndex)
{
	TreeIterator it(bytecode);
	if (!it.hasChild())	return NO_ERROR;
	it.toChild();

	if (startIndex >= 0) it.jumpTo(startIndex);

	bool hasNextLine;
	do
	{
		LOG.print("line index: ").print(it.getIndex()).endl();

		if (skipNextAfterJump)
		{
			if (!it.next()) break;
			skipNextAfterJump = false;
		}

		VRB(printInfo());

		INT bytecodeIndex = it.index;

		it.toChild();
		// read line data
		INT lineNumber, indentation, commandType;

		it.var().getInt(lineNumber);  it.next();
		it.var().getInt(indentation); it.next();
		it.var().getInt(commandType); it.next();
		it.var().getBool(oneLiner); it.next();

		// assign parsed tree from bytecode
		INT* rawTreeDataPtr  = it.tree.data.ptr(it.index + 3);
		INT  rawTreeDataSize = it.var().size() - 2;
		Tree parsedCommand(rawTreeDataPtr, rawTreeDataSize);

		CHECK_CALL(executeLine(indentation, bytecodeIndex, commandType, parsedCommand));

		it.toParent();

		if (returnCalled)
		{
			returnCalled = false;
			return NO_ERROR;
		}
		if (!it.hasNext() && jumpIndex < 0)
		{
			// end of file. make a call to end if's and loops. possibly jump back by moving bytecode index.
			bool executeLine = false;
			CHECK_CALL(lineIndentationInit(0, executeLine));
		}
		if (jumpIndex >= 0)
		{
			it.jumpTo(jumpIndex);
			ASSERT(it.isType(NODE_SUBTREE));
			jumpIndex = -1;
			hasNextLine = true;
		}
		else
		{
			hasNextLine = it.next();
		}
	}
	while(hasNextLine);

	return NO_ERROR;
}
// Accept std::function so callers can pass:
//   - a plain C++ function pointer         paula::addCallback("f", myFn)
//   - a capturing lambda                   paula::addCallback("f", [x](Args& a){ ... })
//   - a C callback via the paula_c bridge  paula_add_callback("f", myCFn)
//     (paula_c.cpp wraps the C fn pointer in a lambda before calling here)
ERROR_STATUS core::Engine::addCallback(const char* callbackName, std::function<const Error*(Args&)> _action)
{
	INT tmp[MAX_VAR_NAME_DATA_LENGTH];
	Array<INT> nameData (tmp, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData(callbackName, nameData);
	if (isReservedName(nameData.ptr())) return &RESERVED_NAME;
	
	if (numCallbacks >= MAX_USER_CALLBACKS) return &CALLBACK_ERROR;
	callbacks[numCallbacks].setup(nameData, _action);
	numCallbacks++;
	return NO_ERROR;
}
ERROR_STATUS paula::core::Engine::callProcedure(INT address, Args& args, bool function)
{
	INT savedBytecodeIndex = bytecodeIndex;
	INT savedIndetation = currentIndentation;
	skipNextAfterJump = true;
	if (function)
	{
		// block execution
		startProcedureOrFunction(function);
		currentIndentation = 1;
		CHECK_CALL(runBytecode(address));
		bytecodeIndex = savedBytecodeIndex; // reset
		currentIndentation = savedIndetation;
		args.returnData(stack.topVar());
		stack.pop();		
	}
	else
	{
		CHECK_CALL(jump(address));
		startProcedureOrFunction(function);
	}
	return NO_ERROR;
}

ERROR_STATUS core::Engine::jump(INT bytecodeIndex)
{
	// move bytecode iterator
	ASSERT(jumpIndex < 0);
	jumpIndex = bytecodeIndex;
	return NO_ERROR;
}

ERROR_STATUS paula::core::Engine::backFromProcedureOrFunction(bool function)
{
	// pop until reach function/procedure block base
	Block& block = blockStack[blockStackSize-1];

	while(block.blockType != (function ? BLOCK_TYPE_FUNCTION : BLOCK_TYPE_PROCEDURE))
	{
		blockStackSize--;
		block = blockStack[blockStackSize-1];
	}

	// NOTE: similar to line indentation check

	//blockStackSize--;

	if (!function)
	{
		CHECK_CALL(jump(block.startBytecodeIndex)); // blocking function call handles bytecodeIndex
		skipNextAfterJump = true;
	}
	else
	{
		blockStackSize--;
		returnCalled = true;
		skipNextAfterJump = false;
	}
	
	// set callers args
	//blockStack[blockStackSize].argsBasePtr = globalArgs.stackBase;
	globalArgs = Args(this, block.argsBasePtr);
	return NO_ERROR;
}
ERROR_STATUS paula::core::Engine::addProcedure(char* procedureName, INT address, bool function)
{
	INT tmp[MAX_VAR_NAME_DATA_LENGTH];
	Array<INT> nameData (tmp, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData(procedureName, nameData);
	if (isReservedName(nameData.ptr())) return &RESERVED_NAME;

	if (numProcedures >= MAX_SCRIPT_PROCEDURES) return &CALLBACK_ERROR;
	procedures[numProcedures] = ProcedureCallback(procedureName, address, function);
	numProcedures++;
	return NO_ERROR;
}

void paula::core::Engine::printInfo()
{
	LOG.println("-------- INFO --------");
	LOG.println("stack");
	stack.printValues();
	LOG.println("global args");
	globalArgs.print();
	LOG.print("blockStackSize: ").print(blockStackSize).endl();
	for(INT i=0; i<blockStackSize; i++)
	{
		Block& block = blockStack[i];
		LOG.print("block[").print(i).print("] type: ").print(blockTypeName(block.blockType)).print(", indentation: ").print(block.indentation).endl();
		if (block.argsBasePtr != nullptr)
		{
			LOG.println("---- BLOCK ARGS ----");
			Args a(this, block.argsBasePtr);
			a.print();
			LOG.println("--------------------");
		}
	}
	LOG.println("----------------------");
}

const char * paula::core::Engine::blockTypeName(INT blockType)
{
	switch(blockType)
	{
	case BLOCK_TYPE_LOOP: return "LOOP";
	case BLOCK_TYPE_CONDITIONAL: return "CONDITIONAL";
	case BLOCK_TYPE_PROCEDURE: return "PROCEDURE";
	case BLOCK_TYPE_FUNCTION: return "SCRIPT FUNCTION";
	}
	return " -.- ERROR -.-";
}



ERROR_STATUS core::Engine::lineIndentationInit(INT indentation, bool& executeLine)
{
	// handle code block start and end according to previous and current line indentation

	executeLine = true;
	currentIndentation = indentation;

	if (skipIndentation >= 0)
	{
		LOG.println("-------- SKIP INDENTATION --------");
		if (indentation >= skipIndentation)
		{
			LOG.println("-------- SKIP LINE --------");
			// inside a block
			executeLine = false;
			return NO_ERROR;
		}
		LOG.println("-------- END SKIP --------");
		skipIndentation = -1;
	}

	if (blockStackSize == 0)
	{
		CHECK(indentation == blockStackSize, INDENTATION_ERROR);
	}
	else
	{
		Block& block = blockStack[blockStackSize-1];
		if (indentation == block.indentation)
		{
			LOG.println("stay inside the block");
		}
		else while (indentation < block.indentation)
		{
			LOG.println("end of the block");
			if (block.blockType == BLOCK_TYPE_LOOP)
			{
				LOG.println("-------- JUMP BACK --------");
				LOG.print("address: ").print(block.startBytecodeIndex).endl();
				LOG.print("indentation: ").print(block.indentation).endl();
				LOG.print("current blockStackSize: ").print(blockStackSize).endl();
				CHECK_CALL(jump(block.startBytecodeIndex));
				blockStackSize--;
				LOG.print("jump back, pop stack, stack size: ").print(blockStackSize).endl();
				executeLine = false;
				return NO_ERROR;
			}
			else if (block.blockType == BLOCK_TYPE_CONDITIONAL)
			{
				LOG.println("-------- END IF BLOCK --------");
				blockStackSize--;
				if (blockStackSize > 0)
				{
					// continue as several blocks might have ended
					block = blockStack[blockStackSize-1];
				}
				else
				{
					return NO_ERROR;
				}
			}
			else if (block.blockType == BLOCK_TYPE_PROCEDURE)
			{
				LOG.println("-------- END OF PROCEDURE --------");

				while (stack.topPtr() != block.argsBasePtr)
				{
					LOG.println("--------      pop         --------");
					stack.pop();
				}

				blockStackSize--;

				CHECK_CALL(jump(block.startBytecodeIndex));
				executeLine = false;
				skipNextAfterJump = true;

				// set callers args
				//blockStack[blockStackSize].argsBasePtr = globalArgs.stackBase;
				globalArgs = Args(this, block.argsBasePtr);

				return NO_ERROR;
			}
			else if (block.blockType == BLOCK_TYPE_FUNCTION)
			{
				return &FUNCTION_WITHOUT_RETURN;
			}
			else
			{
				ERR.print("block type: ").print(block.blockType).endl();
				ASSERT(false);
			}
		}
	}
	return NO_ERROR;
}

ERROR_STATUS core::Engine::executeLine(INT indentation, INT _bytecodeIndex, INT lineType, Tree& tree)
{
	bool executeLine = false;
	CHECK_CALL(lineIndentationInit(indentation, executeLine));

	if (!executeLine) return NO_ERROR;

	LOG.print("\nEXECUTE LINE >> ").print(indentation).print(" ");
	tree.printCompact();

	bytecodeIndex = _bytecodeIndex;

	//stack.clear();

	if (lineType == LINE_ASSIGNMENT)
	{
		// TRG : SRC

		VRB(LOG.println("--------- ASSIGN  VAR ---------"));
		TreeIterator it(tree);
		it.toChild(); // points to variable name
		LOG.print("variable name: ").print(it).print(" ");

		// new or override?

		VRB(vars.print();)

		INT index = findVariableIndex(it.getTextData(), vars);

		if (index >= 0)
		{
			VRB(LOG.println("OVERWRITE"));
			// variable already exists
			TreeIterator data(vars, index); // points to the data
			VRB(LOG.print("old value: ").print(data).endl());

			it.next(); // move to SRC
			CHECK_CALL(pushExprArg(it));
			StackIterator src(stack);
			LOG.print("overwrite: ").print(src.var()).endl();
			if (src.type() == NODE_TEXT || data.type() == NODE_TEXT) return &TEXT_VARIABLE_OVERWRITE;
			data.overwrite(src.var());
			stack.pop();
		}
		else
		{
			VRB(LOG.println("NEW VAR"));
			if (isReservedName(it.getTextData())) return &RESERVED_NAME;
			INT kvIndex = vars.addSubtree(0, NODE_KV_TREE);
			// new
			vars.addData(kvIndex, it); // add variable name to KV
			it.next(); // move to SRC
			CHECK_CALL(pushExprArg(it));
			StackIterator src(stack);
			LOG.print(" new: ").print(src.var()).endl();
			vars.addData(kvIndex, src.var()); // add value to KV
			stack.pop();
		}
		vars.print();
		// VRB(vars.print();)
		LOG.println("--------- ASSIGN  END ---------");
	}
	else if (lineType == LINE_CALL)
	{
		// COMMAND { funcName ( args ) }

		VRB(LOG.print("execute COMMAND: indentation=").print(indentation).endl());
		TreeIterator it(tree);
		it.toChild(); // points to command name

		auto cmd = findCommand(it.getTextData());
		CHECK_ERR(cmd != nullptr, UNKNOWN_COMMAND, it);
		it.next();
		CHECK(!it.hasNext(), SYNTAX_ERROR); // extra tokens after ()
		CHECK_CALL(pushArgListAndExecute(it, cmd));
	}
	else
	{
		ASSERT(false);
	}
	if (blockStackSize == 0 && skipIndentation < 0)
	{
		// TODO: onko t�m� tarpeellinen?
#ifndef PAULA_MINI
		automata.clearBuffer();
#endif
	}
	return NO_ERROR;
}

void core::Engine::startLoop()
{
	LOG.println("-------- START LOOP --------");
	ASSERT(blockStackSize>=0 && blockStackSize<MAX_BLOCK_DEPTH);
	blockStack[blockStackSize].startBytecodeIndex = bytecodeIndex;
	blockStack[blockStackSize].indentation = currentIndentation+1;
	blockStack[blockStackSize].blockType = BLOCK_TYPE_LOOP;
	blockStack[blockStackSize].argsBasePtr = nullptr;
	blockStackSize++;
}
void core::Engine::startIf()
{
	LOG.println("-------- START IF --------");
	ASSERT(blockStackSize>=0 && blockStackSize<MAX_BLOCK_DEPTH);
	blockStack[blockStackSize].startBytecodeIndex = -123456; // not needed
	blockStack[blockStackSize].indentation = currentIndentation+1;
	blockStack[blockStackSize].blockType = BLOCK_TYPE_CONDITIONAL;
	blockStack[blockStackSize].argsBasePtr = nullptr;
	blockStackSize++;
}
void core::Engine::startProcedureOrFunction(bool function)
{
	LOG.println("-------- START PROCEDURE/FUNCTION --------");
	ASSERT(blockStackSize>=0 && blockStackSize<MAX_BLOCK_DEPTH);
	blockStack[blockStackSize].startBytecodeIndex = bytecodeIndex; // not needed
	blockStack[blockStackSize].indentation = 1;
	blockStack[blockStackSize].blockType = function ? BLOCK_TYPE_FUNCTION : BLOCK_TYPE_PROCEDURE;
	blockStack[blockStackSize].argsBasePtr = globalArgs.stackBase;
	blockStackSize++;

	globalArgs = Args(this, stack.topPtr()); // set globalArgs to point procedure call args
	
	globalArgs.print();
}

void core::Engine::skipBlock()
{
	LOG.println("-------- SKIP BLOCK --------");
	
	// skip lines with indentation equal or greater than skipIndentation
	// eg. code in 'if' block.

	skipIndentation = currentIndentation + 1;
}

ERROR_STATUS core::Engine::pushArgListAndExecute(TreeIterator& _it, ICallback * cmd)
{
	// push list of expressions, eg. ( 1, f(x), y )
	// --> pushExprArg("1"), pushExprArg("f(x)"), pushExprArg("y")


	TreeIterator it(_it);
	CHECK(it.isType(NODE_SUBTREE), SYNTAX_ERROR);

	INT numArgs = 0;

	if (it.hasChild()) // else empty ()
	{
		it.toChild();
		do
		{
			CHECK_CALL(pushExprSubtreeArg(it));
			numArgs ++;
		}
		while(it.next());

	}
	stack.pushInt(numArgs); // TODO: oma datatyyppi
	Args args(this, stack.topPtr());

	stack.printValues();

	CHECK_CALL(cmd->execute(*this, args));

	// pop function arguments after call and push return value
	
	if (cmd->retunsValue())
	{
		while (numArgs-- >= 0) stack.pop();

		if (args.hasReturnValue())
		{
			stack.pushData(returnValue.topPtr());
		}
	}

	return NO_ERROR;
}

ERROR_STATUS core::Engine::pushAtomicValue(TreeIterator&_it)
{
	// push a value in expression,
	// eg. "1" in "f(1)" or "2" in "f(2+3)" or "(4+5)" in "f((4+5)+6)"

	INT stackSizeBefore = stack.itemCount();
	TreeIterator it(_it);

	VRB(LOG.print("push atomic value: ").print(it).endl());

	if (it.isType(NODE_INTEGER) || it.isType(NODE_DOUBLE) || it.isType(NODE_BOOL) || it.isType(NODE_TEXT))
	{
		stack.pushData(it);
	}
	else if(it.isType(NODE_SUBTREE))
	{
		// arg. in (), eg. "(2+3)" in "1 + (2+3)"
		TreeIterator it(_it);
		ASSERT(it.isType(NODE_SUBTREE));
		it.toChild(); // 1 + ( to here <EXPR> )
		CHECK(!it.hasNext(), SYNTAX_ERROR);
		CHECK_CALL(pushExprSubtreeArg(it));
	}
	else if(it.isType(NODE_NAME))
	{
		VRB(LOG.print("find variable: ").print(it).endl());
		CHECK_CALL(pushVariable(it));
	}
	else
	{
		ASSERT(false);
	}

	INT stackSizeAfter = stack.itemCount();
	CHECK(stackSizeBefore + 1 == stackSizeAfter, EMPTY_ARGUMENT_VALUE);
	return NO_ERROR;
}

ERROR_STATUS core::Engine::pushVariable(TreeIterator& name)
{
	INT index = findVariableIndex(name.getTextData(), constants);
	if (index >= 0)
	{
		TreeIterator out(constants, index);
		CHECK_CALL(pushAtomicValue(out));
		return NO_ERROR;
	}
	index = findVariableIndex(name.getTextData(), vars);
	if (index >= 0)
	{
		TreeIterator out(vars, index);
		CHECK_CALL(pushAtomicValue(out));
		return NO_ERROR;
	}
	
	return &VARIABLE_NOT_FOUND;
}

INT core::Engine::findVariableIndex(INT* nameData, Tree& variableMap)
{

	// iterate variables and find name. return true if found.

	TreeIterator it(variableMap);
	if (!it.hasChild())	return -1;
	it.toChild();
	do
	{
		it.toChild(); // first child is the name
		if (matchTextData(it.getTextData(), nameData))
		{
			it.next(); // found! move to data
			return it.index;
		}
		it.toParent();
	}
	while(it.next());
	return -1; // variable not found in the tree
}

ERROR_STATUS core::Engine::pushExprArg(TreeIterator& it)
{
	INT stackSizeBefore = stack.itemCount();
	// 'it' now points to first element of the expression, eg. "x" in "x + 1"

	if (!it.hasNext())
	{
		// eg. "y" in "f(x,y,z)"
		CHECK_CALL(pushAtomicValue(it));
	}
	else
	{
		// next can be
		// a) () -> function call, eg. "f(x,y)"
		// b) operator, eg. "x+y"

		if (it.isNextType(NODE_SUBTREE))
		{
			VRB(LOG.println("push function return value"));
			auto cmd = findCommand(it.getTextData());
			CHECK_ERR(cmd != nullptr, UNKNOWN_COMMAND, it);
			it.next(); // it points to "(...)" in "f(...)"
			CHECK(!it.hasNext(), SYNTAX_ERROR);
			CHECK_CALL(pushArgListAndExecute(it, cmd));
		}
		else if (it.isNextType(NODE_LOGICAL))
		{
			// eg. "a xor b"
			// read logical operands a and b. push value, read from top, and pop.

			CHECK_CALL(pushAtomicValue(it));
			bool a = false;
			CHECK_ERR(stack.topVar().getBool(a), SYNTAX_ERROR, it);
			stack.pop();

			// read the logical operator

			it.next();
			VRB(LOG.print("LOGICAL:").print(it));
			INT op=-1;
			it.var().getLogical(op);

			it.next();
			CHECK(!it.hasNext(), SYNTAX_ERROR);
			CHECK_CALL(pushAtomicValue(it));
			bool b = false;
			CHECK_ERR(stack.topVar().getBool(b), SYNTAX_ERROR, it);
			stack.pop();

			VRB(LOG.print(" ").print(a).print(" x ").print(b).endl());

			// push result

			     if (op == LOGICAL_AND) stack.pushBool(a && b);
			else if (op == LOGICAL_OR)  stack.pushBool(a || b);
			else if (op == LOGICAL_XOR) stack.pushBool(a != b);
			else return &SYNTAX_ERROR;
		}
		else if (it.isNextType(NODE_OPERATOR))
		{
			VRB(LOG.println("int [op] int operator")); // eg. "a + b"

			// get the first value

			CHECK_CALL(pushAtomicValue(it));
			INT a = stack.popInt();

			// read the operator

			it.next();
			CHAR op = '\0';
			it.var().getOp(op);

			// get the second value

			it.next();
			CHECK(!it.hasNext(), SYNTAX_ERROR);
			CHECK_CALL(pushAtomicValue(it));
			INT b = stack.popInt();

			VRB(LOG.print("a=").print(a).print(" ").print(op).print(" b=").print(b).endl());
			CHECK_CALL(operatorPush(op, a, b));
		}
		else
		{
			CHECK(false, SYNTAX_ERROR);
		}
	}

	// check that one argument has actually been pushed

	INT stackSizeAfter = stack.itemCount();
	CHECK(stackSizeBefore + 1 == stackSizeAfter, EMPTY_ARGUMENT_VALUE);
	return NO_ERROR;
}

ERROR_STATUS core::Engine::pushExprSubtreeArg(TreeIterator& _it)
{
	// push an argument that is wrapped in an expression

	TreeIterator it(_it);

	CHECK(it.isType(NODE_EXPR_TREE), SYNTAX_ERROR);
	CHECK(it.hasChild(), SYNTAX_ERROR);
	it.toChild(); // 'it' now points to first element of the expression, eg. "x" in "x + 1"
	CHECK_CALL(pushExprArg(it));

	return NO_ERROR;
}
ERROR_STATUS core::Engine::operatorPush(CHAR op, INT a, INT b)
{
	switch(op)
	{
	case '<': stack.pushBool(a <  b); return NO_ERROR;
	case '>': stack.pushBool(a >  b); return NO_ERROR;
	case '=': stack.pushBool(a == b); return NO_ERROR;

	case '+': stack.pushInt (a +  b); return NO_ERROR;
	case '-': stack.pushInt (a -  b); return NO_ERROR;
	case '*': stack.pushInt (a *  b); return NO_ERROR;
	case '/': CHECK(b!=0, DIV_ZERO); stack.pushInt(a / b); return NO_ERROR;
	}
	ERR.printCharSymbol(op);
	return &INVALID_OPERATOR;
}
ICallback * core::Engine::findCommand(INT * textData)
{
	// 'it' points to command name
	INT i;
	for (i=0; i<NUM_COMMANDS; i++)
	{
		if (matchTextData(textData, commands[i].name)) return &commands[i];
	}
	for (i=0; i<numCallbacks; i++)
	{
		if (matchTextData(textData, callbacks[i].name)) return &callbacks[i];
	}
	for (i=0; i<numProcedures; i++)
	{
		if (matchTextData(textData, procedures[i].name)) return &procedures[i];
	}
	return nullptr;
}
bool core::Engine::isReservedName(INT * textData)
{
	if (matchTextData(textData, andKeywordData)) return true;
	if (matchTextData(textData,  orKeywordData)) return true;
	if (matchTextData(textData, xorKeywordData)) return true;
	if (findCommand(textData) != nullptr) return true;
	if (findVariableIndex(textData, constants) >= 0) return true;
	if (findVariableIndex(textData, vars) >= 0) return true;
	return false;
}
