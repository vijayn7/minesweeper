#ifndef BOARD_H
#define BOARD_H

#include <vector>

class Board {
public:
    enum ClickMode { REVEAL, FLAG };
    enum GameState { PLAYING, WON, LOST };
    
private:
    static const int GRID_SIZE = 9;
    ClickMode currentClickMode = REVEAL;
    GameState currentGameState = PLAYING;
    std::vector<std::vector<int>> gridData;
    std::vector<std::vector<bool>> revealedGrid;
    std::vector<std::vector<bool>> flaggedGrid;
    int selectedX = 0;
    int selectedY = 0;
    int totalMines = 10;
    
    enum CellVal { ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, BOMB };
    
public:
    Board();
    
    // Grid properties
    int getGridSize() const { return GRID_SIZE; }
    
    // Selection management
    int getSelectedX() const { return selectedX; }
    int getSelectedY() const { return selectedY; }
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    bool setSelectedCell(int x, int y);
    
    // Game state
    GameState getGameState() const { return currentGameState; }
    ClickMode getClickMode() const { return currentClickMode; }
    void toggleClickMode();
    
    // Cell queries
    int getCellVal(int x, int y) const;
    bool isRevealed(int x, int y) const;
    bool isFlagged(int x, int y) const;
    bool searchCell(int x, int y) const;
    
    // Game actions
    void revealCell(int x, int y);
    void handleClick(int x, int y);
    bool algoClick();
    void reset();
    
    // Utility
    std::vector<std::vector<int>> getPlayerView() const;
    
private:
    void spawnMines();
    void solveForCellValues();
    bool isMine(int x, int y);
    void revealAllMines();
    void checkWinCondition();
};

#endif
