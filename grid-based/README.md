# Grid Based Cellular Automata

## Rules

### Nutrient Diffusion

Nutrients spread to neighbors (only direct neighbors: up, down, left, right)

Constraints
- Nutrient values must stay within bounds [0.0; 1.0]
- Boundary cells act as nutrient sources

<!--Add nutient source specification-->

### Nutrient Consumption

Consumption rates:
- Alive cells consume nutrients at a high rate
- Quiescent cells consume at a lower rate
- Necrotic cells consume nothing
- Empty cells consume nothing

Consumption happens after diffusion.

### State Transitions

Cells react to their local environment.

Define two thresholds:
- divideThreshold
- deathThreshold

#### Transitions

| Current State | Condition | New State |
| --------------------------------------- |
|Alive | nutrient < T_divide | Quiescent |
| Quiescent | nutrient > T_divide | Alive |
| Alive / Quiescent | nutrient < T_death | Necrotic |
| Necrotic | always | Necrotic |

- Necrotic cells never recover
- Empty cells never change state on their own

### Cell Division

Make the tumor grow.

When a Cell Can Divide

A cell divides only if:
- State is Alive
- Nutrient > divideThreshold
- At least one neighboring cell is Empty

#### Division Steps

When division happens:
- Choose one empty neighbor at random
- Place a new Alive cell there
- Reduce nutrient of the parent cell

Constraints:
- One division per cell per step
- Division is stochastic (not deterministic)

### Visualization Rules

|State|Color|
|-----------|
|Empty|Black|
|Alive|Bright green|
|Quiescent|Yellow|
|Necrotic|Dark red|

<!--Nutrient field Blue heatmap (optional overlay)

Rules:

Cells override nutrient color

Optional alpha blending for nutrients
-->