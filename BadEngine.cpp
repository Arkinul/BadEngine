#include <bits/stdc++.h>
#include <chrono>

using namespace std;
using ll = long long;


static const int None = 0;
static const int King = 1;
static const int Pawn = 2;
static const int Knight = 3;
static const int Bishop = 4;
static const int Rook = 5;
static const int Queen = 6;

static const int White = 8;
static const int Black = 16;

static const string startingFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static const vector<int> dirOffsets = {8,-8,-1,1,7,-7,9,-9};
static const vector<int> knightOffsets = {6,-10,15,-17,17,-15,10,-6};
static vector<vector<int>> numSquaresToEdge(64,vector<int>(8));
static const int KCastle = 1;
static const int QCastle = 2;
static const int KnightProm = 3;
static const int BishopProm = 4;
static const int RookProm = 5;
static const int QueenProm = 6;
static const int doublePawnPush = 7;
static const int enPassant = 8;
static random_device randDev;

static const int pawnValue = 100;
static const int knightValue = 300;
static const int bishopValue = 301;
static const int rookValue = 500;
static const int queenValue = 900;
static const vector<int> pieceValues = {0,0,pawnValue,knightValue,bishopValue,rookValue,queenValue,0,
                                        0,0,pawnValue,knightValue,bishopValue,rookValue,queenValue,0,
                                        0,0,-pawnValue,-knightValue,-bishopValue,-rookValue,-queenValue,0};


static void precomputeMoveData(){
    for (int file = 0;file<8;file++){
        for(int rank = 0;rank<8;rank++){
            int numUp = 7-rank;
            int numDown = rank;
            int numLeft = file;
            int numRight = 7-file;

            int squareIndex = rank*8+file;

            numSquaresToEdge[squareIndex] = {numUp,numDown,numLeft,numRight,min(numUp,numLeft),min(numDown,numRight),min(numUp,numRight),min(numDown,numLeft)};
        }
    }
}
static bool isColor(int piece, int color){
    if((piece & color) == 0){
        return false;
    }
    return true;
}
static bool isSlidingPiece(int piece){
    int colorless = piece & 7;
    if(colorless == Bishop or colorless == Rook or colorless == Queen){
        return true;
    }
    return false;
}
static int oppositeColor(int color){
    if(color == White){
        return Black;
    }else{
        return White;
    }
}


class chessMove{
public:
    int startSquare;
    int endSquare;
    int specialty;
    chessMove(int start,int end){
        startSquare = start;
        endSquare = end;
        specialty = 0;
    }
    chessMove(int start,int end, int spec){
        startSquare = start;
        endSquare = end;
        specialty = spec;
    }
};

class chessPosition {
public:
    vector<int> board;
    char turn;
    vector<bool> castlingRights;
    int enPassantTarget;
    int halfmoves;
    int fullmoves;


    int whiteKingSquare;
    int blackKingSquare;
    vector<chessMove*> pseudoLegalMoves;
    vector<chessMove*> legalMoves;
    int material;

