#ifndef EQUATORIAL_POSITION_HPP
#define EQUATORIAL_POSITION_HPP

#include "vec3.hpp"

double obliquityOfEcliptic(double T);
Vec3 eclipticToEquatorial(const Vec3& eclipticPos, double obliquity_deg);

struct EquatorialCoords {
    double rightAscension_hours;
    double declination_deg;
};

EquatorialCoords vectorToRaDec(const Vec3& equatorialPos);

#endif // EQUATORIAL_POSITION_HPP