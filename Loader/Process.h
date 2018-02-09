#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <Psapi.h>
#include <Shlwapi.h>
#include <TlHelp32.h>

#pragma comment(lib, "shlwapi.lib")

class Process {
public:

	Process(HANDLE proccessHandle, int pid);
	~Process();
	static std::shared_ptr<Process> FromName(std::string processName);
	static std::string CurrentPath();
	static bool IsRunning(std::string processName);
	void WriteRawMemory(byte * buffer, void * memoryPointer, int size);
	LPVOID AllocateMemory(unsigned int length, DWORD allocationType, DWORD memoryProtection);
	LPVOID AllocateAndWrite(byte* buffer, int bufferSize, DWORD allocationType, DWORD memoryProtection);

	int pid;
	HANDLE processHandle;
};
