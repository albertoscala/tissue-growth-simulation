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
    // RNG
    std::random_device rd;
    std::mt19937 gen;

    // Simulation parameters
    const float diffusionSpeed;
    const float deathThreshold;
    const float divideThreshold;
    const float divideCost;

    // Grids
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> cellGrid;
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> nutrientGrid;
    std::vector<std::pair<int, int>> aliveCells;
    
    // Framebuffer
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
    void diffuseNutrients();
    void updateCells();
    void divideCells();
    void step();
    void render();
};

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