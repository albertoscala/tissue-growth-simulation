#include <cellular_automata.hpp>

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

void generate_small_random_cluster(
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cells,
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrients,
    int count = 100
)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(0.7f);

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(200, GRID_SIZE - 201);

    for (int k = 0; k < count; ++k)
    {
        int i = dist(gen);
        int j = dist(gen);
        cells[i][j] = CellState::Alive;
    }
}

void generate_central_disk(
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cells,
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrients,
    int radius = 12,
    float initNutrient = 0.75f
)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(initNutrient);

    int cx = GRID_SIZE / 2;
    int cy = GRID_SIZE / 2;

    for (int i = cx - radius; i <= cx + radius; ++i)
    {
        for (int j = cy - radius; j <= cy + radius; ++j)
        {
            int dx = i - cx;
            int dy = j - cy;
            if (dx * dx + dy * dy <= radius * radius)
                cells[i][j] = CellState::Alive;
        }
    }
}

void generate_multiple_clusters(
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cells,
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrients,
    int clusters = 5,
    int radius = 6,
    float initNutrient = 0.70f
)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(initNutrient);

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(50, GRID_SIZE - 51);

    for (int c = 0; c < clusters; ++c)
    {
        int cx = dist(gen);
        int cy = dist(gen);

        for (int i = cx - radius; i <= cx + radius; ++i)
        {
            for (int j = cy - radius; j <= cy + radius; ++j)
            {
                int dx = i - cx;
                int dy = j - cy;
                if (dx * dx + dy * dy <= radius * radius)
                    cells[i][j] = CellState::Alive;
            }
        }
    }
}

void generate_sparse_noise(
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cells,
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrients,
    float probability = 0.002f,
    float initNutrient = 0.80f
)
{
    cells = make_empty_cell_grid();
    nutrients = make_uniform_nutrient_grid(initNutrient);

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 1; i < GRID_SIZE - 1; ++i)
    {
        for (int j = 1; j < GRID_SIZE - 1; ++j)
        {
            if (dist(gen) < probability)
                cells[i][j] = CellState::Alive;
        }
    }
}

//TODO: Add custom diffusion model support
//TODO: Add metabolic consumption
int main()
{
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> cells;
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> nutrients;

    /*
    generate_small_random_cluster(cells, nutrients);

    CellularAutomata sim = SimulationBuilder(cells, nutrients)
        .setDiffusionSpeed(0.06f)
        .setDeathThreshold(0.20f)
        .setDivideThreshold(0.60f)
        .setDivisionCost(0.12f)
        .build();
    
    generate_central_disk(cells, nutrients);

    CellularAutomata sim =
        SimulationBuilder(cells, nutrients)
            .setDiffusionSpeed(0.05f)
            .setDeathThreshold(0.25f)
            .setDivideThreshold(0.65f)
            .setDivisionCost(0.10f)
            .build();
    
    generate_multiple_clusters(cells, nutrients);
    
    CellularAutomata sim =
        SimulationBuilder(cells, nutrients)
            .setDiffusionSpeed(0.055f)
            .setDeathThreshold(0.22f)
            .setDivideThreshold(0.62f)
            .setDivisionCost(0.11f)
            .build();
    */
    generate_sparse_noise(cells, nutrients);

    CellularAutomata sim =
        SimulationBuilder(cells, nutrients)
            .setDiffusionSpeed(0.01f)
            .setDeathThreshold(0.22f)
            .setDivideThreshold(0.62f)
            .setDivisionCost(0.18f)
            .build();

    for (uint64_t epoch=0;;epoch++)
    {
        std::cout << "Epoch: " << epoch << std::endl; 
        sim.step();
    }
}