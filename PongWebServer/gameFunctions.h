#pragma once
#include "inc.h"

//Player and game mutex must be locked before calling
game initGame(player* p1, player* p2, u64 gameId) {
	game out;
	p1->p.x = -8.0;
	p1->p.y = 0.0;
	p2->p.x = 8.0;
	p2->p.y = 0.0;
	out.gameId = gameId;
	out.p1Paddle = &p1->p;
	out.p2Paddle = &p2->p;
	out.b = initBall();
	out.player1Score = 0;
	out.player2Score = 0;
	return out;
}

int getGameIndex(u64 gameIndex, vector<game>* games) {
	for (int i = 0; i < games->size(); i++) {
		if (games->at(i).gameId == gameIndex)
			return i;
	}
	return -1;
}

u64 getUnusedGameId(vector<game>*games) {
	u64 largestId = 0;
	for (int i = 0; i < games->size(); i++) {
		if (games->at(i).gameId > largestId)
			largestId = games->at(i).gameId;
	}
	largestId++;
	return largestId;
}