#pragma once
#include <Windows.h>

struct Player { // TODO: make a constructor for this struct to automatically load all pointers
	LPVOID posX;
	LPVOID poxY;
	LPVOID cursorX;
	LPVOID cursorY;
};

struct Tee : Player {

};