#include <iostream>
#include "heliocentric_position.hpp"

int main() {
    // Circular orbit (e=0), E=0 -> should sit at (a, 0, 0), i.e. perihelion on the x-axis
    Vec3 p1 = orbitalPlanePosition(1.0, 0.0, 0.0);
    std::cout << "a=1, e=0, E=0 -> (" << p1.x << ", " << p1.y << ", " << p1.z
               << ") (expected ~(1, 0, 0))\n";

    // Circular orbit (e=0), E=90 -> should sit at (0, a, 0), quarter way around
    Vec3 p2 = orbitalPlanePosition(1.0, 0.0, 90.0);
    std::cout << "a=1, e=0, E=90 -> (" << p2.x << ", " << p2.y << ", " << p2.z
               << ") (expected ~(0, 1, 0))\n";

    // Eccentric orbit, E=0 -> should sit at (a*(1-e), 0, 0), i.e. perihelion distance
    Vec3 p3 = orbitalPlanePosition(1.0, 0.5, 0.0);
    std::cout << "a=1, e=0.5, E=0 -> (" << p3.x << ", " << p3.y << ", " << p3.z
               << ") (expected ~(0.5, 0, 0))\n";

    return 0;
}