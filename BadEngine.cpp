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
    chessMove(int start,int end){
        startSquare = start;
        endSquare = end;
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
    list<chessMove*> pseudoLegalMoves;
    list<chessMove*> legalMoves;

    chessPosition(){
        for(int i = 0; i<64;i++){
            board.push_back(0);
        }
        enPassantTarget = -1;
        turn = 8;
        halfmoves = 0;
        fullmoves = 1;
        isInCheck = false;
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
        enPassantTarget = -1;
        turn = 8;
        halfmoves = 0;
        fullmoves = 1;
        isInCheck = false;
        //TODO: actually use the FEN here too
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
                moves.push_back(new chessMove(startSquare,startSquare+8));
                if(rank ==1){
                    if(this->board[startSquare+16] == None){
                        moves.push_back(new chessMove(startSquare,startSquare+16));
                    }
                }
            }
            if(file != 0 and (isColor(this->board[startSquare+7], Black) or this->board[startSquare+7]==this->enPassantTarget)){
                moves.push_back(new chessMove(startSquare,startSquare+7));
            }
            if(file != 7 and (isColor(this->board[startSquare+9], Black) or this->board[startSquare+9]==this->enPassantTarget)){
                moves.push_back(new chessMove(startSquare,startSquare+9));
            }
        }else{
            if(this->board[startSquare-8] == None){
                moves.push_back(new chessMove(startSquare,startSquare-8));
                if(rank == 6){
                    if(this->board[startSquare-16] == None){
                        moves.push_back(new chessMove(startSquare,startSquare-16));
                    }
                }
            }
            if(file != 0 and (isColor(this->board[startSquare-9], White) or this->board[startSquare-9]==this->enPassantTarget)){
                moves.push_back(new chessMove(startSquare,startSquare-9));
            }
            if(file != 7 and (isColor(this->board[startSquare-7], White) or this->board[startSquare-7]==this->enPassantTarget)){
                moves.push_back(new chessMove(startSquare,startSquare-7));
            }
        }
        //TODO En passant target saving, maybe?, Promotion
    }
    void generateKingMoves(int startSquare,list<chessMove*> &moves){
        //TODO
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
    for(auto k: diffTestPos->pseudoLegalMoves){
        cout<<k->startSquare<<" "<<k->endSquare<<endl;
    }




    return 0;
}
