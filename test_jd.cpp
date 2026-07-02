#include <iostream>
#include "include/time_system.hpp"

int main(){
    double jd1 = calendarToJulianDay(2000, 1, 1, 12, 0, 0);
    std::cout << std::fixed;
    std::cout << "JD (2000-01-01 12:00) = " << jd1 << " (expected 2451545.000000)\n";

    double jd2 = calendarToJulianDay(2026, 5, 22, 12, 0, 0);
    std::cout << "JD (2026-05-22 12:00) = " << jd2 << " (expected 2461183.000000)\n";

    double T = julianCenturiesSinceJ2000(jd1); // using your Jan 1, 2000 JD from before
    std::cout << "T (2000-01-01 12:00) = " << T << " (expected 0.000000)\n";
    return 0;
}