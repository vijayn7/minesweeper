#include "IBoardSolver.h"
#include "Board.h"
#include "solverUtilities.cpp"
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

    void queueRevealCell(pair<int, int> cell) {
        cellsToReveal.push(cell);
    }

    void queueFlagCell(pair<int, int> cell) {
        cellsToFlag.push(cell);
    }

    void queueRevealCells(const vector<pair<int, int>>& cells) {
        for (const auto& cell : cells) {
            cellsToReveal.push(cell);
        }
    }

    void queueFlagCells(const vector<pair<int, int>>& cells) {
        for (const auto& cell : cells) {
            cellsToFlag.push(cell);
        }
    }


    void nextRevealQueue(std::pair<int, int> cellParam) {
        gameBoard.setClickMode(IBoardSolver::REVEAL);
        auto cell = cellsToReveal.front();
        cellsToReveal.pop();
        gameBoard.setSelectedCell(cell.first, cell.second);
        renderer->startClickAnimation();
        gameBoard.algoClick();
        moveClock.restart();
        return;
    }

    void nextFlagQueue(std::pair<int, int> cellParam) {
        gameBoard.setClickMode(IBoardSolver::FLAG);
        auto cell = cellsToFlag.front();
        cellsToFlag.pop();
        gameBoard.setSelectedCell(cell.first, cell.second);
        renderer->startClickAnimation();
        gameBoard.algoClick();
        moveClock.restart();
        return;
    }

    bool preformNextAction() {
        if (!cellsToReveal.empty()) {
            nextRevealQueue(cellsToReveal.front());
            return true;
        }

        if (!cellsToFlag.empty()) {
            nextFlagQueue(cellsToFlag.front());
            return true;
        }

        return false;
    }

    void randomGuess() {
        vector<pair<int, int>> unrevealedCells = gameBoard.getAllUnrevealedCells();
        if (unrevealedCells.empty()) return; // No moves available
        pair<int, int> move = solverUtilities::makeRandomMove(unrevealedCells);
        queueRevealCell(move);
    }

    // if a cell has a 1 and only one unrevealed neighbor, flag that neighbor
    void flagCornersOfOnes() {
        vector<pair<int, int>> ones = gameBoard.getOnes();

        for (const auto& cell : ones) {
            vector<pair<int, int>> unrevealedNeighbors = gameBoard.getUnrevealedNeighbors(cell.first, cell.second);
            if (unrevealedNeighbors.size() == 1) {
                queueFlagCell(unrevealedNeighbors[0]);
            }
        }
    }

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

        // Go through queued actions first

        if (preformNextAction()) {
            return;
        }

        // both queues are empty, make new decisions

        //Flag corners of ones
        flagCornersOfOnes();

        // random guess
        randomGuess();

        // Execute the move
        preformNextAction();

        // Reset the clock after making a move
        moveClock.restart();
    }

};