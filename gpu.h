#pragma once
#include "includes.h"

class GPU {
public:
	bool setupGraphics();
	bool setupInput();
	void drawGraphics();
	void setKeys();
	u8 graphics[64 * 32];
};