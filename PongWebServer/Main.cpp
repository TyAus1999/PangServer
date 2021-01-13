#pragma once
#include "inc.h"

using namespace std;

NoTLSServer s;

mutex playerMutex;
vector<player> players;

mutex gameMutex;
vector<game> games;

mutex sendToAllMutex;
vector<string> sendToAll;

mutex findGameMutex;
vector<u64> playerIdFindGameQueue;

u64 getCurrentTimeMS() {
    u64 out = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    return out;
}

void lockMutex(mutex *m) {
    while (!m->try_lock())
        this_thread::sleep_for(chrono::milliseconds(5));
}

bool equalHDL(connection_hdl a, connection_hdl b) {
    return !a.owner_before(b) && !b.owner_before(a);
}

//Locks mutex but does not unlock the mutex
int getPlayerIndex(connection_hdl hdl) {
    lockMutex(&playerMutex);
    //Need to make this a multithreaded search
    for (int i = 0; i < players.size(); i++) {
        if (equalHDL(players[i].hdl, hdl))
            return i;
    }
    return -1;
}

//Does not lock
u64 getUnusedGameID() {
    u64 largestId = 0;
    for (int i = 0; i < games.size(); i++) {
        if (games[i].gameId > largestId) 
            largestId = games[i].gameId;
    }
    largestId++;
    return largestId;
}

game initGame(player p1, player p2, u64 gameId) {
    game out;
    ball outBall;
    outBall.x = 0;
    outBall.y = 0;
    outBall.xVel = ((double)((rand() % 3) + 1)) / 10.0;
    outBall.yVel = ((double)((rand() % 3) + 1)) / 10.0;
    if (outBall.xVel == outBall.yVel) {
        int r = rand() % 2;
        if (r) {
            if (outBall.xVel - 0.1 == 0.0)outBall.xVel -= 0.2;
            else outBall.xVel -= 0.1;
        }
        else {
            if (outBall.yVel - 0.1 == 0.0)outBall.yVel -= 0.2;
            else outBall.yVel -= 0.1;
        }
    }
    outBall.width = ballDiameter;
    outBall.height = ballDiameter;
    outBall.timeOfLastMove = getCurrentTimeMS();

    p1.p.x = -8.0;
    p1.p.y = .0;

    p2.p.x = 8.0;
    p2.p.y = 0.0;

    out.gameId = gameId;
    out.p1 = p1;
    out.p2 = p2;
    out.b = outBall;
    out.player1Score = 0;
    out.player2Score = 0;

    return out;
}

//Does not lock
int getGameIndex(u64 gameId) {
    for (int i = 0; i < games.size(); i++) {
        if (games[i].gameId == gameId)
            return i;
    }
    return -1;
}

//appends to the send all vector
void appendSendAll(string s) {
    lockMutex(&sendToAllMutex);
    sendToAll.push_back(s);
    sendToAllMutex.unlock();
}

//Not locked, must lock players vector before using this
int getIndexOfPlayerId(u64 *playerId, u64 amount, int *indexs) {
    int currIndex = 0;
    for (int i = 0; i < players.size(); i++) {
        for (u64 p = 0; p < amount; p++) {
            if (players[i].playerId == playerId[p]) {
                indexs[currIndex] = i;
                currIndex++;
            }
        }
    }
    return currIndex;
}

