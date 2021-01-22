#pragma once
#include "inc.h"

ball initBall() {
	ball b;
	memset(&b, 0, sizeof(ball));
	b.x = 0;
	b.y = 0;
	for (int i = 0; i < 200; i++) {
		b.xVel = (double)((rand() % 3) + 1) / 10.0;
		b.yVel = (double)((rand() % 3) + 1) / 10.0;
		if (b.xVel != b.yVel)
			break;
	}
	b.timeOfLastMove = getCurrentTimeMS();

	return b;
}

double pythag(double x, double y) {
	double out = x * x;
	out += y * y;
	out = sqrt(out);
	return out;
}

void setTimeOfArrival(ball* b) {
	//d=vt
	double actualVelocity=pythag(b->xVel,b->yVel);
	double distanceX = b->destX - b->x;
	double distanceY = b->destY - b->y;
	double actualDistance = pythag(distanceX, distanceY);
	//t=d/v
	double time = actualDistance / actualVelocity;
	b->timeOfArrival = (u64)time;
}

u64 whenWillBallHitX(ball* b, double x) {
	double xDistance = b->x - x;
	double time = xDistance / b->xVel;
	return (u64)time;
}

u64 whenWillBallHitY(ball* b, double y) {
	double yDistance = b->y - y;
	double time = yDistance / b->yVel;
	return (u64)time;
}

void calculateNextHit(ball* b) {
	/*u64 amountOfTime[2];
	amountOfTime[0] = (b->xVel < 0) ? whenWillBallHitX(b, minX) : whenWillBallHitX(b, maxX);
	amountOfTime[1] = (b->yVel < 0) ? whenWillBallHitY(b, minY) : whenWillBallHitY(b, maxY);
	if (amountOfTime[0] < amountOfTime[1]) {
		X is closer than Y

	}
	else {
		Y is closer than X
	}*/
	
}