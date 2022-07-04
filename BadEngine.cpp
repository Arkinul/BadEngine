#include <bits/stdc++.h>

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

static const string startingFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

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
    bool isInCheck;
    int whiteKingSquare;
    int blackKingSquare;
    list<chessMove*> pseudoLegalMoves;
    list<chessMove*> legalMoves;

    chessPosition(){
        for(int i = 0; i<64;i++){
            board.push_back(0);
        }
        castlingRights= {true,true,true,true};
        enPassantTarget = -1;
        turn = 8;
        halfmoves = 0;
        fullmoves = 1;
        isInCheck = false;
        whiteKingSquare = -1;
        blackKingSquare = -1;
    }
    explicit chessPosition(string fen){
        auto pieceFromSymbol = new map<char, char>  {
                {'k',King}, {'p', Pawn}, {'n',Knight}, {'b',Bishop},{'r',Rook},{'q',Queen}
        };
        string fenboard = fen.substr(0,fen.find_first_of(' '));
        int file = 0,rank = 7;
        for(int i = 0; i<64;i++){
            board.push_back(None);
        }
        for(auto symbol: fenboard){
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
        castlingRights= {true,true,true,true};
        enPassantTarget = -1;
        turn = 8;
        halfmoves = 0;
        fullmoves = 1;
        //TODO: actually use the FEN here too
        isInCheck = false;
        for(int i = 0;i<63;i++){
            if(board[i]==(White|King)){
                whiteKingSquare = i;
            }else if(board[i]==(Black|King)){
                blackKingSquare = i;
            }
        }

    }
    list<chessMove*> generatePseudolegal(){
        list<chessMove*> moves;
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
    void generateSlidingMoves(int startSquare, int piece,list<chessMove*> &moves){
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
    void generateKnightMoves(int startSquare,list<chessMove*> &moves){
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
    void generatePawnMoves(int startSquare,list<chessMove*> &moves){
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
            if(file != 0 and (isColor(this->board[startSquare+7], Black) or this->board[startSquare+7]==this->enPassantTarget)){
                if(rank == 6){
                    moves.push_back(new chessMove(startSquare,startSquare+7,KnightProm));
                    moves.push_back(new chessMove(startSquare,startSquare+7,BishopProm));
                    moves.push_back(new chessMove(startSquare,startSquare+7,RookProm));
                    moves.push_back(new chessMove(startSquare,startSquare+7,QueenProm));
                }else{
                    moves.push_back(new chessMove(startSquare,startSquare+7));
                }
            }
            if(file != 7 and (isColor(this->board[startSquare+9], Black) or this->board[startSquare+9]==this->enPassantTarget)){
                if(rank == 6){
                    moves.push_back(new chessMove(startSquare,startSquare+9,KnightProm));
                    moves.push_back(new chessMove(startSquare,startSquare+9,BishopProm));
                    moves.push_back(new chessMove(startSquare,startSquare+9,RookProm));
                    moves.push_back(new chessMove(startSquare,startSquare+9,QueenProm));
                }else{
                    moves.push_back(new chessMove(startSquare,startSquare+9));
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
            if(file != 0 and (isColor(this->board[startSquare-9], White) or this->board[startSquare-9]==this->enPassantTarget)){
                if(rank == 1){
                    moves.push_back(new chessMove(startSquare,startSquare-9,KnightProm));
                    moves.push_back(new chessMove(startSquare,startSquare-9,BishopProm));
                    moves.push_back(new chessMove(startSquare,startSquare-9,RookProm));
                    moves.push_back(new chessMove(startSquare,startSquare-9,QueenProm));
                }else{
                    moves.push_back(new chessMove(startSquare,startSquare-9));
                }
            }
            if(file != 7 and (isColor(this->board[startSquare-7], White) or this->board[startSquare-7]==this->enPassantTarget)){
                if(rank == 1){
                    moves.push_back(new chessMove(startSquare,startSquare-7,KnightProm));
                    moves.push_back(new chessMove(startSquare,startSquare-7,BishopProm));
                    moves.push_back(new chessMove(startSquare,startSquare-7,RookProm));
                    moves.push_back(new chessMove(startSquare,startSquare-7,QueenProm));
                }else{
                    moves.push_back(new chessMove(startSquare,startSquare-7));
                }
            }
        }

    }
    void generateKingMoves(int startSquare,list<chessMove*> &moves){
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
        //does not copy the lists of moves
        board = oldPos.board;
        turn = oldPos.turn;
        castlingRights = oldPos.castlingRights;
        enPassantTarget = oldPos.enPassantTarget;
        halfmoves = oldPos.halfmoves;
        fullmoves = oldPos.fullmoves;
        isInCheck = oldPos.isInCheck;
        whiteKingSquare = oldPos.whiteKingSquare;
        blackKingSquare = oldPos.blackKingSquare;

    }
    chessPosition makeMove(chessMove move){
        chessPosition newPos = *this;
        if(newPos.turn == White){
            newPos.turn = Black;
        }else{
            newPos.fullmoves++;
            newPos.turn = White;
        }
        int piece = newPos.board[move.startSquare];
        newPos.board[move.startSquare] = None;

        newPos.board[move.endSquare] = piece;
        newPos.enPassantTarget = -1;
        if(piece == (White | King)){
            newPos.whiteKingSquare = move.endSquare;
            newPos.castlingRights[0] = false;
            newPos.castlingRights[1] = false;
        }else if(piece == (Black | King)){
            newPos.blackKingSquare = move.endSquare;
            newPos.castlingRights[2] = false;
            newPos.castlingRights[3] = false;
        }else if(piece == (White | Rook)){
            if(move.startSquare == 7){
                newPos.castlingRights[0] = false;
            }else if(move.startSquare == 0){
                newPos.castlingRights[1] = false;
            }
        }else if(piece == (Black | Rook)){
            if(move.startSquare == 63){
                newPos.castlingRights[2] = false;
            }else if(move.startSquare == 56){
                newPos.castlingRights[3] = false;
            }
        }
        if(move.endSquare == 7){
            newPos.castlingRights[0] = false;
        }else if(move.endSquare == 0){
            newPos.castlingRights[1] = false;
        }else if(move.endSquare == 63){
            newPos.castlingRights[2] = false;
        }else if(move.endSquare == 56){
            newPos.castlingRights[3] = false;
        }
        if(move.specialty == None){

        }else if(move.specialty == doublePawnPush){
            if(newPos.turn == White){
                newPos.enPassantTarget = move.startSquare+8;
            }else if(newPos.turn == Black){
                newPos.enPassantTarget = move.startSquare-8;
            }
        }else if(move.specialty == KCastle){
            if(newPos.turn == White){
                newPos.board[7] = None;
                newPos.board[5] = (White | Rook);
            }else if(newPos.turn == Black){
                newPos.board[63] = None;
                newPos.board[61] = (Black | Rook);
            }
        }else if(move.specialty == QCastle){
            if(newPos.turn == White){
                newPos.board[0] = None;
                newPos.board[3] = (White | Rook);
            }else if(newPos.turn == Black){
                newPos.board[56] = None;
                newPos.board[59] = (Black | Rook);
            }
        }else if(move.specialty == KnightProm){
            newPos.board[move.endSquare] = (newPos.turn | Knight);
        }else if(move.specialty == BishopProm){
            newPos.board[move.endSquare] = (newPos.turn | Bishop);
        }else if(move.specialty == RookProm){
            newPos.board[move.endSquare] = (newPos.turn | Rook);
        }else if(move.specialty == QueenProm){
            newPos.board[move.endSquare] = (newPos.turn | Queen);
        }
        //TODO: update material
        return newPos;
    }
    bool isAttacked(int Square){
        for(int dirIndex= 0;dirIndex<4;dirIndex++){
            for(int n=0;n<numSquaresToEdge[Square][dirIndex];n++){
                int targetSquare = Square + dirOffsets[dirIndex] * (n+1);
                int pieceOnTargetSquare = this->board[targetSquare];
                if(isColor(pieceOnTargetSquare,this->turn)){
                    break;
                }
                if(pieceOnTargetSquare == (Bishop | oppositeColor(this->turn)) or pieceOnTargetSquare == (Queen | oppositeColor(this->turn))){
                    return true;
                }
            }
        }
        for(int dirIndex= 4;dirIndex<8;dirIndex++){
            for(int n=0;n<numSquaresToEdge[Square][dirIndex];n++){
                int targetSquare = Square + dirOffsets[dirIndex] * (n+1);
                int pieceOnTargetSquare = this->board[targetSquare];
                if(isColor(pieceOnTargetSquare,this->turn)){
                    break;
                }
                if(pieceOnTargetSquare == (Rook | oppositeColor(this->turn)) or pieceOnTargetSquare == (Queen | oppositeColor(this->turn))){
                    return true;
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

        return false;
    }
};



//TODO: first the board must work, then other things



int main() {
    precomputeMoveData();
    string diffcultTestPosFEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    auto testpos = new chessPosition("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR");
    auto diffTestPos = new chessPosition(diffcultTestPosFEN);
    for(int rank = 7;rank>=0;rank--){
        for(int file = 0;file<8;file++){
            cout<<diffTestPos->board[rank*8+file]<<" ";
        }
        cout<<endl;
    }
    diffTestPos->pseudoLegalMoves = diffTestPos->generatePseudolegal();
    cout<<diffTestPos->pseudoLegalMoves.size()<<endl;
    for(auto k: diffTestPos->pseudoLegalMoves){
        cout<<k->startSquare<<" "<<k->endSquare<<endl;

    }
    chessPosition afterMove = testpos->makeMove(*new chessMove(6,21));
    cout<<afterMove.board[6]<<endl;
    cout<<afterMove.board[21]<<endl;
    cout<<testpos->board[6]<<endl;
    cout<<testpos->board[21]<<endl;




    return 0;
}
