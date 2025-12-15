#include "IBoardSolver.h"
#include "Board.h"
#include "solverUtilities.cpp"
#include <queue>
#include <set>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

class BoardRenderer;

class heatmapSolver {
private:
    IBoardSolver& gameBoard;
    BoardRenderer* renderer;
    bool firstMove = true;
    sf::Clock moveClock;
    float speed = 1.0f;
    float baseMoveDelay = 0.5f;
    bool waitingToClick = false;
    sf::Clock clickDelay;
    float preClickDelay = 0.2f;
    
    float getMoveDelay() const {
        return baseMoveDelay / speed;
    }

    queue<std::pair<int, int>> cellsToReveal;
    queue<std::pair<int, int>> cellsToFlag;
    set<std::pair<int, int>> queuedForFlagging;
    bool algoActive = false; // Solver stopped by default
    bool inRandomGuessPhase = true;
    bool nextRevealIsGuess = false;
    
    // Statistics tracking
    int wins = 0;
    int losses = 0;
    bool gameWasCounted = false;
    
    // Stuck detection
    int consecutiveEmptyQueues = 0;
    const int MAX_EMPTY_QUEUE_ATTEMPTS = 5;
    
    // Safe start mode
    bool safeStartEnabled = false;

    void queueRevealCell(pair<int, int> cell) {
        // Don't queue if already revealed
        if (gameBoard.searchCell(cell.first, cell.second)) {
            return;
        }
        // Don't queue if flagged (to avoid revealing cells we think are bombs)
        vector<pair<int, int>> flaggedCells = gameBoard.getAllFlaggedCells();
        for (const auto& flagged : flaggedCells) {
            if (flagged.first == cell.first && flagged.second == cell.second) {
                return;
            }
        }
        cellsToReveal.push(cell);
    }

    void queueFlagCell(pair<int, int> cell) {
        if (gameBoard.searchCell(cell.first, cell.second)) {
            return;
        }
        
        // Don't flag if we've already reached the maximum number of flags
        const int totalMines = 10; // Standard minesweeper 9x9 grid has 10 mines
        vector<pair<int, int>> currentlyFlagged = gameBoard.getAllFlaggedCells();
        if (currentlyFlagged.size() >= static_cast<size_t>(totalMines)) {
            cout << "[Heatmap] Already have " << currentlyFlagged.size() << " flags (max: " << totalMines << "). Not flagging more." << endl;
            return;
        }
        
        if (queuedForFlagging.find(cell) == queuedForFlagging.end()) {
            cellsToFlag.push(cell);
            queuedForFlagging.insert(cell);
        }
    }

