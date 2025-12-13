#include <SFML/Graphics.hpp>
#include "Board.h"
#include "BoardRenderer.h"
#include <iostream>
#include <optional>
#include "algoSolver.cpp"

int main() {    
    // Create the main window with initial size (board + mode indicator)
    sf::RenderWindow window(sf::VideoMode({450, 480}), "Minesweeper");
    
    Board board;
    BoardRenderer renderer(board, window);
    algoSolver solver(board, &renderer);

    // Game loop
    while (window.isOpen()) {
        // Process events
        while (const std::optional event = window.pollEvent()) {
            // Handle input events
            if (!event) continue;

            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::KeyPressed>()) {
                const auto& keyEvent = event->getIf<sf::Event::KeyPressed>();

                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Escape)
                    window.close();

                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Space)
                    board.toggleClickMode();

                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Left || keyEvent->code == sf::Keyboard::Key::A))
                    board.moveLeft();

                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Right || keyEvent->code == sf::Keyboard::Key::D))
                    board.moveRight();

                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Up || keyEvent->code == sf::Keyboard::Key::W))
                    board.moveUp();

                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Down || keyEvent->code == sf::Keyboard::Key::S))
                    board.moveDown();

                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Enter) {
                    board.handleClick(board.getSelectedX(), board.getSelectedY());
                    renderer.startClickAnimation();
                }

                if (keyEvent && keyEvent->code == sf::Keyboard::Key::R) {
                    board.reset();
                }
                
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::F) {
                    renderer.setDebugOverlay(true);
                }
            }
            
            if (event->is<sf::Event::KeyReleased>()) {
                const auto& keyEvent = event->getIf<sf::Event::KeyReleased>();
                
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::F) {
                    renderer.setDebugOverlay(false);
                }
            }

            if (event->is<sf::Event::MouseButtonPressed>()) {
                const auto& mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
                if (mouseEvent) {
                    int x = mouseEvent->position.x / static_cast<int>(renderer.getCellSize());
                    int y = mouseEvent->position.y / static_cast<int>(renderer.getCellSize());
                    
                    board.handleClick(x, y);
                    renderer.startClickAnimation();
                }
            }
        }

        // Render
        renderer.render();

        solver.makeMove();
        
    }
}