    chessPosition(){
        for(int i = 0; i<64;i++){
            board.push_back(0);
        }
        castlingRights= {true,true,true,true};
        enPassantTarget = -1;
        turn = 8;
        halfmoves = 0;
        fullmoves = 1;

        whiteKingSquare = -1;
        blackKingSquare = -1;
        material = 0;
    }
    explicit chessPosition(string fen){
        auto pieceFromSymbol = new map<char, char>  {
                {'k',King}, {'p', Pawn}, {'n',Knight}, {'b',Bishop},{'r',Rook},{'q',Queen}
        };
        string fenBoard = fen.substr(0,fen.find_first_of(' '));
        string fenRemainder = fen.substr(fen.find_first_of(' ')+1,fen.size()-fen.find_first_of(' ')-1);
        int file = 0,rank = 7;
        for(int i = 0; i<64;i++){
            board.push_back(None);
        }
        for(auto symbol: fenBoard){
            if(symbol == '/'){
                file = 0;
                rank--;
            }else{
                if(isdigit(symbol)){
                    file += stoi(&symbol);
                }else{
                    int pieceColor = isupper(symbol) ? White : Black;
                    int pieceType = pieceFromSymbol->at(tolower(symbol));
                    board[rank*8+file] = pieceColor | pieceType;
                    file++;
                }
            }
        }
        string fenTurn = fenRemainder.substr(0,fenRemainder.find_first_of(' '));
        fenRemainder =  fenRemainder.substr(fenRemainder.find_first_of(' ')+1,fenRemainder.size()-fenRemainder.find_first_of(' ')-1);
        if(fenTurn == "w"){
            turn = 8;
        }else if(fenTurn == "b"){
            turn = 16;
        }else{
            cout<<"invalid FEN"<<endl;
        }
        string fenRights = fenRemainder.substr(0,fenRemainder.find_first_of(' '));
        fenRemainder =  fenRemainder.substr(fenRemainder.find_first_of(' ')+1,fenRemainder.size()-fenRemainder.find_first_of(' ')-1);
        castlingRights= {false,false,false,false};
        for(auto symbol : fenRights){
            if(symbol == '-'){
                break;
            }else if(symbol == 'K'){
                castlingRights[0] = true;
            }else if(symbol == 'Q'){
                castlingRights[1] = true;
            }else if(symbol == 'k'){
                castlingRights[2] = true;
            }else if(symbol == 'q'){
                castlingRights[3] = true;
            }else{
                cout<<"invalid FEN"<<endl;
            }
        }
        string fenEnPassant = fenRemainder.substr(0,fenRemainder.find_first_of(' '));
        fenRemainder =  fenRemainder.substr(fenRemainder.find_first_of(' ')+1,fenRemainder.size()-fenRemainder.find_first_of(' ')-1);
        enPassantTarget = -1;
        //TODO: actually use the FEN here too, stringtoSquare
        string fenHalfMoves = fenRemainder.substr(0,fenRemainder.find_first_of(' '));
        fenRemainder =  fenRemainder.substr(fenRemainder.find_first_of(' ')+1,fenRemainder.size()-fenRemainder.find_first_of(' ')-1);

        halfmoves = stoi(fenHalfMoves);


        fullmoves = stoi(fenRemainder);
        material = 0;
        for(int i = 0;i<64;i++){
            material += pieceValues[board[i]];
            if(board[i]==(White|King)){
                whiteKingSquare = i;
            }else if(board[i]==(Black|King)){
                blackKingSquare = i;
            }
        }


    }
    vector<chessMove*> generatePseudolegal(){
        vector<chessMove*> moves;
        for(int startSquare = 0;startSquare<64;startSquare++){
            int piece = this->board[startSquare];
            if(isColor(piece,this->turn)){
                if(isSlidingPiece(piece)){
                    generateSlidingMoves(startSquare,piece,moves);
                }else if((piece & 7) == Knight){
                    generateKnightMoves(startSquare,moves);
                }else if((piece & 7) == Pawn){
                    generatePawnMoves(startSquare,moves);
                }else{
                    generateKingMoves(startSquare,moves);
                }
            }
        }
        return moves;
    }
    void generateSlidingMoves(int startSquare, int piece,vector<chessMove*> &moves){
        int startDirIndex = ((piece & 7) == Bishop) ? 4 : 0;
        int endDirIndex = ((piece & 7) == Rook) ? 4 : 8;

        for (int dirIndex = startDirIndex; dirIndex<endDirIndex;dirIndex++){
            for (int n = 0; n < numSquaresToEdge[startSquare][dirIndex];n++){
                int targetSquare = startSquare + dirOffsets[dirIndex] * (n+1);
                int pieceOnTargetSquare = this->board[targetSquare];
                if(isColor(pieceOnTargetSquare,this->turn)){
                    break;
                }
                moves.push_back(new chessMove(startSquare,targetSquare));
                if(isColor(pieceOnTargetSquare, oppositeColor(this->turn))){
                    break;
                }
            }
        }
    }
    void generateKnightMoves(int startSquare,vector<chessMove*> &moves){
        int file = startSquare % 8;
        int startDirIndex;
        int endDirIndex;
        switch (file) {
            case 0:
                startDirIndex = 4;
                endDirIndex = 8;
                break;
            case 1:
                startDirIndex = 2;
                endDirIndex = 8;
                break;
            case 6:
                startDirIndex = 0;
                endDirIndex = 6;
                break;
            case 7:
                startDirIndex = 0;
                endDirIndex = 4;
                break;
            default:
                startDirIndex = 0;
                endDirIndex = 8;
                break;
        }
        for(int dirIndex = startDirIndex;dirIndex<endDirIndex;dirIndex++){
            int targetSquare = startSquare+knightOffsets[dirIndex];
            if(0<=targetSquare and targetSquare<64){
                if(!isColor(this->board[targetSquare],this->turn)){
                    moves.push_back(new chessMove(startSquare,targetSquare));
                }

            }
        }


    }
    void generatePawnMoves(int startSquare,vector<chessMove*> &moves){
        int file = startSquare % 8;
        int rank = (startSquare - file)/8;
        if(this->turn==White){
            if(this->board[startSquare+8] == None){
                if(rank == 6){
                    moves.push_back(new chessMove(startSquare,startSquare+8,KnightProm));
                    moves.push_back(new chessMove(startSquare,startSquare+8,BishopProm));
                    moves.push_back(new chessMove(startSquare,startSquare+8,RookProm));
                    moves.push_back(new chessMove(startSquare,startSquare+8,QueenProm));
                }else{
                    moves.push_back(new chessMove(startSquare,startSquare+8));
                }
                if(rank ==1){
                    if(this->board[startSquare+16] == None){
                        moves.push_back(new chessMove(startSquare,startSquare+16,doublePawnPush));
                    }
                }
            }
            if(file != 0){
                if(isColor(this->board[startSquare+7], Black)){
                    if(rank == 6){
                        moves.push_back(new chessMove(startSquare,startSquare+7,KnightProm));
                        moves.push_back(new chessMove(startSquare,startSquare+7,BishopProm));
                        moves.push_back(new chessMove(startSquare,startSquare+7,RookProm));
                        moves.push_back(new chessMove(startSquare,startSquare+7,QueenProm));
                    }else{
                        moves.push_back(new chessMove(startSquare,startSquare+7));
                    }
                }else if(startSquare+7==this->enPassantTarget){
                    moves.push_back(new chessMove(startSquare,startSquare+7,enPassant));
                }

            }
            if(file != 7){
                if(isColor(this->board[startSquare+9], Black)){
                    if(rank == 6){
                        moves.push_back(new chessMove(startSquare,startSquare+9,KnightProm));
                        moves.push_back(new chessMove(startSquare,startSquare+9,BishopProm));
                        moves.push_back(new chessMove(startSquare,startSquare+9,RookProm));
                        moves.push_back(new chessMove(startSquare,startSquare+9,QueenProm));
                    }else{
                        moves.push_back(new chessMove(startSquare,startSquare+9));
                    }
                }else if(startSquare+9==this->enPassantTarget){
                    moves.push_back(new chessMove(startSquare,startSquare+9,enPassant));
                }

            }

        }else{
            if(this->board[startSquare-8] == None){
                if(rank == 1){
                    moves.push_back(new chessMove(startSquare,startSquare-8,KnightProm));
                    moves.push_back(new chessMove(startSquare,startSquare-8,BishopProm));
                    moves.push_back(new chessMove(startSquare,startSquare-8,RookProm));
                    moves.push_back(new chessMove(startSquare,startSquare-8,QueenProm));
                }else{
                    moves.push_back(new chessMove(startSquare,startSquare-8));
                }
                if(rank == 6){
                    if(this->board[startSquare-16] == None){
                        moves.push_back(new chessMove(startSquare,startSquare-16,doublePawnPush));
                    }
                }
            }
            if(file != 0){
                if(isColor(this->board[startSquare-9], White)){
                    if(rank == 1){
                        moves.push_back(new chessMove(startSquare,startSquare-9,KnightProm));
                        moves.push_back(new chessMove(startSquare,startSquare-9,BishopProm));
                        moves.push_back(new chessMove(startSquare,startSquare-9,RookProm));
                        moves.push_back(new chessMove(startSquare,startSquare-9,QueenProm));
                    }else{
                        moves.push_back(new chessMove(startSquare,startSquare-9));
                    }
                }else if(startSquare-9==this->enPassantTarget){
                    moves.push_back(new chessMove(startSquare,startSquare-9,enPassant));
                }

            }
            if(file != 7){
                if(isColor(this->board[startSquare-7], White)){
                    if(rank == 1){
                        moves.push_back(new chessMove(startSquare,startSquare-7,KnightProm));
                        moves.push_back(new chessMove(startSquare,startSquare-7,BishopProm));
                        moves.push_back(new chessMove(startSquare,startSquare-7,RookProm));
                        moves.push_back(new chessMove(startSquare,startSquare-7,QueenProm));
                    }else{
                        moves.push_back(new chessMove(startSquare,startSquare-7));
                    }
                }else if(startSquare-7==this->enPassantTarget){
                    moves.push_back(new chessMove(startSquare,startSquare-7,enPassant));
                }

            }
        }

    }
    void generateKingMoves(int startSquare,vector<chessMove*> &moves){
        for(int dirIndex = 0;dirIndex<8;dirIndex++){
            if (numSquaresToEdge[startSquare][dirIndex] !=0){
                int targetSquare = startSquare + dirOffsets[dirIndex];
                int pieceOnTargetSquare = this->board[targetSquare];
                if(isColor(pieceOnTargetSquare,this->turn)){
                    continue;
                }
                moves.push_back(new chessMove(startSquare,targetSquare));
            }
        }
        if(this->turn == White and startSquare == 4){

            if(this->board[7]==(White | Rook) and this->castlingRights[0] and this->board[5]==None and this->board[6]==None){
                if(!(isAttacked(4) or isAttacked(5))){
                    moves.push_back(new chessMove(startSquare,startSquare+2,KCastle));
                }

            }
            if(this->board[0]==(White | Rook) and this->castlingRights[1] and this->board[1]==None and this->board[2]==None and this->board[3]==None){
                if(!(isAttacked(2) or isAttacked(3) or isAttacked(4))){
                    moves.push_back(new chessMove(startSquare,startSquare-2,QCastle));
                }

            }
        }
        if(this->turn == Black and startSquare == 60){
            if(this->board[63]==(Black | Rook) and this->castlingRights[2] and this->board[61]==None and this->board[62]==None){
                if(!(isAttacked(60) or isAttacked(61))){
                    moves.push_back(new chessMove(startSquare,startSquare+2,KCastle));
                }

            }
            if(this->board[56]==(Black | Rook) and this->castlingRights[3] and this->board[57]==None and this->board[58]==None and this->board[59]==None){
                if(!(isAttacked(58) or isAttacked(59) or isAttacked(60))){
                    moves.push_back(new chessMove(startSquare,startSquare-2,QCastle));
                }

            }
        }

    }
    chessPosition(const chessPosition &oldPos){
        //does not copy the lists of moves!!
        board = oldPos.board;
        turn = oldPos.turn;
        castlingRights = oldPos.castlingRights;
        enPassantTarget = oldPos.enPassantTarget;
        halfmoves = oldPos.halfmoves;
        fullmoves = oldPos.fullmoves;

        whiteKingSquare = oldPos.whiteKingSquare;
        blackKingSquare = oldPos.blackKingSquare;
        material = oldPos.material;

    }
    chessPosition makeMove(chessMove* move){
        chessPosition newPos = *this;
        if(newPos.turn == White){
            newPos.turn = Black;
        }else{
            newPos.fullmoves++;
            newPos.turn = White;
        }
        int piece = newPos.board[move->startSquare];
        newPos.board[move->startSquare] = None;

        newPos.board[move->endSquare] = piece;
        newPos.enPassantTarget = -1;
        if(piece == (White | King)){
            newPos.whiteKingSquare = move->endSquare;
            newPos.castlingRights[0] = false;
            newPos.castlingRights[1] = false;
        }else if(piece == (Black | King)){
            newPos.blackKingSquare = move->endSquare;
            newPos.castlingRights[2] = false;
            newPos.castlingRights[3] = false;
        }else if(piece == (White | Rook)){
            if(move->startSquare == 7){
                newPos.castlingRights[0] = false;
            }else if(move->startSquare == 0){
                newPos.castlingRights[1] = false;
            }
        }else if(piece == (Black | Rook)){
            if(move->startSquare == 63){
                newPos.castlingRights[2] = false;
            }else if(move->startSquare == 56){
                newPos.castlingRights[3] = false;
            }
        }
        if(move->endSquare == 7){
            newPos.castlingRights[0] = false;
        }else if(move->endSquare == 0){
            newPos.castlingRights[1] = false;
        }else if(move->endSquare == 63){
            newPos.castlingRights[2] = false;
        }else if(move->endSquare == 56){
            newPos.castlingRights[3] = false;
        }
        if(move->specialty == None){

        }else if(move->specialty == doublePawnPush){
            if(newPos.turn == White){
                newPos.enPassantTarget = move->startSquare-8;
            }else if(newPos.turn == Black){
                newPos.enPassantTarget = move->startSquare+8;
            }
        }else if(move->specialty == KCastle){
            if(this->turn == White){
                newPos.board[7] = None;
                newPos.board[5] = (White | Rook);
            }else if(this->turn == Black){
                newPos.board[63] = None;
                newPos.board[61] = (Black | Rook);
            }
        }else if(move->specialty == QCastle){
            if(this->turn == White){
                newPos.board[0] = None;
                newPos.board[3] = (White | Rook);
            }else if(this->turn == Black){
                newPos.board[56] = None;
                newPos.board[59] = (Black | Rook);
            }
        }else if(move->specialty == KnightProm){
            newPos.board[move->endSquare] = (this->turn | Knight);
        }else if(move->specialty == BishopProm){
            newPos.board[move->endSquare] = (this->turn | Bishop);
        }else if(move->specialty == RookProm){
            newPos.board[move->endSquare] = (this->turn | Rook);
        }else if(move->specialty == QueenProm){
            newPos.board[move->endSquare] = (this->turn | Queen);
        }else if(move->specialty == enPassant){
            if(this->turn == White){
                newPos.board[move->endSquare-8] = None;
            }else if(this->turn == Black){
                newPos.board[move->endSquare+8] = None;
            }
        }
        newPos.material -= pieceValues[this->board[move->endSquare]];

        return newPos;
    }
    bool isAttacked(int Square){
        for(int dirIndex= 0;dirIndex<4;dirIndex++){
            for(int n=0;n<numSquaresToEdge[Square][dirIndex];n++){
                int targetSquare = Square + dirOffsets[dirIndex] * (n+1);
                int pieceOnTargetSquare = this->board[targetSquare];
                if(pieceOnTargetSquare == (Rook | oppositeColor(this->turn)) or pieceOnTargetSquare == (Queen | oppositeColor(this->turn))){
                    return true;
                }else if(pieceOnTargetSquare == None){
                    continue;
                }else{
                    break;
                }
            }
        }
        for(int dirIndex= 4;dirIndex<8;dirIndex++){
            for(int n=0;n<numSquaresToEdge[Square][dirIndex];n++){
                int targetSquare = Square + dirOffsets[dirIndex] * (n+1);
                int pieceOnTargetSquare = this->board[targetSquare];
                if(pieceOnTargetSquare == (Bishop | oppositeColor(this->turn)) or pieceOnTargetSquare == (Queen | oppositeColor(this->turn))){
                    return true;
                }else if(pieceOnTargetSquare == None){
                    continue;
                }else{
                    break;
                }
            }
        }
        int file = Square % 8;
        int startDirIndex;
        int endDirIndex;
        switch (file) {
            case 0:
                startDirIndex = 4;
                endDirIndex = 8;
                break;
            case 1:
                startDirIndex = 2;
                endDirIndex = 8;
                break;
            case 6:
                startDirIndex = 0;
                endDirIndex = 6;
                break;
            case 7:
                startDirIndex = 0;
                endDirIndex = 4;
                break;
            default:
                startDirIndex = 0;
                endDirIndex = 8;
                break;
        }
        for(int dirIndex = startDirIndex;dirIndex<endDirIndex;dirIndex++){
            int targetSquare = Square+knightOffsets[dirIndex];
            if(0<=targetSquare and targetSquare<64){
                if(this->board[targetSquare]== (oppositeColor(this->turn)|Knight)){
                    return true;
                }

            }
        }
        if(file != 0){
            if(this->turn == White and this->board[Square+7]==(Black|Pawn)){
                return true;
            }else if(this->turn == Black and this->board[Square-9]==(White|Pawn)){
                return true;
            }
        }
        if(file != 7){
            if(this->turn == White and this->board[Square+9]==(Black|Pawn)){
                return true;
            }else if(this->turn == Black and this->board[Square-7]==(White|Pawn)){
                return true;
            }
        }
        for(int dirIndex = 0;dirIndex<8;dirIndex++){
            if (numSquaresToEdge[Square][dirIndex] != 0){
                int targetSquare = Square + dirOffsets[dirIndex];
                int pieceOnTargetSquare = this->board[targetSquare];
                if(this->turn == White and pieceOnTargetSquare == (Black|King)){
                    return true;
                }else if(this->turn == Black and pieceOnTargetSquare == (White|King)){
                    return true;
                }

            }
        }


        return false;
    }
    chessMove* randomMove(){

        int x = randDev() % (this->legalMoves.size());
        return this->legalMoves[x];
    }
    vector<chessMove*> generateLegalMoves(){
        vector<chessMove*> moves;
        chessPosition resultPos;
        for(auto move: pseudoLegalMoves){
            resultPos = this->makeMove(move);
            if(resultPos.turn==White){
                resultPos.turn = Black;
                if(!resultPos.isAttacked(resultPos.blackKingSquare)){
                    moves.push_back(move);
                }else{
                    //cout<<"removed "<<moveToPrintMove(move)<<endl;
                }
            }else if(resultPos.turn == Black){
                resultPos.turn = White;
                if(!resultPos.isAttacked(resultPos.whiteKingSquare)){
                    moves.push_back(move);
                }else{
                    //cout<<"removed "<<moveToPrintMove(move)<<endl;
                }
            }

        }
        shuffle(moves.begin(),moves.end(),randDev);
        return moves;
    }
    void generateMoves(){
        this->pseudoLegalMoves = this->generatePseudolegal();
        this->legalMoves = this->generateLegalMoves();
    }
    static string squareToString(int square){
        int file = square % 8;
        int rank = (square-file) / 8;

        string result;
        file = file+char(97);
        rank = rank+49;
        char filechar = char(file);
        char rankchar = char(rank);

        result = result + filechar;

        result = result + rankchar;

        return result;
    }
    string moveToPrintMove(chessMove* move){
        char piece;
        switch(this->board[move->startSquare] & 7){
            case King:
                piece = 'K';
                break;
            case Pawn:
                piece = 'P';
                break;
            case Knight:
                piece = 'N';
                break;
            case Bishop:
                piece = 'B';
                break;
            case Rook:
                piece = 'R';
                break;
            case Queen:
                piece = 'Q';
                break;
        }
        string result;
        if(this->turn == White){
            result = result + to_string(this->fullmoves)+ ". ";
        }
        if(move->specialty == KCastle){
            result = result + "0-0";
            return result;
        }else if(move->specialty == QCastle){
            result = result + "0-0-0";
            return result;
        }
        result = result + piece + squareToString(move->startSquare);
        if(this->board[move->endSquare] != None or move->specialty == enPassant){
            result = result + "x";
        }
        result = result + squareToString(move->endSquare);
        if(move->specialty == KnightProm){
            result = result + "=N";
        }else if(move->specialty == BishopProm){
            result = result + "=B";
        }else if(move->specialty == RookProm){
            result = result + "=R";
        }else if(move->specialty == QueenProm){
            result = result + "=Q";
        }
        auto resultPos = this->makeMove(move);
        if(resultPos.turn==White){
            if(resultPos.isAttacked(resultPos.whiteKingSquare)){
                result = result + "+";
            }
        }else if(resultPos.turn == Black){
            if(resultPos.isAttacked(resultPos.blackKingSquare)){
                result = result + "+";
            }
        }
        return result;
        //TODO:better with checkmate
    }
    bool playerInCheck(){
        if(this->turn == White and this->isAttacked(this->whiteKingSquare)){
            return true;
        }else if(this->turn == Black and this->isAttacked(this->blackKingSquare)){
            return  true;
        }
        return false;
    }
    void displayBoard(){
        for(int rank = 7;rank>=0;rank--){
            for(int file = 0;file<8;file++){
                cout<<this->board[rank*8+file]<<" ";
            }
            cout<<endl;
        }
    }
};



