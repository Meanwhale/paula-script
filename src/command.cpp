#include"command.h"
#include"utils.h"
#include"array.h"

paula::Command::Command(const char* str)
{
//	void bytesToInts(const unsigned char * bytes, int bytesOffset, Array<INT>& ints, int intsOffset, int bytesLength);

	// save name data to an int array, as it is in trees

	INT length = strlen(str);
	Array<INT> nameData (name, 12);
	nameData[0] = length;
	bytesToInts((const unsigned char *)str, 0, nameData, 1, length);
}

void paula::Command::execute(Paula& paula, Tree& tree)
{
	LOGLINE("Command: EXECUTE!");
}

