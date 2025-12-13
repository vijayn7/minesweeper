#include "Board.h"
#include "solverUtilities.cpp"
#include <iostream>

using namespace std;

Board::Board() : gridData(GRID_SIZE, vector<CellVal>(GRID_SIZE, ZERO)), 
                 revealedGrid(GRID_SIZE, vector<bool>(GRID_SIZE, false)),
                 flaggedGrid(GRID_SIZE, vector<bool>(GRID_SIZE, false)) {
    spawnMines();
    solveForCellValues();
}

void Board::moveLeft() {
    if (selectedX > 0) selectedX--;
}

void Board::moveRight() {
    if (selectedX < GRID_SIZE - 1) selectedX++;
}

void Board::moveUp() {
    if (selectedY > 0) selectedY--;
}

void Board::moveDown() {
    if (selectedY < GRID_SIZE - 1) selectedY++;
}

int Board::getCellVal(int x, int y) const {
    return static_cast<int>(gridData[x][y]);
}

bool Board::isRevealed(int x, int y) const {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return false;
    return revealedGrid[x][y];
}

bool Board::isFlagged(int x, int y) const {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return false;
    return flaggedGrid[x][y];
}

void Board::revealCell(int x, int y) {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return;
    if (revealedGrid[x][y]) return; // Already revealed
    if (currentGameState != PLAYING) return; // Game is over
    
    revealedGrid[x][y] = true;
    cout << "Revealed cell (" << x << ", " << y << ") with value: " << static_cast<int>(gridData[x][y]) << endl;
    
    // If hit a bomb, game over
    if (gridData[x][y] == BOMB) {
        currentGameState = LOST;
        revealAllMines();
        return;
    }
    
    // Flood fill for empty cells (value 0)
    if (gridData[x][y] == ZERO) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                revealCell(x + dx, y + dy);
            }
        }
    }
    
    // Check win condition after each reveal
    checkWinCondition();
}

void Board::revealAllMines() {
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (gridData[x][y] == BOMB) {
                revealedGrid[x][y] = true;
            }
        }
    }
}

void Board::checkWinCondition() {
    if (currentGameState != PLAYING) return;
    
    // Win if all non-mine cells are revealed
    int revealedCount = 0;
    int totalNonMineCells = (GRID_SIZE * GRID_SIZE) - totalMines;
    
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (revealedGrid[x][y] && gridData[x][y] != BOMB) {
                revealedCount++;
            }
        }
    }
    
    if (revealedCount == totalNonMineCells) {
        currentGameState = WON;
        cout << "You won!" << endl;
    }
}

void Board::handleClick(int x, int y) {
    selectedX = x;
    selectedY = y;
    
    // If game is over, don't process clicks
    if (currentGameState != PLAYING) return;
    
    cout << "Clicked on cell: (" << x << ", " << y << ")\n Click mode: " 
         << (currentClickMode == REVEAL ? "REVEAL" : "FLAG") << " Cell value: " 
         << static_cast<int>(gridData[x][y]) << endl;
    
    if (currentClickMode == REVEAL) {
        // Can't reveal a flagged cell
        if (!flaggedGrid[x][y]) {
            revealCell(x, y);
        }
    } else {
        // Can't flag a revealed cell
        if (!revealedGrid[x][y]) {
            flaggedGrid[x][y] = !flaggedGrid[x][y];
            cout << (flaggedGrid[x][y] ? "Flagged" : "Unflagged") << " cell (" << x << ", " << y << ")" << endl;
        }
    }
}

void Board::toggleClickMode() {
    if (currentClickMode == REVEAL)
        currentClickMode = FLAG;
    else
        currentClickMode = REVEAL;
}

void Board::spawnMines() {
    int minesToSpawn = totalMines;

    while (minesToSpawn > 0) {
        int x = solverUtilities::getRandomInt(0, GRID_SIZE - 1);
        int y = solverUtilities::getRandomInt(0, GRID_SIZE - 1);

        if (gridData[x][y] != BOMB) {
            gridData[x][y] = BOMB; 
            minesToSpawn--;
        }
    }
}

void Board::solveForCellValues() {
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (gridData[x][y] == BOMB) continue; 

            int mineCount = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue; 
                    if (isMine(x + dx, y + dy)) {
                        mineCount++;
                    }
                }
            }
            gridData[x][y] = static_cast<CellVal>(mineCount);
        }   
    }
}

bool Board::isMine(int x, int y) {
    if (x < 0 || x > GRID_SIZE - 1 || y < 0 || y > GRID_SIZE - 1)
        return false;
    return gridData[x][y] == BOMB;
}

void Board::reset() {
    gridData = vector<vector<CellVal>>(GRID_SIZE, vector<CellVal>(GRID_SIZE, ZERO));
    revealedGrid = vector<vector<bool>>(GRID_SIZE, vector<bool>(GRID_SIZE, false));
    flaggedGrid = vector<vector<bool>>(GRID_SIZE, vector<bool>(GRID_SIZE, false));
    spawnMines();
    solveForCellValues();
    selectedX = 0;
    selectedY = 0;
    currentClickMode = REVEAL;
    currentGameState = PLAYING;
}

vector<vector<int>> Board::getPlayerView() const {
    vector<vector<int>> playerView(GRID_SIZE, vector<int>(GRID_SIZE, -1)); // -1 for unrevealed

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (revealedGrid[x][y]) {
                playerView[x][y] = static_cast<int>(gridData[x][y]);
            } else if (flaggedGrid[x][y]) {
                playerView[x][y] = -2; // -2 for flagged
            }
        }
    }

    return playerView;
}

bool Board::setSelectedCell(int x, int y) {
    if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
        selectedX = x;
        selectedY = y;
        return true;
    }
    return false;
}

bool Board::algoClick() {
    handleClick(selectedX, selectedY);
    cout << "Algo clicked cell: (" << selectedX << ", " << selectedY << ")\n";
    return true;
}

bool Board::searchCell(int x, int y) const {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return false;
    return revealedGrid[x][y];
}

bool Board::isGameOver() const {
    return currentGameState == WON || currentGameState == LOST;
}

vector<pair<int, int>> Board::getAllUnrevealedCells() const {
    vector<pair<int, int>> revealedCells;
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (!revealedGrid[x][y]) {
                revealedCells.emplace_back(x, y);
            }
        }
    }
    return revealedCells;
}

void Board::setClickMode(ClickMode mode) {
    currentClickMode = mode;
}

vector<pair<int, int>> Board::getAllFlaggedCells() const {
    vector<pair<int, int>> flaggedCells;
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (flaggedGrid[x][y]) {
                flaggedCells.emplace_back(x, y);
            }
        }
    }
    return flaggedCells;
}

vector<pair<int, int>> Board::getUnrevealedNeighbors(int x, int y) const {
    vector<pair<int, int>> neighbors;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
                if (!revealedGrid[nx][ny]) {
                    neighbors.emplace_back(nx, ny);
                }
            }
        }
    }
    return neighbors;
}

vector<pair<int, int>> Board::getFlaggedNeighbors(int x, int y) const {
    vector<pair<int, int>> neighbors;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
                if (flaggedGrid[nx][ny]) {
                    neighbors.emplace_back(nx, ny);
                }
            }
        }
    }
    return neighbors;
}

vector<pair<int, int>> Board::getOnes() const {
    vector<pair<int, int>> ones;
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            if (gridData[x][y] == ONE) {
                ones.emplace_back(x, y);
            }
        }
    }
    return ones;
}