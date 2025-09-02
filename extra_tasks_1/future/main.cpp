#include "task.h"
#include "singleDijkstra.h"

#if defined(_LIBCPP_QUEUE) || defined(_LIBCPP_SET)
#error "using priority_queue or set is forbidden"
#endif

#include <cassert>
#include <fstream>
#include <iostream>
#include <chrono>

void loadGraphFromFile(const std::string &filename, std::vector<std::tuple<int, int, int>> &edges, int &n, int &m)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file");
    }

    file >> n >> m;
    int a, b;
    int w;
    while (file >> a >> b >> w)
    {
        edges.emplace_back(a, b, w);
    }
    file.close();
}

void testDijkstra1(const std::string &inputFilename)
{
    std::vector<std::tuple<int, int, int>> edges;
    int n, m;
    loadGraphFromFile(inputFilename, edges, n, m);

    Dijkstra dijkstra(n, m, 3);
    for (auto &[a, b, w] : edges)
    {
        dijkstra.addEdge(a, b, w);
    }

    dijkstra.run(1, 5);
    auto path = dijkstra.getPath(1, 5);
    std::deque<int> expectedPath = {1, 4, 3, 5};

    assert(path == expectedPath);
    if (path == expectedPath)
    {
        std::cout << "Test1 passed: Correct path found." << std::endl;
    }
    else
    {
        std::cout << "Test1 failed: Incorrect path." << std::endl;
    }
}

void testDijkstra2(const std::string &inputFilename)
{
    std::vector<std::tuple<int, int, int>> edges;
    int n, m;
    loadGraphFromFile(inputFilename, edges, n, m);

    Dijkstra dijkstra(n, m, 3);
    for (auto &[a, b, w] : edges)
    {
        dijkstra.addEdge(a, b, w);
    }

    dijkstra.run(1, n);
    auto path = dijkstra.getPath(1, n);
    std::deque<int> expectedPath = {1, 4, 6, 10};

    assert(path == expectedPath);

    if (path == expectedPath)
    {
        std::cout << "Test2 passed: Correct path found." << std::endl;
    }
    else
    {
        std::cout << "Test2 failed: Incorrect path." << std::endl;
    }
}

void testDijkstraNoPath(const std::string &inputFilename)
{
    std::vector<std::tuple<int, int, int>> edges;
    int n, m;
    loadGraphFromFile(inputFilename, edges, n, m);

    Dijkstra dijkstra(n, m, 3);
    for (auto &[a, b, w] : edges)
    {
        dijkstra.addEdge(a, b, w);
    }

    dijkstra.run(1, 3);
    auto path = dijkstra.getPath(1, 3);

    std::deque<int> expectedPath = {};

    assert(path == expectedPath);

    if (path == expectedPath)
    {
        std::cout << "Test3 passed: detected there is no path." << std::endl;
    }
    else
    {
        std::cout << "Test3 failed: Incorrect path." << std::endl;
    }
}

void testEfficiency(const std::string &inputFilename)
{
    int n, m;
    std::vector<std::tuple<int, int, int>> edges;
    loadGraphFromFile(inputFilename, edges, n, m);

    SingleDijkstra single_dijkstra(n, m);
    Dijkstra dijkstra(n, m, 5);

    for (auto &[a, b, w] : edges)
    {
        single_dijkstra.addEdge(a, b, w);
        dijkstra.addEdge(a, b, w);
    }

    auto startSingle = std::chrono::high_resolution_clock::now();
    single_dijkstra.runSingle(1, n);
    auto endSingle = std::chrono::high_resolution_clock::now();

    auto durationSingle = std::chrono::duration_cast<std::chrono::milliseconds>(endSingle - startSingle).count();

    auto startMulti = std::chrono::high_resolution_clock::now();
    dijkstra.run(1, n);
    auto endMulti = std::chrono::high_resolution_clock::now();

    auto durationMulti = std::chrono::duration_cast<std::chrono::milliseconds>(endMulti - startMulti).count();

    std::cout << "Single-threaded duration: " << durationSingle << " ms\n";
    std::cout << "Multi-threaded duration: " << durationMulti << " ms\n";

    std::cout << "Efficiency coefficient = " << durationSingle * 1.0 / durationMulti << std::endl;

    assert((durationSingle * 1.0 / durationMulti) >= 1.155);

    std::deque<int> expectedPath = {1, 3026, 7847, 7848, 4571, 4284, 10000};

    assert(dijkstra.getPath(1, n) == expectedPath);
}

int main()
{
    testDijkstra1("small_test.txt");
    testDijkstra2("middle_test.txt");
    testDijkstraNoPath("test_no_path.txt");
    testEfficiency("ultra_large_test.txt");

    return 0;
}
