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

	Player player(
		(LPVOID)((uintptr_t)playerBase + Offsets::Player::ID),
		(LPVOID)((uintptr_t)playerBase + Offsets::Player::PosX),
		(LPVOID)((uintptr_t)playerBase + Offsets::Player::PosY),
		(LPVOID)((uintptr_t)playerBase + Offsets::Player::CursorX),
		(LPVOID)((uintptr_t)playerBase + Offsets::Player::CursorY),
		(LPVOID)((uintptr_t)playerBase + Offsets::Player::isVisible)
	);

	LPVOID lpTeeAmount = (LPVOID)((uintptr_t)playerBase + Offsets::Game::playerAmount);
	int32_t teeAmount = 0;
	ReadMemory(pid, lpTeeAmount, &teeAmount, sizeof(teeAmount));
	std::cout << "Tee Amount : " << teeAmount << std::endl;

	std::vector<Tee> tees;
	for (int i = 0; i < teeAmount; i++) {
		Tee tee(
			(LPVOID)((uintptr_t)player.posX + Offsets::NextTee * i),
			(LPVOID)((uintptr_t)player.posY + Offsets::NextTee * i),
			(LPVOID)((uintptr_t)player.isVisible + Offsets::NextTee * i)
		);
		tees.push_back(tee);
	}

	bool aim = false;
	clock_t rate = clock();

	int32_t vPlayerID;
	int32_t vPlayerX, vPlayerY;
	float vPlayerCursorX, vPlayerCursorY;

	clock_t refreshRate = clock();

	while (true) {
		if ((clock() - refreshRate) < 1) continue;
		else refreshRate = clock();

		ReadMemory(pid, player.ID, &vPlayerID, sizeof(vPlayerID));
		ReadMemory(pid, lpTeeAmount, &teeAmount, sizeof(teeAmount));
		if (teeAmount == 0) continue;
		if (vPlayerID < 0) continue;
		if (player.posX != (LPVOID)((uintptr_t)playerBase + Offsets::Player::PosX + Offsets::NextTee * vPlayerID)) {
			std::cout << "ID: " << vPlayerID << std::endl;
			player.posX = (LPVOID)((uintptr_t)playerBase + Offsets::Player::PosX + Offsets::NextTee * vPlayerID);
			player.posY = (LPVOID)((uintptr_t)playerBase + Offsets::Player::PosY + Offsets::NextTee * vPlayerID);
			player.isVisible = (LPVOID)((uintptr_t)playerBase + Offsets::Player::isVisible + Offsets::NextTee * vPlayerID);
		}

		if ((GetKeyState('O') & 0x8000 || GetKeyState(VK_XBUTTON1) & 0x8000) && (clock() - rate > 200)) {
			aim = !aim;
			rate = clock();
		}

		if (aim) {
			ReadMemory(pid, player.posX, &vPlayerX, sizeof(vPlayerX));
			ReadMemory(pid, player.posY, &vPlayerY, sizeof(vPlayerY));

			if (vPlayerX == 0 || vPlayerY == 0) {
				continue;
			}

			ReadMemory(pid, player.cursorX, &vPlayerCursorX, sizeof(vPlayerCursorX));
			ReadMemory(pid, player.cursorY, &vPlayerCursorY, sizeof(vPlayerCursorY));

			int32_t pCurX = vPlayerX + (int)vPlayerCursorX; // real time coordinates (what players have)
			int32_t pCurY = vPlayerY + (int)vPlayerCursorY;

			int32_t newX, newY;
			float x = 0, y = 0, distance = 0xFFFFFFFF;
			int32_t vIsVisible;
			bool check = false;
			
			for (int i = 0; i < teeAmount; i++) { // TODO: find out why posX and posY sometimes can be 0 instead of normal value
				if (i == vPlayerID) continue;

				Tee* tee = &tees[i];
				ReadMemory(pid, tee->isVisible, &vIsVisible, sizeof(vIsVisible));
				if (vIsVisible == 0) continue;

				ReadMemory(pid, tee->posX, &newX, sizeof(newX));
				ReadMemory(pid, tee->posY, &newY, sizeof(newY));

				if (newX == 0 || newY == 0) {
					//std::cout << "[POINTER] " << std::hex << tee->posX << " - " << tee->posY << std::dec << " | " << newX << " : " << newY << " | " << std::endl;
					check = true;
					break;
				}

				float tmp = sqrt((pCurX - newX) * (pCurX - newX) + (pCurY - newY) * (pCurY - newY));
				if (distance > tmp) {
					//std::cout << "[DISTANCE]" << tmp << " : " << newX << " - " << newY << " : " << pCurX << " - " << pCurY << " | " << std::endl;
					// TODO: make equations in float for better coordinates
					x = (float)(newX - vPlayerX); //+ (vPlayerCursorX - (int)vPlayerCursorX);
					y = (float)(newY - vPlayerY); //+ (vPlayerCursorY - (int)vPlayerCursorY);
					distance = tmp;
				}
			}

			if (check) continue;

			//std::cout << "[COORDS]" << x << " : " << y << " | " << newX << " : " << newY << " | " << vPlayerX << " : " << vPlayerY << " [AMOUNT] " << teeAmount << std::endl << std::endl;
			WriteMemory(pid, player.cursorX, &x, sizeof(x));
			WriteMemory(pid, player.cursorY, &y, sizeof(y));
		}
	}

	return 0;
}