#include "conhost.h"
#include <Windows.h>

namespace neon
{
	void setColor(WORD color)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}
}