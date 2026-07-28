# Solar System Engine

A C++ engine that computes real 3D positions of the eight planets, for any
calendar date, purely from published orbital mechanics formulas -- no
external astronomy libraries. It prints exact heliocentric (Sun-relative)
and geocentric (Earth-relative) coordinates, and renders a compressed ASCII
top-down map plus a true-scale distance bar chart.

Built as a learning exercise: every function is written by hand, understood
conceptually, and checked against known reference values -- not just "it
compiles and runs."

> **Status:** Phase 1 (heliocentric positions) complete. Phase 2 in
> progress -- geocentric positions done; equatorial coordinates (RA/Dec) and
> sky coordinates are next. See [Roadmap](#roadmap).

> **Full write-up:** a detailed PDF walkthrough and a dev.to article covering
> the math and design decisions behind this project are in progress -- links
> will be added here once published.

---

## Table of Contents

- [Big picture](#big-picture)
- [Folder structure](#folder-structure)
- [How to build & run](#how-to-build--run)
- [Sample run](#sample-run)
- [Modules](#modules)
- [Data source & accuracy](#data-source--accuracy)
- [Verification approach](#verification-approach)
- [Roadmap](#roadmap)

---

## Big picture

Given a calendar date, compute where each planet actually is in 3D space --
first relative to the Sun (heliocentric), then relative to Earth
(geocentric). This happens in stages, each its own module:

```
Calendar date
     |
     v
[time_system]            -> Julian Day -> centuries since J2000 (T)
     |
     v
[orbital_elements]       -> each planet's orbit shape/orientation evolved to T
     |                       -> mean anomaly (M), arg. of perihelion (w), etc.
     v
[kepler_solver]          -> solve M = E - e*sin(E) for eccentric anomaly (E)
     |
     v
[heliocentric_position]  -> position in orbital plane -> rotated into ecliptic (x,y,z)
     |                       -> geocentricPosition() subtracts Earth's vector
     v                         to get each planet's position relative to Earth
[visualizer]              -> ASCII top-down map + AU bar chart
```

---

## Folder structure

```
solar-system-engine/
|-- README.md
|-- main.cpp
|-- include/
|   |-- time_system.hpp
|   |-- orbital_elements.hpp
|   |-- kepler_solver.hpp
|   |-- heliocentric_position.hpp
|   |-- vec3.hpp
|   `-- visualizer.hpp
|-- src/
|   |-- time_system.cpp
|   |-- orbital_elements.cpp
|   |-- kepler_solver.cpp
|   |-- heliocentric_position.cpp
|   `-- visualizer.cpp
`-- tests/
    `-- test_main.cpp      # consolidated suite, all modules (24 checks)
```

`.hpp` files declare what a function does; `.cpp` files hold the
implementation -- standard C++ practice for separating interface from
internals.

---

## How to build & run

From the project root:

```
g++ -std=c++17 -Iinclude src\time_system.cpp src\orbital_elements.cpp src\kepler_solver.cpp src\heliocentric_position.cpp src\visualizer.cpp main.cpp -o main.exe
.\main.exe
```

Run the test suite:

```
g++ -std=c++17 -Iinclude src\time_system.cpp src\orbital_elements.cpp src\kepler_solver.cpp src\heliocentric_position.cpp tests\test_main.cpp -o tests\test_main.exe
.\tests\test_main.exe
```

The program prompts interactively for a date (year, month, day, hour,
minute, second -- UTC). Input is validated and re-prompted on both
non-numeric and out-of-range entries, and day ranges account for leap years
(`daysInMonth()` in `main.cpp`).

---

## Sample run

Date: **January 1, 2000, 12:00 UTC** (i.e. exactly J2000.0):

```
Julian Day: 2.45154e+06
Centuries since J2000: 0

Earth:
  Distance from Sun: 0.983307 AU
  Position (x, y, z): (-0.177171, 0.967214, -2.58449e-07)
```

Earth's z-component is effectively zero, as expected -- Earth's orbit
*defines* the reference plane (the ecliptic), so by construction Earth never
leaves z ~= 0. Every other planet's position is computed the same way, then
also converted to a geocentric (Earth-relative) vector by subtracting
Earth's own heliocentric position.

---

## Modules

**Time System** -- converts a calendar date to Julian Day (via Meeus's
method), then to `T`, the number of Julian centuries since J2000.0
(Jan 1, 2000, 12:00 UTC), since orbital drift rates are published as
"change per century."

**Orbital Elements** -- holds each planet's six Keplerian elements (semi-major
axis, eccentricity, inclination, mean longitude, longitude of perihelion,
longitude of ascending node) as a J2000 base value plus a per-century drift
rate, from JPL/Standish (2006). `elementsAtTime()` evolves these to time `T`
and derives mean anomaly (`M = L - long_peri`) and argument of perihelion
(`w = long_peri - long_node`).

**Kepler Solver** -- solves Kepler's equation `M = E - e*sin(E)` for
eccentric anomaly `E`, via Newton-Raphson iteration (tolerance `1e-6` rad,
capped at 100 iterations -- converges in under 10 in practice).

**Heliocentric Position** -- places the planet in its orbital plane
(`x' = a(cosE - e)`, `y' = a*sqrt(1-e^2)*sinE`), then rotates that into the
ecliptic frame shared by all planets, using the three orientation angles
(w, i, node), giving `(x, y, z)` in AU relative to the Sun. Also provides
`geocentricPosition()`, which subtracts Earth's heliocentric vector from
another planet's, giving that planet's position relative to Earth.

**Visualizer** -- renders a log-scale-compressed ASCII top-down orbit map
(for visual intuition; Mercury at 0.35 AU and Neptune at 30 AU can't
otherwise share a screen) and a true-linear-scale AU bar chart, which is the
accurate source for real distances.

**Main** -- wires it all together: validated date input, computes every
planet's heliocentric position while capturing Earth's specifically, then
computes every other planet's geocentric position, and prints both plus the
visualization.

---

## Data source & accuracy

Orbital elements come from **JPL/Standish (2006), "Keplerian Elements for
Approximate Positions of the Major Planets"** -- valid roughly 3000 BC-3000
AD. This is a two-body approximation: it models each planet orbiting the Sun
independently, ignoring gravitational perturbations between planets (e.g.
Jupiter tugging on Saturn). Accurate to a fraction of a degree -- good enough
to see "where is Mars right now" correctly, not precise enough for
spacecraft navigation or exact eclipse timing.

---

## Verification approach

`tests/test_main.cpp` is a consolidated, assertion-based suite (24 checks)
covering every module, checked against independently known correct answers
rather than just "does it produce *a* number":

- **JD and T at J2000.0** are documented constants -- matched exactly.
- **Earth's z-coordinate ~= 0** at every date, since the ecliptic is
  *defined* by Earth's orbital plane.
- **Earth's geocentric position relative to itself** is exactly zero.
- Planet distances from the Sun and from Earth fall within each planet's
  known real-world ranges.

---

## Roadmap

- **Phase 2 -- Sky coordinates** (in progress)
  - [x] Geocentric position
  - [ ] Equatorial coordinates (Right Ascension / Declination)
  - [ ] Local Sidereal Time
  - [ ] Horizontal coordinates (Azimuth/Altitude)
- **Phase 3 -- Rendering & validation**
  - [ ] SVG rendering in place of the ASCII map
  - [ ] Accuracy check against JPL Horizons reference data