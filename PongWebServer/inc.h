#pragma once
#define ballDiameter 1.0
#define paddleWidth 1.0
#define paddleHeight 6.0

#define ballRadius ballDiameter/2.0
#define hPaddleWidth paddleWidth/2.0
#define hPaddleHeight paddleHeight/2.0
#define minDistanceX hPaddleWidth+ballDiameter
#define minDistanceY hPaddleHeight+ballDiameter
#include <iostream>
#include <thread>
#include <string>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <chrono>
#include <mutex>
typedef unsigned long long u64;
u64 getCurrentTimeMS() {
    u64 out = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return out;
}

#include "WebsocketStuff.h"
#include "PongServerData.h"
#include "PaddleFunctions.h"
#include "playerFunctions.h"
#include "ballFunctions.h"
#include "gameFunctions.h"
#include "vectorFunctions.h"








