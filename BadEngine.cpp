#include <bits/stdc++.h>

using namespace std;
using ll = long long;


static const char None = 0;
static const char King = 1;
static const char Pawn = 2;
static const char Knight = 3;
static const char Bishop = 4;
static const char Rook = 5;
static const char Queen = 6;

class Position {
public:
    vector<char> board;
    char turn;
    vector<bool> castlingRights;
    int halfmoves;
    int fullmoves;
    bool isInCheck;

    Position(){
        for(int i = 0; i<64;i++){
            board.push_back(0);
        }
        turn = 8;
        halfmoves = 0;
        fullmoves = 0;
        isInCheck = false;
    }
    explicit Position(string fen){
        auto pieceFromSymbol = new map<char, char>  {
                {'k',King}, {'p', Pawn}, {'n',Knight}, {'b',Bishop},{'r',Rook},{'q',Queen}
        };
        string fenboard = fen.substr(0,fen.find_first_of(' '));
        int file = 0,rank = 7;
        for(int i = 0; i<64;i++){
            board.push_back(0);
        }
        for(auto symbol: fenboard){
            if(symbol == '/'){
                file = 0;
                rank--;
            }else{
                if(isdigit(symbol)){
                    file += atoi(&symbol);
                }else{
                    //TODO
                }
            }
        }
    }
};

//TODO: first the board must work, then other things



int main() {
    cout << atoi("0") << endl;
    return 0;
}
