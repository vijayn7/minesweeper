#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

using namespace std;

class Board {
private:
    static const int GRID_SIZE = 9;
    static constexpr float CELL_SIZE = 50.0f;
    enum clickMode { REVEAL, FLAG }; 
    enum gameState { PLAYING, WON, LOST };
    enum selectionType { SELECT, SEARCH, CLICK };
    clickMode currentClickMode = REVEAL;
    gameState currentGameState = PLAYING;
    vector<vector<int>> gridData;
    vector<vector<bool>> revealedGrid;
    vector<vector<bool>> flaggedGrid;
    int selectedX = 0;
    int selectedY = 0;
    int totalMines = 10;

    sf::RenderWindow* windowPtr = nullptr;
    sf::Clock clickAnimationClock;
    bool showClickAnimation = false;
    float clickAnimationDuration = 0.3f; // Show click color for 0.3 seconds

    enum cellVal { ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, BOMB };

public:
    Board(sf::RenderWindow& window) : gridData(GRID_SIZE, vector<int>(GRID_SIZE, 0)), 
                                        revealedGrid(GRID_SIZE, vector<bool>(GRID_SIZE, false)),
                                        flaggedGrid(GRID_SIZE, vector<bool>(GRID_SIZE, false)),
                                        windowPtr(&window) {
        spawnMines();
        solveForCellValues();
    }

    int getGridSize() const { return GRID_SIZE; }
    float getCellSize() const { return CELL_SIZE; }
    float getWidth() const { return GRID_SIZE * CELL_SIZE; }
    float getHeight() const { return GRID_SIZE * CELL_SIZE; }

    int getSelectedX() const { return selectedX; }
    int getSelectedY() const { return selectedY; }

    void moveLeft() {
        if (selectedX > 0) selectedX--;
    }

    void moveRight() {
        if (selectedX < GRID_SIZE - 1) selectedX++;
    }

    void moveUp() {
        if (selectedY > 0) selectedY--;
    }

    void moveDown() {
        if (selectedY < GRID_SIZE - 1) selectedY++;
    }

    int getCellVal(int x, int y) const { return gridData[x][y]; }

    void revealCell(int x, int y) {
        if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return;
        if (revealedGrid[x][y]) return; // Already revealed
        if (currentGameState != PLAYING) return; // Game is over
        
        revealedGrid[x][y] = true;
        cout << "Revealed cell (" << x << ", " << y << ") with value: " << gridData[x][y] << endl;
        
        // If hit a bomb, game over
        if (gridData[x][y] == BOMB) {
            currentGameState = LOST;
            revealAllMines();
            return;
        }
        
        // Flood fill for empty cells (value 0)
        if (gridData[x][y] == 0) {
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
    
    void revealAllMines() {
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int y = 0; y < GRID_SIZE; y++) {
                if (gridData[x][y] == BOMB) {
                    revealedGrid[x][y] = true;
                }
            }
        }
    }
    
