#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
//#include <bits/stdc++.h>

#include <iostream>
#include <vector>
#include <queue>
#include <cfloat>
#define pb push_back
#define mp make_pair
#define infinity FLT_MAX
using namespace std;
int N,M,K, time_left, playerIndex;
int ourPlayer;

int utilityPlayerOne, utilityPlayerTwo;

struct wall
{
    int orientation;
    int rowCenter;
    int colCenter;
    
    wall(int o, int r, int c) {
        orientation = o;
        rowCenter = r;
        colCenter = c;
    }
};

struct qMove {
    int type;
    int row;
    int col;
    
    qMove(int t, int r, int c) {
        type = t;
        row = r;
        col = c;
    }
    
    qMove() {
        type = 0;
        row = 0;
        col = 0;
    }
    
    qMove(const qMove& other)
    {
        type=other.type;
        row=other.row;
        col=other.col;
    }
};

struct moveEval
{
    qMove move;
    float eval;
    
    moveEval(qMove m, float f) {
        move = m;
        eval = f;
    }
    
    moveEval(float f) {
        eval = f;
    }
};

struct position
{
    int row;
    int col;
};

vector<position> playerPaths[2];

struct player
{
    position pos;
    int wallsLeft;
    
    player(int initialRow, int initialCol, int totalWalls)
    {
        pos.row = initialRow;
        pos.col = initialCol;
        wallsLeft = totalWalls;
    }
    
    player()
    {
        pos.row = 9;
        pos.col = 9;
        wallsLeft = 10;
    }
};


struct gameState
{
    int n; // Length
    int m; // Breath
    int currentPlayer;
    bool wasPreviousMoveChangingPath;
    qMove previousMove;
    player players[2];
    vector<gameState> children;
    vector<wall>  wallsPlacedSoFar; // (Orientation, Row Centre, Col Centre) -> Orientation 0 for horizontal, 1 for vertical
    
    gameState(int length, int breadth, int totalWalls)
    {
        currentPlayer = 0;
        n = length;
        m = breadth;
        this->players[0] = player(1, (m+1)/2, totalWalls);
        this->players[1] = player(n, (m+1)/2,totalWalls);
        this->previousMove = qMove();
        cout<<"robin " <<totalWalls;
        wasPreviousMoveChangingPath = false;
    }
    
};


//function definitions
moveEval maxValue(gameState gameData, float alpha, float beta);
bool arePositionsAdjacent(gameState currentState, position pos0, position pos1);
bool arePlayersAdjacent(gameState currentState);
bool isValidPosition(position pos, gameState currentState);
vector<position> neighbours(position pos, gameState currentState);
bool isValidMove(gameState currentState, qMove myMove);
gameState moveState(gameState currentState, qMove myMove);
moveEval minValue(gameState gameData, float alpha, float beta, int depth);
moveEval maxValue(gameState gameData, float alpha, float beta, int depth);
bool isGoalState(gameState currentState, position pos, int playerIndex);

//minimax stuff

vector<qMove> validMoves(gameState currentState)
{
    vector<qMove> currentMoves;
    int currentPlayerIndex = currentState.currentPlayer;
    int otherPlayerIndex = 1 - currentPlayerIndex;
    position currentPlayerPos = currentState.players[currentPlayerIndex].pos;
    position otherPlayerPos = currentState.players[otherPlayerIndex].pos;
    vector<position> currentPlayerNeighbours = neighbours(currentPlayerPos, currentState);
    vector<position> otherPlayerNeighbours;
    if (!isGoalState(currentState, currentPlayerPos, currentPlayerIndex)) {
        if (arePlayersAdjacent(currentState)) {
            otherPlayerNeighbours = neighbours(otherPlayerPos, currentState);
        }
        for (int i = 0; i < currentPlayerNeighbours.size(); i++) {
            qMove potentialMove(0, currentPlayerNeighbours[i].row, currentPlayerNeighbours[i].col);
            if (isValidMove(currentState, potentialMove)) {
                currentMoves.pb(potentialMove);
            }
        }
        for (int i = 0; i < otherPlayerNeighbours.size(); i++) {
            qMove potentialMove(0, otherPlayerNeighbours[i].row, otherPlayerNeighbours[i].col);
            if (isValidMove(currentState, potentialMove)) {
                currentMoves.pb(potentialMove);
            }
        }
    }
    if (currentState.players[currentPlayerIndex].wallsLeft > 0) {
        for (int i = 2; i <= currentState.n; i++) {
            for (int j = 2; j <= currentState.m; j++) {
                qMove potentialMove1(1, i, j);
                if (isValidMove(currentState, potentialMove1)) {
                    currentMoves.pb(potentialMove1);
                }
                qMove potentialMove2(2, i, j);
                if (isValidMove(currentState, potentialMove2)) {
                    currentMoves.pb(potentialMove2);
                }
            }
        }
    }
    return currentMoves;
}

