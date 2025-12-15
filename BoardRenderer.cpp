#include "BoardRenderer.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>

BoardRenderer::BoardRenderer(Board& b, sf::RenderWindow& w) 
    : board(&b), window(&w) {}

void BoardRenderer::render() {
    if (!window) return;
    
    window->clear(sf::Color::White);
    drawCells();
    
    // Check if click animation should still be shown
    if (showClickAnimation && clickAnimationClock.getElapsedTime().asSeconds() > (baseClickAnimationDuration / animationSpeed)) {
        showClickAnimation = false;
        isGuessMove = false; // Reset guess flag when animation completes
    }
    
    // Draw selection box with appropriate color
    SelectionType selectionType = SELECT;
    if (showClickAnimation) {
        selectionType = isGuessMove ? GUESS : CLICK;
    }
    drawSelectionBox(selectionType);
    
    // Draw inspection box if active
    if (isInspecting) {
        drawInspectionBox();
    }
    
    drawModeIndicator();
}

void BoardRenderer::finishFrame() {
    drawGameOverScreen();
    window->display();
}

void BoardRenderer::drawCells() {
    int gridSize = board->getGridSize();
    
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (board->isRevealed(i, j)) {
                drawRevealedCell(i, j);
            } else {
                drawCoveredCell(i, j);
            }
            
            // Debug overlay - show all values when enabled
            if (debugOverlayEnabled) {
                drawDebugOverlay(i, j);
            }
        }
    }
}

void BoardRenderer::drawRevealedCell(int x, int y) {
    // Draw revealed cell (flat, recessed)
    sf::RectangleShape cell({CELL_SIZE, CELL_SIZE});
    cell.setPosition({x * CELL_SIZE, y * CELL_SIZE});
    cell.setFillColor(sf::Color(192, 192, 192));
    window->draw(cell);
    
    // Subtle border for grid definition
    sf::RectangleShape border({CELL_SIZE, CELL_SIZE});
    border.setPosition({x * CELL_SIZE, y * CELL_SIZE});
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(-1);
    border.setOutlineColor(sf::Color(128, 128, 128));
    window->draw(border);
    
    // Draw cell content
    int cellVal = board->getCellVal(x, y);
    if (cellVal == Board::BOMB) { // BOMB
        float centerX = x * CELL_SIZE + CELL_SIZE / 2;
        float centerY = y * CELL_SIZE + CELL_SIZE / 2;
        drawBomb(centerX, centerY);
    } else if (cellVal != Board::ZERO) {
        drawNumber(x, y, cellVal);
    }
}

void BoardRenderer::drawCoveredCell(int x, int y) {
    // Draw covered cell with classic raised 3D look
    sf::RectangleShape cell({CELL_SIZE, CELL_SIZE});
    cell.setPosition({x * CELL_SIZE, y * CELL_SIZE});
    cell.setFillColor(sf::Color(189, 189, 189));
    window->draw(cell);
    
    // Light top-left border (raised effect)
    sf::RectangleShape topBorder({CELL_SIZE, 3});
    topBorder.setPosition({x * CELL_SIZE, y * CELL_SIZE});
    topBorder.setFillColor(sf::Color(255, 255, 255));
    window->draw(topBorder);
    
    sf::RectangleShape leftBorder({3, CELL_SIZE});
    leftBorder.setPosition({x * CELL_SIZE, y * CELL_SIZE});
    leftBorder.setFillColor(sf::Color(255, 255, 255));
    window->draw(leftBorder);
    
    // Dark bottom-right border (shadow)
    sf::RectangleShape bottomBorder({CELL_SIZE, 3});
    bottomBorder.setPosition({x * CELL_SIZE, y * CELL_SIZE + CELL_SIZE - 3});
    bottomBorder.setFillColor(sf::Color(128, 128, 128));
    window->draw(bottomBorder);
    
    sf::RectangleShape rightBorder({3, CELL_SIZE});
    rightBorder.setPosition({x * CELL_SIZE + CELL_SIZE - 3, y * CELL_SIZE});
    rightBorder.setFillColor(sf::Color(128, 128, 128));
    window->draw(rightBorder);
    
    // Draw flag if cell is flagged
    if (board->isFlagged(x, y)) {
        float centerX = x * CELL_SIZE + CELL_SIZE / 2;
        float centerY = y * CELL_SIZE + CELL_SIZE / 2;
        drawFlag(centerX, centerY);
    }
}

