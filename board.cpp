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

public:
    Board() : gridData(GRID_SIZE, vector<int>(GRID_SIZE, 0)) {
        spawnMines();
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

                // Draw cell at (i, j)
                sf::RectangleShape cell({CELL_SIZE - 2, CELL_SIZE - 2});
                cell.setPosition({i * CELL_SIZE + 1, j * CELL_SIZE + 1});
                cell.setFillColor(sf::Color(200, 200, 200));
                cell.setOutlineThickness(1);
                cell.setOutlineColor(sf::Color::Black);
                window.draw(cell);
                
                //Draw number
                if (gridData[i][j] != 0) {
                    static sf::Font font("font.ttf");
                    sf::Text numberText(font, std::to_string(gridData[i][j]), 24);
                    numberText.setFillColor(sf::Color::Black);
                    numberText.setPosition({i * CELL_SIZE + CELL_SIZE / 4, j * CELL_SIZE + CELL_SIZE / 8});
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

    void drawSelectionBox(sf::RenderWindow& window) {
        if (selectedX >= 0 && selectedY >= 0) {
            sf::RectangleShape selectionBox({CELL_SIZE, CELL_SIZE});
            selectionBox.setPosition({selectedX * CELL_SIZE, selectedY * CELL_SIZE});
            selectionBox.setFillColor(sf::Color(0, 0, 0, 0));
            selectionBox.setOutlineThickness(3);
            selectionBox.setOutlineColor(sf::Color::Red);
            window.draw(selectionBox);
        }
    }

};