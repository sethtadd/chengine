#include <string>
#include <iostream>

#include "StateTree.hpp"

/*
 * NOTE Different compilations have different effects!
 * 
 * Full Testing Compilation (SLOW):
 * g++ -std=c++14 -Wall -fsanitize=undefined,address -g Main.cpp StateTree.cpp && ./a.out
 * 
 * Distro Compilation (FAST):
 * g++ -std=c++14 -Ofast Main.cpp StateTree.cpp && ./a.out
 * 
 */

// ATTENTION genLevels(x) then minimaxEval() must be called before pushComputerState()!!!

// ATTENTION Relevent tree should be being preserved when moves are made so keep that in mind when generating move levels

// ATTENTION Check that post on chess.com that you first read to see evaluation function suggestions

void getPlayerMove(int &_x1, int &_y1, int &_x2, int &_y2);

int main()
{
    StateTree st;
    
    char playerIsWhite;
    std::string str1;
    std::cout << "Player as white? (y/n): ";
    std::getline(std::cin, str1);
    playerIsWhite = str1[0];
    
    if (playerIsWhite != 'y' && playerIsWhite != 'n')
    {
        std::cout << "Bad answer!\n";
        return 1;
    }
    
    int levels;
    std::string str2;
    std::cout << "Number of computer levels? (between 1 and 4, inclusive): ";
    std::getline(std::cin, str2);
    levels = std::stoi(str2);
    
    if (levels < 1 || levels > 4)
    {
        std::cout << "Can't do that!\n";
        return 1;
    }
    
    st.genLevels(levels);
    
    st.printCurrent();
    
    // NOTE assuming abs(evaluation) > 50000 means king is dead since king==100000
    while (st.pastStates[st.pastStates.size()-1]->evaluation > -50000 && st.pastStates[st.pastStates.size()-1]->evaluation < 50000)
    {
        if (playerIsWhite == 'n')
        {
            st.pushComputerState();
            std::cout << st.moveList.back();
            st.printCurrent();
            std::cout << st.pastStates[st.pastStates.size()-1]->evaluation << '\n';
            st.genLevels(1);
        }
        
        bool validMove = false;
        while (!validMove)
        {
            int x1,y1,x2,y2;
            getPlayerMove(x1,y1,x2,y2);
            validMove = st.pushPlayerState(x1,y1,x2,y2);
            st.printCurrent();
            std::cout << st.pastStates[st.pastStates.size()-1]->evaluation << '\n';
        }
        st.genLevels(1);
        
        if (playerIsWhite == 'y')
        {
            st.pushComputerState();
            st.printCurrent();
            std::cout << st.pastStates[st.pastStates.size()-1]->evaluation << '\n';
            st.genLevels(1);
        }
    }
    
    return 0;
}

void getPlayerMove(int &_x1, int &_y1, int &_x2, int &_y2)
{
    std::string move1;
    std::string move2;
    std::cout << "Move From\n>";
    std::getline(std::cin, move1);
    _x1 = (int)move1[0] - 'A';
    _y1 = (int)move1[1] - '0'-1;
    std::cout << "Move To\n>";
    std::getline(std::cin, move2);
    _x2 = (int)move2[0] - 'A';
    _y2 = (int)move2[1] - '0'-1;
}
