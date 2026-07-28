#ifndef HELIOCENTRIC_POSITION_HPP
#define HELIOCENTRIC_POSITION_HPP

#include "vec3.hpp"

Vec3 orbitalPlanePosition(double a, double e, double eccentricAnomaly_deg);
Vec3 orbitalToEcliptic(const Vec3& orbitalPos, double argPeri_deg, double inclination_deg, double node_deg);
Vec3 geocentricPosition(const Vec3& planetHeliocentric, const Vec3& earthHeliocentric);
#endif // HELIOCENTRIC_POSITION_HPP