void BoardRenderer::drawFlag(float centerX, float centerY) {
    // Flag pole (thinner, cleaner)
    sf::RectangleShape pole({2, 18});
    pole.setPosition({centerX - 1, centerY - 12});
    pole.setFillColor(sf::Color(60, 60, 60));
    window->draw(pole);
    
    // Flag base
    sf::RectangleShape base({8, 2});
    base.setPosition({centerX - 4, centerY + 6});
    base.setFillColor(sf::Color(60, 60, 60));
    window->draw(base);
    
    // Flag fabric (cleaner triangle)
    sf::ConvexShape flag(3);
    flag.setPoint(0, {centerX + 1, centerY - 12});
    flag.setPoint(1, {centerX + 11, centerY - 6});
    flag.setPoint(2, {centerX + 1, centerY});
    flag.setFillColor(sf::Color(220, 20, 20));
    window->draw(flag);
}

void BoardRenderer::drawBomb(float centerX, float centerY) {
    // Bomb body (clean circle)
    sf::CircleShape bombBody(10);
    bombBody.setPosition({centerX - 10, centerY - 10});
    bombBody.setFillColor(sf::Color(30, 30, 30));
    window->draw(bombBody);
    
    // Bomb spikes (8 directions, cleaner)
    for (int angle = 0; angle < 360; angle += 45) {
        sf::RectangleShape spike({1.5f, 6});
        spike.setOrigin({0.75f, 0});
        spike.setPosition({centerX, centerY});
        spike.setRotation(sf::degrees(angle));
        spike.setFillColor(sf::Color(30, 30, 30));
        window->draw(spike);
    }
    
    // Small highlight for depth
    sf::CircleShape highlight(3);
    highlight.setPosition({centerX - 5, centerY - 6});
    highlight.setFillColor(sf::Color(80, 80, 80));
    window->draw(highlight);
}

void BoardRenderer::drawNumber(int x, int y, int value) {
    // Draw number with standardized minesweeper colors
    static sf::Font font("font.ttf");
    sf::Text numberText(font, std::to_string(value), 32);
    
    // Standard minesweeper color scheme
    switch (value) {
        case 1: numberText.setFillColor(sf::Color(0, 0, 255)); break;      // Blue
        case 2: numberText.setFillColor(sf::Color(0, 128, 0)); break;      // Green
        case 3: numberText.setFillColor(sf::Color(255, 0, 0)); break;      // Red
        case 4: numberText.setFillColor(sf::Color(0, 0, 128)); break;      // Dark blue
        case 5: numberText.setFillColor(sf::Color(128, 0, 0)); break;      // Maroon
        case 6: numberText.setFillColor(sf::Color(0, 128, 128)); break;    // Teal
        case 7: numberText.setFillColor(sf::Color(0, 0, 0)); break;        // Black
        case 8: numberText.setFillColor(sf::Color(128, 128, 128)); break;  // Gray
    }
    
    numberText.setStyle(sf::Text::Bold);
    
    // Perfect center alignment
    sf::FloatRect bounds = numberText.getLocalBounds();
    numberText.setOrigin({bounds.size.x / 2 + bounds.position.x, 
                         bounds.size.y / 2 + bounds.position.y});
    numberText.setPosition({x * CELL_SIZE + CELL_SIZE / 2, 
                           y * CELL_SIZE + CELL_SIZE / 2});
    window->draw(numberText);
}

