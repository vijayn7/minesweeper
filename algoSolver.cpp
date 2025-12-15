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
    float speed = 1.0f; // Speed multiplier (1.0 = normal, 2.0 = 2x faster, 0.5 = 2x slower)
    float baseMoveDelay = 0.5f; // Base delay between moves
    bool waitingToClick = false;
    sf::Clock clickDelay;
    float preClickDelay = 0.2f; // Show selection before clicking
    
    float getMoveDelay() const {
        return baseMoveDelay / speed;
    }

    queue<std::pair<int, int>> cellsToReveal;
    queue<std::pair<int, int>> cellsToFlag;
    set<std::pair<int, int>> queuedForFlagging; // Track cells already queued for flagging
    bool algoActive = true; // Controls whether the algorithm should continue making moves
    bool inRandomGuessPhase = true; // Track if we're still in random guessing phase
    bool nextRevealIsGuess = false; // Track if the next reveal is from a random guess
    
    // Statistics tracking
    int wins = 0;
    int losses = 0;
    bool gameWasCounted = false; // Prevent counting the same game multiple times

    void queueRevealCell(pair<int, int> cell) {
        // Don't queue if already revealed
        if (gameBoard.searchCell(cell.first, cell.second)) {
            return;
        }
        cellsToReveal.push(cell);
    }

    void queueFlagCell(pair<int, int> cell) {
        // Don't queue if already revealed or already queued for flagging
        if (gameBoard.searchCell(cell.first, cell.second)) {
            return;
        }
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
        // Trigger selection animation from current position to target
        int oldX = gameBoard.getSelectedX();
        int oldY = gameBoard.getSelectedY();
        gameBoard.setSelectedCell(cell.first, cell.second);
        renderer->startSelectionAnimation(oldX, oldY);
        if (renderer && nextRevealIsGuess) {
            renderer->setGuessMove(true);
            nextRevealIsGuess = false; // Reset flag after using it
        }
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
        // Trigger selection animation from current position to target
        int oldX = gameBoard.getSelectedX();
        int oldY = gameBoard.getSelectedY();
        gameBoard.setSelectedCell(cell.first, cell.second);
        renderer->startSelectionAnimation(oldX, oldY);
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
        nextRevealIsGuess = true; // Mark that the next reveal is a guess
        queueRevealCell(move);
    }

    int numFlaggedNeighbors(int x, int y) {
        vector<pair<int, int>> flaggedNeighbors = gameBoard.getFlaggedNeighbors(x, y);
        return static_cast<int>(flaggedNeighbors.size());
    }

    vector<pair<int, int>> getAllRevealedNumberedCells() {
        vector<pair<int, int>> numberedCells;
        int gridSize = gameBoard.getGridSize();
        for (int x = 0; x < gridSize; x++) {
            for (int y = 0; y < gridSize; y++) {
                if (gameBoard.searchCell(x, y)) {
                    int val = gameBoard.getCellVal(x, y);
                    if (val >= 1 && val <= 8) {
                        numberedCells.emplace_back(x, y);
                    }
                }
            }
        }
        return numberedCells;
    }

    // if a numbered cell has all its mines in unrevealed neighbors, flag them
    void flagCornersOfOnes() {
        vector<pair<int, int>> numberedCells = getAllRevealedNumberedCells();

        for (const auto& cell : numberedCells) {
            int x = cell.first;
            int y = cell.second;
            
            // Show inspection box
            if (renderer) renderer->startInspection(x, y);

            int cellValue = gameBoard.getCellVal(x, y);
            vector<pair<int, int>> unrevealedNeighbors = gameBoard.getUnrevealedNeighbors(x, y);
            vector<pair<int, int>> flaggedNeighbors = gameBoard.getFlaggedNeighbors(x, y);
            int flaggedCount = static_cast<int>(flaggedNeighbors.size());
            int unrevealedCount = static_cast<int>(unrevealedNeighbors.size());

            // If unrevealed + flagged == cellValue and we have unflagged unrevealed cells, flag them one at a time
            if (unrevealedCount + flaggedCount == cellValue && unrevealedCount > 0) {
                for (const auto& neighbor : unrevealedNeighbors) {
                    // Only queue if not already flagged
                    bool isFlagged = false;
                    for (const auto& flagged : flaggedNeighbors) {
                        if (flagged.first == neighbor.first && flagged.second == neighbor.second) {
                            isFlagged = true;
                            break;
                        }
                    }
                    if (!isFlagged) {
                        queueFlagCell(neighbor);
                        return; // Only queue one cell at a time
                    }
                }
            }
        }
    }



    // reveal remaining neighbors of cells where the number of flagged neighbors equals the cell's value
    void revealSatisfiedCells() {
        vector<pair<int, int>> numberedCells = getAllRevealedNumberedCells();

        for (const auto& cell : numberedCells) {
            int x = cell.first;
            int y = cell.second;
            
            // Show inspection box
            if (renderer) renderer->startInspection(x, y);

            int cellValue = gameBoard.getCellVal(x, y);
            int flaggedCount = numFlaggedNeighbors(x, y);

            if (flaggedCount == cellValue) {
                vector<pair<int, int>> unrevealedNeighbors = gameBoard.getUnrevealedNeighbors(x, y);
                vector<pair<int, int>> flaggedNeighbors = gameBoard.getFlaggedNeighbors(x, y);
                
                for (const auto& neighbor : unrevealedNeighbors) {
                    // Only queue if not flagged
                    bool isFlagged = false;
                    for (const auto& flagged : flaggedNeighbors) {
                        if (flagged.first == neighbor.first && flagged.second == neighbor.second) {
                            isFlagged = true;
                            break;
                        }
                    }
                    if (!isFlagged) {
                        queueRevealCell(neighbor);
                        return; // Only queue one cell at a time
                    }
                }
            }
        }
    }

    void resetSolverState() {
        // Track game result before resetting
        if (!gameWasCounted && gameBoard.isGameOver()) {
            if (gameBoard.getGameState() == IBoardSolver::WON) {
                wins++;
                cout << "Game Won! Total: " << wins << " wins, " << losses << " losses" << endl;
            } else if (gameBoard.getGameState() == IBoardSolver::LOST) {
                losses++;
                cout << "Game Lost! Total: " << wins << " wins, " << losses << " losses" << endl;
            }
            gameWasCounted = true;
        }
        
        while (!cellsToReveal.empty()) cellsToReveal.pop();
        while (!cellsToFlag.empty()) cellsToFlag.pop();
        queuedForFlagging.clear();
        if (renderer) renderer->stopInspection();
        gameBoard.reset();
        firstMove = true;
        algoActive = true;
        inRandomGuessPhase = true;
        gameWasCounted = false; // Reset for next game
        nextRevealIsGuess = false; // Reset guess flag
    }

    // Cell - flagged neighbors == 1 && unrev neighbors - flagged neighbors == 1
    // flag the unrev neighbor that is not flagged
    void subtractionFlagging() {
        vector<pair<int, int>> numberedCells = getAllRevealedNumberedCells();

        for (const auto& cell : numberedCells) {
            int x = cell.first;
            int y = cell.second;
            
            // Show inspection box
            if (renderer) renderer->startInspection(x, y);

            int cellValue = gameBoard.getCellVal(x, y);
            vector<pair<int, int>> unrevealedNeighbors = gameBoard.getUnrevealedNeighbors(x, y);
            vector<pair<int, int>> flaggedNeighbors = gameBoard.getFlaggedNeighbors(x, y);
            int flaggedCount = static_cast<int>(flaggedNeighbors.size());
            int unrevCount = static_cast<int>(unrevealedNeighbors.size());

            if (flaggedCount == cellValue - 1 && unrevCount - flaggedCount == 1) {
                for (const auto& neighbor : unrevealedNeighbors) {
                    // Only queue if not flagged
                    bool isFlagged = false;
                    for (const auto& flagged : flaggedNeighbors) {
                        if (flagged.first == neighbor.first && flagged.second == neighbor.second) {
                            isFlagged = true;
                            break;
                        }
                    }
                    if (!isFlagged) {
                        queueFlagCell(neighbor);
                        return; // Only queue one cell at a time
                    }
                }
            }
        }
    }

    void processGrid() {
        // Only run each function if both queues are still empty
        
        // Flags
        if (cellsToReveal.empty() && cellsToFlag.empty()) {
            flagCornersOfOnes();
        }
        
        // Subtraction flagging
        if (cellsToReveal.empty() && cellsToFlag.empty()) {
            subtractionFlagging();
        }

        // Reveals
        if (cellsToReveal.empty() && cellsToFlag.empty()) {
            revealSatisfiedCells();
        }
    }

