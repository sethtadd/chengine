#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <memory>

#include "StateTree.hpp"

// NOTE helper functions at bottom

StateTree::StateTree()
{
    std::unique_ptr<GameState> initial = std::make_unique<GameState>(nullptr);
    deepestLevel.push_back(initial.get()); // NOTE This comes first since after std::move(p), p in this scope is empty!
    pastStates.push_back(std::move(initial)); // Create starting board
    
    // ---------- STARTING BOARD ----------
    // white pieces
    pastStates[0]->board[0][0] = PieceType::W_ROOK;
    pastStates[0]->board[1][0] = PieceType::W_KNIGHT;
    pastStates[0]->board[2][0] = PieceType::W_BISHOP;
    pastStates[0]->board[3][0] = PieceType::W_QUEEN;
    pastStates[0]->board[4][0] = PieceType::W_KING;
    pastStates[0]->board[5][0] = PieceType::W_BISHOP;
    pastStates[0]->board[6][0] = PieceType::W_KNIGHT;
    pastStates[0]->board[7][0] = PieceType::W_ROOK;
    // white pawns
    for (int x = 0; x < 8; x++)
    {
        pastStates[0]->board[x][1] = PieceType::W_PAWN;
    }
    // empty spaces
    for (int x = 0; x < 8; x++)
    {
        for (int y = 2; y < 6; y++)
        {
            pastStates[0]->board[x][y] = PieceType::EMPTY;
        }
    }
    // black pawns
    for (int x = 0; x < 8; x++)
    {
        pastStates[0]->board[x][6] = PieceType::B_PAWN;
    }
    // black pieces
    pastStates[0]->board[0][7] = PieceType::B_ROOK;
    pastStates[0]->board[1][7] = PieceType::B_KNIGHT;
    pastStates[0]->board[2][7] = PieceType::B_BISHOP;
    pastStates[0]->board[3][7] = PieceType::B_QUEEN;
    pastStates[0]->board[4][7] = PieceType::B_KING;
    pastStates[0]->board[5][7] = PieceType::B_BISHOP;
    pastStates[0]->board[6][7] = PieceType::B_KNIGHT;
    pastStates[0]->board[7][7] = PieceType::B_ROOK;
}

void StateTree::printCurrent()
{
    // print info
    std::cout << "Move " << (int)pastStates.size()-1 << ". " << "Some info\n";
    
    // print board
    for (int y = 7; y >= 0; y--)
    {
        for (int x = 0; x < 8; x++)
        {
            std::cout << (char)pastStates.back()->board[x][y] << " ";
        }
        std::cout << '\n';
    }
}

void StateTree::printBoard(GameState* _gs)
{
    for (int y = 7; y >= 0; y--)
    {
        for (int x = 0; x < 8; x++)
        {
            std::cout << (char)_gs->board[x][y] << " ";
        }
        std::cout << '\n';
    }
}

void StateTree::regenDeepestLevel(GameState* _gs)
{
    if ((int)_gs->nextLevel.size() != 0)
    {
        for (int i = 0; i < (int)_gs->nextLevel.size(); i++)
        {
            GameState* _childGS = _gs->nextLevel[i].get();
            regenDeepestLevel(_childGS);
        }
    }
    else
    {
        deepestLevel.push_back(_gs);
    }
}

