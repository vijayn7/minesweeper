#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "IBoardSolver.h"

class Board : public IBoardSolver {
public:
    enum ClickMode { REVEAL, FLAG };
    enum GameState { PLAYING, WON, LOST };
    
    enum CellVal { ZERO = 0, ONE = 1, TWO = 2, THREE = 3, FOUR = 4, FIVE = 5, SIX = 6, SEVEN = 7, EIGHT = 8, BOMB = 9 };
    
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
    GameState getGameState() const { return currentGameState; }
    ClickMode getClickMode() const { return currentClickMode; }
    void toggleClickMode();
    
    // Cell queries
    int getCellVal(int x, int y) const override;
    bool isRevealed(int x, int y) const;
    bool isFlagged(int x, int y) const;
    bool searchCell(int x, int y) const override;
    
    // Game actions
    void revealCell(int x, int y);
    void handleClick(int x, int y);
    bool algoClick() override;
    void reset();
    
    // Utility
    std::vector<std::vector<int>> getPlayerView() const override;
    
private:
    void spawnMines();
    void solveForCellValues();
    bool isMine(int x, int y);
    void revealAllMines();
    void checkWinCondition();
};

#endif