static int moveGenTest(chessPosition* pos, int depth){
    if (depth == 0){
        return 1;
    }
    pos->generateMoves();
    int numberOfPositions= 0;
    for(auto move:pos->legalMoves){
        chessPosition newPos = pos->makeMove(move);
//        int test = moveGenTest(newPos,depth-1);
//        if(depth>0){
//            cout<<pos.moveToPrintMove(move)<<" "<<test<<endl;
//        }


        numberOfPositions += moveGenTest(&newPos,depth-1);
    }
    //cout<<numberOfPositions<<endl;
    return numberOfPositions;
}

static chessMove* stringToMove(string movestring){
    char file = movestring[0];
    char rank = movestring[1];
    int fileint = file - 97;
    int rankint = rank - 49;
    int startsquare = rankint * 8 + fileint;
    file = movestring[2];
    rank = movestring[3];
    fileint = file -97;
    rankint = rank - 49;
    int endsquare = rankint * 8 + fileint;
    if(movestring.size()>4){
        int spec = 0;
        if(movestring.size() == 5){
            if(movestring[4] == 'D'){
                spec = doublePawnPush;
            }else if(movestring[4] == 'P'){
                spec = enPassant;
            }else if(movestring[4] == 'N'){
                spec = KnightProm;
            }else if(movestring[4] == 'B'){
                spec = BishopProm;
            }else if(movestring[4] == 'R'){
                spec = RookProm;
            }else if(movestring[4] == 'Q'){
                spec = QueenProm;
            }
        }else if(movestring.size() == 6){
            if(movestring[4] == 'K'){
                spec = KCastle;
            }else if(movestring[4] == 'Q'){
                spec = QCastle;
            }
        }
        return new chessMove(startsquare,endsquare,spec);
    }
    return new chessMove(startsquare, endsquare);
}