void BoardRenderer::drawModeIndicator() {
    // Mode indicator panel at bottom
    float indicatorHeight = 30;
    float boardHeight = board->getGridSize() * CELL_SIZE;
    float indicatorY = boardHeight;
    
    sf::RectangleShape indicatorBg({board->getGridSize() * CELL_SIZE, indicatorHeight});
    indicatorBg.setPosition({0, indicatorY});
    indicatorBg.setFillColor(sf::Color(220, 220, 220));
    window->draw(indicatorBg);
    
    // Mode text
    static sf::Font font("font.ttf");
    sf::Text modeText(font);
    
    if (board->getClickMode() == Board::REVEAL) {
        modeText.setString("MODE: REVEAL (Space to toggle)");
        modeText.setFillColor(sf::Color(0, 100, 200));
    } else {
        modeText.setString("MODE: FLAG (Space to toggle)");
        modeText.setFillColor(sf::Color(200, 20, 20));
    }
    
    modeText.setCharacterSize(18);
    modeText.setStyle(sf::Text::Bold);
    
    sf::FloatRect textBounds = modeText.getLocalBounds();
    modeText.setOrigin({textBounds.size.x / 2 + textBounds.position.x, 
                       textBounds.size.y / 2 + textBounds.position.y});
    modeText.setPosition({board->getGridSize() * CELL_SIZE / 2, indicatorY + indicatorHeight / 2});
    window->draw(modeText);
}

void BoardRenderer::drawGameOverScreen() {
    if (board->getGameState() == Board::PLAYING) return;
    
    float boardWidth = board->getGridSize() * CELL_SIZE;
    float boardHeight = board->getGridSize() * CELL_SIZE;
    
    // Semi-transparent overlay
    sf::RectangleShape overlay({boardWidth, boardHeight});
    overlay.setPosition({0, 0});
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window->draw(overlay);
    
    // Game over panel
    float panelWidth = 300;
    float panelHeight = 150;
    float panelX = (boardWidth - panelWidth) / 2;
    float panelY = (boardHeight - panelHeight) / 2;
    
    sf::RectangleShape panel({panelWidth, panelHeight});
    panel.setPosition({panelX, panelY});
    panel.setFillColor(sf::Color(240, 240, 240));
    panel.setOutlineThickness(3);
    panel.setOutlineColor(sf::Color(100, 100, 100));
    window->draw(panel);
    
    // Title text
    static sf::Font font("font.ttf");
    sf::Text titleText(font);
    
    if (board->getGameState() == Board::WON) {
        titleText.setString("YOU WIN!");
        titleText.setFillColor(sf::Color(0, 150, 0));
    } else {
        titleText.setString("GAME OVER");
        titleText.setFillColor(sf::Color(180, 0, 0));
    }
    
    titleText.setCharacterSize(36);
    titleText.setStyle(sf::Text::Bold);
    
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin({titleBounds.size.x / 2 + titleBounds.position.x, 
                        titleBounds.size.y / 2 + titleBounds.position.y});
    titleText.setPosition({boardWidth / 2, panelY + 50});
    window->draw(titleText);
    
    // Instructions text
    sf::Text instructionText(font, "Press R to restart", 20);
    instructionText.setFillColor(sf::Color(60, 60, 60));
    
    sf::FloatRect instrBounds = instructionText.getLocalBounds();
    instructionText.setOrigin({instrBounds.size.x / 2 + instrBounds.position.x, 
                               instrBounds.size.y / 2 + instrBounds.position.y});
    instructionText.setPosition({boardWidth / 2, panelY + 105});
    window->draw(instructionText);
}

void BoardRenderer::drawSelectionBox(SelectionType type) {
    sf::Color boxColor;
    
    switch (type) {
        case SELECT:
            boxColor = sf::Color(255, 255, 255, 20); // Subtle white highlight
            break;
        case SEARCH:
            boxColor = sf::Color(0, 255, 0, 50); // Subtle green highlight
            break;
        case CLICK:
            boxColor = sf::Color(255, 0, 0, 50); // Subtle red highlight
            break;
        case GUESS:
            boxColor = sf::Color(0, 255, 0, 80); // Green highlight for random guess
            break;
    }
    
    int selectedX = board->getSelectedX();
    int selectedY = board->getSelectedY();
    
    // Calculate interpolated position for animation
    float drawX = static_cast<float>(selectedX);
    float drawY = static_cast<float>(selectedY);
    
    if (isAnimatingSelection) {
        float elapsedTime = selectionAnimationClock.getElapsedTime().asSeconds();
        float adjustedDuration = baseSelectionAnimationDuration / animationSpeed;
        float progress = std::min(elapsedTime / adjustedDuration, 1.0f);
        
        // Ease-out cubic function for smooth deceleration
        float easedProgress = 1.0f - std::pow(1.0f - progress, 3.0f);
        
        // Interpolate between previous and current position
        drawX = prevSelectedX + (selectedX - prevSelectedX) * easedProgress;
        drawY = prevSelectedY + (selectedY - prevSelectedY) * easedProgress;
        
        // End animation when complete
        if (progress >= 1.0f) {
            isAnimatingSelection = false;
        }
    }
    
    if (selectedX >= 0 && selectedY >= 0) {
        // Subtle selection highlight
        sf::RectangleShape selectionBox({CELL_SIZE, CELL_SIZE});
        selectionBox.setPosition({drawX * CELL_SIZE, drawY * CELL_SIZE});
        selectionBox.setFillColor(boxColor);
        selectionBox.setOutlineThickness(-2);
        selectionBox.setOutlineColor(sf::Color(80, 80, 80, 180));
        window->draw(selectionBox);
    }
}

