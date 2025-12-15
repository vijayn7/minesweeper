#ifndef BOARDRENDERER_H
#define BOARDRENDERER_H

#include <SFML/Graphics.hpp>
#include <map>
#include "Board.h"

class BoardRenderer {
public:
    enum SelectionType { SELECT, SEARCH, CLICK, GUESS };
    
private:
    static constexpr float CELL_SIZE = 50.0f;
    Board* board;
    sf::RenderWindow* window;
    sf::Clock clickAnimationClock;
    bool showClickAnimation = false;
    float baseClickAnimationDuration = 0.3f;
    bool debugOverlayEnabled = false;
    float animationSpeed = 1.0f; // Speed multiplier for all animations
    bool isGuessMove = false; // Track if current move is a random guess
    
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
    
    // Start/Stop button bounds
    float buttonX = 0;
    float buttonY = 0;
    float buttonWidth = 0;
    float buttonHeight = 0;
    
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
    void drawStatsAndControls(int wins, int losses, float speed, const std::string& solverName, bool solverActive, 
                             const std::map<std::pair<int, int>, float>* heatmapData = nullptr);
    bool isStartStopButtonClicked(float mouseX, float mouseY) const;
    
    // Animation control
    void startClickAnimation();
    void startSelectionAnimation(int oldX, int oldY);
    void startInspection(int x, int y);
    void stopInspection();
    void setGuessMove(bool isGuess) { isGuessMove = isGuess; }
    
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
    void drawHeatmap(float x, float y, float size, const std::map<std::pair<int, int>, float>& heatmapData);
};

#endif
