# Solar System Engine

A C++ engine that computes the real 3D positions of the eight planets, for any
calendar date, purely from published orbital mechanics formulas — no external
astronomy libraries. It prints exact heliocentric coordinates and renders a
compressed ASCII top-down map plus a distance bar chart.

This project was built as a learning exercise: every function is written by
hand, understood conceptually, and checked against known reference values —
not just "it compiles and runs."

> **Status: Phase 1 complete.** All modules below are implemented, wired
> together in `main.cpp`, and produce verified output. See
> [Roadmap](#roadmap--whats-next) for Phase 2/3 plans (geocentric positions,
> sky coordinates, SVG rendering).

---

## Table of Contents

- [Where this lives](#where-this-lives)
- [Big picture: what problem are we solving?](#big-picture-what-problem-are-we-solving)
- [Folder structure](#folder-structure)
- [How to build & run](#how-to-build--run)
- [Sample run](#sample-run)
- [Module 1 — Time System](#module-1--time-system)
- [Module 2 — Orbital Elements](#module-2--orbital-elements)
- [Module 3 — Kepler Solver](#module-3--kepler-solver)
- [Module 4 — Heliocentric Position](#module-4--heliocentric-position)
- [Module 5 — Visualizer](#module-5--visualizer)
- [Module 6 — Main Program](#module-6--main-program)
- [Data source & accuracy](#data-source--accuracy)
- [Verification approach](#verification-approach)
- [Roadmap / what's next](#roadmap--whats-next)

---

## Where this lives

Project root:

```
C:\Users\Drishti\Desktop\projectss\solar-system-engine\
```

All commands below are run **from this root folder**, not from inside `src\`
or `include\` — paths like `-Iinclude` and `src\time_system.cpp` are relative
to wherever you run the command from.

---

## Big picture: what problem are we solving?

Given a calendar date (e.g. "January 1, 2000, 12:00 UTC"), compute the 3D
position of each planet relative to the Sun (**heliocentric coordinates**, in
AU — astronomical units).

This happens in four conceptual stages, each its own module:

1. **Time conversion** — turn a messy calendar date into a single clean
   number astronomers can do math with (Julian Day, then centuries since a
   reference epoch).
2. **Orbital elements** — look up each planet's orbit shape/size/orientation
   (published data), and evolve those values forward/backward to the date we
   care about, since orbits slowly drift over centuries.
3. **Position solving** — solve Kepler's equation to find exactly where on
   the ellipse the planet is *right now*, then rotate that into a shared 3D
   coordinate system (the ecliptic) so all planets are directly comparable.
4. **Visualization** — render the result as a human-readable ASCII map and
   distance chart.

```
Calendar date
     │
     ▼
[time_system]        → Julian Day → centuries since J2000 (T)
     │
     ▼
[orbital_elements]   → base elements + drift rates, evolved to T
     │                  → mean anomaly (M), arg. of perihelion (ω), etc.
     ▼
[kepler_solver]      → solve M = E − e·sin(E) for eccentric anomaly (E)
     │
     ▼
[heliocentric_position] → position in orbital plane → rotate into ecliptic (x, y, z)
     │
     ▼
[visualizer]         → ASCII top-down map + AU bar chart
```

---

## Folder structure

```
solar-system-engine/
├── README.md
├── main.cpp
├── include/
│   ├── time_system.hpp
│   ├── orbital_elements.hpp
│   ├── kepler_solver.hpp
│   ├── heliocentric_position.hpp
│   ├── vec3.hpp
│   └── visualizer.hpp
├── src/
│   ├── time_system.cpp
│   ├── orbital_elements.cpp
│   ├── kepler_solver.cpp
│   ├── heliocentric_position.cpp
│   └── visualizer.cpp
└── tests/
    ├── test_jd.cpp
    ├── test_deg.cpp
    ├── test_elements.cpp
    ├── test_elements2.cpp
    ├── test_kepler.cpp
    ├── test_helio.cpp
    └── test_ecliptic.cpp
```

**Why split `.hpp` and `.cpp`?** The header declares *what* a function is
called and what it takes/returns, without showing *how* it works. The `.cpp`
holds the real implementation. This lets other files use a function just by
including its header, without needing to see (or recompile) its internals
every time. Standard C++ practice, not specific to this project.

---

## How to build & run

From the project root:

```
g++ -std=c++17 -Iinclude src\time_system.cpp src\orbital_elements.cpp src\kepler_solver.cpp src\heliocentric_position.cpp src\visualizer.cpp main.cpp -o main.exe
.\main.exe
```

**Common gotcha:** if you accidentally compile `main.cpp` alone without the
`src\*.cpp` files, the linker fails with `undefined reference to 'WinMain'`
on MinGW — that error is misleading; it really means the other object files
(and therefore `main()`'s dependencies) never got linked in. Always include
all five `src\*.cpp` files plus `main.cpp` in the same command.

The program then prompts interactively for a date:

```
Year (e.g. 2026): 2000
Month (1-12): 1
Day (1-31): 1
Hour, UTC (0-23): 12
Minute (0-59): 0
Second (0-59): 0
```

Input is validated and re-prompted on both non-numeric entries and
out-of-range values (e.g. typing "a" for the year, or "13" for the month),
and the day range is computed per month/year so leap years are handled
correctly (`daysInMonth()` in `main.cpp`).

---

## Sample run

Date: **January 1, 2000, 12:00 UTC** (i.e. exactly J2000.0):

```
Julian Day: 2.45154e+06
Centuries since J2000: 0

=== Planet Positions ===

Earth:
  Distance from Sun: 0.983307 AU
  Position (x, y, z): (-0.177171, 0.967214, -2.58449e-07)
```

The z-component is effectively zero for Earth, as expected — Earth's orbit
*defines* the reference plane (the ecliptic), so by construction Earth never
leaves z ≈ 0.

The engine also renders a compressed top-down map and a true-scale distance
bar chart:

```
=== Real Distance from Sun (AU) ===

Mercury     0.47 AU  
Venus       0.72 AU  
Earth       0.98 AU  #
Mars        1.39 AU  #
Jupiter     4.97 AU  ######
Saturn      9.17 AU  ############
Uranus     19.92 AU  ##########################
Neptune    30.12 AU  ########################################
```

---

## Module 1 — Time System

**Files:** `include/time_system.hpp`, `src/time_system.cpp`

### Problem it solves

Calendar dates are awkward to do math on (months have different lengths,
leap years, etc.). Astronomy formulas instead use a single continuous day
count: **Julian Day (JD)**. Once you have JD, "how far apart are two dates"
is just subtraction, and every downstream orbital formula is built on top of
it.

### `calendarToJulianDay()`

```cpp
double calendarToJulianDay(int year, int month, int day, int hour, int minute, double second);
```

Uses **Meeus's method**:

1. If `month ≤ 2`, treat it as month `M+12` of the previous year (Meeus's
   formula is built around a "year" starting in March — an old convention
   that simplifies leap-day handling).
2. `A = floor(year / 100)`
3. `B = 2 − A + floor(A / 4)` — the Gregorian calendar correction.
4. 
   ```
   JD = floor(365.25 × (year + 4716))
      + floor(30.6001 × (month + 1))
      + day + B − 1524.5
      + (hour + minute/60 + second/3600) / 24
   ```

**Reference point:** J2000.0 = JD 2451545.0 = Jan 1, 2000, 12:00 UTC. This is
the anchor every orbital element formula measures time from.

### `julianCenturiesSinceJ2000()`

```cpp
double julianCenturiesSinceJ2000(double jd);
```

Rescales JD into `T` = number of Julian centuries since J2000.0, because
orbital element drift rates are published as "change per century," not
"change per day":

```
T = (JD − 2451545.0) / 36525.0
```

(36525.0 = number of days in a Julian century = 365.25 × 100)

**Verified:**

| Input | Expected JD | Got |
|---|---|---|
| 2000-01-01 12:00:00 | 2451545.000000 | 2451545.000000 ✅ |
| 2026-05-22 12:00:00 | 2461183.000000 | 2461183.000000 ✅ |

And `T = 0.000000` exactly at J2000.0 itself — the defining property of the
epoch. ✅

---

## Module 2 — Orbital Elements

**Files:** `include/orbital_elements.hpp`, `src/orbital_elements.cpp`

### Problem it solves

Each planet's orbit is described by six numbers (Keplerian elements). These
aren't fixed — they drift slowly over centuries due to gravitational
influence from other planets. So each element is published as a value at
J2000 *plus* a rate of change per century, and the real value "right now" is
a simple linear extrapolation:

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

This is real published data — **JPL/Standish (2006) low-precision elements**,
valid roughly 3000 BC–3000 AD — not something derived by hand. The code's job
is the structure around this data, not inventing the numbers. It lives as a
static table in `referenceTable()`, one row per planet, each with a J2000
base value and a per-century drift rate for all six elements.

### `elementsAtTime()`

```cpp
OrbitalElementsAtEpoch elementsAtTime(const OrbitalElements& base, double T);
```

What it actually does, line by line:

```cpp
double a          = base.a0 + base.a_dot * T;
double e          = base.e0 + base.e_dot * T;
double i_deg      = base.i0 + base.i_dot * T;
double L          = base.L0 + base.L_dot * T;
double long_peri  = base.long_peri0 + base.long_peri_dot * T;
double long_node  = base.long_node0 + base.long_node_dot * T;
double node_deg   = long_node;

double meanAnomaly_deg = wrapTo180(L - long_peri);
double argPeri_deg     = long_peri - long_node;   // NOT wrapped
```

Two derived angles are computed from the raw elements, because they're what
the Kepler solver and the ecliptic rotation actually need:

- **Mean anomaly:** `M = L − ϖ` — how far along the orbit the planet would be
  if it moved at constant angular speed. Wrapped into `(-180°, 180°]` via
  `wrapTo180()`, since raw subtraction can land outside a sane degree range.
- **Argument of perihelion:** `ω = ϖ − Ω` — orientation of the ellipse within
  its own orbital plane. Note this one is **left un-wrapped** in the current
  implementation.

`wrapTo180()`:

```cpp
double wrapTo180(double deg) {
    while (deg > 180.0)  deg -= 360.0;
    while (deg <= -180.0) deg += 360.0;
    return deg;
}
```

---

## Module 3 — Kepler Solver

**Files:** `include/kepler_solver.hpp`, `src/kepler_solver.cpp`

### Problem it solves

Mean anomaly tells you where a planet *would* be if it moved at constant
speed around a circle — but real orbits are ellipses, so speed varies (faster
near the Sun, slower far away). **Kepler's Equation** relates mean anomaly to
the true position on the ellipse (eccentric anomaly), but it has no
closed-form solution — it must be solved iteratively.

### Kepler's Equation

```
M = E − e·sin(E)
```

where `M` = mean anomaly (known), `e` = eccentricity (known), `E` =
eccentric anomaly (unknown — what we're solving for). Only valid for
elliptical orbits (`e < 1`), which all 8 planets satisfy.

### `solveKeplerEquation()`

Solved with **Newton-Raphson iteration**: start with a guess for `E`
(`E₀ = M` is used here), and repeatedly refine it using the function's
derivative until it stops changing meaningfully:

```
f(E)  = E − e·sin(E) − M
f'(E) = 1 − e·cos(E)

E_new = E − f(E) / f'(E)
```

Implementation details:
- **Tolerance:** `1e-6` radians — iteration stops once a step (`ΔE`)
  changes `E` by less than this.
- **Max iterations:** 100 (safety cap; in practice this converges in well
  under 10 iterations for all planets' actual eccentricities).
- Angles enter and leave this function in **degrees** (`degToRad` /
  `radToDeg` helpers), even though the math is done in radians internally.

---

## Module 4 — Heliocentric Position

**Files:** `include/heliocentric_position.hpp`, `src/heliocentric_position.cpp`
(plus `include/vec3.hpp`, which just defines the plain `Vec3{ x, y, z }`
struct shared by this module and the visualizer)

### Problem it solves

Once we know a planet's position *within its own orbital plane* (from the
Kepler solver), we need to rotate that into a shared 3D coordinate system —
the **ecliptic** (Earth's orbital plane) — so all planets' positions are
directly comparable and can be plotted together.

### Step 1 — Position within the orbital plane

```cpp
Vec3 orbitalPlanePosition(double a, double e, double eccentricAnomaly_deg);
```

```
x' = a · (cos E − e)
y' = a · √(1 − e²) · sin E
z' = 0
```

This places the planet in a 2D plane where the Sun sits at one focus of the
ellipse (not the center) — the `− e` term shifts the ellipse so the Sun is
correctly offset.

### Step 2 — Rotate into the ecliptic

```cpp
Vec3 orbitalToEcliptic(const Vec3& orbitalPos, double argPeri_deg, double inclination_deg, double node_deg);
```

This composes three rotations, using the three orientation angles:

- **ω** (argument of perihelion) — rotates within the orbital plane
- **i** (inclination) — tilts the orbital plane relative to the ecliptic
- **Ω** (longitude of ascending node) — rotates the tilted plane into its
  final ecliptic orientation

The combined rotation matrix applied is:

```
x = (cosΩ·cosω − sinΩ·sinω·cosi)·x'  +  (−cosΩ·sinω − sinΩ·cosω·cosi)·y'
y = (sinΩ·cosω + cosΩ·sinω·cosi)·x'  +  (−sinΩ·sinω + cosΩ·cosω·cosi)·y'
z = (sinω·sini)·x'                    +  (cosω·sini)·y'
```

**Output:** `(x, y, z)` in AU, per planet, relative to the Sun, all expressed
in the same ecliptic frame — this is why Earth's `z` comes out ≈ 0 at every
date (Earth's orbit *defines* the ecliptic plane by convention).

---

## Module 5 — Visualizer

**Files:** `include/visualizer.hpp`, `src/visualizer.cpp`

### Problem it solves

Raw `(x, y, z)` numbers are hard to build intuition from. The visualizer
renders two complementary views: a proportionally-*distorted* map that fits
all 8 planets on one screen, and a proportionally-*accurate* bar chart so
the real scale isn't lost.

### Top-down ASCII map

Real orbital radii range from 0.47 AU (Mercury) to 30 AU (Neptune) — plotted
at true linear scale, the inner four planets would collapse into a single
pixel. To keep every planet visible and distinguishable, radii are
compressed with a **log scale** before being placed on the grid:

```
compressedR = log10(distanceAU + 1)
scaledR     = (compressedR / log10(maxDistanceAU + 1)) × maxGridRadius
```

For each planet, a full ring is traced first (`'.'` characters, stepping
around the circle in small angle increments) so the reader can see the orbit
path, then the planet's actual current position is stamped on top as a
digit (`1` = Mercury … `8` = Neptune). The Sun (`*`) is drawn last at dead
center so it's never overwritten.

The vertical coordinate is scaled by `0.5` when converting to a screen row
(`py = centerY − round(scaledR·sin(angle)·0.5)`) to compensate for terminal
character cells being taller than they are wide — without this, circles
would render as tall ellipses.

After drawing, fully blank rows above/below the used circle are trimmed so
the output isn't padded with empty whitespace for narrower configurations.

**Important:** this map is for *visual intuition only* — spacing is
deliberately non-linear. The next section (the AU bar chart) is what to trust
for real distances.

### Distance bar chart

Drawn at **true linear scale**, relative to whichever planet is currently
farthest from the Sun:

```
barLength = (planet.distanceAU / maxDistanceAU) × 40
```

Each planet gets one `#` per proportional unit, so Mercury vs. Neptune's bar
lengths are honestly comparable, unlike the compressed map above.

---

## Module 6 — Main Program

**Files:** `main.cpp`

Wires every module together:

1. **Validated input helpers** — `readIntInRange()` and `readDoubleInRange()`
   loop until a well-formed value in range is entered, clearing `std::cin`'s
   fail state and discarding bad tokens on non-numeric input rather than
   getting stuck in an infinite loop of the same failed read.
2. **`daysInMonth()`** — computes the correct day upper bound per month,
   including leap-year handling for February (`year % 4 == 0 && year % 100
   != 0) || (year % 400 == 0)`), so day input validation is calendar-correct.
3. For each of the 8 planets in `referenceTable()`:
   - Evolve elements to the current `T` via `elementsAtTime()`.
   - Wrap mean anomaly, solve Kepler's equation for eccentric anomaly.
   - Compute orbital-plane position, then rotate to ecliptic `(x, y, z)`.
   - Compute distance from the Sun as `√(x² + y² + z²)`.
4. Print each planet's distance and position, then hand the full result set
   to `printSolarSystemMap()` for the ASCII visualization.

---

## Data source & accuracy

Orbital elements come from **JPL/Standish (2006), "Keplerian Elements for
Approximate Positions of the Major Planets"** — the standard low-precision
planetary element set, valid over roughly 3000 BC to 3000 AD.

This is a **two-body approximation**: it models each planet orbiting the Sun
independently, ignoring gravitational perturbations between planets (e.g.
Jupiter tugging on Saturn). For most casual and educational purposes the
result is accurate to a fraction of a degree — good enough to see "where is
Mars right now" correctly, but not precise enough for spacecraft navigation
or exact eclipse timing. High-precision work would use full JPL Horizons
ephemerides (Chebyshev-polynomial fits) instead of this element table.

---

## Verification approach

Code that compiles and runs can still be silently wrong. Wherever possible,
each function is checked against an independently known correct answer
rather than just "does it produce *a* number":

- **JD at J2000.0** is a documented constant (2451545.0) — matched exactly.
- **T = 0 at J2000.0** is a mathematical certainty — matched exactly.
- **Earth's z-coordinate ≈ 0** at every date, because the ecliptic is
  *defined* by Earth's orbital plane — this held true in every sample run.
- Planet distances from the Sun fall within each planet's known
  perihelion/aphelion range (e.g. Mars between ~1.38 and ~1.67 AU) across
  different test dates.

A separate ad-hoc scratch test file was used during development for each
module, rather than one shared test file for everything:

| Test file | Module it checks |
|---|---|
| `test_jd.cpp` | Time System — Julian Day |
| `test_deg.cpp` | Kepler Solver — `degToRad` / `radToDeg` |
| `test_elements.cpp`, `test_elements2.cpp` | Orbital Elements — `referenceTable`, `elementsAtTime` |
| `test_kepler.cpp` | Kepler Solver — `solveKeplerEquation` |
| `test_helio.cpp` | Heliocentric Position — `orbitalPlanePosition` |
| `test_ecliptic.cpp` | Heliocentric Position — `orbitalToEcliptic` |

These are one-off scratch programs (each compiled and run manually against
hand-checked expected values), not an automated assertion-based test suite —
a good candidate for future cleanup if the project grows further.

---

## Roadmap / what's next

- **Phase 2 — Geocentric & sky coordinates**
  - Geocentric position (subtract Earth's vector from each planet's vector)
  - Special-case handling for the Sun and Moon
  - Equatorial (Right Ascension / Declination) and horizontal
    (Azimuth/Altitude) sky coordinates
  - Local Sidereal Time
- **Phase 3 — Rendering & validation**
  - SVG rendering (proper top-down solar system view, optional sky-chart
    view) instead of the ASCII map
  - Final accuracy validation against JPL Horizons reference data
