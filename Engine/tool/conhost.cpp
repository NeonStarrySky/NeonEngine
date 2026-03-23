#include "conhost.h"

#include <Windows.h>

namespace neon::tool
{
	void tool::setColor(WORD color)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}
}