    void checkWinCondition() {
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

    void handleClick(int x, int y) {
        selectedX = x;
        selectedY = y;
        
        // If game is over, don't process clicks
        if (currentGameState != PLAYING) return;
        
        cout << "Clicked on cell: (" << x << ", " << y << ")\n Click mode: " 
             << (currentClickMode == REVEAL ? "REVEAL" : "FLAG") << " Cell value: " 
             << gridData[x][y] << endl;
        
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

    void toggleClickMode() {
        if (currentClickMode == REVEAL)
            currentClickMode = FLAG;
        else
            currentClickMode = REVEAL;
    }

    void drawCells() {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {

                // Check if cell is revealed
                if (revealedGrid[i][j]) {
                    // Draw revealed cell (flat, recessed)
                    sf::RectangleShape cell({CELL_SIZE, CELL_SIZE});
                    cell.setPosition({i * CELL_SIZE, j * CELL_SIZE});
                    cell.setFillColor(sf::Color(192, 192, 192));
                    windowPtr->draw(cell);
                    
                    // Subtle border for grid definition
                    sf::RectangleShape border({CELL_SIZE, CELL_SIZE});
                    border.setPosition({i * CELL_SIZE, j * CELL_SIZE});
                    border.setFillColor(sf::Color::Transparent);
                    border.setOutlineThickness(-1);
                    border.setOutlineColor(sf::Color(128, 128, 128));
                    windowPtr->draw(border);
                } else {
                    // Draw covered cell with classic raised 3D look
                    sf::RectangleShape cell({CELL_SIZE, CELL_SIZE});
                    cell.setPosition({i * CELL_SIZE, j * CELL_SIZE});
                    cell.setFillColor(sf::Color(189, 189, 189));
                    windowPtr->draw(cell);
                    
                    // Light top-left border (raised effect)
                    sf::RectangleShape topBorder({CELL_SIZE, 3});
                    topBorder.setPosition({i * CELL_SIZE, j * CELL_SIZE});
                    topBorder.setFillColor(sf::Color(255, 255, 255));
                    windowPtr->draw(topBorder);
                    
                    sf::RectangleShape leftBorder({3, CELL_SIZE});
                    leftBorder.setPosition({i * CELL_SIZE, j * CELL_SIZE});
                    leftBorder.setFillColor(sf::Color(255, 255, 255));
                    windowPtr->draw(leftBorder);
                    
                    // Dark bottom-right border (shadow)
                    sf::RectangleShape bottomBorder({CELL_SIZE, 3});
                    bottomBorder.setPosition({i * CELL_SIZE, j * CELL_SIZE + CELL_SIZE - 3});
                    bottomBorder.setFillColor(sf::Color(128, 128, 128));
                    windowPtr->draw(bottomBorder);
                    
                    sf::RectangleShape rightBorder({3, CELL_SIZE});
                    rightBorder.setPosition({i * CELL_SIZE + CELL_SIZE - 3, j * CELL_SIZE});
                    rightBorder.setFillColor(sf::Color(128, 128, 128));
                    windowPtr->draw(rightBorder);
                    
                    // Draw flag if cell is flagged
                    if (flaggedGrid[i][j]) {
                        float centerX = i * CELL_SIZE + CELL_SIZE / 2;
                        float centerY = j * CELL_SIZE + CELL_SIZE / 2;
                        
                        // Flag pole (thinner, cleaner)
                        sf::RectangleShape pole({2, 18});
                        pole.setPosition({centerX - 1, centerY - 12});
                        pole.setFillColor(sf::Color(60, 60, 60));
                        windowPtr->draw(pole);
                        
                        // Flag base
                        sf::RectangleShape base({8, 2});
                        base.setPosition({centerX - 4, centerY + 6});
                        base.setFillColor(sf::Color(60, 60, 60));
                        windowPtr->draw(base);
                        
                        // Flag fabric (cleaner triangle)
                        sf::ConvexShape flag(3);
                        flag.setPoint(0, {centerX + 1, centerY - 12});
                        flag.setPoint(1, {centerX + 11, centerY - 6});
                        flag.setPoint(2, {centerX + 1, centerY});
                        flag.setFillColor(sf::Color(220, 20, 20));
                        windowPtr->draw(flag);
                    }
                }
                
                // Draw mines or numbers only if revealed
                if (revealedGrid[i][j]) {
                    if (gridData[i][j] == BOMB) {
                        // Draw bomb
                        float centerX = i * CELL_SIZE + CELL_SIZE / 2;
                        float centerY = j * CELL_SIZE + CELL_SIZE / 2;
                    
                    // Bomb body (clean circle)
                    sf::CircleShape bombBody(10);
                    bombBody.setPosition({centerX - 10, centerY - 10});
                    bombBody.setFillColor(sf::Color(30, 30, 30));
                    windowPtr->draw(bombBody);
                    
                    // Bomb spikes (8 directions, cleaner)
                    for (int angle = 0; angle < 360; angle += 45) {
                        sf::RectangleShape spike({1.5f, 6});
                        spike.setOrigin({0.75f, 0});
                        spike.setPosition({centerX, centerY});
                        spike.setRotation(sf::degrees(angle));
                        spike.setFillColor(sf::Color(30, 30, 30));
                        windowPtr->draw(spike);
                    }
                    
                    // Small highlight for depth
                    sf::CircleShape highlight(3);
                    highlight.setPosition({centerX - 5, centerY - 6});
                    highlight.setFillColor(sf::Color(80, 80, 80));
                    windowPtr->draw(highlight);
                    
                    } else if (gridData[i][j] != 0) {
                        // Draw number with standardized minesweeper colors
                        static sf::Font font("font.ttf");
                        sf::Text numberText(font, std::to_string(gridData[i][j]), 32);
                        
                        // Standard minesweeper color scheme
                        switch (gridData[i][j]) {
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
                        numberText.setPosition({i * CELL_SIZE + CELL_SIZE / 2, 
                                               j * CELL_SIZE + CELL_SIZE / 2});
                        windowPtr->draw(numberText);
                    }
                }

            }
        }
    }

    void spawnMines() {
        int minesToSpawn = totalMines;

        while (minesToSpawn > 0) {
            int x = rand() % GRID_SIZE;
            int y = rand() % GRID_SIZE;

            if (gridData[x][y] != BOMB) {
                gridData[x][y] = BOMB; 
                minesToSpawn--;
            }
        }
    }

    void solveForCellValues() {
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
                gridData[x][y] = mineCount;
            }   
        }
    }

