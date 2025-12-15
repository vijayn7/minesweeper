#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "IBoardSolver.h"

class Board : public IBoardSolver {
    
private:
    static const int GRID_SIZE = 9;
    ClickMode currentClickMode = REVEAL;
    GameState currentGameState = PLAYING;
    std::vector<std::vector<CellVal>> gridData;
    std::vector<std::vector<bool>> revealedGrid;
    std::vector<std::vector<bool>> flaggedGrid;
    int selectedX = 0;
    int selectedY = 0;
    int totalMines = 10;
    
public:
    Board();
    
    // Grid properties
    int getGridSize() const override { return GRID_SIZE; }
    
    // Selection management
    int getSelectedX() const override { return selectedX; }
    int getSelectedY() const override { return selectedY; }
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    bool setSelectedCell(int x, int y) override;
    
    // Game state
    GameState getGameState() const override { return currentGameState; }
    ClickMode getClickMode() const override { return currentClickMode; }
    void toggleClickMode();
    bool isGameOver() const override;
    void setClickMode(ClickMode mode) override;
    
    // Cell queries
    int getCellVal(int x, int y) const override;
    bool isRevealed(int x, int y) const;
    bool isFlagged(int x, int y) const;
    bool searchCell(int x, int y) const override;
    
    // Game actions
    void revealCell(int x, int y);
    void handleClick(int x, int y);
    bool algoClick() override;
    void reset() override;
    
    // Utility
    std::vector<std::vector<int>> getPlayerView() const override;
    std::vector<std::pair<int, int>> getAllUnrevealedCells() const override;
    std::vector<std::pair<int, int>> getAllFlaggedCells() const override;
    std::vector<std::pair<int, int>> getUnrevealedNeighbors(int x, int y) const override;
    std::vector<std::pair<int, int>> getFlaggedNeighbors(int x, int y) const override;
    std::vector<std::pair<int, int>> getOnes() const override;
    
private:
    void spawnMines();
    void solveForCellValues();
    bool isMine(int x, int y);
    void revealAllMines();
    void checkWinCondition();
};

#endif