public:

    algoSolver(IBoardSolver& b, BoardRenderer* r) : gameBoard(b), renderer(r) {}
    
    void setSpeed(float newSpeed) {
        speed = std::max(0.1f, std::min(10.0f, newSpeed)); // Clamp between 0.1x and 10x
        if (renderer) {
            renderer->setAnimationSpeed(speed);
        }
    }
    
    float getSpeed() const {
        return speed;
    }
    
    int getWins() const { return wins; }
    int getLosses() const { return losses; }
    int getTotalGames() const { return wins + losses; }

    void makeMove() {
        // Only make a move if enough time has passed
        if (moveClock.getElapsedTime().asSeconds() < getMoveDelay()) {
            return;
        }

        // check if game is over
        if (gameBoard.isGameOver()) { 
            resetSolverState();
            return;
        }

        // Stop if algorithm is inactive
        if (!algoActive) {
            if (renderer) renderer->stopInspection();
            return;
        }

        // Go through queued actions first
        if (preformNextAction()) {
            return;
        }

        // Both queues are empty, check what phase we're in
        // Clear inspection box when not actively searching
        if (renderer) renderer->stopInspection();
        
        if (inRandomGuessPhase) {
            // Check if we have any 0 cells (cells with no adjacent mines)
            bool hasZeroCells = false;
            int gridSize = gameBoard.getGridSize();
            
            // Look for zero cells among all revealed cells
            for (int x = 0; x < gridSize; x++) {
                for (int y = 0; y < gridSize; y++) {
                    if (gameBoard.searchCell(x, y) && gameBoard.getCellVal(x, y) == 0) {
                        hasZeroCells = true;
                        break;
                    }
                }
                if (hasZeroCells) break;
            }
            
            if (hasZeroCells) {
                // We have zero cells, transition to algorithm phase
                cout << "Found 0 cell. Starting algorithm phase." << endl;
                inRandomGuessPhase = false;
                processGrid();
                
                // If processGrid found actions, perform them
                if (preformNextAction()) {
                    moveClock.restart();
                    return;
                }
                
                // If no actions found after processing, make a random guess
                cout << "No logical moves found after transition. Making random guess." << endl;
                randomGuess();
                if (preformNextAction()) {
                    moveClock.restart();
                    return;
                }
                
                // If no moves available at all, end algo
                cout << "No moves available. Stopping algorithm." << endl;
                if (renderer) renderer->stopInspection();
                algoActive = false;
                return;
            }
            
            // Still in random guess phase, make a random guess
            randomGuess();
            preformNextAction();
            moveClock.restart();
        } else {
            // We're in algorithm phase, queues are empty, so process grid again
            processGrid();
            
            // If processGrid found actions, perform them
            if (preformNextAction()) {
                moveClock.restart();
                return;
            }
            
            // If no actions found after processing, make a random guess
            cout << "No logical moves found. Making random guess." << endl;
            randomGuess();
            if (preformNextAction()) {
                moveClock.restart();
                return;
            }
            
            // If no moves available at all, end algo
            cout << "No moves available. Stopping algorithm." << endl;
            if (renderer) renderer->stopInspection();
            algoActive = false;
        }
    }

};