    void nextRevealQueue(std::pair<int, int> cellParam) {
        gameBoard.setClickMode(IBoardSolver::REVEAL);
        auto cell = cellsToReveal.front();
        cellsToReveal.pop();
        if (cellsToReveal.empty()) {
            cout << "[Heatmap] Reveal queue emptied" << endl;
        }
        int oldX = gameBoard.getSelectedX();
        int oldY = gameBoard.getSelectedY();
        gameBoard.setSelectedCell(cell.first, cell.second);
        renderer->startSelectionAnimation(oldX, oldY);
        if (renderer && nextRevealIsGuess) {
            renderer->setGuessMove(true);
            nextRevealIsGuess = false;
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
            cout << "[Heatmap] Flag queue emptied" << endl;
        }
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

    void randomGuessUntilZero() {
        vector<pair<int, int>> unrevealedCells = gameBoard.getAllUnrevealedCells();
        if (unrevealedCells.empty()) return;
        pair<int, int> move = solverUtilities::makeRandomMove(unrevealedCells);
        nextRevealIsGuess = true;
        queueRevealCell(move);
    }

    int numFlaggedNeighbors(int x, int y) {
        vector<pair<int, int>> flaggedNeighbors = gameBoard.getFlaggedNeighbors(x, y);
        return static_cast<int>(flaggedNeighbors.size());
    }

    // Use subtraction logic to find cells that must be safe based on satisfied numbered cells
    void applySubtractionLogic() {
        vector<pair<int, int>> numberedCells = getAllRevealedNumberedCells();
        int cellsQueued = 0;
        
        // First pass: reveal safe cells from satisfied numbered cells
        for (const auto& numCell : numberedCells) {
            int x = numCell.first;
            int y = numCell.second;
            int cellValue = gameBoard.getCellVal(x, y);
            int flaggedCount = numFlaggedNeighbors(x, y);
            
            // If this cell's mine count is satisfied by flags, all other unrevealed neighbors are safe
            if (flaggedCount == cellValue) {
                vector<pair<int, int>> unrevealedNeighbors = gameBoard.getUnrevealedNeighbors(x, y);
                vector<pair<int, int>> flaggedNeighbors = gameBoard.getFlaggedNeighbors(x, y);
                
                for (const auto& neighbor : unrevealedNeighbors) {
                    // Check if this neighbor is flagged
                    bool isFlagged = false;
                    for (const auto& flagged : flaggedNeighbors) {
                        if (flagged.first == neighbor.first && flagged.second == neighbor.second) {
                            isFlagged = true;
                            break;
                        }
                    }
                    
                    // If not flagged, it's safe to reveal
                    if (!isFlagged) {
                        queueRevealCell(neighbor);
                        cellsQueued++;
                        if (!cellsToReveal.empty()) {
                            cout << "[Heatmap] Subtraction logic: Cell (" << x << "," << y 
                                 << ") with value " << cellValue << " has " << flaggedCount 
                                 << " flags. Revealing safe neighbor (" << neighbor.first << "," << neighbor.second << ")" << endl;
                            return; // Queue one at a time
                        }
                    }
                }
            }
        }
        
        if (cellsQueued > 0) {
            cout << "[Heatmap] Subtraction logic queued " << cellsQueued << " safe cells" << endl;
            consecutiveEmptyQueues = 0;
        }
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

    // Calculate heatmap probabilities for all unrevealed cells
    // Improved to weight based on information density (revealed neighbors)
    map<pair<int, int>, float> calculateHeatmap() {
        map<pair<int, int>, float> heatmap;
        map<pair<int, int>, int> contributionCount; // Track how many cells contribute to each unrevealed cell
        vector<pair<int, int>> unrevealedCells = gameBoard.getAllUnrevealedCells();
        vector<pair<int, int>> numberedCells = getAllRevealedNumberedCells();
        
        // Initialize all unrevealed cells with 0 probability
        for (const auto& cell : unrevealedCells) {
            heatmap[cell] = 0.0f;
            contributionCount[cell] = 0;
        }
        
        // For each numbered cell, calculate contribution to surrounding unrevealed cells
        for (const auto& numCell : numberedCells) {
            int x = numCell.first;
            int y = numCell.second;
            int cellValue = gameBoard.getCellVal(x, y);
            
            vector<pair<int, int>> unrevealedNeighbors = gameBoard.getUnrevealedNeighbors(x, y);
            int unrevealedCount = static_cast<int>(unrevealedNeighbors.size());
            
            // Calculate remaining mines (excluding flagged cells)
            int flaggedCount = numFlaggedNeighbors(x, y);
            int remainingMines = cellValue - flaggedCount;
            
            // Skip if this cell is satisfied (all mines accounted for)
            if (remainingMines <= 0) {
                continue;
            }
            
            // Skip if no unrevealed neighbors
            if (unrevealedCount == 0) {
                continue;
            }
            
            // Calculate base probability for this numbered cell
            float baseProbability = static_cast<float>(remainingMines) / static_cast<float>(unrevealedCount);
            
            // Weight factor: cells with more revealed neighbors (more info) get higher weight
            // A cell with 8 neighbors (max info) vs 3 neighbors (less info)
            int gridSize = gameBoard.getGridSize();
            int totalPossibleNeighbors = 8; // Max neighbors in minesweeper
            
            // Count how many neighbors this numbered cell has (corner/edge cells have fewer)
            int actualNeighborCount = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < gridSize && ny >= 0 && ny < gridSize) {
                        actualNeighborCount++;
                    }
                }
            }
            
            // Calculate how many of this cell's neighbors are revealed
            int revealedNeighborCount = actualNeighborCount - unrevealedCount - flaggedCount;
            
            // Information weight: more revealed neighbors = more reliable information
            // Range from 0.5 (no revealed neighbors) to 2.0 (all neighbors revealed except unrevealed ones)
            float infoWeight = 0.5f + (1.5f * static_cast<float>(revealedNeighborCount) / static_cast<float>(actualNeighborCount));
            
            // Apply weighted probability to each unrevealed neighbor
            float weightedProbability = baseProbability * infoWeight;
            
            for (const auto& neighbor : unrevealedNeighbors) {
                heatmap[neighbor] += weightedProbability;
                contributionCount[neighbor]++;
            }
        }
        
        // Normalize by number of contributions to get average probability
        for (const auto& cell : unrevealedCells) {
            if (contributionCount[cell] > 0) {
                heatmap[cell] /= static_cast<float>(contributionCount[cell]);
            }
        }
        
