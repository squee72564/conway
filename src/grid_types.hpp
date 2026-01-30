#pragma once

#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <cstddef>
#include <functional>

struct V2HashCombine {
    std::size_t operator()(const sf::Vector2i& v) const noexcept {
        std::size_t seed = std::hash<int>{}(v.x);
        seed ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

struct V2HashPackedSplit {
    static inline std::uint64_t splitmix64(std::uint64_t x) noexcept {
        x += 0x9e3779b97f4a7c15ull;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebull;
        return x ^ (x >> 31);
    }

    std::size_t operator()(const sf::Vector2i& v) const noexcept {
        const std::uint64_t ux = static_cast<std::uint32_t>(v.x);
        const std::uint64_t uy = static_cast<std::uint32_t>(v.y);
        const std::uint64_t key = (ux << 32) | uy;
        return static_cast<std::size_t>(splitmix64(key));
    }
};

using V2Hash = V2HashCombine;
