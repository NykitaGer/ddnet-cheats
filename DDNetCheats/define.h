#pragma once
#include <Windows.h>

struct Player { // TODO: make a constructor for this struct to automatically load all pointers
	LPVOID ID;
	LPVOID posX;
	LPVOID posY;
	LPVOID isVisible;
	LPVOID cursorX;
	LPVOID cursorY;

	Player() = default;
	Player(LPVOID X, LPVOID Y, LPVOID Visible) : posX(X), posY(Y), isVisible(Visible) {}
	Player(LPVOID ID, LPVOID X, LPVOID Y, LPVOID curX, LPVOID curY, LPVOID Visible)
		: ID(ID), posX(X), posY(Y), cursorX(curX), cursorY(curY), isVisible(Visible) {}
};

struct Tee : Player {
	Tee(LPVOID X, LPVOID Y, LPVOID Visible) : Player(X, Y, Visible) {}
	Tee(LPVOID ID, LPVOID X, LPVOID Y, LPVOID curX, LPVOID curY, LPVOID Visible)
		: Player(ID, X, Y, curX, curY, Visible) {}
};