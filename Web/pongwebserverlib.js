/*
Pong Server Interface Thing
Tyler Auslitz
20 December 2020

These classes are used for the pong web server
The pongServer class is used to transmit the locations of the ball and the paddles
To the server so that that information can be relayed to the other players that are
In the game that the current client is in.
This will also handle incoming and outgoing text chat
Maybe experiment with VOIP and data streams
*/
class pongServer{
    #socket;
    clientId;
    clientPaddleNumber;
    gameId;
    screenSide;
    opPaddleId;
    isConnected;
    constructor(address, port, ballCoordFunction, opPaddle, playerLeft, startGame){
        this.isConnected = false;
        this.socket=new WebSocket('ws://'+address+':'+port);
        this.socket.addEventListener('open',e=>{
            this.isConnected = true;
            console.log("Connection Opened");
            console.log("Getting the client information.....");
            this.socket.send("c");
        });
        this.socket.addEventListener('message',function(event){
            //console.log("Message from the server: " + event.data);
            let inData=event.data;
            if(inData[0]=='c'){//client information
                let commaIndex=inData.indexOf(",");
                this.clientId=inData.substring(1,commaIndex);
                console.log("Client ID: ", this.clientId);
                this.clientPaddleNumber=inData.substring(commaIndex+1, inData.length);
                console.log("Paddle ID: ", this.clientPaddleNumber);
            }
            else if(inData[0]=='b' && inData[1]!='a'){
                let commaIndex=inData.indexOf(",");
                let ballX=inData.substring(1,commaIndex);
                let ballY=inData.substring(commaIndex+1,inData.length);
                ballCoordFunction(ballX,ballY);
            }
            else if(inData[0]=='p'){
                let lastComma=1;
                let data=[];
                for(let i=1; i<inData.length; i++){
                    if(inData[i]==','){ 
                        data.push(inData.substring(lastComma,i));
                        lastComma=i+1;
                    }
                }
                data.push(inData.substring(lastComma, inData.length));
                console.log("Client ", data[0], " paddle is now at\n\tX: ", data[1], "\n\tY: ", data[2]);
                if(this.clientId!=data[0])  
                    opPaddle(data[1],data[2]);
            }
            else if(inData[0]=='g'){//Gets the game information

                let numList = (inData.slice(1).split(","));

                this.gameId = numList[0];
                this.screenSide = numList[1];
                this.opPlayerId = numList[2];



                startGame(this.screenSide);


            }
            else if(inData[0]=='e'){//Gets the game information

                playerLeft();
            
            }

        });
    }
    //Used to move the paddle to a location
    movePaddle(x,y){
        let toSend="p"+x+","+y;
        //console.log("Sending: ", toSend); 
        this.socket.send(toSend);
    }
    
    findGame(){
    
        var i = 0;

        this.socket.send("f");

    }

}

function ballMovement(x, y){
    console.log("BallMovement\n\tX: ", x, "\n\tY: ", y);
    //Move the ball
}
function opPaddle(x,y){
    //Move the op paddle
}
let s;
function serverExample(){
    s=new pongServer("localhost",1666,ballMovement,opPaddle);
    //s.movePaddle(1,5);
}