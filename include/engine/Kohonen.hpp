#pragma once
#include <cstddef>
#include <vector>

namespace engine {
    struct Point {
        double m_x = 0;
        double m_y = 0;
    };

    struct Cluster {
        Point m_weight;
        Point m_center;
        Point m_scaled;
        std::size_t m_count = 0;
    };

    struct Result {
        std::vector<Cluster> m_clusters;
        std::vector<std::size_t> m_labels;
        double m_microseconds = 0;
        double m_meanError = 0;
    };

    struct Comparison {
        std::vector<Point> m_normalized;
        std::vector<Point> m_initial;
        Result m_classic;
        Result m_fatigue;
    };

    Comparison compare(const std::vector<Point>& points, std::size_t count, double rate);
}
