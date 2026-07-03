#ifndef KEPLER_SOLVER_HPP
#define KEPLER_SOLVER_HPP
#include <string>
#include <vector>

double degToRad(double deg);
double radToDeg(double rad);
double solveKeplerEquation(double meanAnomaly_deg, double eccentricity);

#endif // KEPLER_SOLVER_HPP