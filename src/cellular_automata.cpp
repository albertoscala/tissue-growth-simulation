#include <cellular_automata.hpp>


// Cellular Automata

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
    // Set the grids
    this->cellGrid = cellGrid;
    this->nutrientGrid = nutrientGrid;

    // Set the RNG
    gen = std::mt19937(rd());
}

//TODO: Rendere modulare la parte dove si refillano i nutrienti
void CellularAutomata::diffuseNutrients()
{
    // Create the new state od the grid
    std::array<std::array<float, GRID_SIZE>, GRID_SIZE> newNutrientGrid;
    
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

    this->nutrientGrid = std::move(newNutrientGrid);
}

void CellularAutomata::updateCells()
{
    // Create cell grid for the new state
    std::array<std::array<CellState, GRID_SIZE>, GRID_SIZE> newCellGrid;

    // Clear alive cells
    aliveCells.clear();

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

    this->cellGrid = std::move(newCellGrid);
}

void CellularAutomata::divideCells()
{
    // Mitosis
    // Iterate only over inner cells
    for (auto& cell : aliveCells)
    {
        std::array<Neighbor, 4> availableNeighbors;
        int neighborCount = 0;

        // TOP NEIGHBOR
        if (cellGrid[cell.first - 1][cell.second] == CellState::Empty)
        {
            availableNeighbors[neighborCount] = Neighbor::TOP;
            neighborCount++;
        }
        // LEFT NEIGHBOR
        if (cellGrid[cell.first][cell.second - 1] == CellState::Empty)
        {
            availableNeighbors[neighborCount] = Neighbor::LEFT;
            neighborCount++;
        }
        // BOTTOM NEIGHBOR
        if (cellGrid[cell.first + 1][cell.second] == CellState::Empty)
        {
            availableNeighbors[neighborCount] = Neighbor::BOTTOM;
            neighborCount++;
        }
        // RIGHT NEIGHBOR
        if (cellGrid[cell.first][cell.second + 1] == CellState::Empty)
        {
            availableNeighbors[neighborCount] = Neighbor::RIGHT;
            neighborCount++;
        }

        if (neighborCount == 0)
            continue;

        // Distribution over valid indices
        std::uniform_int_distribution<> dist(0, neighborCount - 1);

        Neighbor finalPos = availableNeighbors[dist(gen)];
        
        switch (finalPos)
        {
            case Neighbor::TOP: 
                cellGrid[cell.first-1][cell.second] = CellState::Alive;
                break;
            case Neighbor::LEFT:
                cellGrid[cell.first][cell.second-1] = CellState::Alive;
                break;
            case Neighbor::BOTTOM:
                cellGrid[cell.first+1][cell.second] = CellState::Alive;
                break;
            case Neighbor::RIGHT: 
                cellGrid[cell.first][cell.second+1] = CellState::Alive;
                break;
        }

        if ((nutrientGrid[cell.first][cell.second] -= divideCost) < MIN_NUTRIENT) nutrientGrid[cell.first][cell.second] = MIN_NUTRIENT;
    }
}

void CellularAutomata::step()
{
    diffuseNutrients();
    updateCells();
    divideCells();
    render();
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

// Simulation Builder

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