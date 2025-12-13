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
    int selectedX = 0;
    int selectedY = 0;

    sf::RenderWindow* windowPtr = nullptr;

public:
    Board(sf::RenderWindow& window) : gridData(GRID_SIZE, vector<int>(GRID_SIZE, 0)), windowPtr(&window) {
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

    void handleClick(int x, int y) {
        selectedX = x;
        selectedY = y;
        cout << "Clicked on cell: (" << x << ", " << y << ")\n Click mode: " 
             << (currentClickMode == REVEAL ? "REVEAL" : "FLAG") << " Cell value: " 
             << gridData[x][y] << endl;
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

                // Draw main cell with gradient-like effect
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
                
                // Draw number with color coding
                if (gridData[i][j] != 0) {
                    static sf::Font font("font.ttf");
                    string displayText = (gridData[i][j] == -1) ? "💣" : std::to_string(gridData[i][j]);
                    sf::Text numberText(font, displayText, 28);
                    
                    // Color code based on value
                    if (gridData[i][j] == -1) {
                        numberText.setFillColor(sf::Color::Red);
                    } else if (gridData[i][j] == 1) {
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

    void spawnMines() {
        int minesToSpawn = 10;

        while (minesToSpawn > 0) {
            int x = rand() % GRID_SIZE;
            int y = rand() % GRID_SIZE;

            if (gridData[x][y] != -1) {
                gridData[x][y] = -1; 
                minesToSpawn--;
            }
        }
    }

    void solveForCellValues() {
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int y = 0; y < GRID_SIZE; y++) {
                if (gridData[x][y] == -1) continue; 

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
        return gridData[x][y] == -1;
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

    void render() {
        if (!windowPtr) return;
        
        windowPtr->clear(sf::Color::White);
        drawCells(*windowPtr);
        drawSelectionBox(*windowPtr);
        windowPtr->display();
    }

    void reset() {
        gridData = vector<vector<int>>(GRID_SIZE, vector<int>(GRID_SIZE, 0));
        spawnMines();
        solveForCellValues();
        selectedX = 0;
        selectedY = 0;
        currentClickMode = REVEAL;
    }
};