void BoardRenderer::startClickAnimation() {
    showClickAnimation = true;
    clickAnimationClock.restart();
}

void BoardRenderer::startSelectionAnimation(int oldX, int oldY) {
    prevSelectedX = oldX;
    prevSelectedY = oldY;
    isAnimatingSelection = true;
    selectionAnimationClock.restart();
}

void BoardRenderer::drawStatsAndControls(int wins, int losses, float speed, const std::string& solverName, bool solverActive,
                                         const std::map<std::pair<int, int>, float>* heatmapData) {
    float boardWidth = board->getGridSize() * CELL_SIZE;
    float boardHeight = board->getGridSize() * CELL_SIZE;
    int totalGames = wins + losses;
    
    // Stats panel on the right side, outside the board
    float statsWidth = 200;
    float statsHeight = 190;
    float statsX = boardWidth + 10; // 10px padding from board edge
    float statsY = 10;
    
    // Semi-transparent background
    sf::RectangleShape statsBg({statsWidth, statsHeight});
    statsBg.setPosition({statsX, statsY});
    statsBg.setFillColor(sf::Color(240, 240, 240, 230));
    statsBg.setOutlineThickness(2);
    statsBg.setOutlineColor(sf::Color(100, 100, 100));
    window->draw(statsBg);
    
    static sf::Font font("font.ttf");
    
    // Win/Loss stats
    sf::Text statsText(font);
    statsText.setCharacterSize(16);
    statsText.setFillColor(sf::Color(30, 30, 30));
    
    std::string statsStr = "Solver: " + solverName + "\n";
    statsStr += "Wins: " + std::to_string(wins) + "\n";
    statsStr += "Losses: " + std::to_string(losses) + "\n";
    
    // Show dash if no games played yet, otherwise show total
    if (totalGames == 0) {
        statsStr += "Total: -\n";
    } else {
        statsStr += "Total: " + std::to_string(totalGames) + "\n";
    }
    
    if (totalGames > 0) {
        float winRate = (static_cast<float>(wins) / totalGames) * 100.0f;
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Win Rate: %.1f%%", winRate);
        statsStr += std::string(buffer) + "\n";
    } else {
        statsStr += "Win Rate: -\n";
    }
    
    char speedBuffer[32];
    snprintf(speedBuffer, sizeof(speedBuffer), "Speed: %.1fx", speed);
    statsStr += std::string(speedBuffer);
    
    statsText.setString(statsStr);
    statsText.setPosition({statsX + 10, statsY + 10});
    window->draw(statsText);
    
    // Draw Start/Stop button (with extra padding above)
    buttonWidth = 180;
    buttonHeight = 35;
    buttonX = statsX + 10;
    buttonY = statsY + statsHeight - buttonHeight - 15; // 15px padding above button
    
    sf::RectangleShape button({buttonWidth, buttonHeight});
    button.setPosition({buttonX, buttonY});
    
    // Button color based on state
    if (solverActive) {
        button.setFillColor(sf::Color(220, 80, 80)); // Red for Stop
    } else {
        button.setFillColor(sf::Color(80, 180, 80)); // Green for Start
    }
    button.setOutlineThickness(2);
    button.setOutlineColor(sf::Color(50, 50, 50));
    window->draw(button);
    
    // Button text
    sf::Text buttonText(font);
    buttonText.setCharacterSize(16);
    buttonText.setFillColor(sf::Color::White);
    buttonText.setStyle(sf::Text::Bold);
    buttonText.setString(solverActive ? "STOP" : "START");
    
    sf::FloatRect buttonTextBounds = buttonText.getLocalBounds();
    buttonText.setOrigin({buttonTextBounds.size.x / 2 + buttonTextBounds.position.x, 
                          buttonTextBounds.size.y / 2 + buttonTextBounds.position.y});
    buttonText.setPosition({buttonX + buttonWidth / 2, buttonY + buttonHeight / 2});
    window->draw(buttonText);
    
    // Controls panel on the right side, below stats
    float controlsWidth = 200;
    float controlsHeight = 190;
    float controlsX = boardWidth + 10; // 10px padding from board edge
    float controlsY = statsY + statsHeight + 15; // 15px below stats panel
    
    sf::RectangleShape controlsBg({controlsWidth, controlsHeight});
    controlsBg.setPosition({controlsX, controlsY});
    controlsBg.setFillColor(sf::Color(240, 240, 240, 230));
    controlsBg.setOutlineThickness(2);
    controlsBg.setOutlineColor(sf::Color(100, 100, 100));
    window->draw(controlsBg);
    
    sf::Text controlsText(font);
    controlsText.setCharacterSize(12);
    controlsText.setFillColor(sf::Color(30, 30, 30));
    std::string controlsStr = "Controls:\n";
    controlsStr += "\nSolver Controls:\n";
    controlsStr += "1 - Algo Solver\n";
    controlsStr += "2 - Heatmap Solver\n";
    controlsStr += "G - Start/Stop\n";
    controlsStr += "\nGame Controls:\n";
    controlsStr += "R - Reset\n";
    controlsStr += "Space - Mode\n";
    controlsStr += "+/- - Speed\n";
    controlsStr += "F - Debug\n";  // Added newline for bottom padding
    controlsText.setString(controlsStr);
    controlsText.setPosition({controlsX + 10, controlsY + 10});
    window->draw(controlsText);
    
    // Add padding rectangle at bottom of controls panel
    sf::RectangleShape bottomPadding({controlsWidth, 10});
    bottomPadding.setPosition({controlsX, controlsY + controlsHeight});
    bottomPadding.setFillColor(sf::Color::Transparent);
    window->draw(bottomPadding);
    
    // Draw heatmap visualization if data is provided
    if (heatmapData != nullptr && !heatmapData->empty()) {
        float heatmapY = controlsY + controlsHeight + 20;
        drawHeatmap(controlsX, heatmapY, 200, *heatmapData);
    }
}

