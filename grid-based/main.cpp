#include <iostream>
#include <array>
#include <cstdint>
#include <vector>
#include <utility>
#include <random>
#include <MiniFB.h>

constexpr size_t GRID_SIZE = 512;
constexpr float MAX_NUTRIENT = 1.0f;
constexpr float MIN_NUTRIENT = 0.0f;

constexpr uint32_t WIDTH  = GRID_SIZE;
constexpr uint32_t HEIGHT = GRID_SIZE;

enum class Neighbor
{
    TOP,   
    LEFT,
    BOTTOM,
    RIGHT, 
};

enum class CellState
{
    Empty,      // no cell
    Alive,      // healthy, dividing
    Quiescent,  // alive but starving
    Necrotic,   // dead tissue
};

class CellularAutomata
{
private:
    const float diffusionSpeed;
    const float deathThreshold;
    const float divideThreshold;
    const float divideCost;

    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> cellGrid;
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> nutrientGrid;
    std::vector<uint32_t> framebuffer = std::vector<uint32_t>(WIDTH * HEIGHT, 0xff000000);
public:
    CellularAutomata(
        std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cellGrid,
        std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrientGrid,
        float diffusionSpeed,
        float deathThreshold,
        float divideThreshold,
        float divideCost
    );
    void diffuse_nutrients();
    void update_cells();
    void render();
};

CellularAutomata::CellularAutomata(
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cellGrid,
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrientGrid,
    float diffusionSpeed,
    float deathThreshold,
    float divideThreshold,
    float divideCost
) : diffusionSpeed(diffusionSpeed),
    deathThreshold(deathThreshold),
    divideThreshold(divideThreshold),
    divideCost(divideCost)
{
    this->cellGrid = cellGrid;
    this->nutrientGrid = nutrientGrid;
}

void CellularAutomata::diffuse_nutrients()
{
    // Init by copying the old grid
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> newNutrientGrid;
    std::copy(nutrientGrid.begin(), nutrientGrid.end(), newNutrientGrid.begin());

    // Iterate only over inner cells
    for (int i = 1; i < GRID_SIZE - 1; i++)
    {
        for (int j = 1; j < GRID_SIZE - 1; j++)
        {
            float nutrientAvg = 0.0f;
            // TOP NEIGHBOR
            nutrientAvg += nutrientGrid[i-1][j];
            // LEFT NEIGHBOR
            nutrientAvg += nutrientGrid[i][j-1];
            // BOTTOM NEIGHBOR
            nutrientAvg += nutrientGrid[i+1][j];
            // RIGHT NEIGHBOR
            nutrientAvg += nutrientGrid[i][j+1];
            nutrientAvg /= 4.0f;

            newNutrientGrid[i][j] = ((nutrientAvg - nutrientGrid[i][j]) * diffusionSpeed) + nutrientGrid[i][j];

            if (newNutrientGrid[i][j] > 1.0f) newNutrientGrid[i][j] = 1.0f;
            if (newNutrientGrid[i][j] < 0.0f) newNutrientGrid[i][j] = 0.0f; 
        }    
    }

    // Refill with nutrients

    // Top and bottom rows
    for (int j = 0; j < GRID_SIZE; ++j)
    {
        newNutrientGrid[0][j] = MAX_NUTRIENT;
        newNutrientGrid[GRID_SIZE - 1][j] = MAX_NUTRIENT;
    }

    // Left and right columns
    for (int i = 0; i < GRID_SIZE; ++i)
    {
        newNutrientGrid[i][0] = MAX_NUTRIENT;
        newNutrientGrid[i][GRID_SIZE - 1] = MAX_NUTRIENT;
    }

    std::swap(nutrientGrid, newNutrientGrid);
}

