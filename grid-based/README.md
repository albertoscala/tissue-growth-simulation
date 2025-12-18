🌊 Rule Set 1 — Nutrient Diffusion
Purpose

Create gradients so:

Edge cells thrive

Core cells starve

Rules

Nutrients spread to neighbors

Use only direct neighbors (up, down, left, right)

Diffusion is:

Slow

Stable

Repeated every step

Constraints

Nutrient values must stay within bounds

Boundary cells act as nutrient sources

You’re proposing:

“Never update boundary cells at all.
Only run diffusion on interior cells
(1 … GRID_SIZE-2).
Boundary cells remain fixed forever.”

This means:

Boundary values are initialized to MAX_NUTRIENT

They are never touched again

Interior cells diffuse toward them

✅ Is This Correct?

Yes. This is absolutely correct
IF you understand what physical model it represents.

This implements fixed Dirichlet boundary conditions.

In plain CS terms:

Boundaries are constant external inputs.



🍽 Rule Set 2 — Nutrient Consumption
Purpose

Make cells compete for resources.

Rules

Alive cells consume nutrients at a high rate

Quiescent cells consume at a lower rate

Necrotic cells consume nothing

Empty cells consume nothing

Consumption happens after diffusion.

🧠 Rule Set 3 — State Transitions
Purpose

Cells react to their local environment.

Define two thresholds:

T_divide

T_death

Transitions:
Current State	Condition	New State
Alive	nutrient < T_divide	Quiescent
Quiescent	nutrient > T_divide	Alive
Alive / Quiescent	nutrient < T_death	Necrotic
Necrotic	always	Necrotic

Rules:

Necrotic cells never recover

Empty cells never change state on their own

🌱 Rule Set 4 — Cell Division
Purpose

Make the tumor grow.

When a Cell Can Divide

A cell divides only if:

State is Alive

Nutrient > T_divide

At least one neighboring cell is Empty

How Division Works

When division happens:

Choose one empty neighbor at random

Place a new Alive cell there

Reduce nutrient of the parent cell

Constraints:

One division per cell per step

Division is stochastic (not deterministic)

☠ Rule Set 5 — Necrotic Core Formation
Purpose

Create realistic tumors.

Rules:

Necrotic cells:

Block nutrient diffusion slightly

Remain visually distinct

Over time:

Core becomes dead

Outer shell keeps growing

No cleanup or removal yet.

🎨 Visualization Rules
Color Mapping (Mandatory)
State	Color
Empty	Black
Alive	Bright green
Quiescent	Yellow
Necrotic	Dark red
Nutrient field	Blue heatmap (optional overlay)

Rules:

Cells override nutrient color

Optional alpha blending for nutrients