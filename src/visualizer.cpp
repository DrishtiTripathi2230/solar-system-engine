#include "visualizer.hpp"
#include <iostream>
#include <cmath>
#include <iomanip>
#include <algorithm>

const double PI = 3.14159265358979323846;

void printSolarSystemMap(const std::vector<PlanetResult>& planets) {
    const int width = 61;
    const int height = 31;
    const int centerX = width / 2;
    const int centerY = height / 2;

    std::vector<std::string> grid(height, std::string(width, ' '));

    int maxRadius = std::min(width / 2 - 1, height / 2 - 1);

    double maxDist = 0.0;
    for (const auto& p : planets) {
        if (p.distanceAU > maxDist) maxDist = p.distanceAU;
    }
    double maxCompressed = log10(maxDist + 1.0);

    // ---- Draw a faint ring for each planet's orbit first, so markers sit on top ----
    for (const auto& p : planets) {
        double compressedR = log10(p.distanceAU + 1.0);
        double scaledR = (compressedR / maxCompressed) * maxRadius;

        // Walk around the full circle in small angle steps, plotting each point.
        for (double a = 0; a < 2 * PI; a += 0.05) {
            int px = centerX + static_cast<int>(round(scaledR * cos(a)));
            int py = centerY - static_cast<int>(round(scaledR * sin(a) * 0.5)); // *0.5 to correct for character cells being taller than wide
            if (px >= 0 && px < width && py >= 0 && py < height && grid[py][px] == ' ') {
                grid[py][px] = '.';
            }
        }
    }

    grid[centerY][centerX] = '*'; // the Sun, drawn after rings so it's never overwritten

    // ---- Now place each planet's actual current position on top of its ring ----
    for (size_t idx = 0; idx < planets.size(); ++idx) {
        const auto& p = planets[idx];
        double r2D = sqrt(p.position.x * p.position.x + p.position.y * p.position.y);
        double angle = atan2(p.position.y, p.position.x);

        double compressedR = log10(r2D + 1.0);
        double scaledR = (compressedR / maxCompressed) * maxRadius;

        int px = centerX + static_cast<int>(round(scaledR * cos(angle)));
        int py = centerY - static_cast<int>(round(scaledR * sin(angle) * 0.5));

        char marker = '1' + static_cast<char>(idx);
        if (px >= 0 && px < width && py >= 0 && py < height) {
            grid[py][px] = marker;
        }
    }

    // ---- Trim blank rows above/below the circle so the map isn't padded with
    //      empty whitespace (the grid is sized to fit the widest orbit, which
    //      leaves unused rows for narrower configurations) ----
    int firstRow = 0;
    while (firstRow < height && grid[firstRow].find_first_not_of(' ') == std::string::npos) {
        ++firstRow;
    }
    int lastRow = height - 1;
    while (lastRow > firstRow && grid[lastRow].find_first_not_of(' ') == std::string::npos) {
        --lastRow;
    }

    std::cout << "\n=== Top-Down Solar System Map ===\n";
    std::cout << "The Sun (*) sits at the center. Dotted rings are each planet's orbit path.\n";
    std::cout << "Numbers mark where each planet is right now on that ring.\n";
    std::cout << "(Note: spacing is compressed so inner and outer planets both fit on screen -\n";
    std::cout << " see the AU chart below for real, accurate distances.)\n\n";

    for (int row = firstRow; row <= lastRow; ++row) {
        std::cout << grid[row] << "\n";
    }

    std::cout << "\nLegend:\n";
    for (size_t idx = 0; idx < planets.size(); ++idx) {
        std::cout << "  " << (idx + 1) << " = " << planets[idx].name << "\n";
    }

    // ---- Distance bar chart ----
    std::cout << "\n=== Real Distance from Sun (AU) ===\n\n";
    const int barScale = 40;

    for (const auto& p : planets) {
        int barLength = static_cast<int>((p.distanceAU / maxDist) * barScale);
        std::cout << std::left << std::setw(9) << p.name
                   << std::right << std::setw(7) << std::fixed << std::setprecision(2) << p.distanceAU << " AU  ";
        for (int i = 0; i < barLength; ++i) std::cout << "#";
        std::cout << "\n";
    }
    std::cout << "\n";
}