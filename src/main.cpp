#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Vector2.hpp>

#include <functional>
#include <optional>
#include <unordered_set>

constexpr int PARTICLE_SIZE = 1;

struct V2Hash {
    std::size_t operator() (const sf::Vector2i& v) const {
        std::size_t seed = std::hash<int>{}(v.x);
        seed ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
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

void add_rectangle(std::unordered_set<sf::Vector2i, V2Hash>& particles, sf::RenderWindow& window, const sf::Vector2f& mouse_pos) {
    const int kernel_size = 25;
    int mouse_x_i = static_cast<int>(mouse_pos.x);
    int mouse_y_i = static_cast<int>(mouse_pos.y);
    for (int i = mouse_x_i - kernel_size/2; i < mouse_x_i + kernel_size/2; ++i) {
        for (int j = mouse_y_i - kernel_size/2; j < mouse_y_i + kernel_size/2; ++j) {
            particles.emplace(
                i, j
            );
        }
    }
}

void moveScreenToMouse(sf::RenderWindow& window, sf::Vector2f mouse_pos_f) {
    sf::View view = window.getView();
    view.move((mouse_pos_f - window.mapPixelToCoords(sf::Mouse::getPosition(window), view)) * 0.012f);
    window.setView(view);
}

int main() {
    sf::VideoMode mode({800,800});
    sf::RenderWindow window(mode, "Conway's Game Of Life");

    std::unordered_set<sf::Vector2i, V2Hash> current;
    std::unordered_set<sf::Vector2i, V2Hash> dead;
    std::unordered_set<sf::Vector2i, V2Hash> next;

    current.reserve(10000);
    dead.reserve(10000);
    next.reserve(10000);

    bool is_paused{false};
    bool is_mbr_down{false};
    bool is_mbl_down{false};
    sf::Vector2f mouse_pos_f{};

    sf::RectangleShape center_rect{{PARTICLE_SIZE, PARTICLE_SIZE}};

    while (window.isOpen()) {

        if (is_mbr_down) {
            moveScreenToMouse(window, mouse_pos_f);
        }

        if (is_mbl_down) {
            add_rectangle(current, window, window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getView()));
        }


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
                    is_mbl_down = true;
                }

                if (maybe_event->button == sf::Mouse::Button::Right) {
                    is_mbr_down = true;
                    mouse_pos_f = window.mapPixelToCoords(maybe_event->position);
                }
            }

            if (auto maybe_event = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (maybe_event->button == sf::Mouse::Button::Left) {
                    is_mbl_down = false;
                }

                if (maybe_event->button == sf::Mouse::Button::Right) {
                    is_mbr_down = false;
                }
            }

        }

        window.clear();

        sf::VertexArray vertices(sf::PrimitiveType::Points, current.size());

        // Draw current
        int i{ 0 };
        for (const auto& particle : current) {
            vertices[i++] = sf::Vertex{{static_cast<float>(particle.x), static_cast<float>(particle.y)}, sf::Color::White};
        }

        window.draw(vertices);
        
        if (!is_paused) {

            std::array<std::pair<int,int>, 8> offsets = {{
                {-1,1}, {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}
            }};
            
            // Check live cells and get adjacent dead cells
            for (const auto& [x,y]: current) {
                int count{ 0 };
                for (const auto& [dx,dy] : offsets) {
                    const sf::Vector2i new_pos{x + dx, y + dy};

                    if (current.count(new_pos)) {
                        count++; 
                    } else if (!dead.count(new_pos)) {
                        dead.emplace(std::move(new_pos));
                    }
                }

                // Survival Rules
                if (count == 3 || count == 2) {
                    next.emplace(x, y);
                }
            }

            // Check dead cells
            for (const auto& [x,y] : dead) {
                int count{ 0 }; 
                for (const auto& [dx,dy] : offsets) {
                    if (current.count({x + dx, y + dy})) count++;
                }

                if (count == 3) next.emplace(x,y);
            }

            // Update next based off current
            current = std::move(next);
            next.clear();
            dead.clear();
        }

        window.display();
    }
    return 0;
}
