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
    float baseClickAnimationDuration = 0.3f;
    bool debugOverlayEnabled = false;
    float animationSpeed = 1.0f; // Speed multiplier for all animations
    
    // Selection animation
    bool isAnimatingSelection = false;
    sf::Clock selectionAnimationClock;
    float baseSelectionAnimationDuration = 0.15f;
    int prevSelectedX = 0;
    int prevSelectedY = 0;
    
    // Inspection animation (blue box for algorithm analysis)
    bool isInspecting = false;
    int inspectX = -1;
    int inspectY = -1;
    sf::Clock inspectionAnimationClock;
    
public:
    BoardRenderer(Board& board, sf::RenderWindow& window);
    
    // Main render function
    void render();
    void finishFrame(); // Call after render() to complete the frame
    
    // Individual render components
    void drawCells();
    void drawModeIndicator();
    void drawGameOverScreen();
    void drawSelectionBox(SelectionType type);
    void drawStatsAndControls(int wins, int losses, float speed);
    
    // Animation control
    void startClickAnimation();
    void startSelectionAnimation(int oldX, int oldY);
    void startInspection(int x, int y);
    void stopInspection();
    
    // Debug features
    void setDebugOverlay(bool enabled) { debugOverlayEnabled = enabled; }
    
    // Speed control
    void setAnimationSpeed(float speed) { animationSpeed = speed; }
    
    // Utility
    float getCellSize() const { return CELL_SIZE; }
    
private:
    void drawRevealedCell(int x, int y);
    void drawCoveredCell(int x, int y);
    void drawFlag(float centerX, float centerY);
    void drawBomb(float centerX, float centerY);
    void drawNumber(int x, int y, int value);
    void drawDebugOverlay(int x, int y);
    void drawInspectionBox();
};

#endif
