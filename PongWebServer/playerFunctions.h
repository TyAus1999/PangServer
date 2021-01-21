#pragma once
#include "inc.h"

u64 getUnusedPlayerId(vector<player>* players) {
	u64 largest = 0;
	for (int i = 0; i < players->size(); i++) {
		if (players->at(i).playerId > largest)
			largest = players->at(i).playerId;
	}
	largest++;
	return largest;
}

player initPlayer(connection_hdl hdl, vector<player>* players) {
	player out;
	memset(&out, 0, sizeof(player));
	out.playerId = getUnusedPlayerId(players);
	out.hdl = hdl;
	return out;
}

//Not locked, must lock player vector before calling function
//Returns the size of the return indexs
int getIndexOfPlayerId(u64* playerIds, u64 amountOfPlayerIds, int* returnIndexs, vector<player>*players) {
	int currIndex = 0;
	for (int i = 0; i < players->size(); i++) {
		for (u64 p = 0; p < amountOfPlayerIds; p++) {
			if (players->at(i).playerId == playerIds[p]) {
				returnIndexs[currIndex] = i;
				currIndex++;
			}
		}
	}
	return currIndex;
}