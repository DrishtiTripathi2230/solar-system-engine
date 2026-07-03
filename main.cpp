#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include "time_system.hpp"
#include "orbital_elements.hpp"
#include "kepler_solver.hpp"
#include "heliocentric_position.hpp"
#include "visualizer.hpp"

// ---- Input helpers -----------------------------------------------------
// Reads an integer within [minVal, maxVal]. Re-prompts on non-numeric input
// (e.g. letters) and on out-of-range values, instead of silently accepting
// bad data or leaving std::cin in a broken state.
static int readIntInRange(const std::string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;

        if (std::cin.fail()) {
            std::cin.clear(); // reset the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard bad input
            std::cout << "  That's not a valid whole number. Please try again.\n";
            continue;
        }

        if (value < minVal || value > maxVal) {
            std::cout << "  Please enter a value between " << minVal << " and " << maxVal << ".\n";
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }
}

// Reads a double within [minVal, maxVal], with the same validation approach.
static double readDoubleInRange(const std::string& prompt, double minVal, double maxVal) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  That's not a valid number. Please try again.\n";
            continue;
        }

        if (value < minVal || value > maxVal) {
            std::cout << "  Please enter a value between " << minVal << " and " << maxVal << ".\n";
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }
}

// Returns the number of days in a given month/year, accounting for leap years,
// so day input can be validated properly (e.g. reject Feb 30).
static int daysInMonth(int year, int month) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return isLeap ? 29 : 28;
    }
    return days[month - 1];
}

int main() {
    std::cout << "=== Solar System Position Calculator ===\n\n";
    std::cout << "Enter a date to compute planet positions for.\n";

    int year = readIntInRange("Year (e.g. 2026): ", 1, 9999);
    int month = readIntInRange("Month (1-12): ", 1, 12);
    int maxDay = daysInMonth(year, month);
    int day = readIntInRange("Day (1-" + std::to_string(maxDay) + "): ", 1, maxDay);
    int hour = readIntInRange("Hour, UTC (0-23): ", 0, 23);
    int minute = readIntInRange("Minute (0-59): ", 0, 59);
    double second = readDoubleInRange("Second (0-59): ", 0.0, 59.999);

    double jd = calendarToJulianDay(year, month, day, hour, minute, second);
    double T = julianCenturiesSinceJ2000(jd);

    std::cout << "\nJulian Day: " << jd << "\n";
    std::cout << "Centuries since J2000: " << T << "\n";

    std::cout << "\n=== Planet Positions ===\n\n";

    std::vector<PlanetResult> results;

    for (const OrbitalElements& base : referenceTable()) {
        OrbitalElementsAtEpoch elem = elementsAtTime(base, T);

        double meanAnomaly = wrapTo180(elem.meanAnomaly_deg);
        double E = solveKeplerEquation(meanAnomaly, elem.e);

        Vec3 orbPos = orbitalPlanePosition(elem.a, elem.e, E);
        Vec3 eclPos = orbitalToEcliptic(orbPos, elem.argPeri_deg, elem.i_deg, elem.node_deg);

        double distanceFromSun = sqrt(eclPos.x * eclPos.x + eclPos.y * eclPos.y + eclPos.z * eclPos.z);

        std::cout << elem.name << ":\n";
        std::cout << "  Distance from Sun: " << distanceFromSun << " AU\n";
        std::cout << "  Position (x, y, z): (" << eclPos.x << ", " << eclPos.y << ", " << eclPos.z << ")\n\n";

        results.push_back({elem.name, distanceFromSun, eclPos});
    }

    printSolarSystemMap(results);

    return 0;
}