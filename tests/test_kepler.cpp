#include "include/kepler_solver.hpp"
#include <iostream>

int main() {
    std::cout << std::fixed;

    // Sanity check 1: circular orbit (e=0) -> E should exactly equal M
    double E1 = solveKeplerEquation(45.0, 0.0);
    std::cout << "e=0, M=45 -> E=" << E1 << " (expected 45.000000)\n";

    // Sanity check 2: Earth-like eccentricity, M=0 -> E should be 0
    double E2 = solveKeplerEquation(0.0, 0.0167);
    std::cout << "e=0.0167, M=0 -> E=" << E2 << " (expected 0.000000)\n";

    return 0;
}