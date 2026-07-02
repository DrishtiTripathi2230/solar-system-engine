# Solar System Engine

A C++ engine that computes the positions of the planets in our solar system at any given date/time, built from published orbital mechanics formulas — no external astronomy libraries.

This project is being built as a **learning exercise**: every function is written by hand, understood conceptually, and verified against known reference values (not just "compiles and runs"). This README documents the concepts, formulas, structure, and current progress so it's easy to pick back up or explain to someone else.

---

## Where this folder lives

Project root:
```
C:\Users\Drishti\Desktop\projectss\solar-system-engine\
```

All commands in this README (and all `g++` compile commands used so far) are run **from this root folder**, not from inside `src\` or `include\`. This matters because paths like `-Iinclude` and `src\time_system.cpp` are relative to wherever you run the command from.

---

## Big picture: what problem are we solving?

Given a calendar date (e.g. "July 2, 2026, 12:00 UTC"), compute the **3D position of each planet** relative to the Sun (heliocentric coordinates, in AU — astronomical units).

This happens in three conceptual stages:

1. **Time conversion** — turn a messy calendar date into a single clean number astronomers can do math with (Julian Day, then centuries since a reference epoch).
2. **Orbital elements** — look up each planet's orbit shape/size/orientation (published data), and evolve those values forward/backward to the date we care about, since orbits slowly drift over centuries.
3. **Position solving** — use those evolved elements to actually solve for *where on the ellipse* the planet is right now (Kepler's equation), then rotate that into a shared 3D coordinate system (the ecliptic) so all planets can be compared/plotted together.

---

## Folder structure

```
solar-system-engine/
├── README.md
├── include/                       <- headers (.hpp) — declarations only, the "table of contents"
│   ├── time_system.hpp
│   ├── orbital_elements.hpp
│   ├── kepler_solver.hpp              (not started yet)
│   └── heliocentric_position.hpp      (not started yet)
├── src/                            <- implementation (.cpp) — the real logic
│   ├── time_system.cpp
│   ├── orbital_elements.cpp
│   ├── kepler_solver.cpp              (not started yet)
│   ├── heliocentric_position.cpp      (not started yet)
│   └── main.cpp                       (not started yet)
└── tests/                          <- verification against known reference values
```

**Why split `.hpp` and `.cpp`?**
The `.hpp` file declares *what* a function is called and what it takes/returns, without showing *how* it works. The `.cpp` file has the real implementation. This lets other files use a function just by including its header, without needing to see (or recompile) its internals every time. It's standard C++ practice, not specific to this project.

---

## Module 1 — Time System ✅ DONE

**Files:** `include/time_system.hpp`, `src/time_system.cpp`

### Problem it solves
Calendar dates are awkward to do math on (months have different lengths, leap years, etc.). Astronomy formulas instead use a single continuous day count: **Julian Day (JD)**. Once you have JD, "how far apart are two dates" is just subtraction, and every orbital formula downstream is built on top of it.

### Function 1: `calendarToJulianDay()`

```cpp
double calendarToJulianDay(int year, int month, int day, int hour, int minute, double second);
```

Converts a calendar date + time (UTC) into a Julian Day number.

**Algorithm (Meeus's method):**
1. If month ≤ 2, treat it as month `M+12` of the *previous* year. (Meeus's formula is built around a "year" starting in March — an old convention that simplifies leap-day handling.)
2. `A = floor(year / 100)`
3. `B = 2 − A + floor(A / 4)` — the Gregorian calendar correction.
4. `JD = floor(365.25 × (year + 4716)) + floor(30.6001 × (month + 1)) + day + B − 1524.5 + (hour + minute/60 + second/3600) / 24`

**Reference point:** `J2000.0` = `JD 2451545.0` = Jan 1, 2000, 12:00 UTC. This is the anchor every orbital element formula measures time from.

**Verified against:**
| Input | Expected JD | Got |
|---|---|---|
| 2000-01-01 12:00:00 | 2451545.000000 | 2451545.000000 ✅ |
| 2026-05-22 12:00:00 | 2461183.000000 | 2461183.000000 ✅ |

### Function 2: `julianCenturiesSinceJ2000()`

```cpp
double julianCenturiesSinceJ2000(double jd);
```

Rescales JD into **T = number of Julian centuries since J2000.0**. Every orbital element formula uses `T`, not raw JD, because element drift rates are published as "change per century."

**Formula:**
```
T = (JD − 2451545.0) / 36525.0
```
(36525.0 = number of days in a Julian century = 365.25 × 100)

**Verified:** T = 0.000000 exactly at J2000.0 itself (the defining property of the epoch) ✅

---

## Module 2 — Orbital Elements 🟡 IN PROGRESS

**Files:** `include/orbital_elements.hpp`, `src/orbital_elements.cpp`

### Problem it solves
Each planet's orbit is described by 6 numbers (Keplerian elements). These aren't fixed — they drift slowly over centuries due to gravitational influence from other planets. So each element is published as a **value at J2000** plus a **rate of change per century**, and the real value "right now" is:

```
value_now = value0 + rate × T
```

using the `T` computed in Module 1.

### The 6 elements, per planet

| Symbol | Meaning |
|---|---|
| `a` | semi-major axis — orbit size, in AU |
| `e` | eccentricity — how stretched the ellipse is (0 = perfect circle) |
| `i` | inclination — tilt of the orbit plane vs. Earth's orbital plane, in degrees |
| `L` | mean longitude — where the planet is along its orbit, in degrees |
| `long_peri` (ϖ) | longitude of perihelion — orientation of the ellipse itself |
| `long_node` (Ω) | longitude of ascending node — where the orbit crosses the reference plane |

This is **real published data** — JPL/Standish (2006) low-precision elements, valid 3000 BC–3000 AD — not something derived by hand. The code's job is the *structure* around this data, not inventing the numbers.

### Struct 1: `OrbitalElements` (raw table row — base values + drift rates)

```cpp
struct OrbitalElements {
    std::string name;
    double a0, a_dot;
    double e0, e_dot;
    double i0, i_dot;
    double L0, L_dot;
    double long_peri0, long_peri_dot;
    double long_node0, long_node_dot;
};
```

### Struct 2: `OrbitalElementsAtEpoch` (a single snapshot at a specific T — no more rates)

```cpp
struct OrbitalElementsAtEpoch {
    std::string name;
    double a;
    double e;
    double i_deg;
    double meanAnomaly_deg;
    double argPeri_deg;
    double node_deg;
};
```

Note this struct holds **mean anomaly** and **argument of perihelion**, not `L` and `long_peri` directly — these are derived quantities we need for the next module (see below).

### Function: `referenceTable()` ✅ implemented and verified

```cpp
const std::vector<OrbitalElements>& referenceTable();
```

Returns a static list of all 8 planets' base J2000 elements + drift rates (the JPL/Standish table). Verified by compiling and printing all 8 planets with correct `a0` values.

### Function: `elementsAtTime()` ❌ NOT YET IMPLEMENTED — next task

```cpp
OrbitalElementsAtEpoch elementsAtTime(const OrbitalElements& base, double T);
```

**What it needs to do:**
1. Evolve all 6 base elements forward using `value0 + rate × T`.
2. Convert `L` (mean longitude) and `long_peri` (longitude of perihelion) into the two angles we actually need downstream:
   - **Mean anomaly**: `M = L − long_peri`
   - **Argument of perihelion**: `ω = long_peri − long_node`
3. Wrap all angles into a sane range (typically `[-180°, 180°)`), since raw subtraction can produce values outside a normal degree range.

This is the next thing to build.

---

## Module 3 — Kepler Solver ❌ NOT STARTED

**Files (planned):** `include/kepler_solver.hpp`, `src/kepler_solver.cpp`

### Problem it solves
Knowing a planet's *mean* anomaly tells you where it would be if it moved at constant speed around a circle — but real orbits are ellipses, so speed varies. Kepler's equation relates mean anomaly to the *true* position on the ellipse (eccentric anomaly), but it has no closed-form solution — it must be solved **iteratively**.

**Kepler's Equation:**
```
M = E − e × sin(E)
```
where `M` = mean anomaly (known), `e` = eccentricity (known), `E` = eccentric anomaly (unknown — what we're solving for).

**Method: Newton-Raphson iteration** — start with a guess for `E`, repeatedly refine it using the derivative of the equation, until the result stops changing meaningfully (converges within a tolerance). Only valid for elliptical orbits (`e < 1`).

---

## Module 4 — Heliocentric Position ❌ NOT STARTED

**Files (planned):** `include/heliocentric_position.hpp`, `src/heliocentric_position.cpp`

### Problem it solves
Once we know a planet's position *within its own orbital plane* (from the Kepler solver), we need to rotate that into a shared 3D coordinate system — the **ecliptic** (Earth's orbital plane) — so all planets' positions are directly comparable.

This involves composing **three rotations**, using three angles:
- `ω` (argument of perihelion) — rotates within the orbital plane
- `i` (inclination) — tilts the orbital plane relative to the ecliptic
- `Ω` (longitude of ascending node) — rotates the tilted plane into final ecliptic orientation

Output: `(x, y, z)` in AU, per planet, relative to the Sun.

---

## Module 5 — Main Program ❌ NOT STARTED

**Files (planned):** `src/main.cpp`

Wires everything together: takes a calendar date as input (CLI args), runs it through all the modules above for each of the 8 planets, and prints/outputs the resulting `(x, y, z)` positions.

---

## How to build (current pieces)

From the project root:

```powershell
g++ -std=c++17 -Iinclude src\time_system.cpp src\orbital_elements.cpp <your_test_file.cpp> -o output.exe
.\output.exe
```

Each new module gets added to this command as it's written.

---

## Why every value is verified, not just "it compiled"

Code that compiles and runs can still be *wrong* — silently. Wherever possible, each function here is checked against an independently known correct answer (e.g. JD at J2000.0 is a documented constant, T=0 at J2000.0 is a mathematical certainty), not just "does it produce *a* number." Later modules (Kepler solver, heliocentric position) will be checked against known JPL Horizons planetary position data the same way.

---

## Status summary

| Module | Status |
|---|---|
| Time System (JD, Julian centuries) | ✅ Done, verified |
| Orbital Elements — structs + reference table | ✅ Done, verified |
| Orbital Elements — `elementsAtTime()` | 🟡 Next up |
| Kepler Solver | ❌ Not started |
| Heliocentric Position | ❌ Not started |
| Main program | ❌ Not started |
| Tests | ❌ Not started (only ad-hoc scratch files so far) |

---

## Roadmap after Phase 1

- **Phase 2** — geocentric position (subtract Earth's vector from each planet's), Sun and Moon special cases, equatorial (RA/Dec) and horizontal (Azimuth/Altitude) sky coordinates, Local Sidereal Time.
- **Phase 3** — SVG rendering (top-down solar system view, optional sky-chart view), final accuracy validation against JPL Horizons.