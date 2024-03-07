#pragma once

#include <windows.h>
#include <vector>
#include <iostream>
#include <TlHelp32.h>
#include <Psapi.h>
#include <stdio.h>
#include <tchar.h>


namespace ts
{
	int* FloatToIntPointer(float& f);
	char* AobsEx(HANDLE hProc, char* pattern, char* mask, char* begin, uintptr_t size);
	uintptr_t Aobs(PCHAR pattern, PCHAR mask, uintptr_t begin, SIZE_T size);
	void HP(PBYTE destination, PBYTE source, SIZE_T size, BYTE* oldBytes);
	void Nop(PBYTE destination, SIZE_T size, BYTE* oldBytes);
	void HPX(HANDLE hProcess, PBYTE destination, PBYTE source, SIZE_T size, BYTE* oldBytes);
	void NopX(HANDLE hProcess, PBYTE destination, SIZE_T size, BYTE* oldBytes);

	bool Hook32(PBYTE hooked, PVOID hook32Template, SIZE_T bytes);
	bool Hook64(PBYTE hooked, PVOID shellcode, SIZE_T shellSize, SIZE_T bytes);

	uintptr_t ResolveAddr(uintptr_t ptr, std::vector<unsigned int> offsets);
	uintptr_t ResolveAddrEx(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);

	DWORD GetPID(const LPCSTR pName);
	PVOID GetMBA(const  LPCSTR mName, DWORD& mSize);
	PVOID GetMBAEx(LPCSTR mName, DWORD_PTR mSize);
}