bool isGoalState(gameState currentState, position pos, int playerInd) {
    return ((pos.row) == ((1-playerInd)*(currentState.n - 1) + 1));
}

vector<position> neighbours(position pos, gameState currentState)
{
    int currentRow = pos.row;
    int currentCol = pos.col;
    position posUp, posDn, posLe, posRi;
    posUp.row = currentRow - 1;
    posUp.col = currentCol;
    posDn.row = currentRow + 1;
    posDn.col = currentCol;
    posRi.row = currentRow;
    posRi.col = currentCol + 1;
    posLe.row = currentRow;
    posLe.col = currentCol - 1;
    vector<position> neighbourPositions;
    if (isValidPosition(posUp, currentState) && arePositionsAdjacent(currentState, posUp, pos))
    {
        neighbourPositions.pb(posUp);
    }
    if (isValidPosition(posDn, currentState) && arePositionsAdjacent(currentState, posDn, pos)) {
        neighbourPositions.pb(posDn);
    }
    if (isValidPosition(posRi, currentState) && arePositionsAdjacent(currentState, posRi, pos)) {
        neighbourPositions.pb(posRi);
    }
    if (isValidPosition(posLe, currentState) && arePositionsAdjacent(currentState, posLe, pos)) {
        neighbourPositions.pb(posLe);
    }
    return neighbourPositions;
}

bool isValidPosition(position pos, gameState currentState)
{
    return (pos.row >= 1 && pos.col >= 1 && pos.row <= currentState.n && pos.col <= currentState.m);
}

bool doesNewMoveChangePath(gameState currentState) {
    if (playerPaths[0].size() == 0) {
        return true;
    }
    else if (playerPaths[1].size() == 0) {
        return true;
    }
    else {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < playerPaths[i].size() - 1; j++) {
                if (!arePositionsAdjacent(currentState, playerPaths[i][j], playerPaths[i][j+1]) )
                {
                    return true;
                }
            }
        }
    }
    return false;
}


int canPlayerReachGoalState(gameState currentState, int playerInd) {
    position pos = currentState.players[playerInd].pos;
    queue<position> visited;
    visited.push(pos);
    int distance[currentState.n + 1][currentState.m + 1];
    position parent[currentState.n + 1][currentState.m + 1];
    memset(distance, -1, sizeof(distance));
    memset(parent, -1, sizeof(parent));
    distance[pos.row][pos.col] = 0;
    if (isGoalState(currentState, pos, playerInd))
    {
        return 0;
    }
    while (!visited.empty()) {
        position currentPos = visited.front();
        visited.pop();
        vector<position> currentNeighbours;
        currentNeighbours = neighbours(currentPos, currentState);
        for (int i = 0; i < currentNeighbours.size(); i++) {
            if (distance[currentNeighbours[i].row][currentNeighbours[i].col] == -1) {
                if (isGoalState(currentState, currentNeighbours[i], playerInd)) {
                    parent[currentNeighbours[i].row][currentNeighbours[i].col] = currentPos;
                    distance[currentNeighbours[i].row][currentNeighbours[i].col] = distance[currentPos.row][currentPos.col] + 1;
                    position current = currentNeighbours[i];
                    while( current.row != -1) {
                        playerPaths[playerInd].clear();
                        playerPaths[playerInd].pb(current);
                        current = parent[current.row][current.col];
                        
                    }
                    return distance[currentNeighbours[i].row][currentNeighbours[i].col];
                }
                visited.push(currentNeighbours[i]);
                parent[currentNeighbours[i].row][currentNeighbours[i].col] = currentPos;
                distance[currentNeighbours[i].row][currentNeighbours[i].col] = distance[currentPos.row][currentPos.col] + 1;
            }
        }
    }
    return -1;
}

