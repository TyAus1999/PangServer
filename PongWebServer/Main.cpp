#pragma once
#include "inc.h"

using namespace std;

NoTLSServer s;

mutex playerGameMutex;
vector<player> players;
vector<game> games;

mutex sendToAllMutex;
vector<string> sendToAll;

mutex findGameMutex;
vector<u64> playerIdFindGameQueue;



void lockMutex(mutex* m) {
    while (!m->try_lock())
        this_thread::sleep_for(chrono::milliseconds(5));
}

//appends to the send all vector
void appendSendAll(string s) {
    lockMutex(&sendToAllMutex);
    sendToAll.push_back(s);
    sendToAllMutex.unlock();
}

void incomingClient(connection_hdl hdl) {
    cout << "Client Connected" << endl;
    lockMutex(&playerGameMutex);
    player p = initPlayer(hdl, &players);
    players.push_back(p);
    playerGameMutex.unlock();
}
void leavingClient(connection_hdl hdl) {
    cout << "Client disconected" << endl;
    lockMutex(&playerGameMutex);
    lockMutex(&findGameMutex);
    for (u64 i = 0; i < players.size(); i++) {
        if (equalHDL(players[i].hdl, hdl)) {
            //Check if player is in a game and then end the game
            //Fix when player leaves vector index error
            player* currentPlayer = &players[i];
            for (int pIndexTemp = 0; pIndexTemp < playerIdFindGameQueue.size(); pIndexTemp++) {
                if (currentPlayer->playerId == playerIdFindGameQueue[pIndexTemp]) {
                    playerIdFindGameQueue.erase(playerIdFindGameQueue.begin() + pIndexTemp);
                }
            }
            if (currentPlayer->currentGame > 0) {
                int gameIndex = getGameIndex(currentPlayer->currentGame, &games);
                if (gameIndex == -1) {
                    players.erase(players.begin() + i);
                    break;
                }
                game* currentGame = &games[gameIndex];
                int playerIndexs[2];
                getPlayerIndexsFromGame(currentGame, &players, playerIndexs);
                if (currentPlayer->playerId == currentGame->player2Id) {
                    s.sendData(players[playerIndexs[0]].hdl, "e");
                    players[playerIndexs[0]].currentGame = 0;
                }
                else {
                    s.sendData(players[playerIndexs[1]].hdl, "e");
                    players[playerIndexs[1]].currentGame = 0;
                }
                if (gameIndex != -1 && games.size() > 0)
                    games.erase(games.begin() + gameIndex);
            }
            players.erase(players.begin() + i);
            break;
        }
    }
    findGameMutex.unlock();
    playerGameMutex.unlock();
    
}
void clientMessage(connection_hdl hdl, server::message_ptr msg) {
    string temp = msg->get_payload();
    //cout << "Message from the client: " << temp << endl;
    lockMutex(&playerGameMutex);
    int playerIndex = getPlayerIndex(hdl, &players);
    //Might need to make a thread to deal with incoming instead
    //Start a thread when the information comes in and hand off the hdl and the msg pointer
    //TODO:
    //split up the command handling from single char commands to multi char commands
    if (temp.size() > 1) {
        if (temp[0] == 'p' && temp[1] != 'a') {//setting the paddle position
            //cout << temp << endl << endl;
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
            player* currentPlayer = &players[playerIndex];
            currentPlayer->p.x = stod(x);
            currentPlayer->p.y = stod(y);
            //start a thread to send to all the players in the current game this player is in
            if (currentPlayer->currentGame > 0) {
                int gameIndex = getGameIndex(currentPlayer->currentGame, &games);
                int playerIndexs[2];
                getPlayerIndexsFromGame(&games[gameIndex], &players, playerIndexs);
                player* recipient;
                if (currentPlayer->playerId == players[playerIndexs[0]].playerId)
                    recipient = &players[playerIndexs[1]];
                else
                    recipient = &players[playerIndexs[0]];
                string toOtherPlayer = "p";
                toOtherPlayer.append(to_string(currentPlayer->playerId));
                toOtherPlayer.append(",");
                toOtherPlayer.append(x);
                toOtherPlayer.append(",");
                toOtherPlayer.append(y);
                s.sendData(recipient->hdl, toOtherPlayer);
            }
        }
    }
    else if (temp == "f") {//find a game
        if (players[playerIndex].currentGame == 0) {
            printf("Player %llu wants to find a game\n", players[playerIndex].playerId);
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
    playerGameMutex.unlock();
}

bool isServerRunning = false;

void console() {
    string s;
    while (isServerRunning) {
        cin >> s;
        if (s == "stop")break;
        else if (s == "list") {
            lockMutex(&playerGameMutex);
            cout << "Total Players: " << players.size() << endl;
            for (int i = 0; i < players.size(); i++)
                cout << "\tPlayer " << players[i].playerId << endl;
            playerGameMutex.unlock();
        }
        else if (s == "games") {
            lockMutex(&playerGameMutex);
            cout << "Amount of games: " << games.size() << endl;
            playerGameMutex.unlock();
        }
        else {
            cout << "Command not recognized" << endl;
        }
    }
}

void gameLogic() {
    while (isServerRunning) {
        lockMutex(&playerGameMutex);
        u64 currentTime = getCurrentTimeMS();
        for (int i = 0; i < games.size(); i++) {
            ball* currentBall = &games[i].b;
            game* currentGame = &games[i];
            if (currentBall->timeOfArrival > currentTime) {
                //means the ball is where it needs to be
                currentBall->x = currentBall->destX;
                currentBall->y = currentBall->destY;
                printf("Prev Time of Arrival: %llu\n", currentBall->timeOfArrival);
                int pIndexs[2];
                getPlayerIndexsFromGame(currentGame, &players, pIndexs);
                player* p1 = &players[pIndexs[0]];
                player* p2 = &players[pIndexs[1]];
                paddle* p1Paddle = &p1->p;
                paddle* p2Paddle = &p2->p;
                //check for paddle hit first
                if (p1Paddle->x + minDistanceX == currentBall->x) {
                    //Hit left paddle
                    currentBall->xVel *= -1;
                    calculateNextHit(currentBall, currentTime);
                    string toSend = makeBallString(currentBall);
                    s.sendData(p1->hdl, toSend);
                    s.sendData(p2->hdl, toSend);
                }
                else if (p2Paddle->x - minDistanceX == currentBall->x) {
                    //Hit right paddle
                    currentBall->xVel *= -1;
                    calculateNextHit(currentBall, currentTime);
                    string toSend = makeBallString(currentBall);
                    s.sendData(p1->hdl, toSend);
                    s.sendData(p2->hdl, toSend);
                }
                //Checking for top/bottom collision
                else if (currentBall->y-ballDiameter==minY) {
                    //Hit floor
                    currentBall->yVel *= -1;
                    calculateNextHit(currentBall, currentTime);
                    string toSend = makeBallString(currentBall);
                    s.sendData(p1->hdl, toSend);
                    s.sendData(p2->hdl, toSend);
                }
                else if (currentBall->y + ballDiameter == maxY) {
                    //Hit top
                    currentBall->yVel *= -1;
                    calculateNextHit(currentBall, currentTime);
                    string toSend = makeBallString(currentBall);
                    s.sendData(p1->hdl, toSend);
                    s.sendData(p2->hdl, toSend);
                }
                //Need to add the check for the bounds
                else if (currentBall->x + ballDiameter == minX) {
                    //Hit left wall
                    currentBall->xVel *= -1;
                    currentBall->x = 0;
                    currentBall->y = 0;
                    s.sendData(p1->hdl, "w");
                    s.sendData(p2->hdl, "l");
                    calculateNextHit(currentBall, currentTime);
                    string toSend = makeBallString(currentBall);
                    s.sendData(p1->hdl, toSend);
                    s.sendData(p2->hdl, toSend);
                }
                else if (currentBall->x - ballDiameter == maxX) {
                    //hit right wall
                    currentBall->xVel *= -1;
                    currentBall->x = 0;
                    currentBall->y = 0;
                    s.sendData(p1->hdl, "l");
                    s.sendData(p2->hdl, "w");
                    calculateNextHit(currentBall, currentTime);
                    string toSend = makeBallString(currentBall);
                    s.sendData(p1->hdl, toSend);
                    s.sendData(p2->hdl, toSend);
                }
                printBall(currentBall);
                printf("Current Time of Arrival: %llu\n", currentBall->timeOfArrival);
                printf("Current Time: %llu\n", currentTime);
                printf("TOA-CT=%llu", currentBall->timeOfArrival - currentTime);
            }
            //if (currentTime - currentBall->timeOfLastMove > 16) {
            //    currentBall->timeOfLastMove = currentTime;
            //    currentBall->x += currentBall->xVel;
            //    currentBall->y += currentBall->yVel;
            //    u64 pIds[2];
            //    pIds[0] = currentGame->player1Id;
            //    pIds[1] = currentGame->player2Id;
            //    int playerIndexs[2];
            //    getPlayerIndexsFromGame(currentGame, &players, playerIndexs);
            //    if (playerIndexs[0] > -1 && playerIndexs[1] > -1) {
            //        u64 currentTime = getCurrentTimeMS();
            //        player* p1=&players[playerIndexs[0]];
            //        player* p2=&players[playerIndexs[1]];
            //        paddle* p1Paddle = &p1->p;
            //        paddle* p2Paddle = &p2->p;
            //        //check for each of the paddles
            //        //Paddle origin is centre
            //        bool touchingP1 = isTouchingPaddle(p1Paddle, currentBall);//Can be optimized
            //        bool touchingP2 = isTouchingPaddle(p2Paddle, currentBall);

            //        //check for the bounds
            //        if (currentBall->x < minX) {//Player 1 Looses round
            //            s.sendData(p1->hdl, "w");
            //            s.sendData(p2->hdl, "l");
            //            currentBall->y = 0;
            //            currentBall->x = 0;
            //            //cout << "Player 1 Loses" << endl;
            //        }
            //        else if (currentBall->x > maxX) {//Player 2 looses round
            //            s.sendData(p1->hdl, "l");
            //            s.sendData(p2->hdl, "w");
            //            currentBall->x = 0;
            //            currentBall->y = 0;
            //            //cout << "Player 2 Loses" << endl;
            //        }
            //        else if (currentBall->y < minY) {//Ball hit the ceil
            //            currentBall->y = -7.0;
            //            currentBall->yVel *= -1.0;
            //        }
            //        else if (currentBall->y > maxY) {//Ball hit floor
            //            currentBall->y = 7.0;
            //            currentBall->yVel *= -1.0;
            //        }
            //        else if (touchingP1 || touchingP2) {
            //            currentBall->xVel *= -1;
            //            currentBall->x += currentBall->xVel + (currentBall->xVel * 0.1);
            //        }
            //        string ballUpdate = "b";
            //        ballUpdate.append(to_string(currentBall->x));
            //        ballUpdate.append(",");
            //        ballUpdate.append(to_string(currentBall->y));
            //        s.sendData(p1->hdl, ballUpdate);
            //        s.sendData(p2->hdl, ballUpdate);
            //    }
            //}
        }
        playerGameMutex.unlock();

        //This should be it's own thread
        lockMutex(&findGameMutex);
        if (playerIdFindGameQueue.size() > 1) {
            for (int i = 0; i < playerIdFindGameQueue.size(); i += 2) {
                u64 playerIds[2];
                playerIds[0] = playerIdFindGameQueue[0];
                playerIds[1] = playerIdFindGameQueue[1];
                playerIdFindGameQueue.erase(playerIdFindGameQueue.begin(), playerIdFindGameQueue.begin() + 2);
                int pIDs[2];
                lockMutex(&playerGameMutex);
                int written = getIndexOfPlayerId(playerIds, 2, pIDs, &players);
                //cout << "Written: " << written << endl;
                u64 gameId = getUnusedGameId(&games);
                game toAdd = initGame(&players[pIDs[0]], &players[pIDs[1]], gameId);
                players[pIDs[0]].currentGame = gameId;
                players[pIDs[1]].currentGame = gameId;
                string toP1 = "g";
                toP1.append(to_string(gameId));
                toP1.append(",");
                toP1.append(to_string(0));
                toP1.append(",");
                toP1.append(to_string(players[pIDs[1]].playerId));
                string toP2 = "g";
                toP2.append(to_string(gameId));
                toP2.append(",");
                toP2.append(to_string(1));
                toP2.append(",");
                toP2.append(to_string(players[pIDs[0]].playerId));
                s.sendData(players[pIDs[0]].hdl, toP1);
                s.sendData(players[pIDs[1]].hdl, toP2);
                games.push_back(toAdd);

                string ballString = makeBallString(&games.back().b);
                s.sendData(players[pIDs[0]].hdl, ballString);
                s.sendData(players[pIDs[1]].hdl, ballString);
                playerGameMutex.unlock();
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
    printf("Server Is Started\n\n");
    console();
    s.stop();
    isServerRunning = false;
    incomingThread.join();
    gameLogicThread.join();
    return 0;
}