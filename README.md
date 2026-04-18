# GraphVerse: Advanced Graph Algorithm Platform

GraphVerse is a high-performance algorithmic suite built with **C++**, **Visual Studio 2026**, and **Qt**. It provides an interactive, detailed environment for visualizing graph theories and solving complex real-world optimization problems through a premium dark-themed interface.

## Technical Architecture (SOLID Design)

GraphVerse is engineered with a strict emphasis on **SOLID principles**, transforming a complex algorithmic suite into a highly maintainable and professional modular system.

### 1. Single Responsibility Principle (SRP)
Each class in GraphVerse has one, and only one, reason to change. 
*   **In Practice:** The **Algorithm Solvers** (e.g., `DijkstraSolver`, `TopologicalSortSolver`) are completely decoupled from the UI. They focus exclusively on processing graph data and returning a sequence of `PathStep` objects. The **Visualizers** (e.g., `VisShortest`, `VisTopologicalSort`) handle only the rendering and user interaction, consuming the solver's output.

### 2. Open/Closed Principle (OCP)
Entities are open for extension but closed for modification.
*   **In Practice:** Adding a new algorithm, such as **Bidirectional Dijkstra**, did not require modifying the existing `Graph` or `ShortestPath` UI structure. We simply implemented a new solver class adhering to the established solver interface. This prevents regression bugs in existing, proven components.

### 3. Liskov Substitution Principle (LSP)
Subtypes must be substitutable for their base types.
*   **In Practice:** The platform utilizes the `IGraphData` abstraction. Whether the underlying implementation is a `DirectedGraph` or an `UndirectedGraph`, all solvers (MST, Flow, Shortest Path) can operate on them seamlessly without knowing the concrete class, as long as the base interface contract is satisfied.

### 4. Interface Segregation Principle (ISP)
Clients should not be forced to depend on methods they do not use.
*   **In Practice:** The monolithic `IGraph` interface was refactored into focused components:
    *   `IGraphData`: For read-only traversal (used by solvers).
    *   `IGraphOperations`: For graph building/editing (used by the UI).
    *   `IGraphMetadata`: For graph-type introspection (used by validators).
    This ensures that a simple pathfinding algorithm doesn't have "knowledge" of node-removal or graph-clearing methods.

### 5. Dependency Inversion Principle (DIP)
High-level modules should not depend on low-level modules; both should depend on abstractions.
*   **In Practice:** The algorithm logic depends entirely on the `IGraphData` interface rather than concrete graph implementations. Similarly, the system uses abstraction layers to pass animation steps, ensuring the data engine remains agnostic of the specific rendering technology (`QPainter`).

## Algorithm Visualizers (Detailed Implementation)

Each visualizer provides a step-by-step trace of the logic, with real-time performance metrics and state tracking.

### 1. Maze Explorer
- **Functionality**: Solves grid-based mazes using BFS and DFS algorithms, visualizing the frontier expansion during the search process. This module demonstrates the fundamental differences between exploration strategies in a controlled grid environment.

![Maze 1](representative-images/graphverse-img4.png)
![Maze 2](representative-images/graphverse-img5.png)

### 2. Shortest Path Algorithms
The engine implements these through a weighted adjacency list structure.
- **Dijkstra**: Uses a **Min-Priority Queue** (`std::priority_queue` with `std::greater`) to always expand the node with the smallest cumulative distance. Implementation includes **edge relaxation** where the distance to a neighbor is updated if a shorter path is found.
- **A***: Enhances Dijkstra with a **heuristic function**. The heuristic is calculated as the Euclidean distance to the target node, scaled by the global minimum weight/distance ratio found in the graph to ensure it remains **admissible**.
- **Bellman-Ford**: Performs **N-1 iterations** of edge relaxations over the entire edge set. In this implementation, it is used for pathfinding on graphs, ensuring correct handling of any arbitrary edge order.
- **Floyd-Warshall**: An all-pairs shortest path algorithm using a **Dynamic Programming** approach with a 2D distance matrix and a successor matrix for path reconstruction.
- **Bidirectional Dijkstra**: Periodically executes two simultaneous searches—one forward from the source and one backward from the target. The process terminates when both frontiers meet, significantly reducing the total search space.

![Shortest Path Visualization](representative-images/Screenshot 2026-04-18 171506.png)

### 3. Spanning Trees (MST)
MST visualizers show the process of connecting all nodes with minimum total edge weight.
- **Kruskal**: Implemented by sorting the global edge list by weight. It uses a **Disjoint Set Union (DSU)** with **Path Compression** and **Rank Optimization** to detect cycles efficiently.
- **Prim**: Starts from a root node and uses a **Priority Queue** to track all edges connecting the current MST component to unvisited nodes.
- **Boruvka**: A multi-phase algorithm that finds the cheapest edge for **each connected component** simultaneously in each step. Components are merged using a DSU structure.

