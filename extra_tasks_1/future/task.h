#pragma once
#include "ThreadPool.h"

#include <algorithm>
#include <vector>
#include <deque>
#include <cstdint>
#include <atomic>
#include <memory>
#include <future>
// нельзя инклюдить set и queue

/*
    Задание -  реализовать многопоточный алгоритм Дейкстры (подробнее: https://neerc.ifmo.ru/wiki/index.php?title=Алгоритм_Дейкстры).
    Вам надо написать также ThreadPool, который необходимо использовать для распараллеливания вычислений.
    Для этого требуется реализовать функци findMinVertex и relaxEdges.

    `findMinVertex` - Находит вершину с минимальным расстоянием, которая ещё не была посещена и возвращает ее.

    `relaxEdges` - Производит релаксацию ребер из полученной в `findMin` вершины по всем исходящим вершинам

    Обе функции должны работать с графом в ThreadPool.

    Чтобы проверить, исполнена задача или нет
    if(future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    получение и обработка результата
    }

    Гарантируется что не случится переполнение int.
*/

// Важно реализовать алгоритм с асимптотикой O(n^2)

class Dijkstra
{
public:
<<<<<<< HEAD
<<<<<<< HEAD
    Dijkstra(int numVertices, int numEdges, int numThreads): N(numVertices), M(numEdges), threadPool(numThreads),
    edges(numVertices), distances(numVertices, INF), visited(numVertices, false), parents(numVertices, -1), NumVertices(numVertices), poolSize(numThreads)
    {}

    ~Dijkstra() {
        threadPool.Terminate(true);
    }

    // Добавляет ребра между двумя вершинами с указанным весом в неориентированном графе.
    void addEdge(int from, int to, int weight) {
        from--;
        to--;
        edges[from].emplace_back(to, weight);
        edges[to].emplace_back(from, weight);
    }

    // Выполняет сам алгоритм Дейкстры многопоточно.
    void run(int start, int finish) {
        start--;
        finish--;
        distances[start] = 0;
        for (int i = 0; i < NumVertices; ++i) {
            int current = findMinVertex();
            if (current == -1 || distances[current] == INF) {
                break;
            }
            visited[current] = true;
            if (current == finish) {
                break;
            }
            relaxEdges(current);
        }
    }

