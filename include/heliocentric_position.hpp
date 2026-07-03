#ifndef HELIOCENTRIC_POSITION_HPP
#define HELIOCENTRIC_POSITION_HPP

#include "vec3.hpp"

Vec3 orbitalPlanePosition(double a, double e, double eccentricAnomaly_deg);
Vec3 orbitalToEcliptic(const Vec3& orbitalPos, double argPeri_deg, double inclination_deg, double node_deg);
#endif // HELIOCENTRIC_POSITION_HPP