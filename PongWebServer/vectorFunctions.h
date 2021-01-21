#pragma once
#include "inc.h"

bool equalHDL(connection_hdl a, connection_hdl b) {
	return !a.owner_before(b) && !b.owner_before(a);
}

int getPlayerIndex(connection_hdl hdl, vector<player>*players) {
	for (int i = 0; i < players->size(); i++) {
		if(equalHDL(players->at(i).hdl,hdl))
			return i;
	}
	return -1;
}