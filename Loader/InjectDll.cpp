#include "InjectDll.h"

void InjectDll::Inject(std::string procName, std::string dllPath)
{
	dllPath = Process::CurrentPath() + "\\" + dllPath;
	auto proc = Process::FromName(procName);
	auto handle = OpenProcess(PROCESS_ALL_ACCESS, 1, proc->pid);

	auto loadLibraryAdd = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	auto lpAddress = proc->AllocateAndWrite((byte*)dllPath.data(), dllPath.length() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	auto t = CreateRemoteThread(handle, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibraryAdd, lpAddress, 0, 0);

	VirtualFreeEx(handle, lpAddress, dllPath.length() + 1, MEM_RELEASE);
	CloseHandle(handle);
}
