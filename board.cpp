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
    
    vector<vector<int>> grid() {
        return vector<vector<int>>(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    }

public:
    Board() {}

    int getGridSize() const { return GRID_SIZE; }
    float getCellSize() const { return CELL_SIZE; }
    float getWidth() const { return GRID_SIZE * CELL_SIZE; }
    float getHeight() const { return GRID_SIZE * CELL_SIZE; }

    void handleClick(int x, int y) {
        cout << "Clicked on cell: (" << x << ", " << y << ")\n Click mode: " 
             << (currentClickMode == REVEAL ? "REVEAL" : "FLAG") << "Cell value: " 
             << grid()[x][y] << endl;
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
                static sf::Font font("font.ttf");
                sf::Text numberText(font, std::to_string(grid()[i][j]), 24);
                numberText.setFillColor(sf::Color::Black);
                numberText.setPosition({i * CELL_SIZE + CELL_SIZE / 4, j * CELL_SIZE + CELL_SIZE / 8});
                window.draw(numberText);

            }
        }
    }
};