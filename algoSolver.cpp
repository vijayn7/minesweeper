#include "IBoardSolver.h"
#include "Board.h"
#include <queue>

using namespace std;

class BoardRenderer;

class algoSolver {
private:
    IBoardSolver& gameBoard;
    BoardRenderer* renderer;
    bool firstMove = true;
    sf::Clock moveClock;
    float moveDelay = 0.5f; // Delay between moves in seconds
    bool waitingToClick = false;
    sf::Clock clickDelay;
    float preClickDelay = 0.2f; // Show selection before clicking

    queue<std::pair<int, int>> cellsToReveal;
    queue<std::pair<int, int>> cellsToFlag;

public:

    algoSolver(IBoardSolver& b, BoardRenderer* r) : gameBoard(b), renderer(r) {}

    void makeMove() {
        // Only make a move if enough time has passed
        if (moveClock.getElapsedTime().asSeconds() < moveDelay) {
            return;
        }

        // check if game is over
        if (gameBoard.isGameOver()) { 
            gameBoard.reset(); 
            firstMove = true;
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
                if (renderer) renderer->startClickAnimation();
                waitingToClick = false;
                
                if (gameBoard.getCellVal(gameBoard.getSelectedX(), gameBoard.getSelectedY()) == Board::ZERO) {
                    firstMove = false;
                }
            }
        }
    }


};