    bool isMine(int x, int y) {
        if (x < 0 || x > GRID_SIZE - 1 || y < 0 || y > GRID_SIZE - 1)
            return false;
        return gridData[x][y] == BOMB;
    }
    
    void drawModeIndicator() {
        // Mode indicator panel at bottom
        float indicatorHeight = 30;
        float indicatorY = getHeight();
        
        sf::RectangleShape indicatorBg({getWidth(), indicatorHeight});
        indicatorBg.setPosition({0, indicatorY});
        indicatorBg.setFillColor(sf::Color(220, 220, 220));
        windowPtr->draw(indicatorBg);
        
        // Mode text
        static sf::Font font("font.ttf");
        sf::Text modeText(font);
        
        if (currentClickMode == REVEAL) {
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
        modeText.setPosition({getWidth() / 2, indicatorY + indicatorHeight / 2});
        windowPtr->draw(modeText);
    }
    
    void drawGameOverScreen() {
        if (currentGameState == PLAYING) return;
        
        // Semi-transparent overlay
        sf::RectangleShape overlay({getWidth(), getHeight()});
        overlay.setPosition({0, 0});
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        windowPtr->draw(overlay);
        
        // Game over panel
        float panelWidth = 300;
        float panelHeight = 150;
        float panelX = (getWidth() - panelWidth) / 2;
        float panelY = (getHeight() - panelHeight) / 2;
        
        sf::RectangleShape panel({panelWidth, panelHeight});
        panel.setPosition({panelX, panelY});
        panel.setFillColor(sf::Color(240, 240, 240));
        panel.setOutlineThickness(3);
        panel.setOutlineColor(sf::Color(100, 100, 100));
        windowPtr->draw(panel);
        
        // Title text
        static sf::Font font("font.ttf");
        sf::Text titleText(font);
        
        if (currentGameState == WON) {
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
        titleText.setPosition({getWidth() / 2, panelY + 50});
        windowPtr->draw(titleText);
        
        // Instructions text
        sf::Text instructionText(font, "Press R to restart", 20);
        instructionText.setFillColor(sf::Color(60, 60, 60));
        
        sf::FloatRect instrBounds = instructionText.getLocalBounds();
        instructionText.setOrigin({instrBounds.size.x / 2 + instrBounds.position.x, 
                                   instrBounds.size.y / 2 + instrBounds.position.y});
        instructionText.setPosition({getWidth() / 2, panelY + 105});
        windowPtr->draw(instructionText);
    }

    void handleEvent(const std::optional<sf::Event>& event) {
        if (!event || !windowPtr) return;

        if (event->is<sf::Event::Closed>())
            windowPtr->close();

        if (event->is<sf::Event::KeyPressed>()) {
            const auto& keyEvent = event->getIf<sf::Event::KeyPressed>();

            if (keyEvent && keyEvent->code == sf::Keyboard::Key::Escape)
                windowPtr->close();

            if (keyEvent && keyEvent->code == sf::Keyboard::Key::Space)
                toggleClickMode();

            if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Left || keyEvent->code == sf::Keyboard::Key::A))
                moveLeft();

            if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Right || keyEvent->code == sf::Keyboard::Key::D))
                moveRight();

