#pragma once
#include <Windows.h>

namespace Offsets {
	const DWORD PlayerBase = 0x4F2908;
	namespace Player {
		const DWORD PosX = 0x582880;
		const DWORD PosY = PosX + 0x4;
		const DWORD CursorX = 0x51FC18;
		const DWORD CursorY = CursorX + 0x4;
	};

	namespace Dummy {
		const DWORD PosX = 0x582978;
		const DWORD PosY = PosX + 0x4;
	}
};