        // For cells with no information, assign a default probability based on global mine density
        // This prevents them from being treated as "safest" when they actually have unknown risk
        const int totalMines = 10;
        vector<pair<int, int>> flaggedCells = gameBoard.getAllFlaggedCells();
        int remainingMines = totalMines - static_cast<int>(flaggedCells.size());
        int totalUnrevealedCells = static_cast<int>(unrevealedCells.size());
        
        if (totalUnrevealedCells > 0 && remainingMines > 0) {
            float globalMineDensity = static_cast<float>(remainingMines) / static_cast<float>(totalUnrevealedCells);
            
            // Assign global density to cells with no local information
            // Make it slightly higher to discourage random guessing
            for (const auto& cell : unrevealedCells) {
                if (contributionCount[cell] == 0) {
                    heatmap[cell] = globalMineDensity * 1.5f; // Penalize unknown cells
                }
            }
        }
        
        return heatmap;
    }

    // Find cells with definite mines (probability >= 1.0)
    void findDefiniteMines() {
        map<pair<int, int>, float> heatmap = calculateHeatmap();
        
        for (const auto& entry : heatmap) {
            // Only flag if probability is >= 1.0 (100% certain it's a mine)
            // Round to avoid floating point precision issues
            float rounded = std::round(entry.second * 100.0f) / 100.0f;
            if (rounded >= 1.0f) {
                queueFlagCell(entry.first);
                if (!cellsToFlag.empty()) return; // Queue one at a time
            }
        }
    }

    // Find cells with zero or very low probability of being a mine
    void findSafeCells() {
        map<pair<int, int>, float> heatmap = calculateHeatmap();
        
        // First, look for cells with 0 probability (definitely safe)
        for (const auto& entry : heatmap) {
            if (entry.second < 0.001f) { // Essentially 0
                queueRevealCell(entry.first);
                if (!cellsToReveal.empty()) return; // Queue one at a time
            }
        }
    }

    // Find the cell with the lowest probability of being a mine
    void revealLowestProbabilityCell() {
        map<pair<int, int>, float> heatmap = calculateHeatmap();
        
        if (heatmap.empty()) return;
        
        // Separate cells into two categories:
        // 1. Cells with probability > 0 (have info)
        // 2. Cells with probability == 0 (no info)
        vector<pair<pair<int, int>, float>> cellsWithInfo;
        vector<pair<int, int>> cellsWithoutInfo;
        
        for (const auto& entry : heatmap) {
            if (entry.second > 0.001f) { // Has information
                cellsWithInfo.push_back(entry);
            } else { // No information
                cellsWithoutInfo.push_back(entry.first);
            }
        }
        
        // Prefer cells with information unless we have none
        if (!cellsWithInfo.empty()) {
            // Find cell with lowest non-zero probability
            auto minCell = cellsWithInfo[0];
            for (const auto& entry : cellsWithInfo) {
                if (entry.second < minCell.second) {
                    minCell = entry;
                }
            }
            cout << "[Heatmap] Revealing cell with probability " << minCell.second << endl;
            queueRevealCell(minCell.first);
            
            // Check if the queue is still empty after attempting to queue
            if (cellsToReveal.empty() && cellsToFlag.empty()) {
                consecutiveEmptyQueues++;
                cout << "[Heatmap] Warning: No moves queued (" << consecutiveEmptyQueues << "/" << MAX_EMPTY_QUEUE_ATTEMPTS << ")" << endl;
                
                // Try to use subtraction logic to find safe cells before giving up
                if (consecutiveEmptyQueues == 2) {
                    cout << "[Heatmap] Attempting subtraction logic to find safe cells..." << endl;
                    applySubtractionLogic();
                }
                
                if (consecutiveEmptyQueues >= MAX_EMPTY_QUEUE_ATTEMPTS) {
                    cout << "[Heatmap] Solver stuck - stopping. All remaining cells may be flagged or unreachable." << endl;
                    algoActive = false;
                    consecutiveEmptyQueues = 0;
                }
            } else {
                consecutiveEmptyQueues = 0;
            }
        } else if (!cellsWithoutInfo.empty()) {
            // No cells with info, need to make a random guess
            // Keep trying until we find a cell that isn't flagged
            bool queued = false;
            for (const auto& cell : cellsWithoutInfo) {
                queueRevealCell(cell);
                if (!cellsToReveal.empty()) {
                    cout << "[Heatmap] No cells with info, making random guess" << endl;
                    nextRevealIsGuess = true;
                    queued = true;
                    break;
                }
            }
            if (!queued) {
                cout << "[Heatmap] All remaining cells are flagged or revealed - stopping solver" << endl;
                algoActive = false;
            }
            consecutiveEmptyQueues = 0;
        } else {
            cout << "[Heatmap] No unrevealed cells available - stopping solver" << endl;
            algoActive = false;
            consecutiveEmptyQueues = 0;
        }
    }

    void processHeatmap() {
        // First, use subtraction logic to find safe cells (like algo solver)
        if (cellsToReveal.empty() && cellsToFlag.empty()) {
            applySubtractionLogic();
        }
        
        // Second, find and reveal definitely safe cells from heatmap (probability = 0)
        if (cellsToReveal.empty() && cellsToFlag.empty()) {
            findSafeCells();
        }
        
        // Third, reveal the cell with lowest probability of being a mine
        if (cellsToReveal.empty() && cellsToFlag.empty()) {
            revealLowestProbabilityCell();
        }
    }

    void resetSolverState() {
        // Track game result before resetting
        if (!gameWasCounted && gameBoard.isGameOver()) {
            if (gameBoard.getGameState() == IBoardSolver::WON) {
                wins++;
                cout << "[Heatmap] Game Won! Total: " << wins << " wins, " << losses << " losses" << endl;
            } else if (gameBoard.getGameState() == IBoardSolver::LOST) {
                losses++;
                cout << "[Heatmap] Game Lost! Total: " << wins << " wins, " << losses << " losses" << endl;
            }
            gameWasCounted = true;
        }
        
        // Preserve the active state so solver continues running after reset
        bool wasActive = algoActive;
        
        while (!cellsToReveal.empty()) cellsToReveal.pop();
        while (!cellsToFlag.empty()) cellsToFlag.pop();
        queuedForFlagging.clear();
        if (renderer) renderer->stopInspection();
        gameBoard.reset();
        if (safeStartEnabled) {
            gameBoard.revealRandomZero();
            inRandomGuessPhase = false; // Skip random guess phase
        }
        firstMove = true;
        algoActive = wasActive; // Maintain start/stop state after reset
        if (!safeStartEnabled) {
            inRandomGuessPhase = true;
        }
        gameWasCounted = false;
        nextRevealIsGuess = false;
        consecutiveEmptyQueues = 0; // Reset stuck detection
    }

