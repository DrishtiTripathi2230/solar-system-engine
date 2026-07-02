#include "time_system.hpp"
#include <cmath>

//MEEUS'S METHOD
double calendarToJulianDay(int year, int month, int day, int hour, int minute, double second){
    if (month <= 2){
        month = month +12;
        year = year -1;
    }
    int A = floor(year/100);
    int B = 2 - A + floor(A/4);
    double JD = floor(365.25*(year + 4716)) + floor(30.6001*(month + 1)) + day + B - 1524.5 + (hour + minute/60.0 + second/3600.0)/24.0;
    return JD;
    
}
double julianCenturiesSinceJ2000(double jd){
    double T = (jd - 2451545.0)/36525.0;
    return T;
}