static int baseEvaluate(chessPosition* pos){
    int eval;
    if (pos->legalMoves.empty()){
        if(pos->playerInCheck()){
                eval = -1000000000;
        }else{
            eval = 0;
        }
    }else{
        eval = pos->material;
    }
    if(pos->turn == White){
        return eval;
    }
    return -eval;
}
static int searchEvaluate(chessPosition* pos,int depth,int alpha, int beta){
    pos->generateMoves();
    if(depth == 0){
        return baseEvaluate(pos);
    }
    if(pos->legalMoves.empty()){
        if(pos->playerInCheck()){
            return -1000000000;
        }
        return 0;
    }


    for(auto move : pos->legalMoves){
        chessPosition newPos = pos->makeMove(move);
        int eval = -searchEvaluate(&newPos,depth-1,-beta,-alpha);
        //cout<<pos->moveToPrintMove(move)<<" "<<eval<<endl;
        if(eval >= beta){
            return beta;
        }

        alpha = max(alpha,eval);

    }
    return alpha;

}
static pair<int,chessMove*> outerEvaluate(chessPosition* pos,int depth){
    pos->generateMoves();
    if(depth == 0){
        return {baseEvaluate(pos), NULL};
    }
    if(pos->legalMoves.empty()){
        if(pos->playerInCheck()){
            return {-1000000000,NULL};
        }
        return {0,NULL};
    }
    int bestEval = -1000000000;
    chessMove* bestMove;
    for(auto move : pos->legalMoves){
        chessPosition newPos = pos->makeMove(move);
        int eval = -searchEvaluate(&newPos,depth-1,-1000000000,1000000000);
        //cout<<pos->moveToPrintMove(move)<<" "<<eval<<endl;

        if(eval>bestEval){
            bestEval = eval;
            bestMove = move;
        }

    }
    return {bestEval,bestMove};

}

