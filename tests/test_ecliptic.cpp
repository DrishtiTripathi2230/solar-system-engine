#include <iostream>
#include "heliocentric_position.hpp"

int main() {
    // Test 1: no rotation at all (w=0, i=0, node=0) -> ecliptic coords should equal orbital coords exactly
    Vec3 orb1 = {1.0, 0.0, 0.0};
    Vec3 e1 = orbitalToEcliptic(orb1, 0.0, 0.0, 0.0);
    std::cout << "no rotation -> (" << e1.x << ", " << e1.y << ", " << e1.z
               << ") (expected ~(1, 0, 0))\n";

    // Test 2: inclination only (i=90), point along y-axis in orbital plane
    // should get pushed entirely into z, since a 90 degree tilt swaps y and z
    Vec3 orb2 = {0.0, 1.0, 0.0};
    Vec3 e2 = orbitalToEcliptic(orb2, 0.0, 90.0, 0.0);
    std::cout << "i=90, orbital y-axis point -> (" << e2.x << ", " << e2.y << ", " << e2.z
               << ") (expected ~(0, 0, 1))\n";

    // Test 3: node rotation only (node=90), point along x-axis in orbital plane
    // should get pushed entirely into y, since a 90 degree Omega rotation swaps x and y
    Vec3 orb3 = {1.0, 0.0, 0.0};
    Vec3 e3 = orbitalToEcliptic(orb3, 0.0, 0.0, 90.0);
    std::cout << "node=90, orbital x-axis point -> (" << e3.x << ", " << e3.y << ", " << e3.z
               << ") (expected ~(0, 1, 0))\n";

    return 0;
}