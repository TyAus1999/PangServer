#pragma once
#include "inc.h"
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