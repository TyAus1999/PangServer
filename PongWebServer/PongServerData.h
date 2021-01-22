#pragma once
#include "inc.h"

typedef unsigned long long u64;

struct ball {
	double x;
	double y;
	double xVel;
	double yVel;
	u64 timeOfLastMove;


	u64 timeOfArrival;
	double destX;
	double destY;
};

struct paddle {
	u64 paddleId;
	double x;
	double y;
	double width;
	double height;
};

struct player {
	u64 playerId;
	u64 currentGame;//game 0 is no Game
	connection_hdl hdl;
	paddle p;
};

struct game {
	u64 gameId;
	u64 player1Id;
	u64 player2Id;
	ball b;

	u64 player1Score;
	u64 player2Score;
};