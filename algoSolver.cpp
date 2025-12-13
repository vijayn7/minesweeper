#include "IBoardSolver.h"
#include "Board.h"
#include "solverUtilities.cpp"
#include <queue>
#include <set>
#include <iostream>

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
    set<std::pair<int, int>> queuedForFlagging; // Track cells already queued for flagging
    bool algoActive = true; // Controls whether the algorithm should continue making moves
    bool inRandomGuessPhase = true; // Track if we're still in random guessing phase

    void queueRevealCell(pair<int, int> cell) {
        cellsToReveal.push(cell);
    }

    void queueFlagCell(pair<int, int> cell) {
        if (queuedForFlagging.find(cell) == queuedForFlagging.end()) {
            cellsToFlag.push(cell);
            queuedForFlagging.insert(cell);
        }
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
        if (cellsToReveal.empty()) {
            cout << "Reveal queue emptied" << endl;
        }
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
        if (cellsToFlag.empty()) {
            cout << "Flag queue emptied" << endl;
        }
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

        for (const auto& oneCell : ones) {
            int x = oneCell.first;
            int y = oneCell.second;

            vector<pair<int, int>> unrevealedNeighbors = gameBoard.getUnrevealedNeighbors(x, y);
            vector<pair<int, int>> flaggedNeighbors = gameBoard.getFlaggedNeighbors(x, y);

            if (unrevealedNeighbors.size() == 1 && flaggedNeighbors.size() == 0) {
                queueFlagCell(unrevealedNeighbors[0]);
            }
        }
    }

    void resetSolverState() {
        while (!cellsToReveal.empty()) cellsToReveal.pop();
        while (!cellsToFlag.empty()) cellsToFlag.pop();
        queuedForFlagging.clear();
        gameBoard.reset();
        firstMove = true;
        algoActive = true;
        inRandomGuessPhase = true;
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
            resetSolverState();
            return;
        }

        // Stop if algorithm is inactive
        if (!algoActive) {
            return;
        }

        // Go through queued actions first
        if (preformNextAction()) {
            return;
        }

        // Both queues are empty, check what phase we're in
        
        if (inRandomGuessPhase) {
            // Try to find flags for corners of ones
            flagCornersOfOnes();
            
            // If we found something to flag, transition out of random guess phase
            if (!cellsToFlag.empty()) {
                cout << "Algo found flags! Transitioning to processing phase." << endl;
                inRandomGuessPhase = false;
                preformNextAction();
                moveClock.restart();
                return;
            }
            
            // Still in random guess phase, make a random guess
            randomGuess();
            preformNextAction();
            moveClock.restart();
        } else {
            // We've completed the queue after finding flags, stop the algorithm
            cout << "Queue completed. Stopping algorithm." << endl;
            algoActive = false;
        }
    }

};