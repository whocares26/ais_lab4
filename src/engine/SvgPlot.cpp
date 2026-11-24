#include "engine/SvgPlot.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <stdexcept>

namespace engine {
    void writePlot(const std::string& path, const std::string& title,
                   const std::vector<Point>& points, const Result* result, bool normalized) {
        const std::array<const char*, 5> colors = {
            "#1967b2", "#bf4031", "#187548", "#8053a0", "#a26200"
        };
        double minX = 0, maxX = 0, minY = 0, maxY = 0;
        for (Point point : points) {
            minX = std::min(minX, point.m_x);
            maxX = std::max(maxX, point.m_x);
            minY = std::min(minY, point.m_y);
            maxY = std::max(maxY, point.m_y);
        }
        if (normalized) {
            minX = minY = -1;
            maxX = maxY = 1;
        }
        const double step = normalized ? 0.5 : 5.0;
        minX = std::floor(minX / step) * step;
        maxX = std::ceil(maxX / step) * step;
        minY = std::floor(minY / step) * step;
        maxY = std::ceil(maxY / step) * step;
        double scale = std::min(650 / std::max(step, maxX - minX),
                                420 / std::max(step, maxY - minY));
        auto x = [&](double value) { return 380 + (value - (minX + maxX) / 2) * scale; };
        auto y = [&](double value) { return 270 - (value - (minY + maxY) / 2) * scale; };

        std::ofstream out(path);
        out.exceptions(std::ios::failbit | std::ios::badbit);
        out << "<svg xmlns='http://www.w3.org/2000/svg' width='760' height='530' viewBox='0 0 760 530'>\n"
            << "<rect width='760' height='530' fill='white'/>\n"
            << "<g font-family='sans-serif' font-size='12' fill='#243047'>\n"
            << "<text x='380' y='31' text-anchor='middle' font-size='20'>" << title << "</text>\n";
        auto line = [&](double x1, double y1, double x2, double y2, const char* color, int width) {
            out << "<line x1='" << x1 << "' y1='" << y1 << "' x2='" << x2 << "' y2='" << y2
                << "' stroke='" << color << "' stroke-width='" << width << "'/>\n";
        };
        for (double tick = minX; tick <= maxX; tick += step) {
            line(x(tick), y(minY), x(tick), y(maxY), tick == 0 ? "#687689" : "#dbe2ea", 1);
            out << "<text x='" << x(tick) << "' y='" << y(minY) + 18
                << "' text-anchor='middle'>" << tick << "</text>\n";
        }
        for (double tick = minY; tick <= maxY; tick += step) {
            line(x(minX), y(tick), x(maxX), y(tick), tick == 0 ? "#687689" : "#dbe2ea", 1);
            out << "<text x='" << x(minX) - 8 << "' y='" << y(tick) + 4
                << "' text-anchor='end'>" << tick << "</text>\n";
        }
        out << "<text x='" << x(maxX) + 13 << "' y='" << y(minY) + 18 << "'>x</text>\n"
            << "<text x='" << x(minX) - 8 << "' y='" << y(maxY) - 12 << "'>y</text>\n";

        for (std::size_t i = 0; i < points.size(); i++) {
            const char* color = result ? colors[result->m_labels[i]] : "#243047";
            out << "<circle cx='" << x(points[i].m_x) << "' cy='" << y(points[i].m_y)
                << "' r='4' fill='" << color << "' fill-opacity='.8'/>\n";
        }
        if (result) {
            for (std::size_t i = 0; i < result->m_clusters.size(); i++) {
                const Cluster& cluster = result->m_clusters[i];
                if (!normalized && cluster.m_count == 0)
                    continue;
                Point end = normalized ? cluster.m_weight : cluster.m_scaled;
                double px = x(end.m_x), py = y(end.m_y);
                double angle = std::atan2(y(0) - py, x(0) - px);
                line(x(0), y(0), px, py, colors[i], 2);
                for (double delta : {-0.48, 0.48})
                    line(px, py, px + 10 * std::cos(angle + delta),
                         py + 10 * std::sin(angle + delta), colors[i], 2);
                out << "<text x='" << 58 + 135 * i << "' y='510' fill='" << colors[i]
                    << "'>Нейрон " << i + 1 << " (" << cluster.m_count << ")</text>\n";
            }
        }
        out << "</g></svg>\n";
        out.close();
    }
}
