#pragma once
#include "inc.h"

typedef unsigned long long u64;
using namespace std;

struct ball {
	u64 gameId;
	double x;
	double y;
	double xVel;
	double yVel;
	double width;
	double height;
	u64 timeOfLastMove;
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
	player p1;
	player p2;
	ball b;

	u64 player1Score;
	u64 player2Score;
};