void BoardRenderer::startInspection(int x, int y) {
    inspectX = x;
    inspectY = y;
    isInspecting = true;
    inspectionAnimationClock.restart();
}

void BoardRenderer::stopInspection() {
    isInspecting = false;
    inspectX = -1;
    inspectY = -1;
}

void BoardRenderer::drawInspectionBox() {
    if (inspectX < 0 || inspectY < 0) return;
    
    // Pulsing animation (speed affects pulse rate)
    float elapsed = inspectionAnimationClock.getElapsedTime().asSeconds();
    float pulse = 0.5f + 0.5f * std::sin(elapsed * 8.0f * animationSpeed); // Oscillate between 0.5 and 1.0
    
    // Blue outline with pulsing opacity
    sf::RectangleShape inspectionBox({CELL_SIZE, CELL_SIZE});
    inspectionBox.setPosition({inspectX * CELL_SIZE, inspectY * CELL_SIZE});
    inspectionBox.setFillColor(sf::Color::Transparent);
    inspectionBox.setOutlineThickness(-3);
    inspectionBox.setOutlineColor(sf::Color(0, 100, 255, static_cast<unsigned char>(100 + 155 * pulse)));
    window->draw(inspectionBox);
    
    // Inner glow effect
    sf::RectangleShape innerGlow({CELL_SIZE - 6, CELL_SIZE - 6});
    innerGlow.setPosition({inspectX * CELL_SIZE + 3, inspectY * CELL_SIZE + 3});
    innerGlow.setFillColor(sf::Color(100, 150, 255, static_cast<unsigned char>(30 * pulse)));
    window->draw(innerGlow);
}