            if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Up || keyEvent->code == sf::Keyboard::Key::W))
                moveUp();

            if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Down || keyEvent->code == sf::Keyboard::Key::S))
                moveDown();

            if (keyEvent && keyEvent->code == sf::Keyboard::Key::Enter) {
                handleClick(selectedX, selectedY);
            }

            if (keyEvent && keyEvent->code == sf::Keyboard::Key::R) {
                reset();
            }
        }

        if (event->is<sf::Event::MouseButtonPressed>()) {
            const auto& mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
            if (mouseEvent) {
                int x = mouseEvent->position.x / static_cast<int>(CELL_SIZE);
                int y = mouseEvent->position.y / static_cast<int>(CELL_SIZE);
                
                handleClick(x, y);
            }
        }
    }

    void render() {
        if (!windowPtr) return;
        
        windowPtr->clear(sf::Color::White);
        drawCells();
        
        // Check if click animation should still be shown
        if (showClickAnimation && clickAnimationClock.getElapsedTime().asSeconds() > clickAnimationDuration) {
            showClickAnimation = false;
        }
        
        // Draw selection box with appropriate color
        drawSelectionBox(showClickAnimation ? selectionType::CLICK : selectionType::SELECT);
        drawModeIndicator();
        drawGameOverScreen();
        windowPtr->display();
    }

    void reset() {
        gridData = vector<vector<int>>(GRID_SIZE, vector<int>(GRID_SIZE, 0));
        revealedGrid = vector<vector<bool>>(GRID_SIZE, vector<bool>(GRID_SIZE, false));
        flaggedGrid = vector<vector<bool>>(GRID_SIZE, vector<bool>(GRID_SIZE, false));
        spawnMines();
        solveForCellValues();
        selectedX = 0;
        selectedY = 0;
        currentClickMode = REVEAL;
        currentGameState = PLAYING;
    }

    vector<vector<int>> getPlayerView() const {
        vector<vector<int>> playerView(GRID_SIZE, vector<int>(GRID_SIZE, -1)); // -1 for unrevealed

        for (int x = 0; x < GRID_SIZE; x++) {
            for (int y = 0; y < GRID_SIZE; y++) {
                if (revealedGrid[x][y]) {
                    playerView[x][y] = gridData[x][y];
                } else if (flaggedGrid[x][y]) {
                    playerView[x][y] = -2; // -2 for flagged
                }
            }
        }

        return playerView;
    }

    bool setSelectedCell(int x, int y) {
        if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
            selectedX = x;
            selectedY = y;
            return true;
        }
        return false;
    }

    bool algoClick() {
        // Trigger click animation
        showClickAnimation = true;
        clickAnimationClock.restart();
        
        handleClick(selectedX, selectedY);
        cout << "Algo clicked cell: (" << selectedX << ", " << selectedY << ")\n";
        return true;
    }

    bool searchCell(int x, int y) const {
        if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return false;
        return revealedGrid[x][y];
    }

    void drawSelectionBox(selectionType type) {

        sf::Color boxColor;

        switch (type)
        {
            case SELECT:
                boxColor = sf::Color(255, 255, 255, 20); // Subtle white highlight
                break;
            case SEARCH:
                boxColor = sf::Color(0, 255, 0, 50); // Subtle green highlight
                break;
            case CLICK:
                boxColor = sf::Color(255, 0, 0, 50); // Subtle red highlight
                break;
        }

        if (selectedX >= 0 && selectedY >= 0) {
            // Subtle selection highlight
            sf::RectangleShape selectionBox({CELL_SIZE, CELL_SIZE});
            selectionBox.setPosition({selectedX * CELL_SIZE, selectedY * CELL_SIZE});
            selectionBox.setFillColor(boxColor);
            selectionBox.setOutlineThickness(-2);
            selectionBox.setOutlineColor(sf::Color(80, 80, 80, 180));
            windowPtr->draw(selectionBox);
        }
    }

};