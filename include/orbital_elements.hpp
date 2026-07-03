#ifndef ORBITAL_ELEMENTS_HPP
#define ORBITAL_ELEMENTS_HPP
#include <string>
#include <vector>
struct OrbitalElements {
    std::string name;
    double a0, a_dot;
    double e0, e_dot;
    double i0, i_dot;
    double L0, L_dot;
    double long_peri0, long_peri_dot;
    double long_node0, long_node_dot;
};
struct OrbitalElementsAtEpoch{
    std::string name;
    double a;
    double e;
    double i_deg;
    double meanAnomaly_deg;
    double argPeri_deg;
    double node_deg;
};
const std::vector<OrbitalElements>& referenceTable();
OrbitalElementsAtEpoch elementsAtTime(const OrbitalElements& base, double T);

#endif // ORBITAL_ELEMENTS_HPP

double wrapTo180(double deg);