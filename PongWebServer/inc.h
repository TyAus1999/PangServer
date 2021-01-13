#pragma once
#include <iostream>
#include <thread>
#include <string>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <chrono>
#include <mutex>
#include "WebsocketStuff.h"
#include "PongServerData.h"
#include "PaddleFunctions.h"

#define ballDiameter 1.0
#define paddleWidth 1.0
#define paddleHeight 6.0

#define ballRadius ballDiameter/2.0
#define hPaddleWidth paddleWidth/2.0
#define hPaddleHeight paddleHeight/2.0



typedef unsigned long long u64;



