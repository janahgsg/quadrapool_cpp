#include <SFML/Graphics.hpp>
#include <cstdint>
#include <array>
#include <vector>
#include <optional>
#include <random>

// Board configuration
static constexpr std::int32_t COLS = 10;
static constexpr std::int32_t ROWS = 20;
static constexpr std::int32_t TILE = 32;

// Colors for tetromino IDs
const std::array<sf::Color, 8> COLORS = {
    sf::Color(0,0,0),            // 0 empty
    sf::Color(0,255,255),        // 1 I
    sf::Color(0,0,255),          // 2 J
    sf::Color(255,165,0),        // 3 L
    sf::Color(255,255,0),        // 4 O
    sf::Color(0,255,0),          // 5 S
    sf::Color(255,0,0),          // 6 Z
    sf::Color(128,0,128)         // 7 T
};

// Tetromino definitions: for each piece, 4 rotation states of 4 cells
using Shape = std::array<std::array<sf::Vector2i,4>,4>;

// Helper to define rotations compactly
static Shape makeShape(const std::array<sf::Vector2i,4>& r0,
                       const std::array<sf::Vector2i,4>& r1,
                       const std::array<sf::Vector2i,4>& r2,
                       const std::array<sf::Vector2i,4>& r3) {
    return { r0, r1, r2, r3 };
}

