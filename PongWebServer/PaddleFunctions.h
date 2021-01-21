#pragma once
#include "inc.h"
bool isTouchingPaddle(paddle* p, ball* b) {
	double xDistance = abs(b->x - p->x);
	double yDistance = abs(b->y - p->y);
	if (xDistance < minDistanceX && yDistance < minDistanceY)
		return true;
	return false;
}