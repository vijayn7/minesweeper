#ifndef IBOARDSOLVER_H
#define IBOARDSOLVER_H

#include <vector>

/**
 * Interface for algo solvers to interact with the game board.
 * Provides a clean API boundary between the solver and game logic.
 */
class IBoardSolver {
public:
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
    
    // Utility
    virtual bool searchCell(int x, int y) const = 0;
    virtual bool isGameOver() const = 0;
    virtual void reset() = 0;
};

#endif
