#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
//#include <intrin.h>

// Toggle testing
#define RUN_TESTS 1

// Toggle all benchmarks
#define RUN_BENCHMARKS 1

// Toggle benchmarking of different algorithms
#define BENCHMARK_IS_POINT_IN_POLYGON 1
#define BENCHMARK_NO_OPT 1
#define BENCHMARK_REFERENCE_IMPL 1

// Toggle benchmarking of algorithms above on different polygons
#define ENABLE_SQUARE_BENCH 1
#define ENABLE_COMPLEX_POLYGON_BENCH 1
#define ENABLE_SPECIAL_BENCH 1



//! Minimal Vector2 structure just to ease our lives
struct Vector2 {
    float x;
    float y;


    Vector2(float x, float y) : x(x), y(y) {
    }

    Vector2() = default;

    Vector2 operator-(const Vector2 other) {
        return Vector2(x - other.x, y - other.y);
    }
};

int totalInvocations = 0;
int firstContinueCount = 0;
int secondContinueCount = 0;
int thirdContinueCount = 0;

bool isPointInPolygonNoOpt(const std::vector<Vector2>& polygon, Vector2 point) {
    Vector2 p0 = *polygon.rbegin();
    Vector2 p1;

    int aboveCount = 0;

    //__debugbreak();
    for (size_t i = 0; i < polygon.size(); ++i, p0 = p1) {
        p1 = polygon[i];
        
        #if 1
        if (((p0.x < point.x) && (p1.x < point.x)) // left
            || ((p0.y < point.y) && (p1.y < point.y)) // below
            || ((p0.y > point.y) && (p1.y > point.y))) // above
        {
            continue;
        }
        #else
        ++totalInvocations;
        if ((p0.x < point.x) && (p1.x < point.x)) // left
        {
            ++firstContinueCount;
            continue;
        }
        if ((p0.y < point.y) && (p1.y < point.y)) // below
        {
            ++secondContinueCount;
            continue;
        }
        if ((p0.y > point.y) && (p1.y > point.y)) // above
        {
            ++thirdContinueCount;
            continue;
        }
        #endif
        
        // Both to the right
        if ((p0.x > point.x) && (p1.x > point.x)) {
            if (p0.y > point.y)
                ++aboveCount;
            if (p1.y > point.y)
                ++aboveCount;
            continue;
        }

        // Compute imaginary cross product
        Vector2 a = point - p0;
        Vector2 b = p1 - p0;
        float z = a.x * b.y - a.y * b.x;

        // RH system -> positive Z = line is to the left of point
        if (z > 0)
            continue;

        // We are exactly on the line
        if (z == 0)
            return true;

        // We cross the line
        if (p0.y > point.y)
            ++aboveCount;
        if (p1.y > point.y)
            ++aboveCount;
    }

    return (aboveCount & 1) == 1;
}

bool isPointInPolygon(const std::vector<Vector2>& polygon, Vector2 point) {
    Vector2 p0 = *polygon.rbegin();
    Vector2 p1;

    int aboveCount = 0;

    for (size_t i = 0; i < polygon.size(); ++i, p0 = p1) {
        p1 = polygon[i];

        // In theory, this should improve the branch prediction for some polygons
        // Using bitwise | and & we have only 1 branch instead of 6 when using || and &&
        if (((p0.x < point.x) & (p1.x < point.x)) // left
            | ((p0.y < point.y) & (p1.y < point.y)) // below
            | ((p0.y > point.y) & (p1.y > point.y))) // above
        {
            continue;
        }

        // Both to the right
        if ((p0.x > point.x) && (p1.x > point.x)) {
            if (p0.y > point.y)
                ++aboveCount;
            if (p1.y > point.y)
                ++aboveCount;
            continue;
        }

        // Compute imaginary cross product
        const Vector2 a = point - p0;
        const Vector2 b = p1 - p0;
        const float z = a.x * b.y - a.y * b.x;

        // RH system -> positive Z = line is to the left of point
        if (z > 0)
            continue;

        // We are exactly on the line
        if (z == 0)
            return true;

        // We cross the line
        if (p0.y > point.y)
            ++aboveCount;
        if (p1.y > point.y)
            ++aboveCount;
    }

    return (aboveCount & 1) == 1;
}

