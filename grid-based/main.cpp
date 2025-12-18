#include <iostream>
#include <array>
#include <cstdint>

constexpr size_t GRID_SIZE = 512;
constexpr float DIFFUSION_SPEED = 0.05f;
constexpr float MAX_NUTRIENT = 1.0f;
constexpr float MIN_NUTRIENT = 0.0f;

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