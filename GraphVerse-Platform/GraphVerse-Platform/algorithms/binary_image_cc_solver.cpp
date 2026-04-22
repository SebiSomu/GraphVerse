#include "binary_image_cc_solver.h"
#include <algorithm>
#include <numeric>

// ─────────────────────────────────────────────────────────────
//  Union-Find (path compression + union by rank)
// ─────────────────────────────────────────────────────────────

int BinaryImageCCSolver::find(std::vector<int>& parent, int x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]]; // path halving
        x = parent[x];
    }
    return x;
}

void BinaryImageCCSolver::unite(std::vector<int>& parent, int a, int b) {
    int ra = find(parent, a);
    int rb = find(parent, b);
    if (ra != rb) parent[ra] = rb;
}

// ─────────────────────────────────────────────────────────────
//  Neighbor / Structuring-Element offsets
// ─────────────────────────────────────────────────────────────

std::vector<std::pair<int,int>>
BinaryImageCCSolver::getNeighborOffsets(Connectivity conn) {
    if (conn == Connectivity::Four) {
        return {{-1,0},{0,-1}};   // only look back (top & left) in first pass
    } else {
        return {{-1,-1},{-1,0},{-1,1},{0,-1}};
    }
}

std::vector<std::pair<int,int>>
BinaryImageCCSolver::getSeOffsets(StructuringElement se) {
    if (se == StructuringElement::Cross) {
        return {{-1,0},{1,0},{0,-1},{0,1}};
    } else {
        std::vector<std::pair<int,int>> res;
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc)
                if (dr != 0 || dc != 0)
                    res.push_back({dr, dc});
        return res;
    }
}

// ─────────────────────────────────────────────────────────────
//  Two-Pass Connected Components Labeling
// ─────────────────────────────────────────────────────────────

LabeledImage BinaryImageCCSolver::solve(
    const std::vector<std::vector<int>>& binaryGrid,
    Connectivity conn,
    std::vector<CCAnimStep>* animSteps)
{
    if (binaryGrid.empty()) return {};

    int ROWS = static_cast<int>(binaryGrid.size());
    int COLS = static_cast<int>(binaryGrid[0].size());

    // Provisional labels
    std::vector<std::vector<int>> provisional(ROWS, std::vector<int>(COLS, 0));
    std::vector<int> parent;
    parent.push_back(0); // 0 = background
    int nextLabel = 1;

    auto neighborOffsets = getNeighborOffsets(conn);

    // ── FIRST PASS ──────────────────────────────────────────
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (binaryGrid[r][c] == 0) continue; // background

            // Collect labels from already-visited neighbors
            std::vector<int> neighborLabels;
            for (auto [dr, dc] : neighborOffsets) {
                int nr = r + dr, nc = c + dc;
                if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                    if (provisional[nr][nc] > 0)
                        neighborLabels.push_back(provisional[nr][nc]);
                }
            }

            if (neighborLabels.empty()) {
                // New label
                provisional[r][c] = nextLabel++;
                parent.push_back(static_cast<int>(parent.size()));
            } else {
                // Minimum root label
                int minLabel = neighborLabels[0];
                for (int lbl : neighborLabels)
                    minLabel = std::min(minLabel, find(parent, lbl));
                provisional[r][c] = minLabel;
                // Union all neighbor labels
                for (int lbl : neighborLabels)
                    unite(parent, lbl, minLabel);
            }

            if (animSteps) {
                animSteps->push_back({r, c, provisional[r][c], 1});
            }
        }
    }

    // ── Resolve equivalences → compact label mapping ────────
    // Map each root → final sequential label
    std::vector<int> rootToFinal(parent.size(), 0);
    int finalLabel = 0;
    for (int lbl = 1; lbl < static_cast<int>(parent.size()); ++lbl) {
        if (find(parent, lbl) == lbl) {
            rootToFinal[lbl] = ++finalLabel;
        }
    }

    // ── SECOND PASS ─────────────────────────────────────────
    LabeledImage result;
    result.rows = ROWS;
    result.cols = COLS;
    result.label.assign(ROWS, std::vector<int>(COLS, 0));
    result.numComponents = finalLabel;
    result.componentSizes.assign(finalLabel, 0);

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (provisional[r][c] == 0) continue;
            int root = find(parent, provisional[r][c]);
            int fl = rootToFinal[root];
            result.label[r][c] = fl;
            result.componentSizes[fl - 1]++;

            if (animSteps) {
                animSteps->push_back({r, c, fl, 2});
            }
        }
    }

    return result;
}

// ─────────────────────────────────────────────────────────────
//  Morphological Operations
// ─────────────────────────────────────────────────────────────

std::vector<std::vector<int>> BinaryImageCCSolver::erode(
    const std::vector<std::vector<int>>& grid,
    StructuringElement se)
{
    int R = static_cast<int>(grid.size());
    int C = R > 0 ? static_cast<int>(grid[0].size()) : 0;
    auto offsets = getSeOffsets(se);
    std::vector<std::vector<int>> out(R, std::vector<int>(C, 0));

    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) {
            if (grid[r][c] == 0) continue;
            bool keep = true;
            for (auto [dr, dc] : offsets) {
                int nr = r + dr, nc = c + dc;
                if (nr < 0 || nr >= R || nc < 0 || nc >= C || grid[nr][nc] == 0) {
                    keep = false;
                    break;
                }
            }
            out[r][c] = keep ? 1 : 0;
        }
    }
    return out;
}

std::vector<std::vector<int>> BinaryImageCCSolver::dilate(
    const std::vector<std::vector<int>>& grid,
    StructuringElement se)
{
    int R = static_cast<int>(grid.size());
    int C = R > 0 ? static_cast<int>(grid[0].size()) : 0;
    auto offsets = getSeOffsets(se);
    std::vector<std::vector<int>> out(R, std::vector<int>(C, 0));

    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) {
            if (grid[r][c] == 1) {
                out[r][c] = 1;
                for (auto [dr, dc] : offsets) {
                    int nr = r + dr, nc = c + dc;
                    if (nr >= 0 && nr < R && nc >= 0 && nc < C)
                        out[nr][nc] = 1;
                }
            }
        }
    }
    return out;
}

std::vector<std::vector<int>> BinaryImageCCSolver::open(
    const std::vector<std::vector<int>>& grid,
    StructuringElement se)
{
    return dilate(erode(grid, se), se);
}

std::vector<std::vector<int>> BinaryImageCCSolver::close(
    const std::vector<std::vector<int>>& grid,
    StructuringElement se)
{
    return erode(dilate(grid, se), se);
}