/*
Adapted from http://geomalgorithms.com/a03-_inclusion.html

Copyright 2000 softSurfer, 2012 Dan Sunday
This code may be freely used and modified for any purpose
providing that this copyright notice is included with it.
SoftSurfer makes no warranty for this code, and cannot be held
liable for any real or imagined damage resulting from its use.
Users of this code must verify correctness for their application.
*/
bool cn_PnPoly(const std::vector<Vector2>& V, Vector2 P) {
    int    cn = 0;    // the  crossing number counter

    Vector2 p0 = *V.rbegin();
    Vector2 p1;
    // loop through all edges of the polygon
    for (size_t i = 0; i < V.size(); ++i, p0 = p1) {    // edge from V[i]  to V[i+1]
        p1 = V[i];
        if (((p0.y <= P.y) && (p1.y > P.y))     // an upward crossing
            || ((p0.y > P.y) && (p1.y <= P.y))) { // a downward crossing
                // compute  the actual edge-ray intersect x-coordinate
            float vt = (float)(P.y - p0.y) / (p1.y - p0.y);
            if (P.x < p0.x + vt * (p1.x - p0.x)) // P.x < intersect
                ++cn;   // a valid crossing of y=P.y right of P.x
        }
    }
    return (cn & 1) == 1;    // 0 if even (out), and 1 if  odd (in)

}

std::vector<Vector2> makePolygon() {
    std::vector<Vector2> polygon;
    polygon.push_back(Vector2(0, 0));
    polygon.push_back(Vector2(0.5f, 0.5f));
    polygon.push_back(Vector2(1, 0));
    polygon.push_back(Vector2(1, 1));
    polygon.push_back(Vector2(0, 1));
    return polygon;
}

std::vector<Vector2> makeSquare() {
    std::vector<Vector2> polygon;
    polygon.push_back(Vector2(0, 0));
    polygon.push_back(Vector2(1, 0));
    polygon.push_back(Vector2(1, 1));
    polygon.push_back(Vector2(0, 1));
    return polygon;
}

std::vector<Vector2> makeComplexPolygon() {
    std::vector<Vector2> polygon;

    polygon.push_back(Vector2(0, 0));
    polygon.push_back(Vector2(0.1f, 0.1f));
    polygon.push_back(Vector2(0.15f, 0.1f));
    polygon.push_back(Vector2(0.2f, 0.05f));
    polygon.push_back(Vector2(0.5f, 0.1f));
    polygon.push_back(Vector2(0.6f, 0.02f));
    
    polygon.push_back(Vector2(1, 0));
    polygon.push_back(Vector2(0.9f, 0.2f));
    polygon.push_back(Vector2(0.8f, 0.5f));
    polygon.push_back(Vector2(0.99f, 0.8f));
    
    polygon.push_back(Vector2(1, 1));
    polygon.push_back(Vector2(0.8f, 0.9f));
    polygon.push_back(Vector2(0.75f, 0.8f));
    polygon.push_back(Vector2(0.4f, 0.65f));
    
    polygon.push_back(Vector2(0, 1));
    polygon.push_back(Vector2(0.01f, 0.8f));
    polygon.push_back(Vector2(0.2f, 0.5f));
    polygon.push_back(Vector2(0.11f, 0.25f));

    return polygon;
}

std::vector<Vector2> makeHalfSplitPolygon() {
    std::vector<Vector2> polygon;

    #if 0
        polygon.push_back(Vector2(0.499f, 0.499f));
        polygon.push_back(Vector2(0.501f, 0.499f));
        polygon.push_back(Vector2(0.501f, 0.501f));
        polygon.push_back(Vector2(0.499f, 0.501f));
    #else
        polygon.push_back(Vector2(0.99f, 0));
        polygon.push_back(Vector2(1.0f, 0));
        polygon.push_back(Vector2(1.0f, 1.0f));
        polygon.push_back(Vector2(0.99f, 1.0f));
    #endif

    return polygon;
}

bool checkIn(const std::vector<Vector2>& polygon, Vector2 point) {
    if (!isPointInPolygon(polygon, point)) {
        std::cout << "Point [" << point.x << "; " << point.y << "] reported OUT" << std::endl;
        return false;
    }
    return true;
}

bool checkOut(const std::vector<Vector2>& polygon, Vector2 point) {
    if (isPointInPolygon(polygon, point)) {
        std::cout << "Point [" << point.x << "; " << point.y << "] reported IN" << std::endl;
        return false;
    }
    return true;
}

void runBasicTests() {
    std::vector<Vector2> polygon = makePolygon();

    bool result = true;
    // Clearly in
    result &= checkIn(polygon, Vector2(0.6f, 0.6f));
    result &= checkIn(polygon, Vector2(0.1f, 0.2f));
    result &= checkIn(polygon, Vector2(0.9f, 0.9f));

    // Clearly out
    result &= checkOut(polygon, Vector2(-1.0f, 0.6f));
    result &= checkOut(polygon, Vector2(0.0f, 2.0f));
    result &= checkOut(polygon, Vector2(1.1f, 0.0f));

    // On edge
    result &= checkIn(polygon, Vector2(0.0f, 0.5f));
    result &= checkIn(polygon, Vector2(1.0f, 0.5f));
    result &= checkIn(polygon, Vector2(0.5f, 0.0f));
    result &= checkIn(polygon, Vector2(0.5f, 1.0f));

    // In vertex
    result &= checkIn(polygon, Vector2(0, 0));
    result &= checkIn(polygon, Vector2(0.5f, 0.5f));
    result &= checkIn(polygon, Vector2(1, 0));
    result &= checkIn(polygon, Vector2(1, 1));
    result &= checkIn(polygon, Vector2(0, 1));

    if (result) {
        std::cout << "Basic tests OK" << std::endl;
    } else {
        std::cout << "Basic tests FAILED" << std::endl;
    }
}

