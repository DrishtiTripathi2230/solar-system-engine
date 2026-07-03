#include "include/orbital_elements.hpp"
#include <iostream>

int main() {
    const auto& table = referenceTable();
    OrbitalElements earth = table[2];  // index 2 = Earth
    OrbitalElementsAtEpoch result = elementsAtTime(earth, 0.0);  // T=0 -> J2000 exactly

    std::cout << std::fixed;
    std::cout << "Name: " << result.name << "\n";
    std::cout << "meanAnomaly_deg = " << result.meanAnomaly_deg << " (expected -2.47311027)\n";
    std::cout << "argPeri_deg = " << result.argPeri_deg << " (expected 102.93768193)\n";
    return 0;
}