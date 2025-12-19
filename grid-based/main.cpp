#include <iostream>
#include <array>
#include <cstdint>
#include <vector>
#include <utility>
#include <random>

constexpr size_t GRID_SIZE = 512;
constexpr float DIFFUSION_SPEED = 0.05f;
constexpr float MAX_NUTRIENT = 1.0f;
constexpr float MIN_NUTRIENT = 0.0f;
constexpr float T_DIVIDE = 0.6f;
constexpr float T_DEATH  = 0.2f;
constexpr float DIVISION_COST = 0.15f;

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

class CellularAutomataSimulation
{
private:
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> cellGrid{CellState::Empty};
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> nutrientGrid{0.0f};
public:
    void diffuse_nutrients();
    void update_cells();
    void render();
};

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
        if (cell.first - 1 > 1 && newCellGrid[cell.first - 1][cell.second] == CellState::Empty)
        {
            availableNeighbors.push_back(Neighbor::TOP);
        }
        // LEFT NEIGHBOR
        if (cell.second - 1 > 1 && newCellGrid[cell.first][cell.second - 1] == CellState::Empty)
        {
            availableNeighbors.push_back(Neighbor::LEFT);
        }
        // BOTTOM NEIGHBOR
        if (cell.first + 1 < GRID_SIZE - 1 && newCellGrid[cell.first + 1][cell.second] == CellState::Empty)
        {
            availableNeighbors.push_back(Neighbor::BOTTOM);
        }
        // RIGHT NEIGHBOR
        if (cell.second + 1 < GRID_SIZE - 1 && newCellGrid[cell.first][cell.second + 1] == CellState::Empty)
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
    
}

int main()
{
    CellularAutomataSimulation sim = CellularAutomataSimulation();

    for (;;)
    {
        sim.diffuse_nutrients();
        sim.update_cells();
        sim.render();
    }
}