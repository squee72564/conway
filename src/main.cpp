#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Vector2.hpp>

#include <functional>
#include <optional>
#include <unordered_set>
#include <iostream>

constexpr int PARTICLE_SIZE = 10;

struct V2Hash {
    std::size_t operator() (const sf::Vector2i& v) {
        std::size_t h1 = std::hash<int>{}(v.x);
        std::size_t h2 = std::hash<int>{}(v.y);
        return h1 ^ (h2 << 1);
    }
};

void draw_particles(const std::unordered_set<sf::Vector2i, V2Hash>& p, sf::RenderWindow& window) {
    sf::RectangleShape rect{{PARTICLE_SIZE, PARTICLE_SIZE}};
    for (const sf::Vector2i& particle : p) {
        const float x_start = particle.x * PARTICLE_SIZE;
        const float y_start = particle.y * PARTICLE_SIZE;

        rect.setPosition({
            x_start, y_start
        });

        window.draw(rect);
    }
}

void add_rectangle(const std::unordered_set<sf::Vector2i, V2Hash>& p, sf::RenderWindow& window, sf::Vector2i mouse_pos) {
    auto global_pos = window.mapPixelToCoords(
        mouse_pos
    );

    std::cout << "POS: " << global_pos.x << ", " << global_pos.y << "\n";
}

void moveScreenToMouse(sf::RenderWindow& window, sf::Vector2f mouse_pos_f) {
    sf::View view = window.getView();
    view.move((mouse_pos_f - window.mapPixelToCoords(sf::Mouse::getPosition())));
    window.setView(view);
}

int main() {
    sf::VideoMode mode({1280,480});
    sf::RenderWindow window(mode, "Conway's Game Of Life");

    std::unordered_set<sf::Vector2i, V2Hash> current;
    std::unordered_set<sf::Vector2i, V2Hash> next;

    bool is_paused{false};
    bool is_mbr_down{false};
    sf::Vector2f mouse_pos_f{};

    sf::RectangleShape center_rect{{100, 100}};

    while (window.isOpen()) {

        if (is_mbr_down) {
            moveScreenToMouse(window, mouse_pos_f);
        }

        window.clear();

        if (std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (auto maybe_event = event->getIf<sf::Event::KeyPressed>()) {
                switch(maybe_event->code) {
                    case sf::Keyboard::Key::P:
                    is_paused = !is_paused;
                        break;
                    case sf::Keyboard::Key::Escape:
                        window.close();
                    default:
                        break;
                }
            }

            if (auto maybe_event = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (maybe_event->button == sf::Mouse::Button::Left) {
                    add_rectangle(current, window, maybe_event->position);
                }

                if (maybe_event->button == sf::Mouse::Button::Right) {
                    is_mbr_down = true;
                    mouse_pos_f = window.mapPixelToCoords(maybe_event->position);
                }
            }

            if (auto maybe_event = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (maybe_event->button == sf::Mouse::Button::Right) {
                    is_mbr_down = false;
                }
            }

        }

        // Do some calculation for the next frame and swap
        
        current = std::move(next);

        window.draw(center_rect);
        window.display();
    }
    return 0;
}