void incomingClient(connection_hdl hdl) {
    cout << "Client Connected" << endl;
    player p;
    memset(&p, 0, sizeof(player));
    p.hdl = hdl;
    lockMutex(&playerMutex);
    u64 largestIndex = 0;
    for (u64 i = 0; i < players.size(); i++) {
        if (players[i].playerId > largestIndex) {
            largestIndex = players[i].playerId;
        }
    }
    if (players.size() > 0)
        largestIndex++;
    p.playerId = largestIndex;
    players.push_back(p);
    playerMutex.unlock();
}
void leavingClient(connection_hdl hdl) {
    cout << "Client disconected" << endl;
    lockMutex(&playerMutex);
    for (u64 i = 0; i < players.size(); i++) {
        if (equalHDL(players[i].hdl, hdl)) {
            //Check if player is in a game and then end the game
            //Fix when player leaves vector index error
            player* currentPlayer = &players[i];
            if (currentPlayer->currentGame > 0) {
                lockMutex(&gameMutex);
                int gameIndex = getGameIndex(currentPlayer->currentGame);
                if (gameIndex == -1) {
                    players.erase(players.begin() + i);
                    gameMutex.unlock();
                    break;
                }
                game* currentGame = &games[gameIndex];
                if (currentPlayer->playerId == currentGame->p2.playerId) {
                    s.sendData(currentGame->p1.hdl, "e");
                    currentGame->p1.currentGame = 0;
                }
                else {
                    s.sendData(currentGame->p2.hdl, "e");
                    currentGame->p2.currentGame = 0;
                }
                if(gameIndex!=-1 && games.size()>0)
                    games.erase(games.begin() + gameIndex);
                gameMutex.unlock();
            }
            players.erase(players.begin() + i);
            break;
        }
    }
    playerMutex.unlock();
}
void clientMessage(connection_hdl hdl, server::message_ptr msg) {
    string temp = msg->get_payload();
    cout << "Message from the client: " << temp << endl;
    int playerIndex = getPlayerIndex(hdl);
    //Might need to make a thread to deal with incoming instead
    //Start a thread when the information comes in and hand off the hdl and the msg pointer
    //TODO:
    //split up the command handling from single char commands to multi char commands
    if (temp.size() > 1) {
        if (temp[0] == 'p' && temp[1] != 'a') {//setting the paddle position
            string x, y;
            int commaIndex = 0;
            for (int i = 1; i < temp.size(); i++) {
                if (temp[i] == ',') {
                    commaIndex = i;
                    break;
                }
            }
            x = temp.substr(1, commaIndex - 1);
            y = temp.substr(commaIndex + 1, temp.size());
            players[playerIndex].p.x = stod(x);
            players[playerIndex].p.y = stod(y);
            //start a thread to send to all the players in the current game this player is in
            string toOtherPlayers = "p";
            toOtherPlayers.append(to_string(players[playerIndex].playerId));
            toOtherPlayers.append(",");
            toOtherPlayers.append(x);
            toOtherPlayers.append(",");
            toOtherPlayers.append(y);
            for (int i = 0; i < players.size(); i++) {
                if (i != playerIndex)
                    s.sendData(players[i].hdl, toOtherPlayers);
            }
            //cout << toOtherPlayers << "\n\tWas not sent to player "
                //<< to_string(players[playerIndex].playerId) << endl;
        }
    }
    else if (temp == "f") {//find a game
        if (players[playerIndex].currentGame == 0) {
            lockMutex(&findGameMutex);
            playerIdFindGameQueue.push_back(players[playerIndex].playerId);
            findGameMutex.unlock();
        }
    }
    else if (temp == "c") {//send the client information
        //get the client vector index
        //gather the information
        //send the data to the client
        string toClient = "c";
        if (playerIndex != -1) {
            toClient.append(to_string(players[playerIndex].playerId));
            toClient.append(",");
            toClient.append(to_string(players[playerIndex].p.paddleId));
        }
        cout << "Sending to Client: " << toClient << endl;
        s.sendData(hdl, toClient);
    }
    
    playerMutex.unlock();
}

bool isServerRunning = false;

void console() {
    string s;
    while (isServerRunning) {
        cin >> s;
        if (s == "stop")break;
        else if (s == "list") {
            lockMutex(&playerMutex);
            cout << "Total Players: " << players.size() <<endl;
            for (int i = 0; i < players.size(); i++)
                cout << "\tPlayer " << players[i].playerId << endl;
            playerMutex.unlock();
        }
        else if (s == "games") {
            lockMutex(&gameMutex);
            cout << "Amount of games: " << games.size() << endl;
            gameMutex.unlock();
        }
        else {
            cout << "Command not recognized" << endl;
        }
    }
}

bool isTouchingPaddle(paddle* p, ball* b) {
    double minDistanceX = hPaddleWidth + ballRadius;
    double minDistanceY = hPaddleHeight + ballRadius;
    double minusX = abs(p->x - b->x);
    double minusY = abs(p->y - b->y);
    if (minusX < minDistanceX) {
        //we have hit it in the x component

        cout << "MinusY: " << minusY <<"\tMinDistanceY: " << minDistanceY << "\t" << p->y << "\t" << b->y << endl;
        return true;
    }
    return false;
}

