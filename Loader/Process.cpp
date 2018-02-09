#include "Process.h"

Process::Process(HANDLE proccessHandle, int pid)
{
	this->processHandle = proccessHandle;
	this->pid = pid;
}

Process::~Process()
{
	CloseHandle(processHandle);
}

std::string Process::CurrentPath()
{
	CHAR path[MAX_PATH];
	GetModuleFileNameA(GetModuleHandleW(NULL), path, MAX_PATH);
	PathRemoveFileSpecA(path);
	return std::string(path);
}

bool Process::IsRunning(std::string processName)
{
	auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	auto pe = PROCESSENTRY32{ sizeof(PROCESSENTRY32) };

	if (Process32First(snapshot, &pe))
	{
		do
		{
			if (!strcmp(processName.c_str(), pe.szExeFile))
			{
				return true;
			}
		} while (Process32Next(snapshot, &pe));
	}

	return false;
}

void Process::WriteRawMemory(byte* buffer, void* memoryPointer, int size)
{
	SIZE_T numberOfByesWritten = 0;
	bool worked = WriteProcessMemory(processHandle, memoryPointer, buffer, size, &numberOfByesWritten);

	if (!worked)
		printf("wpm failed ");
}

LPVOID Process::AllocateMemory(unsigned int length, DWORD allocationType, DWORD memoryProtection)
{
	return VirtualAllocEx(processHandle, NULL, length, allocationType, memoryProtection);
}

LPVOID Process::AllocateAndWrite(byte * buffer, int bufferSize, DWORD allocationType, DWORD memoryProtection)
{
	LPVOID allocatedMemory = AllocateMemory(bufferSize, allocationType, memoryProtection);
	WriteRawMemory(buffer, allocatedMemory, bufferSize);

	return allocatedMemory;
}

std::shared_ptr<Process> Process::FromName(std::string procName) {
	auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	auto pe = PROCESSENTRY32{ sizeof(PROCESSENTRY32) };

	if (Process32First(snapshot, &pe)) {
		do {
			if (!strcmp(procName.c_str(), pe.szExeFile)) {
				CloseHandle(snapshot);
				auto processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
				return std::make_shared<Process>(processHandle, pe.th32ProcessID);
			}
		} while (Process32Next(snapshot, &pe));
	}
	CloseHandle(snapshot);

	throw std::runtime_error("Couldn't find process");
}