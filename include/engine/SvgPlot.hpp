#pragma once
#include "engine/Kohonen.hpp"
#include <string>

namespace engine {
    void writePlot(const std::string& path, const std::string& title,
                   const std::vector<Point>& points, const Result* result = nullptr,
                   bool normalized = false);
}