void StateTree::genLevel()
{
    deepestLevel.clear();
    regenDeepestLevel(pastStates.back().get());
    // iterate through each boardsquare and generate potential next GameStates
    std::vector<GameState*> deepestLevelTemp = deepestLevel; // ATTENTION Make sure that this is copying over and not just pointing to the same object
    deepestLevel.clear(); // the so that deepestLevel can be populated by deeper GameStates
    
    // generate potential next GameStates that branch off of each state in deepestLevel
    for (GameState* parentState : deepestLevelTemp)
    {
        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 8; x++)
            {
                // generate child GameStates
                PieceType piece = parentState->board[x][y];
                if (piece == PieceType::EMPTY) { /*Do nothing, empty square*/ } // move to next square when current is empty
                else if (parentState->whiteTurn && (int)piece < COLOR_THRESHOLD) // NOTE white PieceTypes have values less than 90, black's are greater
                {
                    switch(piece)
                    {
                        case (PieceType::W_PAWN):
                            pawnMove(parentState,x,y);
                            break;
                        case (PieceType::W_KNIGHT):
                            knightMove(parentState,x,y);
                            break;
                        case (PieceType::W_BISHOP):
                            bishopMove(parentState,x,y);
                            break;
                        case (PieceType::W_ROOK):
                            rookMove(parentState,x,y);
                            break;
                        case (PieceType::W_QUEEN):
                            queenMove(parentState,x,y);
                            break;
                        case (PieceType::W_KING):
                            kingMove(parentState,x,y);
                            break;
                        default:
                            std::cout << " ##### Error (1) in genLevel() ##### \n";
                            break;
                    }
                }
                else if (!parentState->whiteTurn && (int)piece > COLOR_THRESHOLD) // black moves
                {
                    switch(piece)
                    {
                        case (PieceType::B_PAWN):
                            pawnMove(parentState,x,y);
                            break;
                        case (PieceType::B_KNIGHT):
                            knightMove(parentState,x,y);
                            break;
                        case (PieceType::B_BISHOP):
                            bishopMove(parentState,x,y);
                            break;
                        case (PieceType::B_ROOK):
                            rookMove(parentState,x,y);
                            break;
                        case (PieceType::B_QUEEN):
                            queenMove(parentState,x,y);
                            break;
                        case (PieceType::B_KING):
                            kingMove(parentState,x,y);
                            break;
                        default:
                            std::cout << " ##### Error (2) in genLevel() ##### \n";
                            break;
                    }
                }
                else { /*Do nothing, enemy piece is on the square*/ }
            }
        }
        
        // now that all possibilities are generated convert pawns
        evalPawnPromotions(parentState);
        evalCastleAbility(parentState);
        // WARNING ASSUMING THIS CONDITIONAL CHECKS IN-CHECK
        // make sure not first move
        if (parentState->parent != nullptr)
        {
            if (parentState->evaluation > 5000 && !parentState->whiteTurn)
            {
                // the parent of this state is in check
                parentState->parent->inCheck_B = true;
            }
            else if (parentState->evaluation < -5000 && parentState->whiteTurn)
            {
                // the parent of this state is in check
                parentState->parent->inCheck_W = true;
            }
        }
    }
}

void StateTree::genLevels(int _levels)
{
    for (int i = 0; i < _levels; i++) { genLevel(); }
}

void StateTree::minimaxEval(GameState* _gs)
{
    if ((int)_gs->nextLevel.size() != 0)
    {
        // recursion
        for (int i = 0; i < (int)_gs->nextLevel.size(); i++)
        {
            GameState* _childState = _gs->nextLevel[i].get();
            minimaxEval(_childState);
        }
        
        // Assign _gs a value based on it's children
        _gs->evaluation = _gs->nextLevel[0]->evaluation;
        for (int i = 0; i < (int)_gs->nextLevel.size(); i++)
        {
            GameState* _childState = _gs->nextLevel[i].get();
            
            // Maximize
            if (_gs->whiteTurn && _childState->evaluation > _gs->evaluation)
            {
                _gs->evaluation = _childState->evaluation;
            }
            
            // Minimize
            else if (!_gs->whiteTurn && _childState->evaluation < _gs->evaluation)
            {
                _gs->evaluation = _childState->evaluation;
            }
        }
    }
    else if ((int)_gs->nextLevel.size() == 0)
    {
        evaluate(_gs);
    }
    else { std::cout << " ##### PHAT ERROR IN MINIMAX ##### \n"; }
}

