#ifndef BOARDRENDERER_H
#define BOARDRENDERER_H

#include <SFML/Graphics.hpp>
#include "Board.h"

class BoardRenderer {
public:
    enum SelectionType { SELECT, SEARCH, CLICK };
    
private:
    static constexpr float CELL_SIZE = 50.0f;
    Board* board;
    sf::RenderWindow* window;
    sf::Clock clickAnimationClock;
    bool showClickAnimation = false;
    float clickAnimationDuration = 0.3f;
    bool debugOverlayEnabled = false;
    
    // Selection animation
    bool isAnimatingSelection = false;
    sf::Clock selectionAnimationClock;
    float selectionAnimationDuration = 0.15f;
    int prevSelectedX = 0;
    int prevSelectedY = 0;
    
public:
    BoardRenderer(Board& board, sf::RenderWindow& window);
    
    // Main render function
    void render();
    
    // Individual render components
    void drawCells();
    void drawModeIndicator();
    void drawGameOverScreen();
    void drawSelectionBox(SelectionType type);
    
    // Animation control
    void startClickAnimation();
    void startSelectionAnimation(int oldX, int oldY);
    
    // Debug features
    void setDebugOverlay(bool enabled) { debugOverlayEnabled = enabled; }
    
    // Utility
    float getCellSize() const { return CELL_SIZE; }
    
private:
    void drawRevealedCell(int x, int y);
    void drawCoveredCell(int x, int y);
    void drawFlag(float centerX, float centerY);
    void drawBomb(float centerX, float centerY);
    void drawNumber(int x, int y, int value);
    void drawDebugOverlay(int x, int y);
};

#endif
