#define _USE_MATH_DEFINES
#include "equatorial_position.hpp"
#include "kepler_solver.hpp"
#include <cmath>

double obliquityOfEcliptic(double T) {
    return 23.439291 - 0.0130042 * T;
}

Vec3 eclipticToEquatorial(const Vec3& eclipticPos, double obliquity_deg) {
    double eps = degToRad(obliquity_deg);

    Vec3 result;
    result.x = eclipticPos.x;
    result.y = eclipticPos.y * cos(eps) - eclipticPos.z * sin(eps);
    result.z = eclipticPos.y * sin(eps) + eclipticPos.z * cos(eps);

    return result;
}
EquatorialCoords vectorToRaDec(const Vec3& equatorialPos) {
    double r = sqrt(equatorialPos.x * equatorialPos.x
                   + equatorialPos.y * equatorialPos.y
                   + equatorialPos.z * equatorialPos.z);

    double dec_rad = asin(equatorialPos.z / r);
    double ra_rad = atan2(equatorialPos.y, equatorialPos.x);

    if (ra_rad < 0) {
        ra_rad += 2 * M_PI; // keep RA in the conventional 0 to 2*pi range
    }

    EquatorialCoords result;
    result.declination_deg = radToDeg(dec_rad);
    result.rightAscension_hours = radToDeg(ra_rad) / 15.0; // 360 degrees = 24 hours, so 15 deg per hour

    return result;
}