void StateTree::pushComputerState()
{
    // Make sure a move can be made
    if ((int)pastStates.back()->nextLevel.size() == 0)
    {
        std::cout << " ##### ERROR No moves to make!!! @ pushComputerState() ##### \n";
        return;
    }
    
    minimaxEval(pastStates.back().get()); // pass current GameState
    
    int bestMoveIndex = 0;
    for (int i = 0; i < (int)pastStates.back()->nextLevel.size(); i++)
    {
        GameState* _childState = pastStates.back()->nextLevel[i].get(); // now we have a pointer "_childState" to the GameState
        
        if (pastStates.back().get()->whiteTurn) // white's move
        {
            if (_childState->evaluation > pastStates.back()->nextLevel[bestMoveIndex]->evaluation) // maximize
            {
                bestMoveIndex = i;
            }
        }
        else if (!pastStates.back().get()->whiteTurn) // black's move
        {
            if (_childState->evaluation < pastStates.back()->nextLevel[bestMoveIndex]->evaluation) // minimize
            {
                bestMoveIndex = i;
            }
        }
        else { std::cout << "##### ERROR in pushComputerState() ##### \n"; }
    }
    
    
    // PRINT MOVE BEGIN
    GameState* nextState = pastStates.back()->nextLevel[bestMoveIndex].get();
    GameState* currentState = pastStates.back().get();
    int x1 = -1;
    int y1 = -1;
    int x2 = -1;
    int y2 = -1;
    for (int y = 0; y < 8; y++)
    {
        for (int x = 7; x >= 0; x--) // run right left so that castle's will be a king move
        {
            // if square is empty but wasn't AND (was white's turn and white was at the square OR was blacks turn and black was at the square)
            if (nextState->board[x][y] == PieceType::EMPTY && currentState->board[x][y] != PieceType::EMPTY
                && ( (currentState->whiteTurn && (int)currentState->board[x][y] < COLOR_THRESHOLD/*emptyness check in previous conditions*/) || (!currentState->whiteTurn && (int)currentState->board[x][y] > COLOR_THRESHOLD) ) // for en passant
            )
            {
                x1 = x;
                y1 = y;
            }
            // if isn't empty and wasn't empty and it's different OR if isn't empty but it was
            else if ((nextState->board[x][y] != PieceType::EMPTY && currentState->board[x][y] != nextState->board[x][y]) || (nextState->board[x][y] != currentState->board[x][y] && currentState->board[x][y] == PieceType::EMPTY))
            {
                x2 = x;
                y2 = y;
            }
        }
    }
    // kingside white
    if (currentState->board[4][0] == PieceType::W_KING && nextState->board[6][0] == PieceType::W_KING)
    { x1=4; y1=0; x2=6; y2=0; }
    // queenside white
    else if (currentState->board[4][0] == PieceType::W_KING && nextState->board[2][0] == PieceType::W_KING)
    { x1=4; y1=0; x2=2; y2=0; }
    else if (currentState->board[4][7] == PieceType::W_KING && nextState->board[6][7] == PieceType::W_KING)
    { x1=4; y1=7; x2=6; y2=7; }
    // queenside white
    else if (currentState->board[4][7] == PieceType::W_KING && nextState->board[2][7] == PieceType::W_KING)
    { x1=4; y1=7; x2=2; y2=7; }
    if (x1==-1 || y1==-1 || x2==-1 || y2==-1) { std::cout << " ##### ERROR IN PRINTMOVE SECTION OF pushComputerState() ##### \n"; }
    pushMove(x1,y1,x2,y2);
    
    std::cout << moveList.back() << '\n';
    // PRINT MOVE END
    
    // now push the move onto pastStates, NOTE this should preserve the tree
    pastStates.push_back(std::move(pastStates.back()->nextLevel[bestMoveIndex]));
    
    // clear the other moves, NOTE this should preserve the tree under the pointer
    pastStates[(int)pastStates.size()-2]->nextLevel.clear();
}

bool StateTree::pushPlayerState(int _x1, int _y1, int _x2, int _y2)
{
    if (_x1 < 0 || _x1 > 7 || _y1 < 0 || _y1 > 7)
    {
        std::cout << "Are you trying to break the rules? Try again\n";
        return false;
    }
    
    GameState* currentState = pastStates.back().get();
    
    if
    (
        (currentState->board[_x1][_y1] == PieceType::EMPTY)
        || (currentState->whiteTurn && (int)currentState->board[_x1][_y1] > COLOR_THRESHOLD)
        || (!currentState->whiteTurn && (int)currentState->board[_x1][_y1] < COLOR_THRESHOLD)
    )
    {
        std::cout << "Opponent tried to do something silly... Try again\n";
        return false;
    }
    
    std::array<std::array<PieceType,8>,8> playerBoard = pastStates.back()->board;
    
    playerBoard[_x2][_y2] = playerBoard[_x1][_y1]; // NOTE all that matters is where they move a piece to --- ATTENTION WRONG!!!!! added stuff below
    playerBoard[_x1][_y1] = PieceType::EMPTY;
    
    // Make sure a move can be made
    if ((int)pastStates.back()->nextLevel.size() == 0)
    {
        std::cout << " ##### ERROR No moves to make!!! @ pushComputerState() ##### \n";
        return false;
    }
    
    int playerMoveIndex = -1;
    
    for (int i = 0; i < (int)pastStates.back()->nextLevel.size(); i++)
    {
        GameState* _childState = pastStates.back()->nextLevel[i].get(); // now we have a pointer "_childState" to the GameState
        if (_childState->board[_x2][_y2] == playerBoard[_x2][_y2] && _childState->board[_x1][_y1] == playerBoard[_x1][_y1]) // white's move
        {
            std::cout << "Move Verified\n";
            playerMoveIndex = i;
            break;
        }
    }
    
    if (playerMoveIndex == -1)
    {
        std::cout << "Invalid Move. Try again\n";
        return false;
    }
    
    // now push the move onto pastStates, NOTE this should preserve the tree
    pastStates.push_back(std::move(pastStates.back()->nextLevel[playerMoveIndex]));
    
    // clear the other moves, NOTE this should preserve the tree under the pointer
    pastStates[(int)pastStates.size()-2]->nextLevel.clear();
    
    pushMove(_x1,_y1,_x2,_y2);
    
    return true;
}

