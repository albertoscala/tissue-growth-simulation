#include <iostream>
#include <array>
#include <cstdint>
#include <vector>
#include <utility>
#include <random>
#include <MiniFB.h>

constexpr size_t GRID_SIZE = 512;
constexpr float DIFFUSION_SPEED = 0.06f;
constexpr float MAX_NUTRIENT = 1.0f;
constexpr float INIT_NUTRIENT = 0.7f;
constexpr float MIN_NUTRIENT = 0.0f;
constexpr float T_DIVIDE = 0.6f;
constexpr float T_DEATH  = 0.15f;
constexpr float DIVISION_COST = 0.10f;
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

std::array<std::array<float, GRID_SIZE>, GRID_SIZE> initNutrientGrid()
{
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> nutrients{{MAX_NUTRIENT}};

    for (int i = 1; i < GRID_SIZE - 1; i++)
    {
        for (int j = 1; j < GRID_SIZE - 1; j++)
        {
            nutrients[i][j] = INIT_NUTRIENT;
        }   
    }

    return nutrients;
}

class CellularAutomataSimulation
{
private:
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> cellGrid{{CellState::Empty}};
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> nutrientGrid = initNutrientGrid();
    std::vector<uint32_t> framebuffer = std::vector<uint32_t>(WIDTH * HEIGHT, 0xff000000);
public:
    CellularAutomataSimulation(std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> cellGrid);
    void diffuse_nutrients();
    void update_cells();
    void render();
};

CellularAutomataSimulation::CellularAutomataSimulation(std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> cellGrid)
{
    this->cellGrid = cellGrid;
}

void CellularAutomataSimulation::diffuse_nutrients()
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

            newNutrientGrid[i][j] = ((nutrientAvg - nutrientGrid[i][j]) * DIFFUSION_SPEED) + nutrientGrid[i][j];

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

void CellularAutomataSimulation::update_cells()
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
                    if (nutrientGrid[i][j] >= T_DIVIDE)
                    {
                        newCellGrid[i][j] = CellState::Alive;
                        aliveCells.push_back({i,j});
                    }
                    else
                        newCellGrid[i][j] = CellState::Quiescent;
                    break;
                case CellState::Quiescent: 
                    if (nutrientGrid[i][j] >= T_DIVIDE)
                    {
                        newCellGrid[i][j] = CellState::Alive;
                        aliveCells.push_back({i,j});
                    }
                    else if (nutrientGrid[i][j] < T_DEATH)
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
            if ((nutrientGrid[cell.first][cell.second] -= DIVISION_COST) < MIN_NUTRIENT) nutrientGrid[cell.first][cell.second] = MIN_NUTRIENT;
        }
    }

    std::swap(cellGrid, newCellGrid);
}

void CellularAutomataSimulation::render()
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

