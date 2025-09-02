#pragma once
#include <algorithm>
#include <vector>
#include <deque>
#include <iostream>

class SingleDijkstra {
private:
    int N, M;
    const int INF = 1e9 + 9;
    std::vector<std::vector<std::pair<int, int>>> edges;
    std::vector<int> distances;
    std::vector<bool> visited;
    std::vector<int> parents;

public:
    SingleDijkstra(int N_, int M_) : N(N_), M(M_), distances(N + 1, INF), visited(N + 1, false), parents(N + 1, -1) {
        edges.resize(N + 1);
    }

    void addEdge(int from, int to, int weight) {
        edges[from].emplace_back(to, weight);
        edges[to].emplace_back(from, weight);
    }

    void runSingle(int start, int finish) {
        distances[start] = 0;
        for (int i = 1; i <= N; i++) {
            int v = -1;
            for (int u = 1; u <= N; u++) {
                if (!visited[u] && (v == -1 || distances[u] < distances[v])) {
                    v = u;
                }
            }

            visited[v] = true;

            for (auto [u, w] : edges[v]) {
                if (distances[u] > distances[v] + w) {
                    distances[u] = distances[v] + w;
                    parents[u] = v;
                }
            }
        }
    }

    std::deque<int> getPath(int start, int finish) {
        std::deque<int> path;
        for (int v = finish; v != -1 && parents[v] != -1; v = parents[v]) {
            path.push_front(v);
        }
        if (path.empty()) {
            std::cout << "No path" << std::endl;
            return {};
        }
        path.push_front(start);
        return path;
    }
};
