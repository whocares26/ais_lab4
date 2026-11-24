#include "engine/Kohonen.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>
#include <random>

namespace engine {
    namespace {
        constexpr int epochs = 100;
        constexpr int repeats = 101;

        double dot(Point a, Point b) {
            return a.m_x * b.m_x + a.m_y * b.m_y;
        }

        Point scale(Point a, double factor) {
            return {a.m_x * factor, a.m_y * factor};
        }

        Point normalize(Point a) {
            return scale(a, 1.0 / std::hypot(a.m_x, a.m_y));
        }

        std::size_t winner(Point point, const std::vector<Point>& weights,
                           const std::vector<double>& fatigue) {
            std::size_t best = 0;
            double bestScore = dot(point, weights[0]) - fatigue[0];
            for (std::size_t i = 1; i < weights.size(); i++) {
                double score = dot(point, weights[i]) - fatigue[i];
                if (score > bestScore) {
                    best = i;
                    bestScore = score;
                }
            }
            return best;
        }

        void train(const std::vector<Point>& points, std::vector<Point>& weights,
                   const std::vector<std::size_t>& order, double rate, bool useFatigue) {
            std::vector<double> fatigue(weights.size(), 0);
            for (std::size_t i = 0; i < order.size(); i++) {
                if (i != 0 && i % points.size() == 0)
                    rate *= 0.96;

                Point point = points[order[i]];
                std::size_t best = winner(point, weights, fatigue);
                Point& weight = weights[best];
                weight = normalize({
                    weight.m_x + rate * (point.m_x - weight.m_x),
                    weight.m_y + rate * (point.m_y - weight.m_y)
                });

                // Штраф победителя влияет на следующий выбор и постепенно затухает.
                if (useFatigue) {
                    for (double& value : fatigue)
                        value *= 0.85;
                    fatigue[best] += 0.08;
                }
            }
        }

        Result classify(const std::vector<Point>& points, const std::vector<Point>& normalized,
                        const std::vector<Point>& weights, double time) {
            Result result;
            result.m_microseconds = time;
            result.m_clusters.resize(weights.size());
            std::vector<double> fatigue(weights.size(), 0);
            for (std::size_t i = 0; i < weights.size(); i++)
                result.m_clusters[i].m_weight = weights[i];

            for (std::size_t i = 0; i < points.size(); i++) {
                std::size_t best = winner(normalized[i], weights, fatigue);
                result.m_labels.push_back(best);
                Cluster& cluster = result.m_clusters[best];
                cluster.m_count++;
                cluster.m_center.m_x += points[i].m_x;
                cluster.m_center.m_y += points[i].m_y;
                result.m_meanError += std::hypot(normalized[i].m_x - weights[best].m_x,
                                                normalized[i].m_y - weights[best].m_y);
            }

            result.m_meanError /= points.size();
            for (Cluster& cluster : result.m_clusters) {
                if (cluster.m_count == 0)
                    continue;
                cluster.m_center = scale(cluster.m_center, 1.0 / cluster.m_count);
                double projection = dot(cluster.m_center, cluster.m_weight);
                cluster.m_scaled = scale(cluster.m_weight, std::max(0.0, projection));
            }
            return result;
        }

        double median(std::vector<double>& times) {
            std::sort(times.begin(), times.end());
            return times[times.size() / 2];
        }

        double timedTrain(const std::vector<Point>& points, std::vector<Point>& weights,
                          const std::vector<std::size_t>& order, double rate, bool useFatigue) {
            auto start = std::chrono::steady_clock::now();
            train(points, weights, order, rate, useFatigue);
            return std::chrono::duration<double, std::micro>(
                std::chrono::steady_clock::now() - start).count();
        }
    }

    Comparison compare(const std::vector<Point>& points, std::size_t count, double rate) {
        Comparison comparison;
        for (Point point : points)
            comparison.m_normalized.push_back(normalize(point));

        // Все точки варианта 10 лежат в нижней полуплоскости.
        std::mt19937 random(10);
        std::uniform_real_distribution<double> coordinate(-1.0, 1.0);
        for (std::size_t i = 0; i < count; i++) {
            Point weight;
            do {
                weight = {coordinate(random), -std::abs(coordinate(random))};
            } while (weight.m_x == 0 && weight.m_y == 0);
            comparison.m_initial.push_back(normalize(weight));
        }

        std::vector<std::size_t> indices(points.size()), order;
        std::iota(indices.begin(), indices.end(), 0);
        for (int epoch = 0; epoch < epochs; epoch++) {
            std::shuffle(indices.begin(), indices.end(), random);
            order.insert(order.end(), indices.begin(), indices.end());
        }

        std::vector<double> classicTimes, fatigueTimes;
        std::vector<Point> classic, fatigue;
        for (int run = 0; run < repeats; run++) {
            classic = comparison.m_initial;
            fatigue = comparison.m_initial;
            if (run % 2 == 0) {
                classicTimes.push_back(timedTrain(comparison.m_normalized, classic, order, rate, false));
                fatigueTimes.push_back(timedTrain(comparison.m_normalized, fatigue, order, rate, true));
            } else {
                fatigueTimes.push_back(timedTrain(comparison.m_normalized, fatigue, order, rate, true));
                classicTimes.push_back(timedTrain(comparison.m_normalized, classic, order, rate, false));
            }
        }

        comparison.m_classic = classify(points, comparison.m_normalized, classic, median(classicTimes));
        comparison.m_fatigue = classify(points, comparison.m_normalized, fatigue, median(fatigueTimes));
        return comparison;
    }
}
