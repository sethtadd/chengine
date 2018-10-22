#ifndef STATETREE_HPP
#define STATETREE_HPP

#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <memory>

const int COLOR_THRESHOLD = 90; // because all white PieceTypes are < 90 and black are > 90

enum struct PieceType : int {EMPTY=45,W_PAWN=80,W_KNIGHT=78,W_BISHOP=66,W_ROOK=82,W_QUEEN=81,W_KING=75,B_PAWN=112,B_KNIGHT=110,B_BISHOP=98,B_ROOK=114,B_QUEEN=113,B_KING=107}; // NOTE values assigned as such so that they can be translated into corresponding chars to be printed // TODO Can int be made into byte for better performace?

struct GameState; // forward declaration so that the GameState struct can be referenced by GameState's definition
struct GameState // TODO Can this be made a private member of StateTree?
{
    // tree properties
    GameState* parent;
    std::vector<std::unique_ptr<GameState>> nextLevel; // references all of the next possible GameStates
    
    // gamestate properties
    float evaluation;
    bool whiteTurn; // true if white's turn, false when black's turn
//     bool inCheck; // if the capture of the king is in any of the next gamestates then this is true
    bool inCheck_W;
    bool inCheck_B;
    bool kingsideRookMoved_W; // NOTE RookMoved is set true for both when king moves, easier that way
    bool kingsideRookMoved_B; // NOTE RookMoved is set true for both when king moves, easier that way
    bool queensideRookMoved_W; // white
    bool queensideRookMoved_B; // black
    bool castled_W;
    bool castled_B;
    std::array<std::array<PieceType,8>,8> board; // x by y ATTENTION - Iterate through y first when iterating through matrix
    
    GameState(GameState* _parent)
    {
        parent = _parent;
        // castling assumed not possible, evaluateCastleAbility() will change this if necessary
        if (_parent != nullptr) {
            whiteTurn = !_parent->whiteTurn;
            kingsideRookMoved_W = _parent->kingsideRookMoved_W;
            queensideRookMoved_W = _parent->queensideRookMoved_W;
            castled_W = _parent->castled_W;
            castled_B = _parent->castled_B;
            kingsideRookMoved_B = _parent->kingsideRookMoved_B;
            queensideRookMoved_B = _parent->queensideRookMoved_B;
            inCheck_W = false; // NOTE this is later changed by children
            inCheck_B = false;
            if (whiteTurn) { evaluation = -10000; }
            else if (!whiteTurn) { evaluation = 10000; }
            else { std::cout << " ##### ERROR in GameState() constructor ##### \n"; }
        }
        else
        {
            whiteTurn = true; evaluation = -10000;
            kingsideRookMoved_W = false;
            queensideRookMoved_W = false;
            kingsideRookMoved_B = false;
            queensideRookMoved_B = false;
            castled_W = false;
            castled_B = false;
            inCheck_W = false;
            inCheck_B = false;
        } // if parent is null then the GameState is the initial GameState
    }
};

/* ATTENTION - Methods of StateTree must be called in the correct order
 * 
 * NOTE For Computer Moves:
 * 1. genLevelS()
 * 2. evaluateLowestLevel()
 * 3. minimaxEval()
 * 4. pushComputerState()
 * 
 * NOTE For player moves:
 * 1. newLevel() - only one level needs to be present in order for the player's move to be verified as a legal move since the next level will contain all possible legal moves
*/

class StateTree
{
public:
    StateTree();
    
    void printCurrent(); // prints the board and info of the current state
    
    void printBoard(GameState* _gs); // prints the board and info of the current state
    
// private:
    std::vector<std::unique_ptr<GameState>> pastStates; // NOTE pastStates.back() will be the most current state which a tree will get branched off of
    std::vector<GameState*> deepestLevel; // points to the GameStates in the deepest level
    std::vector<std::string> moveList; // stores all past moves, indexes in moveList correspond to pastStates-1 (the second pastState corrsponds to the first move)
    
    void regenDeepestLevel(GameState* _gs);
    
    void genLevel(); // generates possible GameStates off of the current and then, if called again, generates possible GameStates off of the lowest level's GameStates, creating a tree
    
    void genLevels(int _levels); // calls genLevel() "_levels" times
    
    void minimaxEval(GameState* _gs); // (recursive) performs a minimax evaluation on the tree which will be used to select the next move, each GameState that isn't the lowest level gets a relative evaluation passed to it as deemed by the minimax algorithm
    
    void pushComputerState(); // push the next state onto pastStates as deemed by the minimax algorithm, NOTE This should also delete all of the other GameStates that are no longer relevent AKA the other GameStates in the level of the state that is getting pushed. // ATTENTION Can I push a GameState onto pastStates efficiently? A GameState will typically have a tree under it, will all of that memory be inefficiently reallocated?
    
    bool pushPlayerState(int _x1, int _y1, int _x2, int _y2); // push the next state onto pastStates as deemed by the player. Returns a bool indicating if the move was valid (true) or invalid (false)
    
    void pushMove(int _x1, int _y1, int _x2, int _y2);
    
    // ---------- MOVE FUNCTIONS ----------
    // these are passed the location of their respective piece and they generate all possible GameStates that the piece can cause and adds them as children to the parent GameState
    
    void pawnMove(GameState* _parentGS, int _x, int _y);
    
    void knightMove(GameState* _parentGS, int _x, int _y);
    
    void bishopMove(GameState* _parentGS, int _x, int _y);
    
    void rookMove(GameState* _parentGS, int _x, int _y);
    
    void queenMove(GameState* _parentGS, int _x, int _y);
    
    void kingMove(GameState* _parentGS, int _x, int _y);
    
    // ----- Helpers
    void evalCastleAbility(GameState* _gs); // changes _gs's castling bool's based on evaluation of castling ability NOTE 0 is false, 1 is queenside, 2 is kingsside, if 3 then both
    
    void evalCheckStatus(GameState* _gs); // change's _gs's inCheck bool based on evaluation of board
    
    void evalPawnPromotions(GameState* _gs); // change's _gs's inCheck bool based on evaluation of board
    
    void processStateGen(GameState* _parentGS, int _m1, int _n1, int _m2, int _n2);
    
    int validMove(GameState* _parentBS, int _m2, int _n2); // returns 0 if the move request is onto a friendly piece, 1 if it is to an empty square, and 2 if it's an enemy piece
    
    bool madeDoubleMove(GameState* _gs, int _x, int _y); // determines whether the piece at a given position double-moved or not
    
    void evaluate(GameState* _gs); // evalutes a GameState, this part is the main factor in determining how the engine plays
};

#endif
