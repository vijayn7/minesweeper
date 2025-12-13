#include <SFML/Graphics.hpp>
#include "Board.h"
#include "BoardRenderer.h"
#include <iostream>
#include <optional>
#include <cstdlib>
#include <ctime>
#include "algoSolver.cpp"

int main() {    
    // Seed random number generator for different results each run
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Create the main window with initial size (board + mode indicator + side panel)
    sf::RenderWindow window(sf::VideoMode({670, 480}), "Minesweeper");
    
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

                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Left || keyEvent->code == sf::Keyboard::Key::A)) {
                    int oldX = board.getSelectedX();
                    int oldY = board.getSelectedY();
                    board.moveLeft();
                    if (board.getSelectedX() != oldX || board.getSelectedY() != oldY) {
                        renderer.startSelectionAnimation(oldX, oldY);
                    }
                }

                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Right || keyEvent->code == sf::Keyboard::Key::D)) {
                    int oldX = board.getSelectedX();
                    int oldY = board.getSelectedY();
                    board.moveRight();
                    if (board.getSelectedX() != oldX || board.getSelectedY() != oldY) {
                        renderer.startSelectionAnimation(oldX, oldY);
                    }
                }

                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Up || keyEvent->code == sf::Keyboard::Key::W)) {
                    int oldX = board.getSelectedX();
                    int oldY = board.getSelectedY();
                    board.moveUp();
                    if (board.getSelectedX() != oldX || board.getSelectedY() != oldY) {
                        renderer.startSelectionAnimation(oldX, oldY);
                    }
                }

                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Down || keyEvent->code == sf::Keyboard::Key::S)) {
                    int oldX = board.getSelectedX();
                    int oldY = board.getSelectedY();
                    board.moveDown();
                    if (board.getSelectedX() != oldX || board.getSelectedY() != oldY) {
                        renderer.startSelectionAnimation(oldX, oldY);
                    }
                }

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
                
                // Speed controls: + or = to increase, - to decrease
                if (keyEvent && (keyEvent->code == sf::Keyboard::Key::Equal || keyEvent->code == sf::Keyboard::Key::Hyphen)) {
                    float currentSpeed = solver.getSpeed();
                    if (keyEvent->code == sf::Keyboard::Key::Equal) {
                        solver.setSpeed(currentSpeed + 0.5f);
                        std::cout << "Speed increased to " << solver.getSpeed() << "x" << std::endl;
                    } else {
                        solver.setSpeed(currentSpeed - 0.5f);
                        std::cout << "Speed decreased to " << solver.getSpeed() << "x" << std::endl;
                    }
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
        renderer.drawStatsAndControls(solver.getWins(), solver.getLosses(), solver.getSpeed());
        renderer.finishFrame();

        solver.makeMove();
        
    }
}