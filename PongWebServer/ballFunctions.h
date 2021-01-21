#pragma once
#include "inc.h"

ball initBall() {
	ball b;
	b.x = 0;
	b.y = 0;
	for (int i = 0; i < 200; i++) {
		b.xVel = (double)((rand() % 3) + 1) / 10.0;
		b.yVel = (double)((rand() % 3) + 1) / 10.0;
		if (b.xVel != b.yVel)
			break;
	}
	b.width = ballDiameter;
	b.height = ballDiameter;
	b.timeOfLastMove = getCurrentTimeMS();

	return b;
}