#define _USE_MATH_DEFINES
#include "kepler_solver.hpp"
#include <cmath>

double degToRad(double deg) {
    return deg * M_PI / 180.0;
}

double radToDeg(double rad) {
    return rad * 180.0 / M_PI;
}

double solveKeplerEquation(double meanAnomaly_deg, double eccentricity){
    double M = degToRad(meanAnomaly_deg);
    double E = M;
    const double tolerance = 1e-6;
    const int maxIterations = 100;

    for (int i = 0; i < maxIterations; ++i) {
        double f = E - eccentricity * sin(E) - M;
        double f_prime = 1 - eccentricity * cos(E);

        double deltaE = -f / f_prime;
        E += deltaE;

        if (fabs(deltaE) < tolerance) {
            break;
        }
    }

    return radToDeg(E);
}