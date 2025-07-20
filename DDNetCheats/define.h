#pragma once
#include <Windows.h>

struct Player { // TODO: make a constructor for this struct to automatically load all pointers
	LPVOID posX;
	LPVOID posY;
	LPVOID cursorX;
	LPVOID cursorY;

	Player() = default;
	Player(LPVOID X, LPVOID Y) : posX(X), posY(Y) {}
	Player(LPVOID X, LPVOID Y, LPVOID curX, LPVOID curY)
		: posX(X), posY(Y), cursorX(curX), cursorY(curY) {}
};

struct Tee : Player {
	Tee(LPVOID X, LPVOID Y) : Player(X, Y) {}
	Tee(LPVOID X, LPVOID Y, LPVOID curX, LPVOID curY)
		: Player(X, Y, curX, curY) {}
};