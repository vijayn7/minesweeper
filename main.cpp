#include <SFML/Graphics.hpp>
#include "board.cpp"
#include <optional>

int main() {
    Board board;
    
    // Create the main window
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned int>(board.getWidth()), 
                                          static_cast<unsigned int>(board.getHeight())}), 
                           "Minesweeper");

    // Start the game loop
    while (window.isOpen()) {

        // Process events
        while (const std::optional event = window.pollEvent()) {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::KeyPressed>()) {
                const auto& keyEvent = event->getIf<sf::Event::KeyPressed>();

                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Escape)
                    window.close();

                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Space)
                    board.toggleClickMode();
            }

            if (event->is<sf::Event::MouseButtonPressed>()) {
                const auto& mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
                if (mouseEvent) {
                    int x = mouseEvent->position.x / static_cast<int>(board.getCellSize());
                    int y = mouseEvent->position.y / static_cast<int>(board.getCellSize());
                    
                    board.handleClick(x, y);
                }
            }
        }

        // Clear screen
        window.clear(sf::Color::White);

        // Draw the board
        board.drawCells(window);

        // Update the window
        window.display();
    }
}