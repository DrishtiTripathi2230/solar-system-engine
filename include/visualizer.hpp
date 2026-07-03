#ifndef VISUALIZER_HPP
#define VISUALIZER_HPP

#include <string>
#include <vector>
#include "vec3.hpp"

struct PlanetResult {
    std::string name;
    double distanceAU;
    Vec3 position;
};

void printSolarSystemMap(const std::vector<PlanetResult>& planets);

#endif // VISUALIZER_HPP