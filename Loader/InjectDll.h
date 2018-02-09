#pragma once
#include <string>
#include "Process.h"

class InjectDll
{
public:
	static void Inject(std::string procName, std::string dllPath);
};