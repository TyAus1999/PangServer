#pragma once
#include "inc.h"
void calculateNextHit(ball* b, u64 currentTime);
ball initBall() {
	ball b;
	memset(&b, 0, sizeof(ball));
	b.x = 0;
	b.y = 0;
	for (int i = 0; i < 200; i++) {
		b.xVel = (double)((rand() % 3) + 1) / 2.0;
		b.yVel = (double)((rand() % 3) + 1) / 2.0;
		if (b.xVel != b.yVel)
			break;
	}
	b.timeOfLastMove = getCurrentTimeMS();
	calculateNextHit(&b, getCurrentTimeMS());
	return b;
}

double pythag(double x, double y) {
	double out = x * x;
	out += y * y;
	out = sqrt(out);
	return out;
}

void printBall(ball* b) {
	char space[] = "========================\0";
	printf("%s\n", space);
	printf("\tx: %llf\n\ty: %llf\n", b->x, b->y);
	printf("\txVel: %llf\n\tyVel: %llf\n", b->xVel, b->yVel);
	double changeX = b->destX - b->x;
	double changeY = b->destY - b->y;
	printf("\tDistance: %llf\n\tVelocity in units per s: %llf\n", pythag(changeX, changeY), pythag(b->xVel, b->yVel));
	printf("\tTime Of Arrival: %llu\n", b->timeOfArrival);
	printf("\tdestX: %llf\n\tdestY: %llf\n", b->destX, b->destY);
	printf("%s\n", space);
}

void setTimeOfArrival(ball* b, u64 currentTime) {
	//d=vt
	double actualVelocity=pythag(b->xVel,b->yVel);
	double distanceX = b->destX - b->x;
	double distanceY = b->destY - b->y;
	double actualDistance = pythag(distanceX, distanceY);
	//t=d/v
	double time = actualDistance / actualVelocity;
	printf("Time: %llf\t", time);
	time *= 1000.0;
	printf("Time: %llf\tCurrent Time: %llu\n", time,currentTime);
	b->timeOfArrival = (u64)time + currentTime;
	printBall(b);
}

double whenWillBallHitX(ball* b, double x) {
	double xDistance = b->x - x;
	double time = xDistance / b->xVel;
	if (time < 0)time *= -1;
	return time;
}

double whenWillBallHitY(ball* b, double y) {
	double yDistance = b->y - y;
	double time = yDistance / b->yVel;
	if (time < 0)time *= -1;
	return time;
}

void calculateNextHit(ball* b, u64 currentTime) {
	//0 is paddle x
	//1 is top/bottom
	double times[3];//intrensic optimization?
	//Need to make this some sort of tree, would speed up comparison for more than 3 params
	times[0] = (b->xVel < 0) ? whenWillBallHitX(b, paddleXLeft+minDistanceX) : whenWillBallHitX(b, paddleXRight-minDistanceX);//Paddles
	times[1] = (b->yVel < 0) ? whenWillBallHitY(b, minY+ballDiameter) : whenWillBallHitY(b, maxY-ballDiameter);//Top down
	times[2] = (b->xVel < 0) ? whenWillBallHitX(b, minX) : whenWillBallHitX(b, maxX);//Left right

	for (int i = 0; i < 3; i++) 
		printf("\tTimes[%i]: %llf\n", i, times[i]);
	if (times[0] < times[1] && times[0] < times[2]) {
		//Hit paddle before hit roof or bottom
		//d=vt
		b->destY = b->y + (b->yVel * times[0]);
		b->destX = (b->xVel < 0) ? paddleXLeft + minDistanceX : paddleXRight - minDistanceX;
		//printf("Going to hit paddle\n");
	}
	else if (times[1] < times[0] && times[1] < times[2]) {
		//Hit top/bottom before paddle
		b->destX = b->x + (b->xVel * times[1]);
		b->destY = (b->yVel < 0) ? minY+ballDiameter : maxY-ballDiameter;
		//printf("Going to hit top/bottom\n");
	}
	else if (times[2] < times[0] && times[2] < times[1]) {
		//Hit left right bounds
		b->destX = (b->xVel < 0) ? minX : maxX;
		b->destY = b->y + (b->yVel * times[2]);
	}
	setTimeOfArrival(b, currentTime);
}

string makeBallString(ball* b) {
	string out = "z";
	out.append(to_string(b->x));
	out.append(",");
	out.append(to_string(b->y));
	out.append(",");
	out.append(to_string(b->xVel));
	out.append(",");
	out.append(to_string(b->yVel));
	out.append(",");
	out.append(to_string(b->timeOfArrival));
	return out;
}