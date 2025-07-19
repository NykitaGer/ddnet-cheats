#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include "offsets.h"
#include "define.h"
#include <vector>

uint32_t GetPID(const char* processName) {
	PROCESSENTRY32 entry{};
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!Process32First(snapshot, &entry)) {
		std::cerr << "Can't proceed process 32 first: " << GetLastError() << std::endl;
		return 1;
	}

	while (Process32Next(snapshot, &entry)) {
		if (_stricmp(entry.szExeFile, processName) == 0) {
			CloseHandle(snapshot);
			return entry.th32ProcessID;
		}
	}

	CloseHandle(snapshot);
	return 0;
}

uintptr_t GetModuleBase(DWORD pid, const char* processName) {
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (snapshot == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to find base address: " << GetLastError() << std::endl;
		return 1;
	}

	MODULEENTRY32 moduleEntry = { sizeof(moduleEntry) };
	if (!Module32First(snapshot, &moduleEntry)) {
		std::cerr << "Failed to find first module: " << GetLastError() << std::endl;
	}

	do {
		if (_stricmp(moduleEntry.szModule, processName) == 0) {
			CloseHandle(snapshot);
			return (uintptr_t)moduleEntry.modBaseAddr;
		}
	} while (Module32Next(snapshot, &moduleEntry));

	CloseHandle(snapshot);
	return 0;
}

bool WriteMemory(DWORD pid, LPVOID address, const void* newValue, size_t size) {
	HANDLE hProcess = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, false, pid);
	if (hProcess == NULL) {
		std::cerr << "Failed to open process: " << GetLastError() << std::endl;
		return false;
	}

	bool success = WriteProcessMemory(hProcess, address, newValue, size, nullptr);
	if (!success) {
		std::cerr << "Failed to write memory: " << GetLastError() << std::endl;
	}
	CloseHandle(hProcess);
	return success;
}

bool ReadMemory(DWORD pid, LPVOID address, void* buffer, size_t size) {
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION, false, pid);
	if (hProcess == NULL) {
		std::cerr << "Failed to open process: " << GetLastError() << std::endl;
		return false;
	}

	bool success = ReadProcessMemory(hProcess, address, buffer, size, nullptr);
	if (!success) {
		std::cerr << "Failed to read memory: " << GetLastError() << std::endl;
	}
	CloseHandle(hProcess);
	return success;
}

int main() {
	const char* processName = "DDNet.exe";
	int pid = GetPID(processName);
	uintptr_t moduleBase = GetModuleBase(pid, processName);
	std::cout << "[PID] " << processName << " : " << pid << std::endl;
	std::cout << "[MODULE] " << processName << " : " << std::hex << "0x" << moduleBase << std::dec << std::endl;

	LPVOID playerBase;
	ReadMemory(pid, (LPVOID)(moduleBase + Offsets::PlayerBase), &playerBase, sizeof(playerBase));
	std::cout << "[PLAYER BASE] " << std::hex << playerBase << std::dec << std::endl;

	std::vector<Tee> tees;
	Player player; 

	LPVOID playerX = (LPVOID)((uintptr_t)playerBase + Offsets::Player::PosX);
	LPVOID playerY = (LPVOID)((uintptr_t)playerBase + Offsets::Player::PosY);
	int32_t vPlayerX, vPlayerY;

	LPVOID playerCursorX = (LPVOID)((uintptr_t)playerBase + Offsets::Player::CursorX);
	LPVOID playerCursorY = (LPVOID)((uintptr_t)playerBase + Offsets::Player::CursorY);
	float vPlayerCursorX, vPlayerCursorY;

	LPVOID dummyX = (LPVOID)((uintptr_t)playerBase + Offsets::Dummy::PosX);
	LPVOID dummyY = (LPVOID)((uintptr_t)playerBase + Offsets::Dummy::PosY);
	int32_t vDummyX, vDummyY;

	bool aim = false;
	clock_t rate = clock();

	while (true) {
		if (GetKeyState('O') & 0x8000 && (clock() - rate > 200)) {
			aim = !aim;
			rate = clock();
		}

		if (aim) {
			ReadMemory(pid, playerX, &vPlayerX, sizeof(vPlayerX));
			ReadMemory(pid, playerY, &vPlayerY, sizeof(vPlayerY));

			ReadMemory(pid, dummyX, &vDummyX, sizeof(vDummyX));
			ReadMemory(pid, dummyY, &vDummyY, sizeof(vDummyY));

			float x, y;
			x = vDummyX - vPlayerX;
			y = vDummyY - vPlayerY;
			//std::cout << x << " : " << y << std::endl;
			WriteMemory(pid, playerCursorX, &x, sizeof(x));
			WriteMemory(pid, playerCursorY, &y, sizeof(y));
		}
	}

	return 0;
}