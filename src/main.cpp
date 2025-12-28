#include <cellular_automata.hpp>

#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <algorithm>

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
make_empty_cell_grid()
{
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> grid{};
    for (auto& row : grid)
        row.fill(CellState::Empty);
    return grid;
}

std::array<std::array<float, GRID_SIZE>, GRID_SIZE>
make_uniform_nutrient_grid(float value)
{
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> grid{};
    for (auto& row : grid)
        row.fill(value);
    return grid;
}

std::mt19937& rng()
{
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

void sim_sparse_noise(auto& cells, auto& nutrients)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(0.8f);

    std::uniform_real_distribution<float> dist(0.f, 1.f);

    for (int i = 1; i < GRID_SIZE - 1; ++i)
        for (int j = 1; j < GRID_SIZE - 1; ++j)
            if (dist(rng()) < 0.002f)
                cells[i][j] = CellState::Alive;
}

void sim_radial_tumor(auto& cells, auto& nutrients)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(0.2f);

    int cx = GRID_SIZE / 2, cy = GRID_SIZE / 2;
    for (int i = 0; i < GRID_SIZE; ++i)
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            float d = std::hypot(i - cx, j - cy);
            nutrients[i][j] = std::clamp(1.f - d / (GRID_SIZE / 2.f), 0.f, 1.f);
            if (d < 6) cells[i][j] = CellState::Alive;
        }
}

void sim_competing_colonies(auto& cells, auto& nutrients)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(0.7f);

    std::uniform_int_distribution<int> pos(20, GRID_SIZE - 20);

    for (int c = 0; c < 12; ++c)
    {
        int cx = pos(rng()), cy = pos(rng());
        for (int i = cx - 4; i <= cx + 4; ++i)
            for (int j = cy - 4; j <= cy + 4; ++j)
                if ((i - cx)*(i - cx) + (j - cy)*(j - cy) <= 16)
                    cells[i][j] = CellState::Alive;
    }
}

void sim_stripes(auto& cells, auto& nutrients)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(0.15f);

    for (int i = 0; i < GRID_SIZE; ++i)
        if ((i / 16) % 2 == 0)
            for (int j = 0; j < GRID_SIZE; ++j)
                nutrients[i][j] = 0.9f;

    for (int j = GRID_SIZE / 3; j < 2 * GRID_SIZE / 3; ++j)
        cells[GRID_SIZE / 2][j] = CellState::Alive;
}

void sim_traveling_wave(auto& cells, auto& nutrients)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(0.1f);

    for (int i = 0; i < GRID_SIZE; ++i)
    {
        float w = 0.5f * (1.f + std::sin(2.f * float(M_PI) * i / 40.f));
        for (int j = 0; j < GRID_SIZE; ++j)
            nutrients[i][j] = 0.1f + w * 0.9f;
    }

    for (int j = GRID_SIZE / 4; j < 3 * GRID_SIZE / 4; ++j)
        cells[2][j] = CellState::Alive;
}

void sim_ring(auto& cells, auto& nutrients)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(0.6f);

    int cx = GRID_SIZE / 2, cy = GRID_SIZE / 2;
    for (int i = 0; i < GRID_SIZE; ++i)
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            float d = std::hypot(i - cx, j - cy);
            if (d > 20 && d < 24)
                cells[i][j] = CellState::Alive;
        }
}

int main(int argc, char** argv)
{
    //TODO: To improve selections
    if (argc < 2)
    {
        std::cout << "Usage: ./build/cellular_automata <0-5>\n";
        return 1;
    }

    int simId = std::stoi(argv[1]);

    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> cells;
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> nutrients;

    switch (simId)
    {
        case 0: sim_sparse_noise(cells, nutrients); break;
        case 1: sim_radial_tumor(cells, nutrients); break;
        case 2: sim_competing_colonies(cells, nutrients); break;
        case 3: sim_stripes(cells, nutrients); break;
        case 4: sim_traveling_wave(cells, nutrients); break;
        case 5: sim_ring(cells, nutrients); break;
        default:
            std::cerr << "Invalid simulation ID\n";
            return 1;
    }

    CellularAutomata sim =
        SimulationBuilder(cells, nutrients)
            .setDiffusionSpeed(0.05f)
            .setDeathThreshold(0.18f)
            .setDivideThreshold(0.55f)
            .setDivisionCost(0.18f)
            .build();

    for (uint64_t epoch = 0;; ++epoch)
    {
        std::cout << "Epoch: " << epoch << "\n";
        sim.step();
    }
}