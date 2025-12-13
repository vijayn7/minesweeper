#ifndef IBOARDSOLVER_H
#define IBOARDSOLVER_H

#include <vector>

/**
 * Interface for algo solvers to interact with the game board.
 * Provides a clean API boundary between the solver and game logic.
 */
class IBoardSolver {
public:
enum ClickMode { REVEAL, FLAG };
enum GameState { PLAYING, WON, LOST };
enum CellVal { ZERO = 0, ONE = 1, TWO = 2, THREE = 3, FOUR = 4, FIVE = 5, SIX = 6, SEVEN = 7, EIGHT = 8, BOMB = 9 };
    virtual ~IBoardSolver() = default;
    
    // Grid information
    virtual int getGridSize() const = 0;
    
    // Cell selection
    virtual int getSelectedX() const = 0;
    virtual int getSelectedY() const = 0;
    virtual bool setSelectedCell(int x, int y) = 0;
    
    // Cell information - returns player's view of the board
    // -1 = unrevealed, -2 = flagged, 0-8 = number, 9 = bomb (only visible when revealed)
    virtual std::vector<std::vector<int>> getPlayerView() const = 0;
    virtual int getCellVal(int x, int y) const = 0;
    
    // Actions
    virtual bool algoClick() = 0;
    virtual void setClickMode(ClickMode mode) = 0;
    
    // Utility
    virtual std::vector<std::pair<int, int>> getAllUnrevealedCells() const = 0;
    virtual std::vector<std::pair<int, int>> getAllFlaggedCells() const = 0;
    virtual std::vector<std::pair<int, int>> getUnrevealedNeighbors(int x, int y) const = 0;
    virtual std::vector<std::pair<int, int>> getOnes() const = 0;

    virtual bool searchCell(int x, int y) const = 0;
    virtual bool isGameOver() const = 0;
    virtual void reset() = 0;
};

#endif