public:
    heatmapSolver(IBoardSolver& b, BoardRenderer* r) : gameBoard(b), renderer(r) {}
    
    void setSpeed(float newSpeed) {
        speed = std::max(0.1f, std::min(10.0f, newSpeed));
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
    
    void setSafeStart(bool enabled) {
        safeStartEnabled = enabled;
    }
    
    // Get current heatmap for visualization
    map<pair<int, int>, float> getHeatmapData() const {
        // Always return heatmap data, even during random guess phase
        return const_cast<heatmapSolver*>(this)->calculateHeatmap();
    }
    
    void start() {
        algoActive = true;
        cout << "[Heatmap] Solver started" << endl;
    }
    
    void stop() {
        algoActive = false;
        if (renderer) renderer->stopInspection();
        cout << "[Heatmap] Solver stopped" << endl;
    }
    
    bool isActive() const {
        return algoActive;
    }

    void makeMove() {
        // Only make a move if enough time has passed
        if (moveClock.getElapsedTime().asSeconds() < getMoveDelay()) {
            return;
        }

        // Check if game is over
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
        if (renderer) renderer->stopInspection();
        
        if (inRandomGuessPhase) {
            // Check if we have any 0 cells
            bool hasZeroCells = false;
            int gridSize = gameBoard.getGridSize();
            
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
                cout << "[Heatmap] Found 0 cell. Starting heatmap analysis." << endl;
                inRandomGuessPhase = false;
                processHeatmap();
                
                if (preformNextAction()) {
                    moveClock.restart();
                    return;
                }
                
                // If no logical moves, processHeatmap will still queue the lowest probability cell
                cout << "[Heatmap] No definite moves, making educated guess." << endl;
                if (preformNextAction()) {
                    moveClock.restart();
                    return;
                }
            }
            
            // Still in random guess phase
            randomGuessUntilZero();
            preformNextAction();
            moveClock.restart();
        } else {
            // We're in heatmap analysis phase
            processHeatmap();
            
            if (preformNextAction()) {
                moveClock.restart();
                return;
            }
            
            // If processHeatmap didn't queue anything (shouldn't happen), log it
            cout << "[Heatmap] Warning: No moves queued, continuing..." << endl;
        }
    }
};