#define _USE_MATH_DEFINES
#include "heliocentric_position.hpp"
#include "kepler_solver.hpp"
#include <cmath>

Vec3 orbitalPlanePosition(double a, double e, double eccentricAnomaly_deg) {
    double E = degToRad(eccentricAnomaly_deg);

    Vec3 pos;
    pos.x = a * (cos(E) - e);
    pos.y = a * sqrt(1 - e * e) * sin(E);
    pos.z = 0.0;

    return pos;
}

Vec3 orbitalToEcliptic(const Vec3& orbitalPos, double argPeri_deg, double inclination_deg, double node_deg) {
    double w = degToRad(argPeri_deg);
    double i = degToRad(inclination_deg);
    double O = degToRad(node_deg);

    double cosO = cos(O), sinO = sin(O);
    double cosw = cos(w), sinw = sin(w);
    double cosi = cos(i), sini = sin(i);

    Vec3 result;
    result.x = (cosO * cosw - sinO * sinw * cosi) * orbitalPos.x
              + (-cosO * sinw - sinO * cosw * cosi) * orbitalPos.y;
    result.y = (sinO * cosw + cosO * sinw * cosi) * orbitalPos.x
              + (-sinO * sinw + cosO * cosw * cosi) * orbitalPos.y;
    result.z = (sinw * sini) * orbitalPos.x
              + (cosw * sini) * orbitalPos.y;

    return result;
}