void CellularAutomata::update_cells()
{
    //TODO: Dividere in due funzioni UPDATE_STATE e MITOSIS

    // Init by copying the old grid
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> newCellGrid;
    std::copy(cellGrid.begin(), cellGrid.end(), newCellGrid.begin());
    std::vector<std::pair<int, int>> aliveCells;

    // Updating states
    // Iterate only over inner cells
    for (int i = 1; i < GRID_SIZE - 1; i++)
    {
        for (int j = 1; j < GRID_SIZE - 1; j++)
        {
            switch (cellGrid[i][j])
            {
                case CellState::Empty: 
                    newCellGrid[i][j] = CellState::Empty; //TODO: Ottimizzabile 
                    break;
                case CellState::Necrotic:
                    newCellGrid[i][j] = CellState::Necrotic; //TODO: Ottimizzabile
                    break;
                case CellState::Alive: 
                    if (nutrientGrid[i][j] >= divideThreshold)
                    {
                        newCellGrid[i][j] = CellState::Alive;
                        aliveCells.push_back({i,j});
                    }
                    else
                        newCellGrid[i][j] = CellState::Quiescent;
                    break;
                case CellState::Quiescent: 
                    if (nutrientGrid[i][j] >= divideThreshold)
                    {
                        newCellGrid[i][j] = CellState::Alive;
                        aliveCells.push_back({i,j});
                    }
                    else if (nutrientGrid[i][j] < deathThreshold)
                    {
                        newCellGrid[i][j] = CellState::Necrotic;
                    }
                    else
                        newCellGrid[i][j] = CellState::Quiescent;
                    break;
            }
        }    
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    // Mitosis
    // Iterate only over inner cells
    for (auto& cell : aliveCells)
    {
        
        std::vector<Neighbor> availableNeighbors;
        // TOP NEIGHBOR
        if (newCellGrid[cell.first - 1][cell.second] == CellState::Empty)
        {
            availableNeighbors.push_back(Neighbor::TOP);
        }
        // LEFT NEIGHBOR
        if (newCellGrid[cell.first][cell.second - 1] == CellState::Empty)
        {
            availableNeighbors.push_back(Neighbor::LEFT);
        }
        // BOTTOM NEIGHBOR
        if (newCellGrid[cell.first + 1][cell.second] == CellState::Empty)
        {
            availableNeighbors.push_back(Neighbor::BOTTOM);
        }
        // RIGHT NEIGHBOR
        if (newCellGrid[cell.first][cell.second + 1] == CellState::Empty)
        {
            availableNeighbors.push_back(Neighbor::RIGHT);
        }

        if (availableNeighbors.size() != 0)
        {
            // Distribution over valid indices
            std::uniform_int_distribution<> dist(0, availableNeighbors.size() - 1);

            Neighbor finalPos = availableNeighbors[dist(gen)];
            switch (finalPos)
            {
                case Neighbor::TOP: 
                    newCellGrid[cell.first-1][cell.second] = CellState::Alive;
                    break;
                case Neighbor::LEFT:
                    newCellGrid[cell.first][cell.second-1] = CellState::Alive;
                    break;
                case Neighbor::BOTTOM:
                    newCellGrid[cell.first+1][cell.second] = CellState::Alive;
                    break;
                case Neighbor::RIGHT: 
                    newCellGrid[cell.first][cell.second+1] = CellState::Alive;
                    break;
            }
            if ((nutrientGrid[cell.first][cell.second] -= divideCost) < MIN_NUTRIENT) nutrientGrid[cell.first][cell.second] = MIN_NUTRIENT;
        }
    }

    std::swap(cellGrid, newCellGrid);
}

void CellularAutomata::render()
{
    static mfb_window *window = nullptr;

    if (!window)
    {
        window = mfb_open_ex("Cellular Automata",
                             WIDTH, HEIGHT,
                             WF_RESIZABLE);
        if (!window)
            return;
    }

    for (int y = 0; y < GRID_SIZE; ++y)
    {
        for (int x = 0; x < GRID_SIZE; ++x)
        {
            uint32_t color = 0xff000000; // default: Empty (black)

            switch (cellGrid[y][x])
            {
                case CellState::Empty:
                    color = 0xff000000; // black
                    break;
                case CellState::Alive:
                    color = 0xff00ff00; // green
                    break;
                case CellState::Quiescent:
                    color = 0xffffff00; // yellow
                    break;
                case CellState::Necrotic:
                    color = 0xff0000ff; // blue
                    break;
            }

            framebuffer[y * WIDTH + x] = color;
        }
    }

    if (mfb_update_ex(window, framebuffer.data(), WIDTH, HEIGHT) < 0)
    {
        mfb_close(window);
        window = nullptr;
        std::exit(0);
    }
}

class SimulationBuilder
{
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cellGrid;
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrientGrid;
    float diffusionSpeed = 0.06f;
    float initNutrient = 0.65f;
    float deathThreshold = 0.20f;
    float divideThreshold = 0.60f;
    float divideCost = 0.12f;

    // Error check
    bool hasCellGrid = false;
    bool hasNutrientGrid = false;
public:
    // Constructor
    SimulationBuilder(
        std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cellGrid,
        std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrientGrid
    );

    // Options
    SimulationBuilder& setDiffusionSpeed(float diffusionSpeed);    
    SimulationBuilder& setInitNutrient(float initNutrient);
    SimulationBuilder& setDeathThreshold(float deathThreshold);
    SimulationBuilder& setDivideThreshold(float divideThreshold);
    SimulationBuilder& setDivisionCost(float divideCost);

    // Builder
    CellularAutomata build() const;
};

SimulationBuilder::SimulationBuilder(
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>& cellGrid,
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE>& nutrientGrid
) : cellGrid(cellGrid),
    nutrientGrid(nutrientGrid)
{}

SimulationBuilder& SimulationBuilder::setDiffusionSpeed(float diffusionSpeed)  
{
    this->diffusionSpeed = diffusionSpeed;
    return *this;
}

SimulationBuilder& SimulationBuilder::setInitNutrient(float initNutrient)
{
    this->initNutrient = initNutrient;
    return *this;
}

SimulationBuilder& SimulationBuilder::setDeathThreshold(float deathThreshold)
{
    this->deathThreshold = deathThreshold;
    return *this;
}

SimulationBuilder& SimulationBuilder::setDivideThreshold(float divideThreshold)
{
    this->divideThreshold = divideThreshold;
    return *this;
}

SimulationBuilder& SimulationBuilder::setDivisionCost(float divideCost)
{
    this->divideCost = divideCost;
    return *this;
}

CellularAutomata SimulationBuilder::build() const
{
    return CellularAutomata(
        cellGrid,
        nutrientGrid,
        diffusionSpeed,
        deathThreshold,
        divideThreshold,
        divideCost
    );
}

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
        sim.diffuse_nutrients();
        sim.update_cells();
        sim.render();
    }
}