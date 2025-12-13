#include <SFML/Graphics.hpp>
#include "board.cpp"
#include <optional>

int main() {    
    // Create the main window with initial size
    sf::RenderWindow window(sf::VideoMode({450, 450}), "Minesweeper");
    
    Board board(window);

    // Game loop
    while (window.isOpen()) {
        // Process events
        while (const std::optional event = window.pollEvent()) {
            board.handleEvent(event);
        }

        // Render
        board.render();
    }
}