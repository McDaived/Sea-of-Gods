#include "tslib.h"


int* ts::FloatToIntPointer(float& f)
{
	float* pointer = &f;
	int* intpointer = (int*)pointer;
	return intpointer;
}


uintptr_t ts::Aobs(PCHAR pattern, PCHAR mask, uintptr_t begin, SIZE_T size)
{
	SIZE_T patternSize = strlen((char*)mask);

	for (int i = 0; i < size; i++)
	{
		bool match = true;
		for (int j = 0; j < patternSize; j++)
		{
			if (*(char*)((uintptr_t)begin + i + j) != pattern[j] && mask[j] != '?')
			{
				match = false;
				break;
			}
		}
		if (match) return (begin + i);
	}
	return 0;
}


char* ts::AobsEx(HANDLE hProc, char* pattern, char* mask, char* begin, SIZE_T size)
{
	char* match{ nullptr };
	SIZE_T bytesRead;
	DWORD oldprotect;
	char* buffer{ nullptr };
	MEMORY_BASIC_INFORMATION mbi;
	mbi.RegionSize = 0x1000; 

	VirtualQueryEx(hProc, (LPCVOID)begin, &mbi, sizeof(mbi));

	for (char* curr = begin; curr < begin + size; curr += mbi.RegionSize)
	{
		if (!VirtualQueryEx(hProc, curr, &mbi, sizeof(mbi))) continue;

		if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

		delete[] buffer;
		buffer = new char[mbi.RegionSize];

		if (VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldprotect))
		{
			ReadProcessMemory(hProc, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead);
			VirtualProtectEx(hProc, mbi.BaseAddress, mbi.RegionSize, oldprotect, &oldprotect);

			char* internalAddr = (char*)Aobs(pattern, mask, (uintptr_t)buffer, (intptr_t)bytesRead);

			if (internalAddr != nullptr)
			{
				match = curr + (internalAddr - buffer); 
				break;
			}
		}
	}
	delete[] buffer;
	return match;
}


void ts::HP(PBYTE destination, PBYTE source, SIZE_T size, BYTE* oldBytes)
{
	DWORD oldProtection;
	VirtualProtect(destination, size, PAGE_EXECUTE_READWRITE, &oldProtection);
	memcpy(oldBytes, destination, size);
	memcpy(destination, source, size);
	VirtualProtect(destination, size, oldProtection, &oldProtection);
}


void ts::Nop(PBYTE destination, SIZE_T size, BYTE* oldBytes)
{
	PBYTE nop = new BYTE[size];
	memset(nop, 0x90, size);
	HP(destination, nop, size, oldBytes);
}


void ts::HPX(HANDLE hProcess, PBYTE destination, PBYTE source, SIZE_T size, BYTE* oldBytes)
{
	DWORD oldProtection;
	VirtualProtectEx(hProcess, destination, size, PAGE_READWRITE, &oldProtection);
	ReadProcessMemory(hProcess, destination, oldBytes, size, nullptr);
	WriteProcessMemory(hProcess, destination, source, size, nullptr);
	VirtualProtectEx(hProcess, destination, size, oldProtection, &oldProtection);
}


void ts::NopX(HANDLE hProcess, PBYTE destination, SIZE_T size, BYTE* oldBytes)
{
	PBYTE nop = new BYTE[size];
	memset(nop, 0x90, size);
	HPX(hProcess, destination, nop, size, oldBytes);
}

bool ts::Hook32(PBYTE hooked, PVOID hook32Template, SIZE_T bytes)
{
	DWORD oldProtection;
	VirtualProtect(hooked, bytes, PAGE_EXECUTE_READWRITE, &oldProtection);
	memset(hooked, 0x90, bytes);
	uintptr_t relativeAddress = ((uintptr_t)hook32Template - (uintptr_t)hooked) - 5;
	*hooked = 0xE8;
	*(DWORD*)(hooked + 1) = (DWORD)relativeAddress;
	VirtualProtect(hooked, bytes, oldProtection, &oldProtection);

	return true;
}

bool ts::Hook64(PBYTE hooked, PVOID shellcode, SIZE_T shellSize, SIZE_T bytes) 
{
	DWORD oldProtection;
	VirtualProtect(hooked, bytes, PAGE_EXECUTE_READWRITE, &oldProtection);

	PBYTE stolenBytes = new BYTE[bytes];
	ts::Nop(hooked, bytes, stolenBytes);


					 // push rdx mov rdx, 0000000000000000          jmp rdx  pop rdx
	char jumpArray[] = "\x52\x48\xba\x00\x00\x00\x00\x00\x00\x00\x00\xff\xe2\x5a";
	char returnArray[] = "\x48\xba\x00\x00\x00\x00\x00\x00\x00\x00\xff\xe2"; // = 12
	PVOID mAllocated = VirtualAlloc(0, shellSize + bytes + 12, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (mAllocated != 0)
	{
		memcpy(jumpArray + 3, &mAllocated, sizeof(mAllocated));
		uintptr_t ptrrtn = (uintptr_t)hooked + bytes;
		memcpy(returnArray + 2, (void*)&ptrrtn, sizeof(hooked));
	}
	else
	{
		return false;
	}


	memcpy(hooked, jumpArray, sizeof(jumpArray) - 1);
	memcpy(mAllocated, shellcode, shellSize);
	memcpy((PBYTE)mAllocated + shellSize, stolenBytes, bytes);
	memcpy((PBYTE)mAllocated + shellSize + bytes, returnArray, 12);


	VirtualProtect(hooked, bytes, oldProtection, &oldProtection);


	return true;
}

uintptr_t ts::ResolveAddr(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		addr = *(uintptr_t*)addr;
		addr += offsets[i];
	}
	return addr;
}

uintptr_t ts::ResolveAddrEx(HANDLE hProcess, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		ReadProcessMemory(hProcess, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

DWORD ts::GetPID(const LPCSTR pName)
{
	DWORD pid = 0;
	PROCESSENTRY32 pCurrent;
	pCurrent.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE) {
		if (Process32First(hSnap, &pCurrent))
		{
			do
			{
				if (!strcmp(pCurrent.szExeFile, pName))
				{
					pid = pCurrent.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &pCurrent));
		}
	}
	if (hSnap != 0)
		CloseHandle(hSnap);
	return pid;
}

PVOID ts::GetMBA(LPCSTR mName, DWORD& mSize)
{
	PVOID addr = 0;
	MODULEINFO hmInfo;
	HMODULE handle = GetModuleHandle(mName);
	if (!handle) return addr;
	GetModuleInformation(GetCurrentProcess(), handle, &hmInfo, sizeof(MODULEINFO));
	addr = (PVOID)hmInfo.lpBaseOfDll;
	if (mSize)
		mSize = hmInfo.SizeOfImage;
	return addr;
}

PVOID ts::GetMBAEx(LPCSTR mName, SIZE_T mSize)
{
	DWORD pid = ts::GetPID(mName);
	PVOID addr = 0;
	MODULEENTRY32 mCurrent;
	mCurrent.dwSize = sizeof(MODULEENTRY32);
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (hSnap != INVALID_HANDLE_VALUE) {
		if (Module32First(hSnap, &mCurrent))
		{
			do
			{
				if (!strcmp(mCurrent.szModule, mName))
				{
					addr = (PVOID)mCurrent.modBaseAddr;
					if (mSize)
						mSize = mCurrent.modBaseSize;
					break;
				}
			} while (Module32Next(hSnap, &mCurrent));
		}
	}
	if (hSnap != 0)
		CloseHandle(hSnap);
	return addr;
}
