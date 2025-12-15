#include <SFML/Graphics.hpp>
#include "Board.h"
#include "BoardRenderer.h"
#include <iostream>
#include <optional>
#include <cstdlib>
#include <ctime>
#include "algoSolver.cpp"
#include "heatmapSolver.cpp"

enum SolverType { ALGO_SOLVER, HEATMAP_SOLVER };

int main() {    
    // Seed random number generator for different results each run
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Create the main window with initial size (board + mode indicator + side panel + heatmap)
    sf::RenderWindow window(sf::VideoMode({670, 830}), "Minesweeper");
    
    Board board;
    BoardRenderer renderer(board, window);
    algoSolver algoSolverInstance(board, &renderer);
    heatmapSolver heatmapSolverInstance(board, &renderer);
    
    // Current solver selection (default to algo solver)
    SolverType currentSolver = ALGO_SOLVER;
    
    // Stop both solvers by default
    algoSolverInstance.stop();
    heatmapSolverInstance.stop();

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
                    if (currentSolver == ALGO_SOLVER) {
                        float currentSpeed = algoSolverInstance.getSpeed();
                        if (keyEvent->code == sf::Keyboard::Key::Equal) {
                            algoSolverInstance.setSpeed(currentSpeed + 0.5f);
                            std::cout << "Speed increased to " << algoSolverInstance.getSpeed() << "x" << std::endl;
                        } else {
                            algoSolverInstance.setSpeed(currentSpeed - 0.5f);
                            std::cout << "Speed decreased to " << algoSolverInstance.getSpeed() << "x" << std::endl;
                        }
                    } else {
                        float currentSpeed = heatmapSolverInstance.getSpeed();
                        if (keyEvent->code == sf::Keyboard::Key::Equal) {
                            heatmapSolverInstance.setSpeed(currentSpeed + 0.5f);
                            std::cout << "Speed increased to " << heatmapSolverInstance.getSpeed() << "x" << std::endl;
                        } else {
                            heatmapSolverInstance.setSpeed(currentSpeed - 0.5f);
                            std::cout << "Speed decreased to " << heatmapSolverInstance.getSpeed() << "x" << std::endl;
                        }
                    }
                }
                
                // Solver selection: 1 for Algo, 2 for Heatmap
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Num1) {
                    // Stop current solver
                    if (currentSolver == ALGO_SOLVER) {
                        algoSolverInstance.stop();
                    } else {
                        heatmapSolverInstance.stop();
                    }
                    currentSolver = ALGO_SOLVER;
                    std::cout << "Switched to Algo Solver (press G to start)" << std::endl;
                }
                
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::Num2) {
                    // Stop current solver
                    if (currentSolver == ALGO_SOLVER) {
                        algoSolverInstance.stop();
                    } else {
                        heatmapSolverInstance.stop();
                    }
                    currentSolver = HEATMAP_SOLVER;
                    std::cout << "Switched to Heatmap Solver (press G to start)" << std::endl;
                }
                
                // Start/Stop solver: G to toggle
                if (keyEvent && keyEvent->code == sf::Keyboard::Key::G) {
                    if (currentSolver == ALGO_SOLVER) {
                        if (algoSolverInstance.isActive()) {
                            algoSolverInstance.stop();
                        } else {
                            algoSolverInstance.start();
                        }
                    } else {
                        if (heatmapSolverInstance.isActive()) {
                            heatmapSolverInstance.stop();
                        } else {
                            heatmapSolverInstance.start();
                        }
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
                    // Check if click is on Start/Stop button
                    if (renderer.isStartStopButtonClicked(mouseEvent->position.x, mouseEvent->position.y)) {
                        // Toggle current solver
                        if (currentSolver == ALGO_SOLVER) {
                            if (algoSolverInstance.isActive()) {
                                algoSolverInstance.stop();
                            } else {
                                algoSolverInstance.start();
                            }
                        } else {
                            if (heatmapSolverInstance.isActive()) {
                                heatmapSolverInstance.stop();
                            } else {
                                heatmapSolverInstance.start();
                            }
                        }
                    } else {
                        // Regular board click - check bounds
                        int x = mouseEvent->position.x / static_cast<int>(renderer.getCellSize());
                        int y = mouseEvent->position.y / static_cast<int>(renderer.getCellSize());
                        
                        // Only handle click if within board boundaries
                        if (x >= 0 && x < board.getGridSize() && y >= 0 && y < board.getGridSize()) {
                            board.handleClick(x, y);
                            renderer.startClickAnimation();
                        }
                    }
                }
            }
        }

        // Render
        renderer.render();
        
        // Display stats for current solver
        if (currentSolver == ALGO_SOLVER) {
            renderer.drawStatsAndControls(algoSolverInstance.getWins(), algoSolverInstance.getLosses(), 
                                         algoSolverInstance.getSpeed(), "Algo", algoSolverInstance.isActive());
        } else {
            // Pass heatmap data for visualization
            auto heatmapData = heatmapSolverInstance.getHeatmapData();
            renderer.drawStatsAndControls(heatmapSolverInstance.getWins(), heatmapSolverInstance.getLosses(), 
                                         heatmapSolverInstance.getSpeed(), "Heatmap", heatmapSolverInstance.isActive(),
                                         &heatmapData);
        }
        
        renderer.finishFrame();

        // Make move with current solver
        if (currentSolver == ALGO_SOLVER) {
            algoSolverInstance.makeMove();
        } else {
            heatmapSolverInstance.makeMove();
        }
        
    }
}