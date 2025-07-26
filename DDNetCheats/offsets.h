#pragma once
#include <Windows.h>

namespace Offsets {
	const DWORD PlayerBase = 0x4F2908;
	const DWORD FirstTee = 0x582820;
	const DWORD Coords = 0x60;
	const DWORD NextTee = 0xF8;

	namespace Player {
		const DWORD ID = 0x5827E0;
		const DWORD PosX = FirstTee + 0x60; // + Coords
		const DWORD PosY = PosX + 0x4;
		const DWORD isVisible = PosX + 0x8;
		const DWORD CursorX = 0x51FC18;
		const DWORD CursorY = CursorX + 0x4;
	};

	namespace Game {
		const DWORD playerAmount = 0x5827E4;
	}
};