bool isValidPlayerMove(gameState currentState, qMove playerMove)
{
    position currentPos = currentState.players[currentState.currentPlayer].pos;
    position otherPos = currentState.players[1-currentState.currentPlayer].pos;
    position movedPos;
    movedPos.row = playerMove.row;
    movedPos.col = playerMove.col;
    //pass not allowed
    if (movedPos.row == currentPos.row && movedPos.col == currentPos.col)
    {
        return false;
    }
    if (arePositionsAdjacent(currentState, currentPos, movedPos))
    {
        if (movedPos.row == otherPos.row && movedPos.col == otherPos.col)
        {
            if (isGoalState(currentState, otherPos, (1-currentState.currentPlayer)))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    if (arePlayersAdjacent(currentState))
    {
        if (arePositionsAdjacent(currentState, otherPos, movedPos)) {
            position priorityPos;
            priorityPos.row = currentPos.row + 2*(otherPos.row - currentPos.row);
            priorityPos.col = currentPos.col + 2*(otherPos.col - currentPos.col);
            if (isValidPosition(priorityPos, currentState) && arePositionsAdjacent(currentState, otherPos, priorityPos)) {
                if (priorityPos.row == movedPos.row && priorityPos.col == movedPos.col) {
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                return true;
            }
        }
    }
    return false;
}

bool isValidWallMove(gameState currentState, qMove wallMove) {
    for (int i=0; i<currentState.wallsPlacedSoFar.size(); i++) {
        int currentOrientation=currentState.wallsPlacedSoFar[i].orientation;
        int currentRow=currentState.wallsPlacedSoFar[i].rowCenter;
        int currentColumn=currentState.wallsPlacedSoFar[i].colCenter;
        
        if (currentOrientation!=wallMove.type-1 && currentRow==wallMove.row && currentColumn==wallMove.col) {
            return false;
        }
        else {
            if (currentOrientation==0 && abs(currentColumn-wallMove.col)<=1 && currentRow == wallMove.row) {
                //horizontal
                return false;
            }
            else if(currentOrientation==1 && abs(currentRow-wallMove.row)<=1 && currentColumn == wallMove.col) {
                //vertical
                return false;
            }
        }
    }
    return true;
}

bool isValidMove(gameState currentState, qMove myMove) {
    int type = myMove.type;
    if (type == 0) {
        if (myMove.row >= 1 && myMove.col >= 1 && myMove.row <= currentState.n && myMove.col <= currentState.m) {
            return isValidPlayerMove(currentState, myMove);
        }
        else {
            return false;
        }
    }
    if (type == 1 || type == 2) {
        if (myMove.row >= 2 && myMove.col >= 2 && myMove.row <= currentState.n && myMove.col <= currentState.m) {
            return isValidWallMove(currentState, myMove);
        }
    }
    return false;
}




qMove bestMove;
int maxDepth=3;


float utility(gameState gameData)
{
    if (doesNewMoveChangePath(gameData) == true)
    {
        utilityPlayerOne=canPlayerReachGoalState(gameData, ourPlayer);
        utilityPlayerTwo=canPlayerReachGoalState(gameData, (1-ourPlayer));
        if(utilityPlayerOne<0 || utilityPlayerTwo<0)
        {
            return infinity;
        }
        return -utilityPlayerOne+0.9*utilityPlayerTwo;
    }
    return -utilityPlayerOne+0.9*utilityPlayerTwo;
}


float minimax(gameState node,int depth, float alpha, float beta,bool maximisingPlayer)
{
    if (depth==0 || isGoalState(node, node.players[ourPlayer].pos, ourPlayer))
    {
        return utility(node);
    }
    int temp;
    if (maximisingPlayer)
    {
        float v=(-infinity);
        vector<qMove>allValidMoves=validMoves(node);
        //        cout<<"fhk"<<allValidMoves.size()<<endl;
        for (int i=0; i<allValidMoves.size(); i++)
        {
            temp=max(v,minimax(moveState(node, allValidMoves[i]), depth-1, alpha,beta,false));
            if(v<temp )
            {
                v=temp;
                if(depth==maxDepth)
                {
                    bestMove=allValidMoves[i];
                }
            }
            
            alpha=max(alpha,v);
            if (beta<=alpha)
            {
                break;
            }
        }
        return v;
    }
    else
    {
        float v=infinity;
        vector<qMove>allValidMoves=validMoves(node);
        for (int i=0; i<allValidMoves.size(); i++)
        {
            v=min(v,minimax(moveState(node, allValidMoves[i]), depth-1, alpha, beta, true));
            beta=min(v,beta);
            if (beta<=alpha)
            {
                break;
            }
        }
        
        return v;
    }
}

//end minimax stuff





bool arePositionsAdjacent(gameState currentState, position pos0, position pos1)
{
    bool potentiallyAdjacent = (abs(pos0.row - pos1.row) + abs(pos0.col - pos1.col) == 1);
    if (potentiallyAdjacent) {
        int orientationRequired = abs(pos0.col - pos1.col);
        for(std::vector<int>::size_type i = 0; i != currentState.wallsPlacedSoFar.size(); i++) {
            wall currentWall = currentState.wallsPlacedSoFar[i];
            if (currentWall.orientation == orientationRequired) {
                if (orientationRequired == 0) {
                    int potentialRowCenter = (pos0.row + pos1.row + 1)/2;
                    int colDifference = currentWall.colCenter - pos0.col;
                    if ((currentWall.rowCenter == potentialRowCenter) && (colDifference >= 0) && (colDifference <=  1)) {
                        return false;
                    }
                }
                if (orientationRequired == 1) {
                    int potentialColCenter = (pos0.col + pos1.col + 1)/2;
                    int rowDifference = currentWall.rowCenter - pos0.row;
                    if ((currentWall.colCenter == potentialColCenter) && (rowDifference >= 0) && (rowDifference <= 1)) {
                        return false;
                    }
                }
            }
        }
    }
    return potentiallyAdjacent;
}

bool arePlayersAdjacent(gameState currentState) {
    position pos0 = currentState.players[0].pos;
    position pos1 = currentState.players[1].pos;
    return arePositionsAdjacent(currentState, pos0, pos1);
}


gameState moveState(gameState currentState, qMove myMove)
{
    gameState afterMoveState = currentState;
    if (myMove.type == 0) {
        afterMoveState.players[currentState.currentPlayer].pos.row = myMove.row;
        afterMoveState.players[currentState.currentPlayer].pos.col = myMove.col;
        afterMoveState.wasPreviousMoveChangingPath = false;
    }
    else if (myMove.type == 1 || myMove.type == 2) {
        int orientation = myMove.type - 1;
        int rowCenter = myMove.row;
        int colCenter = myMove.col;
        wall movedWall(orientation, rowCenter, colCenter);
        afterMoveState.wallsPlacedSoFar.pb(movedWall);
        afterMoveState.players[currentState.currentPlayer].wallsLeft--;
        afterMoveState.wasPreviousMoveChangingPath = true;
    }
    afterMoveState.previousMove = myMove;
    afterMoveState.currentPlayer = 1 - afterMoveState.currentPlayer;
    return afterMoveState;
}

/* Complete the function below to print 1 integer which will be your next move
 */


gameState GS(1,1,1);


int main(int argc, char *argv[])
{
    srand (static_cast <unsigned> (time(0)));
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    char sendBuff[1025];
    struct sockaddr_in serv_addr;
    
    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> <port no> \n",argv[0]);
        //        return 1;
    }
    
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(12345);
    
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }
    
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    
    cout<<"Quoridor will start..."<<endl;
    
    memset(recvBuff, '0',sizeof(recvBuff));
    n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
    recvBuff[n] = 0;
    sscanf(recvBuff, "%d %d %d %d %d", &playerIndex, &N, &M, &K, &time_left);
    
    ourPlayer = playerIndex - 1;
    
    GS=gameState(N,M,K);
    utilityPlayerOne = N - 1;
    utilityPlayerTwo = N - 1;
    cout<<"asdkhf "<<GS.n<<" asdkh "<<GS.m<<" "<<GS.players[0].wallsLeft;
    
    cout<<"Player "<<playerIndex<<endl;
    cout<<"Time "<<time_left<<endl;
    cout<<"Board size "<<N<<"x"<<M<<" :"<<K<<endl;
    float TL;
    int om,oro,oc;
    int m,r,c;
    int d=3;
    char s[100];
    int x=1;
    if(playerIndex == 1)
    {
        
        memset(sendBuff, '0', sizeof(sendBuff));
        string temp;
        //	cin>>m>>r>>c;
        //        qMove moveToMake=alphaBetaSearch(GS, 2);
        minimax(GS,maxDepth, -infinity, infinity,true);
        qMove moveToMake=bestMove;
        GS=moveState(GS, moveToMake);
        if (moveToMake.type==-1)
        {
            //            snprintf(sendBuff, sizeof(sendBuff), "pass");
        }
        else
        {
            snprintf(sendBuff, sizeof(sendBuff), "%d %d %d", moveToMake.type, moveToMake.row , moveToMake.col);
            write(sockfd, sendBuff, strlen(sendBuff));
        }
        
        
        memset(recvBuff, '0',sizeof(recvBuff));
        n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
        recvBuff[n] = 0;
        sscanf(recvBuff, "%f %d", &TL, &d);
        cout<<TL<<" "<<d<<endl;
        if(d==1)
        {
            cout<<"You win!! Yayee!! :D ";
            x=0;
        }
        else if(d==2)
        {
            cout<<"Loser :P ";
            x=0;
        }
    }
    
    while(x)
    {
        memset(recvBuff, '0',sizeof(recvBuff));
        n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
        recvBuff[n] = 0;
        sscanf(recvBuff, "%d %d %d %d", &om,&oro,&oc,&d);
        cout << om<<" "<<oro<<" "<<oc << " "<<d<<endl;
        qMove opponentMove(om,oro,oc);
        GS=moveState(GS,opponentMove);
        
        if(d==1)
        {
            cout<<"You win!! Yayee!! :D ";
            break;
        }
        else if(d==2)
        {
            cout<<"Loser :P ";
            break;
        }
        memset(sendBuff, '0', sizeof(sendBuff));
        string temp;
        //        qMove moveToMake=alphaBetaSearch(GS, 2);
        
        minimax(GS,maxDepth, -infinity, infinity,true);
        qMove moveToMake=bestMove;
        GS=moveState(GS, moveToMake);
        if (moveToMake.type==-1)
        {
            //            snprintf(sendBuff, sizeof(sendBuff), "pass");
        }
        else
        {
            printf("%d %d %d", moveToMake.type, moveToMake.row , moveToMake.col);
            snprintf(sendBuff, sizeof(sendBuff), "%d %d %d", moveToMake.type, moveToMake.row , moveToMake.col);
            write(sockfd, sendBuff, strlen(sendBuff));
        }
        
        memset(recvBuff, '0',sizeof(recvBuff));
        n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
        recvBuff[n] = 0;
        sscanf(recvBuff, "%f %d", &TL, &d);//d=3 indicates game continues.. d=2 indicates lost game, d=1 means game won.
        cout<<TL<<" "<<d<<endl;
        if(d==1)
        {
            cout<<"You win!! Yayee!! :D ";
            break;
        }
        else if(d==2)
        {
            cout<<"Loser :P ";
            break;
        }
    }
    cout<<endl<<"The End"<<endl;
    return 0;
}
