void gameLogic() {
    while (isServerRunning) {
        lockMutex(&gameMutex);
        u64 currentTime = getCurrentTimeMS();
        for (int i = 0; i < games.size(); i++) {
            ball* currentBall=&games[i].b;
            game* currentGame = &games[i];
            if (currentTime - currentBall->timeOfLastMove > 16) {
                currentBall->timeOfLastMove = currentTime;
                currentBall->x += currentBall->xVel;
                currentBall->y += currentBall->yVel;

                //check for each of the paddles
                //Paddle origin is centre
                bool touchingP1 = isTouchingPaddle(&currentGame->p1.p, currentBall);
                bool touchingP2 = isTouchingPaddle(&currentGame->p2.p, currentBall);

                //check for the bounds
                if (!touchingP1 && !touchingP2) {
                    int xTrip = -1;
                    if (currentBall->x < -16) {//Player 1 Looses round
                        s.sendData(currentGame->p1.hdl, "w");
                        s.sendData(currentGame->p2.hdl, "l");
                        currentBall->y = 0;
                        currentBall->x = 0;
                    }
                    else if (currentBall->x > 16) {//Player 2 looses round
                        s.sendData(currentGame->p1.hdl, "l");
                        s.sendData(currentGame->p2.hdl, "w");
                        currentBall->x = 0;
                        currentBall->y = 0;

                    }

                    if (currentBall->y < -7) {//Ball hit the ceil
                        currentBall->y = -7.0;
                        currentBall->yVel *= -1.0;
                    }
                    else if (currentBall->y > 7) {//Ball hit floor
                        currentBall->y = 7.0;
                        currentBall->yVel *= -1.0;
                    }
                }
                else if(touchingP1){
                    currentBall->xVel *= -1;

                }
                else if (touchingP2) {
                    currentBall->xVel *= -1;
                }

                string ballUpdate = "b";
                ballUpdate.append(to_string(currentBall->x));
                ballUpdate.append(",");
                ballUpdate.append(to_string(currentBall->y));
                s.sendData(currentGame->p1.hdl, ballUpdate);
                s.sendData(currentGame->p2.hdl, ballUpdate);
            }
        }
        gameMutex.unlock();

        //This should be it's own thread
        lockMutex(&findGameMutex);
        if (playerIdFindGameQueue.size() > 1) {
            for (int i = 0; i < playerIdFindGameQueue.size(); i+=2) {
                u64 playerIds[2];
                playerIds[0] = playerIdFindGameQueue[0];
                playerIds[1] = playerIdFindGameQueue[1];
                playerIdFindGameQueue.erase(playerIdFindGameQueue.begin(), playerIdFindGameQueue.begin() + 2);
                int pIDs[2];
                lockMutex(&playerMutex);
                lockMutex(&gameMutex);
                int written = getIndexOfPlayerId(playerIds, 2, pIDs);
                //cout << "Written: " << written << endl;
                u64 gameId = getUnusedGameID();
                game toAdd = initGame(players[pIDs[0]], players[pIDs[1]], gameId);
                players[pIDs[0]].currentGame = gameId;
                players[pIDs[1]].currentGame = gameId;
                string toP1="g";
                toP1.append(to_string(gameId));
                toP1.append(",");
                toP1.append(to_string(0));
                toP1.append(",");
                toP1.append(to_string(players[pIDs[1]].playerId));
                string toP2="g";
                toP2.append(to_string(gameId));
                toP2.append(",");
                toP2.append(to_string(1));
                toP2.append(",");
                toP2.append(to_string(players[pIDs[0]].playerId));
                s.sendData(players[pIDs[0]].hdl, toP1);
                s.sendData(players[pIDs[1]].hdl, toP2);
                games.push_back(toAdd);
                gameMutex.unlock();
                playerMutex.unlock();
            }
        }
        findGameMutex.unlock();
    }
}

int main() {
    srand(time(NULL));
    s.setFunctions(incomingClient, leavingClient, clientMessage);
    isServerRunning = true;
    thread incomingThread([]() {s.run(1666); });
    thread gameLogicThread(gameLogic);
    //s.run(80);
    printf("Server Is Started\n\n");
    console();
    s.stop();
    isServerRunning = false;
    incomingThread.join();
    gameLogicThread.join();
	return 0;
}