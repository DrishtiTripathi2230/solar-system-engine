#include "include/kepler_solver.hpp"
#include <iostream>

int main() {
    std::cout << std::fixed;
    std::cout << "90 deg -> " << degToRad(90.0) << " rad (expected ~1.570796)\n";
    std::cout << "1.570796 rad -> " << radToDeg(1.570796) << " deg (expected ~90)\n";
    return 0;
}