static void gameAgainstHuman(){
    //TODO: show eval, engine win
    cout<<"New Game against Human"<<endl;
    chessPosition livePos = *new chessPosition(startingFEN);
    cout<<"choose Color[w/b]:"<<endl;
    string color;
    cin>>color;
    cout<<"show eval?[y/n]:"<<endl;
    string input;
    bool evalOn;
    cin>>input;
    if(input == "y"){
        evalOn = true;
    }else if(input == "n"){
        evalOn = false;
    }
    bool ongoing = true;
    chessMove* move;
    if(color == "w"){
        while(ongoing){

            cin>>input;
            if(input == "stop"){
                ongoing = false;
                break;
            }else if(input == "board"){
                livePos.displayBoard();
                cin>>input;
                move = stringToMove(input);
                livePos = livePos.makeMove(move);
            }else{
                move = stringToMove(input);
                livePos = livePos.makeMove(move);
            }
            livePos.generateMoves();
            if(livePos.legalMoves.empty()){
                if(livePos.isAttacked(livePos.blackKingSquare)){
                    cout<<"Checkmate, you win!"<<endl;
                    ongoing = false;
                    break;
                }else{
                    cout<<"Stalemate!"<<endl;
                    ongoing = false;
                    break;
                }
            }else{
                move = outerEvaluate(&livePos,4).second;
                cout<<livePos.moveToPrintMove(move)<<endl;
                livePos = livePos.makeMove(move);
            }
        }
    }else if(color == "b"){
        while(ongoing){

            livePos.generateMoves();
            if(livePos.legalMoves.empty()){
                if(livePos.isAttacked(livePos.whiteKingSquare)){
                    cout<<"Checkmate, you win!"<<endl;
                    ongoing = false;
                    break;
                }else{
                    cout<<"Stalemate!"<<endl;
                    ongoing = false;
                    break;
                }
            }else{
                move = outerEvaluate(&livePos,4).second;
                cout<<livePos.moveToPrintMove(move)<<endl;
                livePos = livePos.makeMove(move);
            }

            cin>>input;
            if(input == "stop"){
                ongoing = false;
                break;
            }else if(input == "board"){
                livePos.displayBoard();
                cin>>input;
                move = stringToMove(input);
                livePos = livePos.makeMove(move);
            }else {
                move = stringToMove(input);
                livePos = livePos.makeMove(move);
            }
        }
    }
    cout<<"game ended"<<endl;

}

