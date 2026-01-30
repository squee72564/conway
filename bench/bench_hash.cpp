#include <SFML/System/Vector2.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

#include "grid_types.hpp"

namespace {
using Clock = std::chrono::steady_clock;

struct BenchResult {
    std::string name;
    double insert_mops;
    double lookup_mops;
};

std::vector<sf::Vector2i> make_random_points(std::size_t count, int spread, std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(-spread, spread);
    std::vector<sf::Vector2i> points;
    points.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        points.emplace_back(dist(rng), dist(rng));
    }
    return points;
}

std::vector<sf::Vector2i> make_clustered_points(std::size_t count, int spread) {
    const int cluster = std::max(8, spread / 16);
    const int step = std::max(2, cluster / 4);
    std::vector<sf::Vector2i> points;
    points.reserve(count);
    int x = -spread / 2;
    int y = -spread / 2;
    for (std::size_t i = 0; i < count; ++i) {
        points.emplace_back(x, y);
        x += step;
        if (x > spread / 2) {
            x = -spread / 2;
            y += step;
        }
        if (y > spread / 2) {
            y = -spread / 2;
        }
    }
    return points;
}

template <typename Hash>
BenchResult run_bench(const std::string& name,
                      const std::vector<sf::Vector2i>& points,
                      int trials) {
    std::vector<double> insert_samples;
    std::vector<double> lookup_samples;
    insert_samples.reserve(trials);
    lookup_samples.reserve(trials);

    for (int t = 0; t < trials; ++t) {
        std::unordered_set<sf::Vector2i, Hash> set;
        set.reserve(points.size() * 2);

        const auto insert_start = Clock::now();
        for (const auto& p : points) {
            set.emplace(p);
        }
        const auto insert_end = Clock::now();

        std::size_t hits = 0;
        const auto lookup_start = Clock::now();
        for (const auto& p : points) {
            hits += set.count(p);
        }
        const auto lookup_end = Clock::now();

        const auto insert_ns =
            std::chrono::duration_cast<std::chrono::duration<double>>(insert_end - insert_start).count();
        const auto lookup_ns =
            std::chrono::duration_cast<std::chrono::duration<double>>(lookup_end - lookup_start).count();

        const double insert_mops = (points.size() / insert_ns) / 1e6;
        const double lookup_mops = (points.size() / lookup_ns) / 1e6;
        insert_samples.push_back(insert_mops);
        lookup_samples.push_back(lookup_mops);

        if (hits == 0) {
            std::cerr << "unexpected: zero hits\n";
        }
    }

    std::nth_element(insert_samples.begin(),
                     insert_samples.begin() + insert_samples.size() / 2,
                     insert_samples.end());
    std::nth_element(lookup_samples.begin(),
                     lookup_samples.begin() + lookup_samples.size() / 2,
                     lookup_samples.end());

    const double insert_median = insert_samples[insert_samples.size() / 2];
    const double lookup_median = lookup_samples[lookup_samples.size() / 2];

    return {name, insert_median, lookup_median};
}

void print_result(const BenchResult& result) {
    std::cout << std::left << std::setw(28) << result.name
              << " insert: " << std::fixed << std::setprecision(2) << result.insert_mops << " Mops/s"
              << "  lookup: " << std::fixed << std::setprecision(2) << result.lookup_mops << " Mops/s"
              << '\n';
}
} // namespace

int main(int argc, char** argv) {
    std::size_t count = 200000;
    int spread = 4096;
    bool csv_mode = false;
    int trials = 5;

    if (argc > 1) {
        if (std::string(argv[1]) == "--csv") {
            csv_mode = true;
        } else {
            count = static_cast<std::size_t>(std::stoul(argv[1]));
        }
    }
    if (argc > 2 && !csv_mode) {
        spread = std::stoi(argv[2]);
    }
    if (argc > 3 && !csv_mode) {
        trials = std::stoi(argv[3]);
    }

    if (!csv_mode) {
        const auto points = make_random_points(count, spread, 12345u);
        const auto combine = run_bench<V2HashCombine>("combine hash", points, trials);
        const auto splitmix = run_bench<V2HashPackedSplit>("packed splitmix", points, trials);
        print_result(combine);
        print_result(splitmix);
        return 0;
    }

    const std::vector<std::size_t> counts = {50000, 100000, 200000, 500000, 1000000};
    const std::vector<int> spreads = {512, 1024, 2048, 4096, 8192, 16384};

    std::cout << "count,spread,pattern,hash,insert_mops,lookup_mops\n";
    for (const auto c : counts) {
        for (const auto s : spreads) {
            const auto random_points = make_random_points(c, s, 12345u);
            const auto clustered_points = make_clustered_points(c, s);

            const auto combine_rand = run_bench<V2HashCombine>("combine", random_points, trials);
            const auto splitmix_rand = run_bench<V2HashPackedSplit>("splitmix", random_points, trials);
            const auto combine_cluster = run_bench<V2HashCombine>("combine", clustered_points, trials);
            const auto splitmix_cluster = run_bench<V2HashPackedSplit>("splitmix", clustered_points, trials);

            std::cout << c << "," << s << ",random,combine," << combine_rand.insert_mops << "," << combine_rand.lookup_mops
                      << "\n";
            std::cout << c << "," << s << ",random,splitmix," << splitmix_rand.insert_mops << ","
                      << splitmix_rand.lookup_mops << "\n";
            std::cout << c << "," << s << ",clustered,combine," << combine_cluster.insert_mops << ","
                      << combine_cluster.lookup_mops << "\n";
            std::cout << c << "," << s << ",clustered,splitmix," << splitmix_cluster.insert_mops << ","
                      << splitmix_cluster.lookup_mops << "\n";
        }
    }
    return 0;
}
