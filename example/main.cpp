#include "engine/Colors.hpp"
#include "engine/Kohonen.hpp"
#include "engine/SvgPlot.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

void printResult(const char* title, const engine::Result& result) {
    std::cout << YELLOW << BOLD << title << RESET << "\n"
              << "Время обучения (медиана, мкс): " << result.m_microseconds << "\n"
              << "Среднее расстояние до победителя: " << result.m_meanError << "\n";
    for (std::size_t i = 0; i < result.m_clusters.size(); i++) {
        const auto& cluster = result.m_clusters[i];
        std::cout << "Нейрон " << i + 1 << ", точек " << cluster.m_count
                  << ", вес (" << cluster.m_weight.m_x << ", " << cluster.m_weight.m_y << ")";
        if (cluster.m_count != 0)
            std::cout << ", центр (" << cluster.m_center.m_x << ", " << cluster.m_center.m_y
                      << "), масштабированный вес (" << cluster.m_scaled.m_x << ", "
                      << cluster.m_scaled.m_y << ")";
        std::cout << "\n";
    }
}

void run(const std::vector<engine::Point>& points, double rate) {
    auto result = engine::compare(points, 5, rate);
    std::cout << std::fixed << std::setprecision(4) << "Начальные веса:\n";
    for (const auto& weight : result.m_initial)
        std::cout << "(" << weight.m_x << ", " << weight.m_y << ")\n";

    printResult("Победитель забирает всё", result.m_classic);
    printResult("Обучение с утомляемостью", result.m_fatigue);
    std::cout << "Отношение времени (с утомляемостью / классическое): "
              << result.m_fatigue.m_microseconds / result.m_classic.m_microseconds << "\n";

    engine::writePlot("plots/01_points.svg", "Вариант 10. Исходные точки", points);
    engine::writePlot("plots/02_classic_normalized.svg", "Классический алгоритм. Единичные веса",
                      result.m_normalized, &result.m_classic, true);
    engine::writePlot("plots/03_classic_scaled.svg", "Классический алгоритм. Масштабированные веса",
                      points, &result.m_classic);
    engine::writePlot("plots/04_fatigue_normalized.svg", "С утомляемостью. Единичные веса",
                      result.m_normalized, &result.m_fatigue, true);
    engine::writePlot("plots/05_fatigue_scaled.svg", "С утомляемостью. Масштабированные веса",
                      points, &result.m_fatigue);
    std::cout << GREEN << "Графики сохранены в plots/" << RESET << "\n";
}

int main() {
    try {
        std::ifstream input("data/variant10.txt");
        std::vector<engine::Point> points;
        std::string row;
        while (std::getline(input, row)) {
            std::istringstream values(row);
            engine::Point point;
            if (!(values >> point.m_x >> point.m_y) || !(values >> std::ws).eof() ||
                !std::isfinite(std::hypot(point.m_x, point.m_y)) ||
                (point.m_x == 0 && point.m_y == 0))
                throw std::runtime_error("Ожидалась ненулевая пара координат в data/variant10.txt");
            points.push_back(point);
        }
        if (points.size() != 17 || !input.eof())
            throw std::runtime_error("Ожидалось 17 пар в data/variant10.txt");

        std::filesystem::create_directories("plots");

        std::string choice;
        while (true) {
            std::cout << CYAN << BOLD << "\n=== Карта Кохонена. Вариант 10 ===\n" << RESET
                      << "1. Построить график исходных точек\n"
                      << "2. Обучить и сравнить алгоритмы\n"
                      << "0. Выход\n> ";
            if (!std::getline(std::cin, choice) || choice == "0")
                break;
            if (choice == "1") {
                engine::writePlot("plots/01_points.svg", "Вариант 10. Исходные точки", points);
                std::cout << "График сохранён в plots/01_points.svg\n";
            } else if (choice == "2") {
                std::cout << "Начальная скорость обучения [0.3–0.7, Enter = 0.5]: ";
                std::string line;
                if (!std::getline(std::cin, line))
                    break;
                std::replace(line.begin(), line.end(), ',', '.');
                double rate = 0.5;
                std::istringstream value(line);
                if (!line.empty() && (!(value >> rate) || !(value >> std::ws).eof() ||
                                      !(rate >= 0.3 && rate <= 0.7))) {
                    std::cout << RED << "Ожидается число от 0.3 до 0.7\n" << RESET;
                    continue;
                }
                run(points, rate);
            } else {
                std::cout << RED << "Выберите пункт меню\n" << RESET;
            }
        }
    } catch (const std::exception& error) {
        std::cerr << RED << error.what() << RESET << "\n";
        return 1;
    }
}