    // Получает кратчайший путь  в виде последовательности вершин и возвращает его.
    std::deque<int> getPath(int start, int finish) {
        start--;
        finish--;
        std::deque<int> path;
        for (int v = finish; v != -1 && parents[v] != -1; v = parents[v]) {
            path.push_front(v + 1);
        }
        if (path.empty()) {
            return {};
        }
        path.push_front(start + 1);
=======
    Dijkstra(int numVertices, int numEdges, int numThreads)
    : N(numVertices), M(numEdges), threadPool(numThreads),
    edges(numVertices), distances(numVertices, INF),
    visited(numVertices, false), parents(numVertices, -1)
=======
    Dijkstra(int numVertices, int numEdges, int numThreads): N(numVertices), M(numEdges), threadPool(numThreads),
<<<<<<< HEAD
    edges(numVertices), distances(numVertices, INF), visited(numVertices, false), parents(numVertices, -1), NumVertices(numVertices)
>>>>>>> 8a2c77d (fix)
=======
    edges(numVertices), distances(numVertices, INF), visited(numVertices, false), parents(numVertices, -1), NumVertices(numVertices), poolSize(numThreads)
>>>>>>> fdde5f0 (mnogopotok)
    {}

    ~Dijkstra() {
        threadPool.Terminate(true);
    }

    // Добавляет ребра между двумя вершинами с указанным весом в неориентированном графе.
    void addEdge(int from, int to, int weight) {
        from--;
        to--;
        edges[from].emplace_back(to, weight);
        edges[to].emplace_back(from, weight);
    }

    // Выполняет сам алгоритм Дейкстры многопоточно.
    void run(int start, int finish) {
        start--;
        finish--;
        distances[start] = 0;
        for (int i = 0; i < NumVertices; ++i) {
            int current = findMinVertex();
            if (current == -1 || distances[current] == INF) {
                break;
            }
            visited[current] = true;
            if (current == finish) {
                break;
            }
            relaxEdges(current);
        }
    }

    // Получает кратчайший путь  в виде последовательности вершин и возвращает его.
    std::deque<int> getPath(int start, int finish) {
        start--;
        finish--;
        std::deque<int> path;
        for (int v = finish; v != -1 && parents[v] != -1; v = parents[v]) {
            path.push_front(v + 1);
        }
        if (path.empty()) {
            return {};
        }
<<<<<<< HEAD
        path.push_front(start);
>>>>>>> b11061f (future)
=======
        path.push_front(start + 1);
>>>>>>> 8a2c77d (fix)
        return path;
    }

private:
    const int INF = 1e9 + 9;
    int N, M;
    ThreadPool threadPool;
    std::vector<std::vector<std::pair<int, int>>> edges;
    std::vector<int> distances;
    std::vector<bool> visited;
    std::vector<int> parents;
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    int NumVertices, poolSize;

    std::mutex distMutex;
    std::mutex minMutex;
    std::vector<std::future<void>> pendingTasks;
    std::mutex pendingTasksMutex;

    // Находит вершину с минимальным расстоянием, которая ещё не была посещена.
    int findMinVertex() {
        int blockSize = (N + poolSize - 1) / poolSize;
        std::vector<std::future<std::pair<int, int>>> futures; // future от promise, index и dist до local вершины
    
        for (int start = 0; start < N; start += blockSize) {
            int end = std::min(start + blockSize, N); // Делим массив vertex на poolSize равных блоков
            std::promise<std::pair<int, int>> promise; // promise с парами
            futures.push_back(promise.get_future()); // кто обязан вернуть
            auto prom_ptr = std::make_shared<std::promise<std::pair<int, int>>>(std::move(promise)); // умный указатель для лямбды функции в которой запишем по указателю
    
            threadPool.PushTask([this, start, end, prom_ptr]() mutable { // mutable разрешает изменять копии переменных, захваченных по значению.
                int th_min = INF;
                int th_index = -1;
                for (int i = start; i < end; ++i) {
                    if (!visited[i] && distances[i] < th_min) {
                        th_min = distances[i];
                        th_index = i;
                    }
                }
                prom_ptr->set_value({th_index, th_min});
            });
        }
    
        int globalMin = INF, minVertex = -1;
        for (auto& fut : futures) {
            auto [index, dist] = fut.get();
            if (index != -1 && dist < globalMin) {
                globalMin = dist;
                minVertex = index;
            }
        }
        return minVertex;
    }

    // Производит релаксацию ребер из vertex
    void relaxEdges(int vertex) { // аналогично findminvertex используем промисы и фьючи
        int edgeCount = edges[vertex].size();
        int blockSize = (edgeCount + poolSize - 1) / poolSize;
        int currentDistance = distances[vertex];
    
        std::vector<std::future<void>> futures;
    
        for (int start = 0; start < edgeCount; start += blockSize) {
            int end = std::min(start + blockSize, edgeCount);
            std::promise<void> promise;
            futures.push_back(promise.get_future());
            auto prom_ptr = std::make_shared<std::promise<void>>(std::move(promise));
    
            threadPool.PushTask([this, vertex, currentDistance, start, end, prom_ptr]() mutable {
                for (int i = start; i < end; ++i) {
                    auto [to, weight] = edges[vertex][i];
                    int newDist = currentDistance + weight;
    
                    std::lock_guard<std::mutex> lock(distMutex);
                    if (newDist < distances[to]) {
                        distances[to] = newDist;
                        parents[to] = vertex;
                    }
                }
                prom_ptr->set_value();
            });
        }
    
        for (auto& fut : futures) {
            fut.get();
        }
    }
      
=======
=======
    int NumVertices;
>>>>>>> 8a2c77d (fix)
=======
    int NumVertices, poolSize;
>>>>>>> fdde5f0 (mnogopotok)

    std::mutex distMutex;
    std::mutex minMutex;
    std::vector<std::future<void>> pendingTasks;
    std::mutex pendingTasksMutex;

    // Находит вершину с минимальным расстоянием, которая ещё не была посещена.
    int findMinVertex() {
        int blockSize = (N + poolSize - 1) / poolSize;
        std::vector<std::future<std::pair<int, int>>> futures; // future от promise, index и dist до local вершины
    
        for (int start = 0; start < N; start += blockSize) {
            int end = std::min(start + blockSize, N); // Делим массив vertex на poolSize равных блоков
            std::promise<std::pair<int, int>> promise; // promise с парами
            futures.push_back(promise.get_future()); // кто обязан вернуть
            auto prom_ptr = std::make_shared<std::promise<std::pair<int, int>>>(std::move(promise)); // умный указатель для лямбды функции в которой запишем по указателю
    
            threadPool.PushTask([this, start, end, prom_ptr]() mutable { // mutable разрешает изменять копии переменных, захваченных по значению.
                int th_min = INF;
                int th_index = -1;
                for (int i = start; i < end; ++i) {
                    if (!visited[i] && distances[i] < th_min) {
                        th_min = distances[i];
                        th_index = i;
                    }
                }
                prom_ptr->set_value({th_index, th_min});
            });
        }
    
        int globalMin = INF, minVertex = -1;
        for (auto& fut : futures) {
            auto [index, dist] = fut.get();
            if (index != -1 && dist < globalMin) {
                globalMin = dist;
                minVertex = index;
            }
        }
        return minVertex;
    }

    // Производит релаксацию ребер из vertex
    void relaxEdges(int vertex) { // аналогично findminvertex используем промисы и фьючи
        int edgeCount = edges[vertex].size();
        int blockSize = (edgeCount + poolSize - 1) / poolSize;
        int currentDistance = distances[vertex];
    
        std::vector<std::future<void>> futures;
    
        for (int start = 0; start < edgeCount; start += blockSize) {
            int end = std::min(start + blockSize, edgeCount);
            std::promise<void> promise;
            futures.push_back(promise.get_future());
            auto prom_ptr = std::make_shared<std::promise<void>>(std::move(promise));
    
            threadPool.PushTask([this, vertex, currentDistance, start, end, prom_ptr]() mutable {
                for (int i = start; i < end; ++i) {
                    auto [to, weight] = edges[vertex][i];
                    int newDist = currentDistance + weight;
    
                    std::lock_guard<std::mutex> lock(distMutex);
                    if (newDist < distances[to]) {
                        distances[to] = newDist;
                        parents[to] = vertex;
                    }
                }
                prom_ptr->set_value();
            });
        }
<<<<<<< HEAD
<<<<<<< HEAD
    }
>>>>>>> b11061f (future)
=======
      }
=======
    
        for (auto& fut : futures) {
            fut.get();
        }
    }
>>>>>>> fdde5f0 (mnogopotok)
      
>>>>>>> 8a2c77d (fix)
};
