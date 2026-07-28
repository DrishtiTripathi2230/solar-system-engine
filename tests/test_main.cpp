#include <iostream>
#include <cmath>
#include "time_system.hpp"
#include "orbital_elements.hpp"
#include "kepler_solver.hpp"
#include "heliocentric_position.hpp"

int testsRun = 0;
int testsPassed = 0;

void check(const std::string& testName, double actual, double expected, double tolerance = 1e-4) {
    testsRun++;
    bool pass = std::fabs(actual - expected) < tolerance;
    if (pass) testsPassed++;

    std::cout << (pass ? "[PASS] " : "[FAIL] ") << testName
               << " -> got " << actual << ", expected " << expected << "\n";
}

void testTimeSystem() {
    std::cout << "\n--- Module 1: Time System ---\n";

    double jd1 = calendarToJulianDay(2000, 1, 1, 12, 0, 0.0);
    check("JD at J2000.0", jd1, 2451545.0, 1e-6);

    double T1 = julianCenturiesSinceJ2000(jd1);
    check("T at J2000.0", T1, 0.0, 1e-9);

    double jd2 = calendarToJulianDay(2026, 5, 22, 12, 0, 0.0);
    check("JD for 2026-05-22 12:00:00", jd2, 2461183.0, 1e-6);
}

void testOrbitalElements() {
    std::cout << "\n--- Module 2: Orbital Elements ---\n";

    check("wrapTo180(200) wraps to -160", wrapTo180(200.0), -160.0, 1e-9);
    check("wrapTo180(-200) wraps to 160", wrapTo180(-200.0), 160.0, 1e-9);
    check("wrapTo180(90) stays 90", wrapTo180(90.0), 90.0, 1e-9);

    const auto& table = referenceTable();
    const OrbitalElements& earthBase = table[2]; // Mercury, Venus, Earth...

    OrbitalElementsAtEpoch earthAtT0 = elementsAtTime(earthBase, 0.0);
    check("Earth semi-major axis at T=0 matches base a0", earthAtT0.a, earthBase.a0, 1e-9);
    check("Earth eccentricity at T=0 matches base e0", earthAtT0.e, earthBase.e0, 1e-9);
}

void testKeplerSolver() {
    std::cout << "\n--- Module 3: Kepler Solver ---\n";

    // degToRad / radToDeg round-trip
    check("degToRad(90)", degToRad(90.0), 1.570796, 1e-5);
    check("radToDeg(pi/2)", radToDeg(1.570796), 90.0, 1e-3);

    // Circular orbit: E should equal M exactly, regardless of M
    double E1 = solveKeplerEquation(45.0, 0.0);
    check("Circular orbit (e=0), M=45 -> E=45", E1, 45.0, 1e-4);

    // At perihelion (M=0), E=0 regardless of eccentricity
    double E2 = solveKeplerEquation(0.0, 0.0167);
    check("M=0 (perihelion) -> E=0, any eccentricity", E2, 0.0, 1e-4);
}

void testHeliocentricPosition() {
    std::cout << "\n--- Module 4: Heliocentric Position ---\n";

    // Circular orbit, E=0 -> sits at (a, 0, 0)
    Vec3 p1 = orbitalPlanePosition(1.0, 0.0, 0.0);
    check("orbitalPlanePosition circular E=0, x", p1.x, 1.0, 1e-6);
    check("orbitalPlanePosition circular E=0, y", p1.y, 0.0, 1e-6);

    // Eccentric orbit, E=0 -> perihelion distance a*(1-e)
    Vec3 p2 = orbitalPlanePosition(1.0, 0.5, 0.0);
    check("orbitalPlanePosition e=0.5, E=0, x", p2.x, 0.5, 1e-6);

    // No rotation at all -> ecliptic coords equal orbital coords exactly
    Vec3 orb1 = {1.0, 0.0, 0.0};
    Vec3 e1 = orbitalToEcliptic(orb1, 0.0, 0.0, 0.0);
    check("orbitalToEcliptic no rotation, x", e1.x, 1.0, 1e-6);
    check("orbitalToEcliptic no rotation, y", e1.y, 0.0, 1e-6);

    // Inclination=90, orbital y-axis point -> pushed entirely into z
    Vec3 orb2 = {0.0, 1.0, 0.0};
    Vec3 e2 = orbitalToEcliptic(orb2, 0.0, 90.0, 0.0);
    check("orbitalToEcliptic i=90, z", e2.z, 1.0, 1e-6);

    // Earth's own geocentric position (relative to itself) should be exactly zero
    Vec3 earthHelio = {1.0, 2.0, 3.0}; // arbitrary point standing in for "Earth's position"
    Vec3 earthGeo = geocentricPosition(earthHelio, earthHelio);
    check("geocentricPosition of Earth relative to itself, x", earthGeo.x, 0.0, 1e-9);
    check("geocentricPosition of Earth relative to itself, y", earthGeo.y, 0.0, 1e-9);
    check("geocentricPosition of Earth relative to itself, z", earthGeo.z, 0.0, 1e-9);

    // A simple offset case: if a planet sits at (5,5,5) and Earth at (1,1,1),
    // the geocentric vector should be exactly (4,4,4)
    Vec3 planetHelio = {5.0, 5.0, 5.0};
    Vec3 earthHelio2 = {1.0, 1.0, 1.0};
    Vec3 geo = geocentricPosition(planetHelio, earthHelio2);
    check("geocentricPosition simple offset, x", geo.x, 4.0, 1e-9);
    check("geocentricPosition simple offset, y", geo.y, 4.0, 1e-9);
    check("geocentricPosition simple offset, z", geo.z, 4.0, 1e-9);
}

int main() {
    testTimeSystem();
    testOrbitalElements();
    testKeplerSolver();
    testHeliocentricPosition();

    std::cout << "\n=== Results: " << testsPassed << "/" << testsRun << " tests passed ===\n";

    return (testsPassed == testsRun) ? 0 : 1;
}