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
	out.player1Id = p1->playerId;
	out.player2Id = p2->playerId;
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

//indexs is a pointer to an int array of size 2
//indexs[0] is player 1, indexs[1] is player 2
void getPlayerIndexsFromGame(game* g, vector<player>* players, int* indexs) {
	indexs[0] = -1;
	indexs[1] = -1;
	for (int i = 0; i < players->size(); i++) {
		u64 workingPlayerId = players->at(i).playerId;
		if (g->player1Id == workingPlayerId) {
			indexs[0] = i;
		}
		else if (g->player2Id == workingPlayerId) {
			indexs[1] = i;
		}
		else if (indexs[0] > -1 && indexs[1] > -1)
			break;
	}
}