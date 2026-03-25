#ifndef FLOOD_FILL_SOLVER_H
#define FLOOD_FILL_SOLVER_H

#include <vector>
#include <utility>

class FloodFillSolver {
public:
    struct GridPoint {
        int r, c;
    };

    static std::vector<GridPoint> solve(const std::vector<std::vector<int>>& grid, 
                                       int startR, int startC, 
                                       int targetType, int replacementType);
};

#endif // FLOOD_FILL_SOLVER_H