void StateTree::pushMove(int _x1,int _y1,int _x2,int _y2)
{
    char col1 = (char)(65+_x1);
    int row1 = _y1+1;
    char col2 = (char)(65+_x2);
    int row2 = _y2+1;
    
    std::string out = col1 + std::to_string(row1) + "->" + col2 + std::to_string(row2) + '\n';
    
    moveList.push_back(out);
}

// ---------- MOVEMENT ----------

void StateTree::pawnMove(GameState* _parentGS, int _x, int _y)
{
    if (_y == 0 || _y == 7) { return; } // fix this ATTENTION
    
    int moveDir = (_parentGS->whiteTurn ? 1:-1); // since white moves up board and black moves down board
    
    // FORWARD ONE
    if (_parentGS->board[_x][_y+moveDir] == PieceType::EMPTY)
    {
        std::unique_ptr<GameState> childGS (new GameState(_parentGS));
        childGS->board = _parentGS->board;
        // perform move
        childGS->board[_x][_y+moveDir] = childGS->board[_x][_y];
        childGS->board[_x][_y] = PieceType::EMPTY;
        // push as child to parent
        deepestLevel.push_back(childGS.get());
        _parentGS->nextLevel.push_back(std::move(childGS));
        
        // FORWARD TWO - done here bacause will only happen when Forward One can also happen
        if (_parentGS->board[_x][_y+2*moveDir] == PieceType::EMPTY && ((_y == 6 && moveDir == -1) || (_y == 1 && moveDir == 1)))
        {
            std::unique_ptr<GameState> childGS2 (new GameState(_parentGS));
            childGS2->board = _parentGS->board;
            // perform move
            childGS2->board[_x][_y+2*moveDir] = childGS2->board[_x][_y];
            childGS2->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS2.get());
            _parentGS->nextLevel.push_back(std::move(childGS2));
        }
    }

    // ATTACK - not edges
    if (_x>0 && _x<7)
    {
        // WHITE
        //left
        if (moveDir == 1 && (int)_parentGS->board[_x-1][_y+1] > COLOR_THRESHOLD)
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x-1][_y+1] = PieceType::W_PAWN;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
        //right
        if (moveDir == 1 && (int)_parentGS->board[_x+1][_y+1] > COLOR_THRESHOLD)
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x+1][_y+1] = PieceType::W_PAWN;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
        // BLACK
        //left
        if (moveDir == -1 && (int)_parentGS->board[_x-1][_y-1] < COLOR_THRESHOLD && _parentGS->board[_x-1][_y-1] != PieceType::EMPTY)
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x-1][_y-1] = PieceType::B_PAWN;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
        //right
        if (moveDir == -1 && (int)_parentGS->board[_x+1][_y-1] < COLOR_THRESHOLD && _parentGS->board[_x+1][_y-1] != PieceType::EMPTY)
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x+1][_y-1] = PieceType::B_PAWN;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
    }
    // ATTACK - edges
    else if (_x==0 || _x==7)
    {
        int attackDir = (_x==0 ? 1:-1); // NOTE if x!=0 then x==7
        // WHITE
        if (moveDir == 1 && (int)_parentGS->board[_x+attackDir][_y+moveDir] > COLOR_THRESHOLD)
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x+attackDir][_y+moveDir] = PieceType::W_PAWN;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
        // BLACK
        else if (moveDir == -1 && (int)_parentGS->board[_x+attackDir][_y+moveDir] < COLOR_THRESHOLD && _parentGS->board[_x+attackDir][_y+moveDir] != PieceType::EMPTY)
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x+attackDir][_y+moveDir] = PieceType::B_PAWN;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
    }
    else
    { std::cout << "Something broke in pawnMove()::attack\n"; }

    // EN PASSANT
    if (_x>0 && _x<7)
    {
        // WHITE
        //left
        if (moveDir == 1 && _parentGS->board[_x-1][_y] == PieceType::B_PAWN && madeDoubleMove(_parentGS,_x-1,_y))
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x-1][_y+1] = PieceType::W_PAWN;
            childGS->board[_x-1][_y] = PieceType::EMPTY;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
        //right
        if (moveDir == 1 && _parentGS->board[_x+1][_y] > PieceType::B_PAWN && madeDoubleMove(_parentGS,_x+1,_y))
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x+1][_y+1] = PieceType::W_PAWN;
            childGS->board[_x+1][_y] = PieceType::EMPTY;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
        // BLACK
        //left
        if (moveDir == -1 && _parentGS->board[_x-1][_y] == PieceType::W_PAWN && madeDoubleMove(_parentGS,_x-1,_y))
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x-1][_y-1] = PieceType::B_PAWN;
            childGS->board[_x-1][_y] = PieceType::EMPTY;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
        //right
        if (moveDir == -1 && _parentGS->board[_x+1][_y] == PieceType::W_PAWN && madeDoubleMove(_parentGS,_x+1,_y))
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x+1][_y-1] = PieceType::B_PAWN;
            childGS->board[_x+1][_y] = PieceType::EMPTY;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
    }
    // ATTACK - edges
    else if (_x==0 || _x==7)
    {
        int attackDir = (_x==0 ? 1:-1); // NOTE if x!=0 then x==7
        // WHITE
        if (moveDir == 1 && _parentGS->board[_x+attackDir][_y] == PieceType::B_PAWN && madeDoubleMove(_parentGS,_x+attackDir,_y))
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x+attackDir][_y+1] = PieceType::W_PAWN;
            childGS->board[_x+attackDir][_y] = PieceType::EMPTY;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
        // BLACK
        else if (moveDir == -1 && _parentGS->board[_x+attackDir][_y] == PieceType::W_PAWN && madeDoubleMove(_parentGS,_x+attackDir,_y))
        {
            // create new GameState and copy old one
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // perform move
            childGS->board[_x+attackDir][_y-1] = PieceType::B_PAWN;
            childGS->board[_x+attackDir][_y] = PieceType::EMPTY;
            childGS->board[_x][_y] = PieceType::EMPTY;
            // push as child to parent
            deepestLevel.push_back(childGS.get());
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
    }
    else
    { std::cout << "Something broke in pawnMove()::enPassant\n"; }
    
}

