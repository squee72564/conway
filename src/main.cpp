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

#include <optional>
#include <unordered_set>
#include <unordered_map>

constexpr int PARTICLE_SIZE = 3;

struct V2Hash {
    std::size_t operator() (const sf::Vector2i& v) const {
        std::size_t seed = std::hash<int>{}(v.x);
        seed ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

void add_rectangle(std::unordered_set<sf::Vector2i, V2Hash>& particles, sf::RenderWindow& window, const sf::Vector2f& mouse_pos);
void moveScreenToMouse(sf::RenderWindow& window, sf::Vector2f mouse_pos_f);

int main() {
    sf::VideoMode mode({1240,800});
    sf::RenderWindow window(mode, "Conway's Game Of Life");
    window.setFramerateLimit(60);

    std::unordered_set<sf::Vector2i, V2Hash> current;
    std::unordered_set<sf::Vector2i, V2Hash> next;
    std::unordered_map<sf::Vector2i, int, V2Hash> neighbor_counts;

    sf::VertexArray vertices(sf::PrimitiveType::Points);

    current.reserve(2048);
    next.reserve(2048);
    neighbor_counts.reserve(2048 * 8);

    bool is_paused{false};
    bool is_mbr_down{false};
    bool is_mbl_down{false};
    bool vertices_dirty{true};
    sf::Vector2f mouse_pos_f{};

    constexpr std::array<std::pair<int,int>, 8> offsets = {{
        {-1,1}, {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1}, {-1,-1}, {-1,0}
    }};

    while (window.isOpen()) {

        // Input handling
        if (is_mbr_down) {
            moveScreenToMouse(window, mouse_pos_f);
        }

        if (is_mbl_down) {
            add_rectangle(current, window, window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getView()));
            vertices_dirty = true;
        }

        for (auto event = window.pollEvent(); event.has_value(); event = window.pollEvent()) {
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
                        break;
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

        // Draw current cells
        if (vertices_dirty) {
            vertices.resize(current.size());
            int i = 0;
            for (const auto& pos : current) {
                vertices[i++] = sf::Vertex{
                    {
                        static_cast<float>(pos.x * PARTICLE_SIZE),
                        static_cast<float>(pos.y * PARTICLE_SIZE)
                    },
                    sf::Color::White
                };
            }
            vertices_dirty = false;
        }

        window.draw(vertices);

        if (!is_paused) {
            neighbor_counts.clear();
            neighbor_counts.reserve(current.size() * 8);
            next.clear();

            for (const auto& pos : current) {
                for (const auto& [dx, dy] : offsets) {
                    const sf::Vector2i neighbor{pos.x + dx, pos.y + dy};
                    neighbor_counts[neighbor]++;
                }
            }

            for (const auto& [pos, count] : neighbor_counts) {
                if (count == 3 || (count == 2 && current.count(pos))) {
                    next.emplace(pos);
                }
            }

            std::swap(current, next);
            vertices_dirty = true;
        }

        window.display();
    }
}

void add_rectangle(std::unordered_set<sf::Vector2i, V2Hash>& particles, sf::RenderWindow& window, const sf::Vector2f& mouse_pos) {
    const int kernel_size = 17;
    int mouse_x_i = static_cast<int>(mouse_pos.x / PARTICLE_SIZE);
    int mouse_y_i = static_cast<int>(mouse_pos.y / PARTICLE_SIZE);
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
    view.move((mouse_pos_f - window.mapPixelToCoords(sf::Mouse::getPosition(window), view)) * 0.07f);
    window.setView(view);
}