inline int clamp(int v, int lo, int hi)
{
    return std::max(lo, std::min(v, hi));
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
make_empty_grid()
{
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> grid;

    for (auto &row : grid)
        row.fill(CellState::Empty);

    return grid;
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
generate_small_random_cluster(std::size_t count = 100)
{
    auto grid = make_empty_grid();

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(20, GRID_SIZE - 21);

    for (std::size_t k = 0; k < count; ++k)
    {
        int i = dist(gen);
        int j = dist(gen);
        grid[i][j] = CellState::Alive;
    }

    return grid;
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
generate_sparse_noise(float probability = 0.001f)
{
    auto grid = make_empty_grid();

    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 1; i < GRID_SIZE - 1; ++i)
    {
        for (int j = 1; j < GRID_SIZE - 1; ++j)
        {
            if (dist(gen) < probability)
                grid[i][j] = CellState::Alive;
        }
    }

    return grid;
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
generate_central_disk(int radius = 10)
{
    auto grid = make_empty_grid();
    int cx = GRID_SIZE / 2;
    int cy = GRID_SIZE / 2;

    int i0 = clamp(cx - radius, 1, GRID_SIZE - 2);
    int i1 = clamp(cx + radius, 1, GRID_SIZE - 2);
    int j0 = clamp(cy - radius, 1, GRID_SIZE - 2);
    int j1 = clamp(cy + radius, 1, GRID_SIZE - 2);

    for (int i = i0; i <= i1; ++i)
    {
        for (int j = j0; j <= j1; ++j)
        {
            int dx = i - cx;
            int dy = j - cy;
            if (dx * dx + dy * dy <= radius * radius)
                grid[i][j] = CellState::Alive;
        }
    }

    return grid;
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
generate_multiple_clusters(int clusters = 5, int radius = 6)
{
    auto grid = make_empty_grid();

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(50, GRID_SIZE - 51);

    for (int c = 0; c < clusters; ++c)
    {
        int cx = dist(gen);
        int cy = dist(gen);

        int i0 = clamp(cx - radius, 1, GRID_SIZE - 2);
        int i1 = clamp(cx + radius, 1, GRID_SIZE - 2);
        int j0 = clamp(cy - radius, 1, GRID_SIZE - 2);
        int j1 = clamp(cy + radius, 1, GRID_SIZE - 2);

        for (int i = i0; i <= i1; ++i)
        {
            for (int j = j0; j <= j1; ++j)
            {
                int dx = i - cx;
                int dy = j - cy;
                if (dx * dx + dy * dy <= radius * radius)
                    grid[i][j] = CellState::Alive;
            }
        }
    }

    return grid;
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
generate_ring(int inner_r = 10, int outer_r = 15)
{
    auto grid = make_empty_grid();
    int cx = GRID_SIZE / 2;
    int cy = GRID_SIZE / 2;

    int i0 = clamp(cx - outer_r, 1, GRID_SIZE - 2);
    int i1 = clamp(cx + outer_r, 1, GRID_SIZE - 2);
    int j0 = clamp(cy - outer_r, 1, GRID_SIZE - 2);
    int j1 = clamp(cy + outer_r, 1, GRID_SIZE - 2);

    for (int i = i0; i <= i1; ++i)
    {
        for (int j = j0; j <= j1; ++j)
        {
            int d2 = (i - cx) * (i - cx) + (j - cy) * (j - cy);
            if (d2 >= inner_r * inner_r && d2 <= outer_r * outer_r)
                grid[i][j] = CellState::Alive;
        }
    }

    return grid;
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
generate_vertical_stripe(int width = 5)
{
    auto grid = make_empty_grid();

    int x0 = GRID_SIZE / 4;
    int x1 = clamp(x0 + width, 1, GRID_SIZE - 2);

    for (int x = x0; x < x1; ++x)
        for (int y = 1; y < GRID_SIZE - 1; ++y)
            grid[y][x] = CellState::Alive;

    return grid;
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
generate_cross(int arm = 15)
{
    auto grid = make_empty_grid();
    int c = GRID_SIZE / 2;

    int i0 = clamp(c - arm, 1, GRID_SIZE - 2);
    int i1 = clamp(c + arm, 1, GRID_SIZE - 2);

    for (int i = i0; i <= i1; ++i)
    {
        grid[c][i] = CellState::Alive;
        grid[i][c] = CellState::Alive;
    }

    return grid;
}

std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE>
generate_random_blob(int radius = 12, float jitter = 0.3f)
{
    auto grid = make_empty_grid();
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> noise(-jitter, jitter);

    int cx = GRID_SIZE / 2;
    int cy = GRID_SIZE / 2;

    for (int i = cx - radius; i <= cx + radius; ++i)
    {
        for (int j = cy - radius; j <= cy + radius; ++j)
        {
            float dx = i - cx + noise(gen);
            float dy = j - cy + noise(gen);
            if (dx*dx + dy*dy <= radius*radius)
                grid[i][j] = CellState::Alive;
        }
    }
    return grid;
}

int main()
{
    // Generate initial conditions
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> sim1 = generate_small_random_cluster(100);
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> sim2 = generate_central_disk(10);
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> sim3 = generate_multiple_clusters(5, 6);
    //std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> sim4 = generate_ring(10, 15);
    //std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> sim5 = generate_sparse_noise(0.001f);
    //std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> sim6 = generate_vertical_stripe(5);
    //std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> sim7 = generate_cross(15);
    //std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> sim8 = generate_random_blob(12, 0.3f);

    // Choose which one to run
    CellularAutomataSimulation sim(sim3);
    
    for (int epoch=0;;epoch++)
    {
        std::cout << "Epoch: " << epoch << std::endl; 
        sim.diffuse_nutrients();
        sim.update_cells();
        sim.render();
    }
}