Vector2 generateRandomPoint() {
    return Vector2(
        static_cast<float>(std::rand()) / RAND_MAX,
        static_cast<float>(std::rand()) / RAND_MAX
    );
}

void runRandomInSquareTest() {
    std::vector<Vector2> square = makeSquare();
    std::srand(123);
    bool result = true;
    for (int i = 0; i < 10000; ++i) {
        result &= checkIn(square, generateRandomPoint());
    }

    if (result) {
        std::cout << "RandomInSquare test OK" << std::endl;
    } else {
        std::cout << "RandomInSquare test FAILED" << std::endl;
    }
}

constexpr int ITER_COUNT = 10000000;
constexpr int SEED = 42;

void runBenchmark(const std::vector<Vector2>& polygon) {
    using fact_mili = std::chrono::duration<float, std::milli>;

    #if BENCHMARK_IS_POINT_IN_POLYGON
    {
        std::srand(SEED);
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER_COUNT; ++i) {
            isPointInPolygon(polygon, generateRandomPoint());
        }
        fact_mili elapsed = std::chrono::high_resolution_clock::now() - start;
        std::cout << "isPointInPolygon elapsed: " << elapsed.count() << std::endl;
    }

    {
        std::srand(SEED);
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER_COUNT; ++i) {
            isPointInPolygon(polygon, generateRandomPoint());
        }
        fact_mili elapsed = std::chrono::high_resolution_clock::now() - start;
        std::cout << "isPointInPolygon elapsed: " << elapsed.count() << std::endl;
    }
    #endif

    #if BENCHMARK_NO_OPT
    {
        std::srand(SEED);
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER_COUNT; ++i) {
            isPointInPolygonNoOpt(polygon, generateRandomPoint());
        }
        fact_mili elapsed = std::chrono::high_resolution_clock::now() - start;
        std::cout << "isPointInPolygon not optimized elapsed: " << elapsed.count() << std::endl;
    }

    {
        std::srand(SEED);
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER_COUNT; ++i) {
            isPointInPolygonNoOpt(polygon, generateRandomPoint());
        }
        fact_mili elapsed = std::chrono::high_resolution_clock::now() - start;
        std::cout << "isPointInPolygon not optimized elapsed: " << elapsed.count() << std::endl;
    }
    #endif

    #if BENCHMARK_REFERENCE_IMPL
    {
        std::srand(SEED);
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER_COUNT; ++i) {
            cn_PnPoly(polygon, generateRandomPoint());
        }
        fact_mili elapsed = std::chrono::high_resolution_clock::now() - start;
        std::cout << "cn_PnPoly elapsed:        " << elapsed.count() << std::endl;
    }

    {
        std::srand(SEED);
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER_COUNT; ++i) {
            cn_PnPoly(polygon, generateRandomPoint());
        }
        fact_mili elapsed = std::chrono::high_resolution_clock::now() - start;
        std::cout << "cn_PnPoly elapsed:        " << elapsed.count() << std::endl;
    }
    #endif
}

void runBenchmarks() {
    #if ENABLE_SQUARE_BENCH
        std::cout << "\nBenchmark with square" << std::endl;
        runBenchmark(makeSquare());
    #endif

    #if ENABLE_COMPLEX_POLYGON_BENCH
        std::cout << "\nBenchmark with complex polygon" << std::endl;
        runBenchmark(makeComplexPolygon());
    #endif

    #if ENABLE_SPECIAL_BENCH
        std::cout << "\nBenchmark with half split square" << std::endl;
        runBenchmark(makeHalfSplitPolygon());
    #endif
}

int main() {
    #if RUN_TESTS
        runBasicTests();
        runRandomInSquareTest();
    #endif

    #if RUN_BENCHMARKS
        runBenchmarks();
    #endif

    #if 0
        std::cout << "\nTotal invocations: " << totalInvocations << std::endl;
        std::cout << "Total continue count: " << firstContinueCount + secondContinueCount + thirdContinueCount << std::endl;
        std::cout << "First continue count: " << firstContinueCount << std::endl;
        std::cout << "Second continue count: " << secondContinueCount << std::endl;
        std::cout << "Third continue count: " << thirdContinueCount << std::endl;
    #endif

    std::cout << "Done" << std::endl;
}