// Classic Tetris pieces
const std::array<Shape,7> PIECES = {
    // I
    makeShape(
        std::array<sf::Vector2i,4>{sf::Vector2i{0,1}, {1,1}, {2,1}, {3,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{2,0}, {2,1}, {2,2}, {2,3}},
        std::array<sf::Vector2i,4>{sf::Vector2i{0,2}, {1,2}, {2,2}, {3,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {1,1}, {1,2}, {1,3}}
    ),
    // J
    makeShape(
        std::array<sf::Vector2i,4>{sf::Vector2i{0,0}, {0,1}, {1,1}, {2,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {2,0}, {1,1}, {1,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{0,1}, {1,1}, {2,1}, {2,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {1,1}, {0,2}, {1,2}}
    ),
    // L
    makeShape(
        std::array<sf::Vector2i,4>{sf::Vector2i{2,0}, {0,1}, {1,1}, {2,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {1,1}, {1,2}, {2,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{0,1}, {1,1}, {2,1}, {0,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{0,0}, {1,0}, {1,1}, {1,2}}
    ),
    // O
    makeShape(
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {2,0}, {1,1}, {2,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {2,0}, {1,1}, {2,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {2,0}, {1,1}, {2,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {2,0}, {1,1}, {2,1}}
    ),
    // S
    makeShape(
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {2,0}, {0,1}, {1,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {1,1}, {2,1}, {2,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,1}, {2,1}, {0,2}, {1,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{0,0}, {0,1}, {1,1}, {1,2}}
    ),
    // Z
    makeShape(
        std::array<sf::Vector2i,4>{sf::Vector2i{0,0}, {1,0}, {1,1}, {2,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{2,0}, {1,1}, {2,1}, {1,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{0,1}, {1,1}, {1,2}, {2,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {0,1}, {1,1}, {0,2}}
    ),
    // T
    makeShape(
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {0,1}, {1,1}, {2,1}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {1,1}, {2,1}, {1,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{0,1}, {1,1}, {2,1}, {1,2}},
        std::array<sf::Vector2i,4>{sf::Vector2i{1,0}, {0,1}, {1,1}, {1,2}}
    )
};

struct Piece {
    std::int32_t id = 0;           // 1..7
    std::int32_t rot = 0;          // 0..3
    sf::Vector2i pos {3, 0};       // board position
};

using Board = std::array<std::array<std::int32_t, COLS>, ROWS>;

bool collision(const Board& b, const Piece& p) {
    const auto& shape = PIECES[p.id - 1][p.rot];
    for (const auto& cell : shape) {
        sf::Vector2i c = p.pos + cell;
        if (c.x < 0 || c.x >= COLS || c.y < 0 || c.y >= ROWS) return true;
        if (b[c.y][c.x] != 0) return true;
    }
    return false;
}

void lockPiece(Board& b, const Piece& p) {
    const auto& shape = PIECES[p.id - 1][p.rot];
    for (const auto& cell : shape) {
        sf::Vector2i c = p.pos + cell;
        if (c.y >= 0 && c.y < ROWS && c.x >= 0 && c.x < COLS)
            b[c.y][c.x] = p.id;
    }
}

std::int32_t clearLines(Board& b) {
    std::int32_t cleared = 0;
    for (std::int32_t y = ROWS - 1; y >= 0; --y) {
        bool full = true;
        for (std::int32_t x = 0; x < COLS; ++x)
            if (b[y][x] == 0) { full = false; break; }
        if (full) {
            ++cleared;
            // pull everything down
            for (std::int32_t yy = y; yy > 0; --yy)
                b[yy] = b[yy - 1];
            b[0].fill(0);
            ++y; // re-check same row after pull
        }
    }
    return cleared;
}

Piece randomPiece(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(1,7);
    Piece p;
    p.id = dist(rng);
    p.rot = 0;
    p.pos = {3, 0};
    return p;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({COLS * TILE, ROWS * TILE}, 32), "SFML 3 Tetris");
    window.setFramerateLimit(60);

    Board board{};
    for (auto& row : board) row.fill(0);

    std::random_device rd;
    std::mt19937 rng(rd());
    Piece current = randomPiece(rng);
    float gravity = 0.5f;     // tiles per second
    float dropAcc = 0.0f;
    bool gameOver = false;
    std::int32_t score = 0;

    sf::Clock clock;

    while (window.isOpen()) {
        // Event handling (SFML 3 style)
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (gameOver) continue;

                if (kp->scancode == sf::Keyboard::Scancode::Left) {
                    Piece test = current; test.pos.x -= 1;
                    if (!collision(board, test)) current = test;
                } else if (kp->scancode == sf::Keyboard::Scancode::Right) {
                    Piece test = current; test.pos.x += 1;
                    if (!collision(board, test)) current = test;
                } else if (kp->scancode == sf::Keyboard::Scancode::Up) {
                    Piece test = current; test.rot = (test.rot + 1) % 4;
                    // simple wall kick: try small offsets
                    if (collision(board, test)) {
                        for (int dx : {-1, 1, -2, 2}) {
                            Piece kick = test; kick.pos.x += dx;
                            if (!collision(board, kick)) { current = kick; goto rotated; }
                        }
                    } else {
                        current = test;
                    }
                    rotated: ;
                } else if (kp->scancode == sf::Keyboard::Scancode::Down) {
                    // soft drop
                    Piece test = current; test.pos.y += 1;
                    if (!collision(board, test)) current = test;
                    else { lockPiece(board, current); score += clearLines(board) * 100; current = randomPiece(rng);
                           if (collision(board, current)) gameOver = true; }
                } else if (kp->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                }
            }
        }

        // Gravity
        float dt = clock.restart().asSeconds();
        dropAcc += gravity * dt;
        while (dropAcc >= 1.0f && !gameOver) {
            dropAcc -= 1.0f;
            Piece test = current; test.pos.y += 1;
            if (!collision(board, test)) {
                current = test;
            } else {
                lockPiece(board, current);
                score += clearLines(board) * 100;
                current = randomPiece(rng);
                if (collision(board, current)) gameOver = true;
            }
        }

        // Draw
        window.clear(sf::Color(30, 30, 30));

        // Draw locked board
        for (std::int32_t y = 0; y < ROWS; ++y) {
            for (std::int32_t x = 0; x < COLS; ++x) {
                if (board[y][x] != 0) {
                    sf::RectangleShape r({static_cast<float>(TILE - 1), static_cast<float>(TILE - 1)});
                    r.setPosition({static_cast<float>(x * TILE), static_cast<float>(y * TILE)});
                    r.setFillColor(COLORS[board[y][x]]);
                    window.draw(r);
                }
            }
        }

        // Draw current piece
        if (!gameOver) {
            const auto& shape = PIECES[current.id - 1][current.rot];
            for (const auto& cell : shape) {
                sf::Vector2i c = current.pos + cell;
                sf::RectangleShape r({static_cast<float>(TILE - 1), static_cast<float>(TILE - 1)});
                r.setPosition({static_cast<float>(c.x * TILE), static_cast<float>(c.y * TILE)});
                r.setFillColor(COLORS[current.id]);
                window.draw(r);
            }
        }

        // Simple “score / game over” banner
        // (You can replace with sf::Text after adding a font)
        if (gameOver) {
            // Draw a translucent overlay
            sf::RectangleShape overlay({static_cast<float>(COLS * TILE), static_cast<float>(ROWS * TILE)});
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);
        }

        window.display();
    }

    return 0;
}
