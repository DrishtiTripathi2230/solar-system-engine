#include "include/orbital_elements.hpp"
#include <iostream>

int main() {
    const auto& table = referenceTable();
    std::cout << "Loaded " << table.size() << " planets\n";
    for (const auto& p : table) {
        std::cout << p.name << ": a0=" << p.a0 << "\n";
    }
    return 0;
}