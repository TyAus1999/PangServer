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
	//0 is paddle x
	//1 is top/bottom
	u64 times[2];//intrensic optimization?
	times[0] = (b->xVel < 0) ? whenWillBallHitX(b, paddleXLeft) : whenWillBallHitX(b, paddleXRight);
	times[1] = (b->yVel < 0) ? whenWillBallHitY(b, minY) : whenWillBallHitY(b, maxY);

	if (times[0] < times[1]) {
		//Hit paddle before hit roof or bottom
		//d=vt
		b->destY = b->y + (b->yVel * (double)times[0]);
		b->destX = (b->xVel < 0) ? paddleXLeft : paddleXRight;
	}
	else {
		//Hit top/bottom before paddle
		b->destX = b->x + (b->xVel * (double)times[1]);
		b->destY = (b->yVel < 0) ? minY : maxY;
	}
	setTimeOfArrival(b);
}

void printBall(ball* b) {
	char space[] = "========================\0";
	printf("%s\n", space);
	printf("\tx: %llf\n\ty: %llf\n", b->x, b->y);
	printf("\txVel: %llf\n\tyVel: %llf\n", b->xVel, b->yVel);
	printf("\tTime Of Arrival: %llu\n", b->timeOfArrival);
	printf("\tdestX: %llf\n\tdestY: %llf\n", b->destX, b->destY);
	printf("%s\n", space);
}