#include "IBoardSolver.h"
#include "Board.h"

class algoSolver {
private:
    IBoardSolver& gameBoard;
    bool firstMove = true;
    sf::Clock moveClock;
    float moveDelay = 0.5f; // Delay between moves in seconds
    bool waitingToClick = false;
    sf::Clock clickDelay;
    float preClickDelay = 0.2f; // Show selection before clicking

public:

    algoSolver(IBoardSolver& b) : gameBoard(b) {}

    void makeMove() {
        // Only make a move if enough time has passed
        if (moveClock.getElapsedTime().asSeconds() < moveDelay) {
            return;
        }

        makeFirstMove();
        
        // Reset the clock after making a move
        moveClock.restart();
    }

    void makeFirstMove() {
        // randomly reveal until we get a 0 cell
        if (firstMove) {
            if (!waitingToClick) {
                // Select a new random cell
                int x = rand() % gameBoard.getGridSize();
                int y = rand() % gameBoard.getGridSize();
                
                if (gameBoard.getPlayerView()[x][y] == -1) { // Unrevealed
                    gameBoard.setSelectedCell(x, y);
                    waitingToClick = true;
                    clickDelay.restart();
                }
            } else if (clickDelay.getElapsedTime().asSeconds() >= preClickDelay) {
                // Enough time has passed, now click
                gameBoard.algoClick();
                waitingToClick = false;
                
                if (gameBoard.getCellVal(gameBoard.getSelectedX(), gameBoard.getSelectedY()) == Board::ZERO) {
                    firstMove = false;
                }
            }
        }
    }


};