void BoardRenderer::drawDebugOverlay(int x, int y) {
    int cellVal = board->getCellVal(x, y);
    
    // Semi-transparent background for better visibility
    sf::RectangleShape overlayBg({CELL_SIZE * 0.6f, CELL_SIZE * 0.6f});
    overlayBg.setPosition({x * CELL_SIZE + CELL_SIZE * 0.2f, y * CELL_SIZE + CELL_SIZE * 0.2f});
    overlayBg.setFillColor(sf::Color(0, 0, 0, 150));
    window->draw(overlayBg);
    
    // Draw cell value
    static sf::Font font("font.ttf");
    sf::Text debugText(font);
    
    if (cellVal == Board::BOMB) { // BOMB
        debugText.setString("B");
        debugText.setFillColor(sf::Color(255, 255, 0)); // Yellow for bombs
    } else {
        debugText.setString(std::to_string(cellVal));
        debugText.setFillColor(sf::Color(255, 255, 255)); // White for numbers
    }
    
    debugText.setCharacterSize(24);
    debugText.setStyle(sf::Text::Bold);
    
    sf::FloatRect bounds = debugText.getLocalBounds();
    debugText.setOrigin({bounds.size.x / 2 + bounds.position.x, 
                         bounds.size.y / 2 + bounds.position.y});
    debugText.setPosition({x * CELL_SIZE + CELL_SIZE / 2, 
                           y * CELL_SIZE + CELL_SIZE / 2});
    window->draw(debugText);
}

void BoardRenderer::drawHeatmap(float x, float y, float size, const std::map<std::pair<int, int>, float>& heatmapData) {
    int gridSize = board->getGridSize();
    float miniCellSize = size / gridSize;
    
    // Draw background panel
    sf::RectangleShape heatmapBg({size, size});
    heatmapBg.setPosition({x, y});
    heatmapBg.setFillColor(sf::Color(240, 240, 240, 230));
    heatmapBg.setOutlineThickness(2);
    heatmapBg.setOutlineColor(sf::Color(100, 100, 100));
    window->draw(heatmapBg);
    
    // Find max probability for normalization
    float maxProb = 0.0f;
    for (const auto& entry : heatmapData) {
        if (entry.second > maxProb) {
            maxProb = entry.second;
        }
    }
    
    // Draw each cell
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            float cellX = x + i * miniCellSize;
            float cellY = y + j * miniCellSize;
            
            sf::RectangleShape miniCell({miniCellSize - 1, miniCellSize - 1});
            miniCell.setPosition({cellX, cellY});
            
            // Check if cell is revealed
            if (board->isRevealed(i, j)) {
                miniCell.setFillColor(sf::Color(192, 192, 192)); // Gray for revealed
            } else if (board->isFlagged(i, j)) {
                miniCell.setFillColor(sf::Color(255, 100, 100)); // Red for flagged
            } else {
                // Color based on probability
                auto it = heatmapData.find({i, j});
                if (it != heatmapData.end() && maxProb > 0) {
                    float prob = it->second / maxProb; // Normalize to 0-1
                    // Green (safe) to Yellow to Red (dangerous)
                    int red = static_cast<int>(255 * prob);
                    int green = static_cast<int>(255 * (1.0f - prob * 0.5f));
                    int blue = static_cast<int>(50 * (1.0f - prob));
                    miniCell.setFillColor(sf::Color(red, green, blue));
                } else {
                    miniCell.setFillColor(sf::Color(100, 100, 100)); // Dark gray for no info
                }
            }
            
            miniCell.setOutlineThickness(-0.5f);
            miniCell.setOutlineColor(sf::Color(80, 80, 80));
            window->draw(miniCell);
        }
    }
    
    // Draw title
    static sf::Font font("font.ttf");
    sf::Text titleText(font);
    titleText.setCharacterSize(12);
    titleText.setFillColor(sf::Color(30, 30, 30));
    titleText.setString("Probability Heatmap");
    titleText.setPosition({x + 5, y - 18});
    window->draw(titleText);
}

bool BoardRenderer::isStartStopButtonClicked(float mouseX, float mouseY) const {
    return mouseX >= buttonX && mouseX <= buttonX + buttonWidth &&
           mouseY >= buttonY && mouseY <= buttonY + buttonHeight;
}
