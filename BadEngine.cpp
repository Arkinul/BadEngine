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

struct chessMove;

class chessPosition {
public:
    vector<int> board;
    char turn;
    vector<bool> castlingRights;
    int halfmoves;
    int fullmoves;
    bool isInCheck;
    list<chessMove> pseudoLegalMoves;
    list<chessMove> legalMoves;

    chessPosition(){
        for(int i = 0; i<64;i++){
            board.push_back(0);
        }
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
        turn = 8;
        halfmoves = 0;
        fullmoves = 1;
        isInCheck = false;
        //TODO: actually use the FEN here too
    }

};

class chessMove{
public:
    int startSquare;
    int endSquare;
};

//TODO: first the board must work, then other things



int main() {
    precomputeMoveData();
    auto testpos = new chessPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    for(int rank = 7;rank>=0;rank--){
        for(int file = 0;file<8;file++){
            cout<<testpos->board[rank*8+file]<<" ";
        }
        cout<<endl;
    }
    return 0;
}
