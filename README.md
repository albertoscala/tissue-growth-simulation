# Grid-Based Cellular Automata (Nutrient-Driven Growth)

A **2D grid-based cellular automata** written in modern C++, modeling **cell growth, quiescence, necrosis, and division** driven by a diffusing nutrient field.  
The simulation is rendered in real time using **MiniFB**.

![Demo Simulation](demo.gif)

---

## Overview

The simulation consists of two coupled grids:

- **Cell Grid** — discrete cell states (Empty, Alive, Quiescent, Necrotic)
- **Nutrient Grid** — continuous scalar field in the range **[0.0, 1.0]**

At each simulation step:

1. Nutrients diffuse across the grid
2. Cells update their state based on local nutrient levels
3. Alive cells may divide into neighboring empty cells
4. The grid is rendered to a window

The model is inspired by **tumor-like growth dynamics** and reaction-diffusion systems.

---

## Grid Configuration

| Parameter | Value |
|---------|-------|
| Grid size | `512 × 512` |
| Neighborhood | Von Neumann (up, down, left, right) |
| Boundary condition | Nutrient source (fixed maximum) |

---

## Cell States

```cpp
enum class CellState
{
    Empty,
    Alive,
    Quiescent,
    Necrotic
};
```

| State | Description |
|-----|------------|
| **Empty** | No cell present |
| **Alive** | Actively dividing, well-fed |
| **Quiescent** | Alive but nutrient-limited |
| **Necrotic** | Dead tissue (irreversible) |

---

## Nutrient Diffusion

Nutrients spread through the grid via **discrete diffusion**.

### Diffusion Rule

For each non-boundary cell:

```
new_value = current + diffusionSpeed × (neighbor_average − current)
```

Only **direct neighbors** are considered:
- Top
- Left
- Bottom
- Right

### Constraints

- Nutrient values are clamped to **[0.0, 1.0]**
- Boundary cells are continuously refilled to `MAX_NUTRIENT`
- Diffusion is applied **before** cell updates

---

## State Transitions

Cells react to the **local nutrient concentration** using two thresholds:

- `divideThreshold`
- `deathThreshold`

### Transition Rules

| Current State | Condition | New State |
|--------------|----------|-----------|
| Alive | nutrient < divideThreshold | Quiescent |
| Quiescent | nutrient ≥ divideThreshold | Alive |
| Quiescent | nutrient < deathThreshold | Necrotic |
| Alive | nutrient < deathThreshold | Necrotic |
| Necrotic | always | Necrotic |
| Empty | always | Empty |

### Notes

- Necrotic cells **never recover**
- Empty cells **do not change state**
- Only **Alive cells** are eligible for division

---

## Cell Division (Mitosis)

Alive cells can divide stochastically.

### Division Conditions

A cell may divide if:

- State is **Alive**
- Nutrient ≥ `divideThreshold`
- At least one **neighboring cell is Empty**

### Division Process

1. Collect all empty neighboring cells
2. Choose **one at random**
3. Place a new **Alive** cell there
4. Reduce parent cell nutrient by `divideCost`

### Constraints

- Maximum **one division per cell per step**
- Division is **probabilistic**, not deterministic
- Nutrient cannot go below `0.0`

---

## Visualization

Rendering is handled via **MiniFB** in real time.

### Color Mapping

| Cell State | Color |
|-----------|------|
| Empty | Black |
| Alive | Bright Green |
| Quiescent | Yellow |
| Necrotic | Blue |

Cells fully override the nutrient field visually.

---

## Simulation Builder

The simulation uses a **builder pattern** for clean configuration:

```cpp
CellularAutomata sim =
    SimulationBuilder(cells, nutrients)
        .setDiffusionSpeed(0.08f)
        .setDeathThreshold(0.18f)
        .setDivideThreshold(0.58f)
        .setDivisionCost(0.20f)
        .build();
```

### Tunable Parameters

| Parameter | Meaning |
|--------|--------|
| `diffusionSpeed` | Rate of nutrient diffusion |
| `deathThreshold` | Nutrient level causing necrosis |
| `divideThreshold` | Nutrient level required to divide |
| `divideCost` | Nutrient lost during division |

---

## Initial Conditions

Several initialization helpers are provided in `main.cpp`:

- **Sparse random noise**
- **Central disk**
- **Multiple clusters**
- **Small random cluster**

Example:

```cpp
generate_sparse_noise(cells, nutrients, 0.002f, 0.8f);
```

---

## Build Instructions

### Compile

```bash
./comp.sh
```

### Run

```bash
./build/cellular_automata
```

Close the window to terminate the program.

---

## Dependencies

- **C++17**
- **MiniFB**

---

## Planned Extensions (TODO)

- Custom diffusion models
- Metabolic nutrient consumption
- Nutrient visualization overlay
- Performance optimizations (minimizing copies)

---

## License

[MIT](https://choosealicense.com/licenses/mit/)
