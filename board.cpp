#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

using namespace std;

class Board {
private:
    static const int GRID_SIZE = 9;
    static constexpr float CELL_SIZE = 50.0f;
    enum clickMode { REVEAL, FLAG }; 
    clickMode currentClickMode = REVEAL;
    vector<vector<int>> gridData;
    vector<vector<bool>> revealedGrid;
    int selectedX = 0;
    int selectedY = 0;

    sf::RenderWindow* windowPtr = nullptr;

    enum cellVal { ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, BOMB };

public:
    Board(sf::RenderWindow& window) : gridData(GRID_SIZE, vector<int>(GRID_SIZE, 0)), 
                                        revealedGrid(GRID_SIZE, vector<bool>(GRID_SIZE, false)),
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
        
        revealedGrid[x][y] = true;
        cout << "Revealed cell (" << x << ", " << y << ") with value: " << gridData[x][y] << endl;
        
        // If hit a bomb, game over
        if (gridData[x][y] == BOMB) {
            GameOver();
            return;
        }
        
        // TODO: Implement flood fill - if cell is 0, recursively reveal adjacent cells
        // Flood fill for empty cells (value 0)
        if (gridData[x][y] == 0) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    revealCell(x + dx, y + dy);
                }
            }
        }
    }

    void handleClick(int x, int y) {
        selectedX = x;
        selectedY = y;
        cout << "Clicked on cell: (" << x << ", " << y << ")\n Click mode: " 
             << (currentClickMode == REVEAL ? "REVEAL" : "FLAG") << " Cell value: " 
             << gridData[x][y] << endl;
        
        if (currentClickMode == REVEAL) {
            revealCell(x, y);
        } else {
            // TODO: Implement flag logic
        }
    }

    void toggleClickMode() {
        if (currentClickMode == REVEAL)
            currentClickMode = FLAG;
        else
            currentClickMode = REVEAL;
    }

    void drawCells(sf::RenderWindow& window) {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {

                // Draw cell shadow for depth
                sf::RectangleShape shadow({CELL_SIZE - 4, CELL_SIZE - 4});
                shadow.setPosition({i * CELL_SIZE + 3, j * CELL_SIZE + 3});
                shadow.setFillColor(sf::Color(100, 100, 100, 80));
                window.draw(shadow);

                // Check if cell is revealed
                if (revealedGrid[i][j]) {
                    // Draw revealed cell (lighter color)
                    sf::RectangleShape cell({CELL_SIZE - 4, CELL_SIZE - 4});
                    cell.setPosition({i * CELL_SIZE + 2, j * CELL_SIZE + 2});
                    cell.setFillColor(sf::Color(220, 220, 220));
                    cell.setOutlineThickness(1);
                    cell.setOutlineColor(sf::Color(180, 180, 180));
                    window.draw(cell);
                } else {
                    // Draw covered cell (raised 3D look)
                    sf::RectangleShape cell({CELL_SIZE - 4, CELL_SIZE - 4});
                    cell.setPosition({i * CELL_SIZE + 2, j * CELL_SIZE + 2});
                    cell.setFillColor(sf::Color(189, 189, 189));
                    cell.setOutlineThickness(2);
                    cell.setOutlineColor(sf::Color(245, 245, 245));
                    window.draw(cell);
                    
                    // Draw inner border for 3D effect
                    sf::RectangleShape innerBorder({CELL_SIZE - 8, CELL_SIZE - 8});
                    innerBorder.setPosition({i * CELL_SIZE + 4, j * CELL_SIZE + 4});
                    innerBorder.setFillColor(sf::Color::Transparent);
                    innerBorder.setOutlineThickness(1);
                    innerBorder.setOutlineColor(sf::Color(140, 140, 140));
                    window.draw(innerBorder);
                }
                
                // Draw mines or numbers only if revealed
                if (revealedGrid[i][j]) {
                    if (gridData[i][j] == BOMB) {
                        // Draw bomb
                        float centerX = i * CELL_SIZE + CELL_SIZE / 2;
                        float centerY = j * CELL_SIZE + CELL_SIZE / 2;
                    
                    // Bomb body (large circle)
                    sf::CircleShape bombBody(12);
                    bombBody.setPosition({centerX - 12, centerY - 12});
                    bombBody.setFillColor(sf::Color::Black);
                    window.draw(bombBody);
                    
                    // Highlight on bomb (small circle for shine)
                    sf::CircleShape highlight(4);
                    highlight.setPosition({centerX - 6, centerY - 9});
                    highlight.setFillColor(sf::Color(100, 100, 100));
                    window.draw(highlight);
                    
                    // Bomb spikes (lines radiating out)
                    for (int angle = 0; angle < 360; angle += 45) {
                        float rad = angle * 3.14159f / 180.0f;
                        sf::RectangleShape spike({2, 8});
                        spike.setPosition({centerX - 1, centerY - 18});
                        spike.setFillColor(sf::Color::Black);
                        spike.setOrigin({1, 0});
                        spike.setRotation(sf::degrees(angle));
                        spike.setPosition({centerX, centerY});
                        window.draw(spike);
                    }
                    
                    // Fuse
                    sf::RectangleShape fuse({2, 6});
                    fuse.setPosition({centerX + 8, centerY - 16});
                    fuse.setFillColor(sf::Color(139, 69, 19)); // Brown
                    fuse.setRotation(sf::degrees(-30));
                    window.draw(fuse);
                    
                    // Spark at end of fuse
                    sf::CircleShape spark(2);
                    spark.setPosition({centerX + 11, centerY - 18});
                    spark.setFillColor(sf::Color(255, 165, 0)); // Orange
                    window.draw(spark);
                    
                    } else if (gridData[i][j] != 0) {
                        // Draw number with color coding
                        static sf::Font font("font.ttf");
                        sf::Text numberText(font, std::to_string(gridData[i][j]), 28);
                        
                        // Color code based on value
                        if (gridData[i][j] == 1) {
                            numberText.setFillColor(sf::Color::Blue);
                        } else if (gridData[i][j] == 2) {
                            numberText.setFillColor(sf::Color::Green);
                        } else if (gridData[i][j] == 3) {
                            numberText.setFillColor(sf::Color::Red);
                        } else if (gridData[i][j] == 4) {
                            numberText.setFillColor(sf::Color(0, 0, 139)); // Dark blue
                        } else {
                            numberText.setFillColor(sf::Color(139, 0, 0)); // Dark red
                        }
                        
                        numberText.setStyle(sf::Text::Bold);
                        numberText.setPosition({i * CELL_SIZE + CELL_SIZE / 3, j * CELL_SIZE + CELL_SIZE / 6});
                        window.draw(numberText);
                    }
                }

            }
        }
    }

    void spawnMines() {
        int minesToSpawn = 10;

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

    void drawSelectionBox(sf::RenderWindow& window) {
        if (selectedX >= 0 && selectedY >= 0) {
            // Outer glow
            sf::RectangleShape outerGlow({CELL_SIZE + 4, CELL_SIZE + 4});
            outerGlow.setPosition({selectedX * CELL_SIZE - 2, selectedY * CELL_SIZE - 2});
            outerGlow.setFillColor(sf::Color(255, 215, 0, 50));
            outerGlow.setOutlineThickness(2);
            outerGlow.setOutlineColor(sf::Color(255, 215, 0, 100));
            window.draw(outerGlow);
            
            // Main selection box
            sf::RectangleShape selectionBox({CELL_SIZE, CELL_SIZE});
            selectionBox.setPosition({selectedX * CELL_SIZE, selectedY * CELL_SIZE});
            selectionBox.setFillColor(sf::Color(255, 255, 0, 30));
            selectionBox.setOutlineThickness(3);
            selectionBox.setOutlineColor(sf::Color(255, 215, 0));
            window.draw(selectionBox);
        }
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

    void GameOver() {
        cout << "Game Over" << endl;
        reset();
    }

    void render() {
        if (!windowPtr) return;
        
        windowPtr->clear(sf::Color::White);
        drawCells(*windowPtr);
        drawSelectionBox(*windowPtr);
        windowPtr->display();
    }

    void reset() {
        gridData = vector<vector<int>>(GRID_SIZE, vector<int>(GRID_SIZE, 0));
        revealedGrid = vector<vector<bool>>(GRID_SIZE, vector<bool>(GRID_SIZE, false));
        spawnMines();
        solveForCellValues();
        selectedX = 0;
        selectedY = 0;
        currentClickMode = REVEAL;
    }
};