![MST Visualization](representative-images/graphverse-img9.png)

### 4. Network Flow & Cycle Logic
- **Edmonds-Karp (Max Flow)**: A specific implementation of Ford-Fulkerson that uses **BFS** to find the shortest augmenting path in the residual graph.
- **Min-Cut**: Identified by finding all nodes reachable from the Source in the final **residual graph**.
- **Negative Cycle Elimination**: Identifies negative cycles in the residual graph using a parent-tracking variant and saturates them via **Cycle Canceling**.

![Flow 1](representative-images/graphverse-img11.png)
![Flow 2](representative-images/graphverse-img12.png)
![Flow 3](representative-images/graphverse-img13.png)
![Flow 4](representative-images/graphverse-img14.png)
![Flow 5](representative-images/graphverse-img15.png)

### 5. Connectivity & Components
- **Strongly Connected Components (SCC)**: Implements **Kosaraju’s Algorithm** with two DFS passes (original and transpose graphs).
- **Condensed Graph View**: Collapses each SCC into a single "super-node", revealing the underlying **DAG** structure.

![Components 1](representative-images/graphverse-img6.png)
![Components 2](representative-images/graphverse-img7.png)
![Components 3](representative-images/graphverse-img8.png)

### 6. Topological Sorting
- **Algorithm**: Kahn's Algorithm (Iterative In-degree Reduction).
- **Implementation**: Specifically restricted to **Directed Acyclic Graphs (DAGs)**. It manages a queue of nodes with zero in-degree, progressively removing them and updating neighbors until a linear ordering is achieved or a cycle is detected.

![Topological Sort](representative-images/Screenshot 2026-04-18 172604.png)

---

## Real-World Applications (In-Depth)

GraphVerse translates abstract math into concrete logic for modern systems.

### Friend Suggestion System
- **Algorithm**: Breadth-First Search (BFS).
- **Implementation**: Traverses the social graph to find **2nd-degree neighbors**.
- **Ranking**: Suggestions are ranked by **Connection Density** (mutual friend count) between the root user and candidates.

![Friend Suggestions](representative-images/graphverse-img16.png)

### Translation Network
- **Algorithm**: Kruskal's MST + DFS.
- **Implementation**: Uses a `translations.json` dataset where languages are nodes and translation "difficulties" are edge weights.
- **Logic**: Identifies the **Minimum Spanning Tree** to find the most efficient global translation paths, then generates a hierarchy via **Directed DFS**.

![Translation Network](representative-images/graphverse-img17.png)

### Supermarket Navigator
- **Algorithm**: Dijkstra's Shortest Path on a Constrained Geometry.
- **Implementation**: Includes a **node-clamping system** that restricts section movement to stay within the store floor plan boundaries.
- **Logic**: Dynamically recalculates edges based on proximity for real-time pathfinding.

![Supermarket Navigator](representative-images/graphverse-img18.png)

### RideMatch (Passenger Drive Matching)
- **Problem**: Maximum Bipartite Matching solved via **Max Flow**.
- **Logic**: Creates a virtual flow network where edges exist only if Euclidean distance is within the user-defined **match radius**.

![RideMatch](representative-images/graphverse-img19.png)

---

## Theoretical Applications

Classical problems demonstrating complex graph traversals and optimization.

### 1. Flood Fill Algorithm
- **Logic**: Uses a queue-based **BFS** to identify and traverse contiguous regions of the same "color" or state.
- **Visualisation**: Shows the "wave" expansion from the seed point across the grid.

![Flood Fill 1](representative-images/graphverse-img21.png)
![Flood Fill 2](representative-images/graphverse-img22.png)

### 2. Traveling Salesman Problem (TSP) - Romania
- **Algorithms**: Floyd-Warshall + Kruskal's MST + DFS Approximation.
- **Logic**: Finds a near-optimal Hamiltonian circuit for a 15-city map of Romania.
- **Path**: First computes all-pairs shortest paths, generates a complete graph ($K_n$), finds the MST, and then performs a **Preorder DFS** traversal to generate the final circuit.

![TSP Romania](representative-images/graphverse-img23.png)

---

## User Interface & Experience

The platform features a modern, dashboard-driven design:
- **Main Dashboard**: Quick access to all modules with animated node-edge backgrounds.
- **Responsive Controls**: Speed sliders for animations, pause/resume functionality, and live status labels.

![Main Dashboard](representative-images/graphverse-img20.png)
![Visualizers Menu](representative-images/graphverse-img2.png)
![Applications Menu](representative-images/graphverse-img3.png)

---

## Requirements & Build
- **Environment**: Visual Studio 2026.
- **Framework**: Qt 6.x (integrated via Qt VS Tools).
- **Resources**: Uses a custom JSON parser for translation data and `QPainter` for rendering.

*GraphVerse: Bridging the gap between theoretical graph theory and practical application.*