void StateTree::knightMove(GameState* _parentGS, int _x, int _y)
{
    // LEFT-UP
    processStateGen(_parentGS,_x,_y,_x-2,_y+1);
    // UP-LEFT
    processStateGen(_parentGS,_x,_y,_x-1,_y+2);
    // LEFT-DOWN
    processStateGen(_parentGS,_x,_y,_x-2,_y-1);
    // DOWN-LEFT
    processStateGen(_parentGS,_x,_y,_x-1,_y-2);
    // RIGHT-UP
    processStateGen(_parentGS,_x,_y,_x+2,_y+1);
    // UP-RIGHT
    processStateGen(_parentGS,_x,_y,_x+1,_y+2);
    // RIGHT-DOWN
    processStateGen(_parentGS,_x,_y,_x+2,_y-1);
    // DOWN-RIGHT
    processStateGen(_parentGS,_x,_y,_x+1,_y-2);
}

void StateTree::bishopMove(GameState* _parentGS, int _x, int _y)
{
    // UP-RIGHT
    int xTemp = _x+1;
    int yTemp = _y+1;
    while (int collision = validMove(_parentGS, xTemp, yTemp)) // integers 1 and 2 are considered true
    {
        processStateGen(_parentGS,_x,_y,xTemp,yTemp);
        xTemp++;
        yTemp++;
        
        if (collision == 2) { break; }
    }
    // UP-LEFT
    xTemp = _x-1;
    yTemp = _y+1;
    while (int collision = validMove(_parentGS, xTemp, yTemp)) // integers 1 and 2 are considered true
    {
        processStateGen(_parentGS,_x,_y,xTemp,yTemp);
        xTemp--;
        yTemp++;
        
        if (collision == 2) { break; }
    }
    // DOWN-RIGHT
    xTemp = _x+1;
    yTemp = _y-1;
    while (int collision = validMove(_parentGS, xTemp, yTemp)) // integers 1 and 2 are considered true
    {
        processStateGen(_parentGS,_x,_y,xTemp,yTemp);
        xTemp++;
        yTemp--;
        
        if (collision == 2) { break; }
    }
    // DOWN-LEFT
    xTemp = _x-1;
    yTemp = _y-1;
    while (int collision = validMove(_parentGS, xTemp, yTemp)) // integers 1 and 2 are considered true
    {
        processStateGen(_parentGS,_x,_y,xTemp,yTemp);
        xTemp--;
        yTemp--;
        
        if (collision == 2) { break; }
    }
}

