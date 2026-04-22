#ifndef BINARY_IMAGE_CC_SOLVER_H
#define BINARY_IMAGE_CC_SOLVER_H

#include <vector>

// ─────────────────────────────────────────────────────────────
//  BinaryImageCCSolver
//  Two-Pass Connected Components Labeling + Morphological Ops
// ─────────────────────────────────────────────────────────────

struct LabeledImage {
    int rows = 0;
    int cols = 0;
    // label[r][c] == 0 → background, > 0 → component ID
    std::vector<std::vector<int>> label;
    int numComponents = 0;
    // pixels per component (index = componentId - 1)
    std::vector<int> componentSizes;
};

enum class StructuringElement {
    Cross,   // 4-connectivity (+ shape)
    Square   // 8-connectivity (3x3 box)
};

enum class Connectivity {
    Four,
    Eight
};

// Intermediate step info for animation
struct CCAnimStep {
    int r, c;
    int label;        // label assigned at this step
    int pass;         // 1 = first pass, 2 = second pass (relabeling)
};

class BinaryImageCCSolver {
public:
    // Two-Pass Labeling. Returns labeled image + step sequence for animation.
    static LabeledImage solve(
        const std::vector<std::vector<int>>& binaryGrid,
        Connectivity conn,
        std::vector<CCAnimStep>* animSteps = nullptr
    );

    // Morphological operations. 1 = foreground, 0 = background.
    static std::vector<std::vector<int>> erode(
        const std::vector<std::vector<int>>& grid,
        StructuringElement se = StructuringElement::Cross
    );

    static std::vector<std::vector<int>> dilate(
        const std::vector<std::vector<int>>& grid,
        StructuringElement se = StructuringElement::Cross
    );

    static std::vector<std::vector<int>> open(
        const std::vector<std::vector<int>>& grid,
        StructuringElement se = StructuringElement::Cross
    );

    static std::vector<std::vector<int>> close(
        const std::vector<std::vector<int>>& grid,
        StructuringElement se = StructuringElement::Cross
    );

private:
    // Union-Find helpers
    static int find(std::vector<int>& parent, int x);
    static void unite(std::vector<int>& parent, int a, int b);

    static std::vector<std::pair<int,int>> getNeighborOffsets(Connectivity conn);
    static std::vector<std::pair<int,int>> getSeOffsets(StructuringElement se);
};

#endif // BINARY_IMAGE_CC_SOLVER_H