int main() {
    precomputeMoveData();
    string diffcultTestPosFEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    auto testpos = new chessPosition("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    auto diffTestPos = new chessPosition(diffcultTestPosFEN);
    diffTestPos->displayBoard();





    auto thirdPos= new chessPosition("R1b1k1nr/2Bp4/5p2/4p1p1/2P1P2p/5Q1P/P4PP1/6K1 b k - 1 30");



//    for(int i = 0; i<30;i++){
//        thirdPos.pseudoLegalMoves = thirdPos.generatePseudolegal();
//        thirdPos.legalMoves = thirdPos.generateLegalMoves();
//
//        chessMove* move = thirdPos.randomMove();
//        cout<<thirdPos.moveToPrintMove(move)<<endl;
//        thirdPos = (thirdPos.makeMove(move));
//    }
    thirdPos->displayBoard();

    cout<<thirdPos->material<<endl;
    pair<int,chessMove*> evalMove = outerEvaluate(thirdPos,4);

    cout<<evalMove.first<<" "<<thirdPos->moveToPrintMove(evalMove.second)<<endl;
    //diffTestPos = diffTestPos.makeMove(new chessMove(4,6,KCastle));
//    for(int i = 1;i<6;i++){
//        int test = 0;
//        auto start = chrono::high_resolution_clock::now();
//        test = moveGenTest(diffTestPos,i);
//        auto stop = chrono::high_resolution_clock::now();
//        auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
//        cout<<i<< " " <<duration.count()<< "microseconds"<<endl;
//    }


    gameAgainstHuman();



    return 0;
}