void StateTree::rookMove(GameState* _parentGS, int _x, int _y)
{
    // UP
    int xTemp = _x;
    int yTemp = _y+1;
    while (int collision = validMove(_parentGS, xTemp, yTemp)) // integers 1 and 2 are considered true
    {
        processStateGen(_parentGS,_x,_y,xTemp,yTemp);
        yTemp++;
        
        if (collision == 2) { break; }
    }
    // DOWN
    xTemp = _x;
    yTemp = _y-1;
    while (int collision = validMove(_parentGS, xTemp, yTemp)) // integers 1 and 2 are considered true
    {
        processStateGen(_parentGS,_x,_y,xTemp,yTemp);
        yTemp--;
        
        if (collision == 2) { break; }
    }
    // LEFT
    xTemp = _x-1;
    yTemp = _y;
    while (int collision = validMove(_parentGS, xTemp, yTemp)) // integers 1 and 2 are considered true
    {
        processStateGen(_parentGS,_x,_y,xTemp,yTemp);
        xTemp--;
        
        if (collision == 2) { break; }
    }
    // RIGHT
    xTemp = _x+1;
    yTemp = _y;
    while (int collision = validMove(_parentGS, xTemp, yTemp)) // integers 1 and 2 are considered true
    {
        processStateGen(_parentGS,_x,_y,xTemp,yTemp);
        xTemp++;
        
        if (collision == 2) { break; }
    }
    // NOTE depending on where it is, moving the rook will disable a castling option
}

void StateTree::queenMove(GameState* _parentGS, int _x, int _y)
{
    bishopMove(_parentGS,_x,_y);
    rookMove(_parentGS,_x,_y);
}

void StateTree::kingMove(GameState* _parentGS, int _x, int _y)
{
    // UP-LEFT
    processStateGen(_parentGS,_x,_y,_x-1,_y+1);
    // UP
    processStateGen(_parentGS,_x,_y,_x,_y+1);
    // UP-RIGHT
    processStateGen(_parentGS,_x,_y,_x+1,_y+1);
    // RIGHT
    processStateGen(_parentGS,_x,_y,_x+1,_y);
    // RIGHT-DOWN
    processStateGen(_parentGS,_x,_y,_x+1,_y-1);
    // DOWN
    processStateGen(_parentGS,_x,_y,_x,_y-1);
    // DOWN-LEFT
    processStateGen(_parentGS,_x,_y,_x-1,_y-1);
    // LEFT
    processStateGen(_parentGS,_x,_y,_x-1,_y);
    
    // NOTE moving the king will disable castling on king's and queen's sides
}

// ---------- HELPERS -----------

