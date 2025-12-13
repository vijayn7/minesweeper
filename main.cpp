#include <SFML/Graphics.hpp>

int main()
{
    // Grid configuration
    const int gridSize = 9;
    const float cellSize = 50.0f;
    const float gridWidth = gridSize * cellSize;
    const float gridHeight = gridSize * cellSize;
    
    // Create the main window
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned int>(gridWidth), static_cast<unsigned int>(gridHeight)}), "Minesweeper");

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Clear screen
        window.clear(sf::Color::White);

        // Draw the grid
        for (int i = 0; i < gridSize; i++)
        {
            for (int j = 0; j < gridSize; j++)
            {
                sf::RectangleShape cell({cellSize - 2, cellSize - 2}); // -2 for spacing
                cell.setPosition({i * cellSize + 1, j * cellSize + 1}); // +1 for spacing
                cell.setFillColor(sf::Color(200, 200, 200));
                cell.setOutlineThickness(1);
                cell.setOutlineColor(sf::Color::Black);
                window.draw(cell);
            }
        }

        // Update the window
        window.display();
    }
}