void StateTree::evalCastleAbility(GameState* _parentGS)
{
    if (_parentGS->whiteTurn && _parentGS->inCheck_W) { return; }
    if (!_parentGS->whiteTurn && _parentGS->inCheck_B) { return; }
    
    // WHITE
    if (!_parentGS->kingsideRookMoved_W && !_parentGS->queensideRookMoved_W)
    {
        // queenside
        if (_parentGS->board[1][0] == PieceType::EMPTY && _parentGS->board[2][0] == PieceType::EMPTY && _parentGS->board[3][0] == PieceType::EMPTY)
        {
            // NOTE Queenside castle
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // make move
            childGS->board[2][0] = childGS->board[4][0]; // swap king
            childGS->board[4][0] = PieceType::EMPTY; // delete old king
            childGS->board[3][0] = childGS->board[0][0]; // swap rook
            childGS->board[0][0] = PieceType::EMPTY; // delete old rook
            childGS->castled_W = true;
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
            
        // kingside
        if (_parentGS->board[5][0] == PieceType::EMPTY && _parentGS->board[6][0] == PieceType::EMPTY)
        {
            // NOTE Kingside castle
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // make move
            childGS->board[6][0] = childGS->board[4][0]; // swap king
            childGS->board[4][0] = PieceType::EMPTY; // delete old king
            childGS->board[5][0] = childGS->board[7][0]; // swap rook
            childGS->board[7][0] = PieceType::EMPTY; // delete old rook
            childGS->castled_W = true;
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
    }
    
    // BLACK
    if (!_parentGS->kingsideRookMoved_B && !_parentGS->queensideRookMoved_B)
    {
        // queenside
        if (_parentGS->board[1][7] == PieceType::EMPTY && _parentGS->board[2][7] == PieceType::EMPTY && _parentGS->board[3][7] == PieceType::EMPTY)
        {
            // NOTE Queenside castle
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // make move
            childGS->board[2][7] = childGS->board[4][7]; // swap king
            childGS->board[4][7] = PieceType::EMPTY; // delete old king
            childGS->board[3][7] = childGS->board[0][7]; // swap rook
            childGS->board[0][7] = PieceType::EMPTY; // delete old rook
            childGS->castled_B = true;
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
            
        // kingside
        if (_parentGS->board[5][7] == PieceType::EMPTY && _parentGS->board[6][7] == PieceType::EMPTY)
        {
            // NOTE Kingside castle
            std::unique_ptr<GameState> childGS (new GameState(_parentGS));
            childGS->board = _parentGS->board;
            // make move
            childGS->board[6][7] = childGS->board[4][7]; // swap king
            childGS->board[4][7] = PieceType::EMPTY; // delete old king
            childGS->board[5][7] = childGS->board[7][7]; // swap rook
            childGS->board[7][7] = PieceType::EMPTY; // delete old rook
            childGS->castled_B = true;
            _parentGS->nextLevel.push_back(std::move(childGS));
        }
    }
}
    
void StateTree::evalCheckStatus(GameState* _gs) // ATTENTION not used right now
{}

void StateTree::evalPawnPromotions(GameState* _gs) // WARNING ONLY QUEEN PROMOTION RIGHT NOW
{
    int promotionRow = (_gs->whiteTurn ? 7:0);
    PieceType _gsPawnColor = (_gs->whiteTurn ? PieceType::W_PAWN : PieceType::B_PAWN);
    PieceType _gsQueenColor = (_gs->whiteTurn ? PieceType::W_QUEEN : PieceType::B_QUEEN);
    
    for (int x = 0; x < 8; x++)
    {
        if (_gs->board[x][promotionRow] == _gsPawnColor)
        {
            _gs->board[x][promotionRow] = _gsQueenColor ;
        }
    }
}

void StateTree::processStateGen(GameState* _parentGS, int _x1, int _y1, int _x2, int _y2)
{
    if (!validMove(_parentGS,_x2,_y2)) { return; } // don't do anything if requested move is invalid
    
    std::unique_ptr<GameState> childGS (new GameState(_parentGS));
    childGS->board = _parentGS->board;
    // make move
    childGS->board[_x2][_y2] = childGS->board[_x1][_y1];
    childGS->board[_x1][_y1] = PieceType::EMPTY;
    
    // check if rook move or kingmove
    // if the piece being moved is white or black rook, then check which side it's on (queenside or kingside)
    if ((_parentGS->board[_x1][_y1] == PieceType::W_ROOK || _parentGS->board[_x1][_y1] == PieceType::B_ROOK))
    {
        // the childState has a moved rook if execution reaches here
        if (childGS->whiteTurn)
        {
            if (_x1 == 0) { childGS->queensideRookMoved_W = true; }
            if (_x1 == 7) { childGS->kingsideRookMoved_W = true; }
        }
        else if (!childGS->whiteTurn)
        {
            if (_x1 == 0) { childGS->queensideRookMoved_B = true; }
            if (_x1 == 7) { childGS->kingsideRookMoved_B = true; }
        }
        else { std::cout << " ##### ERROR (1) in evalCastleAbility() ##### \n"; }
    }
    // check if king moves
    if ((_parentGS->board[_x1][_y1] == PieceType::W_KING || _parentGS->board[_x1][_y1] == PieceType::B_KING))
    {
        // the childState has a moved king if execution reaches here
        if (_x1==4 && childGS->whiteTurn)
        {
            childGS->queensideRookMoved_W = true;
            childGS->kingsideRookMoved_W = true;
        }
        else if (_x1==4 && !childGS->whiteTurn)
        {
            childGS->queensideRookMoved_B = true;
            childGS->kingsideRookMoved_B = true;
        }
        //else { std::cout << " ##### ERROR (2) in evalCastleAbility() ##### \n"; }
        // ATTENTION HERE ATTENTION HERE UNSUPRESS THIS ERROR AND FIND OUT WHAT'S UP... ONLY SHOULD HAPPEN WHEN KING MOVES BUT NOT OFF HIS ORIGINAL SQUARE... WHY IS THE ENGINE SPITTING OUT THIS ERROR SO EARLY? ERROR HAPPENS AFTER BLACK PLAYER'S FIRST MOVE WHICH DEPTH OF 4... I GUESS THAT COULD HAPPEN MAYBE
    }
    
    _parentGS->nextLevel.push_back(std::move(childGS));
}

int StateTree::validMove(GameState* _parentGS, int _x2, int _y2)
{
    // stay on the board
    if (_x2 < 0 || _x2 > 7 || _y2 < 0 || _y2 > 7 ) { return 0; }
    else if (_parentGS->board[_x2][_y2] == PieceType::EMPTY) { return 1; } // if empty square
    else if (_parentGS->whiteTurn && (int)_parentGS->board[_x2][_y2] > COLOR_THRESHOLD) { return 2; } // if enemy square (whiteTurn)
    else if (!_parentGS->whiteTurn && (int)_parentGS->board[_x2][_y2] < COLOR_THRESHOLD) { return 2; } // if enemy square (not whiteTurn) NOTE don't need to test for empty since it's already been tested
    else { return 0; } // otherwise it will be occupied by friendly piece meaning invalid move
}

bool StateTree::madeDoubleMove(GameState* _gs, int _x, int _y)
{
    if (_y != 3 && _y != 4) // make sure the correct pieces are being tested
    {
        return false;
    }
    
    if (_gs->whiteTurn)
    {
        // make sure piece at current location is black
        if (_gs->board[_x][_y] != PieceType::B_PAWN) { return false; }
        // make sure piece at initial pawn spot is empty this turn
        if (_gs->board[_x][_y+2] != PieceType::EMPTY) { return false; }
        // make sure piece at current location was empty last turn
        if (_gs->parent->board[_x][_y] != PieceType::EMPTY) { return false; }
        // make sure piece at initial pawn spot was black pawn last turn
        if (_gs->parent->board[_x][_y+2] != PieceType::B_PAWN) { return false; }
        
        // if all conditions pass return true
        return true;
    }
    else if (!_gs->whiteTurn)
    {
        // make sure piece at current location is black
        if (_gs->board[_x][_y] != PieceType::W_PAWN) { return false; }
        // make sure piece at initial pawn spot is empty this turn
        if (_gs->board[_x][_y-2] != PieceType::EMPTY) { return false; }
        // make sure piece at current location was empty last turn
        if (_gs->parent->board[_x][_y] != PieceType::EMPTY) { return false; }
        // make sure piece at initial pawn spot was black pawn last turn
        if (_gs->parent->board[_x][_y-2] != PieceType::W_PAWN) { return false; }
        
        // if all conditions pass return true
        return true;
    }
    else { std::cout << " ##### Error @ madeDoubleMove() #####\n"; return false; }
}

void StateTree::evaluate(GameState* _gs)
{
    float evaluation = 0;
    
    if (_gs->board[6][0] == PieceType::W_KING) { evaluation += 1.3; }
    if (_gs->board[6][7] == PieceType::W_KING) { evaluation -= 1.3; }
    
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            PieceType piece = _gs->board[x][y];
            switch(piece)
            {
                case (PieceType::W_PAWN):
                    evaluation += 1;
                    evaluation += 0.1*(y-1);
                    
                    if ((x==3 || x==4) && y==1) { evaluation -= 0.2; }
                    break;
                case (PieceType::W_ROOK):
                    evaluation += 5;
                    break;
                case (PieceType::W_KNIGHT):
                    evaluation += 3;
                    evaluation += 0.2*(y-1);
                    if (x==0 || x==7) { evaluation -= 1.5; }
                    if (y==0) { evaluation -= 0.6; }
                    break;
                case (PieceType::W_BISHOP):
                    evaluation += 3;
                    evaluation += 0.1*(y-1);
                    if (y==0) { evaluation -= 0.5; }
                    break;
                case (PieceType::W_QUEEN):
                    evaluation += 9;
                    break;
                case (PieceType::W_KING):
                    evaluation += 100000;
                    break;
                case (PieceType::B_PAWN):
                    evaluation -= 1;
                    evaluation += 0.1*(6-y);
                    
                    if ((x==3 || x==4) && y==6) { evaluation += 0.2; }
                    break;
                case (PieceType::B_ROOK):
                    evaluation -= 5;
                    break;
                case (PieceType::B_KNIGHT):
                    evaluation -= 3;
                    evaluation += 0.2*(6-y);
                    if (x==0 || x==7) { evaluation += 1.5; }
                    if (y==7) { evaluation += 0.6; }
                    break;
                case (PieceType::B_BISHOP):
                    evaluation -= 3;
                    evaluation += 0.1*(6-y);
                    if (y==7) { evaluation += 0.5; }
                    break;
                case (PieceType::B_QUEEN):
                    evaluation -= 9;
                    break;
                case (PieceType::B_KING):
                    evaluation -= 100000;
                    break;
                case (PieceType::EMPTY):
                    break;
                default:
                    std::cout << " ##### Error in evaluate() ##### \n";
                    break;
            }
        }
    }
    
    _gs->evaluation = evaluation;
}
