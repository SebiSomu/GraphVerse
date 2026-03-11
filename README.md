# GraphVerse: Advanced Graph Algorithm Platform

GraphVerse is a high-performance algorithmic suite built with **C++**, **Visual Studio 2026**, and **Qt**. It provides an interactive, detailed environment for visualizing graph theories and solving complex real-world optimization problems through a premium dark-themed interface.

---

## ⚡ Algorithm Visualizers (Detailed Implementation)

Each visualizer provides a step-by-step trace of the logic, with real-time performance metrics and state tracking.

### 1. Shortest Path Algorithms
The engine implements these through a weighted adjacency list structure.
- **Dijkstra**: Uses a **Min-Priority Queue** (`std::priority_queue` with `std::greater`) to always expand the node with the smallest cumulative distance. Implementation includes **edge relaxation** where the distance to a neighbor is updated if a shorter path is found.
- **A***: Enhances Dijkstra with a **heuristic function**. The heuristic is calculated as the Euclidean distance to the target node, scaled by the global minimum weight/distance ratio found in the graph to ensure it remains **admissible** (never overestimates the cost).
- **Bellman-Ford**: Performs **N-1 iterations** of edge relaxations over the entire edge set. In this implementation, it is used for pathfinding on graphs (it terminates early if no relaxations occur), ensuring correct handling of any arbitrary edge order.
- **Floyd-Warshall**: An all-pairs shortest path algorithm using a **Dynamic Programming** approach with a 2D distance matrix and a successor matrix for path reconstruction. Complexity is O(V³).

![Shortest Path Visualization](representative-images/graphverse-img7.png)

### 2. Spanning Trees (MST)
MST visualizers show the process of connecting all nodes with minimum total edge weight.
- **Kruskal**: Implemented by sorting the global edge list by weight. It uses a **Disjoint Set Union (DSU)** with **Path Compression** and **Rank Optimization** to detect cycles efficiently.
- **Prim**: Starts from a root node and uses a **Priority Queue** to track all edges connecting the current MST component to unvisited nodes, selecting the minimum cost edge in each step.
- **Boruvka**: A multi-phase algorithm that finds the cheapest edge for **each connected component** simultaneously in each step. Components are merged using a DSU structure until only one component remains.

![MST Visualization](representative-images/graphverse-img6.png)

### 3. Network Flow & Cycle Logic
- **Edmonds-Karp (Max Flow)**: A specific implementation of Ford-Fulkerson that uses **BFS** to find the shortest augmenting path (by number of edges) in the residual graph. This ensures polynomial time complexity and handles integer capacities accurately.
- **Min-Cut**: After the Max-Flow converges, the Min-Cut is identified by finding all nodes reachable from the Source in the final **residual graph**. The cut consists of edges in the original graph that connect a reachable node to an unreachable one.
- **Negative Cycle Elimination**: A specialized module that identifies negative cycles in the residual graph using a parent-tracking Bellman-Ford variant and saturates them with capacity (**Cycle Canceling**) to optimize the cost-flow balance.

| Construction | Augmented Path | Final Max Flow |
|--------------|----------------|----------------|
| ![Flow 1](representative-images/graphverse-img8.png) | ![Flow 2](representative-images/graphverse-img9.png) | ![Flow 3](representative-images/graphverse-img10.png) |

### 4. Connectivity & Components
- **Strongly Connected Components (SCC)**: Implements **Kosaraju’s Algorithm**. It involves two DFS passes: one to determine the finish order on the original graph, and a second pass on the **transpose graph** (all edges reversed) to extract the components.
- **Condensed Graph View**: Automatically collapses each SCC into a single "super-node", revealing the underlying **Directed Acyclic Graph (DAG)** structure of the network.

![Components Visualization](representative-images/graphverse-img5.png)

---

## 🌐 Real-World Applications (In-Depth)

GraphVerse translates abstract math into concrete logic for modern systems.

### 👥 Friend Suggestion System
- **Algorithm**: Breadth-First Search (BFS).
- **Implementation**: The system traverses the social graph to find **2nd-degree neighbors** (friends of friends).
- **Ranking Logic**: Suggestions are not random; they are ranked by **Connection Density**. The algorithm calculates the number of mutual friends between the root user and the candidate. Candidates with the highest mutual friend count are suggested first, mimicking real-world social "closeness".

![Friend Suggestions](representative-images/graphverse-img11.png)

### 🌍 Translation Network
- **Algorithm**: Kruskal's MST + DFS.
- **Implementation**: Uses a dataset (`translations.json`) where languages are nodes and translation "difficulties" (based on language family distance and resource availability) are edge weights.
- **Logic**: It first identifies the **Minimum Spanning Tree** of the entire language universe to find the most efficient way to translate between all languages globally. Then, using a **Directed DFS** from a user-selected root (e.g., English), it generates a translation hierarchy showing how to reach any language with minimal global effort.

![Translation Network](representative-images/graphverse-img12.png)

### 🛒 Supermarket Navigator
- **Algorithm**: Dijkstra's Shortest Path on a Constrained Geometry.
- **Implementation**: Sections (nodes) are placed on a stabilized floor plan.
- **Constraints**: Includes a **node-clamping system** that restricts section movement to stay within the "store walls" defined by the floor plan boundaries. Edges are dynamically recalculated based on a proximity threshold, and the shortest path is found between custom points like "Entrance" and "Cashier".

![Supermarket Navigator](representative-images/graphverse-img13.png)

### 🚖 RideMatch (Passenger ↔ Driver Matching)
- **Problem**: Maximum Bipartite Matching.
- **Implementation via Max Flow**: The system creates a virtual flow network where a super-source connects to all passengers and all drivers connect to a super-sink.
- **Matching Logic**: Directed edges are created between a passenger and a driver only if their Euclidean distance is within a user-defined **match radius**. Running the Ford-Fulkerson algorithm on this network yields the maximum possible simultaneous matches in the system.

![RideMatch](representative-images/graphverse-img14.png)

---

## 🛠️ Requirements & Build
- **Environment**: Visual Studio 2026.
- **Framework**: Qt 6.x (integrated via Qt VS Tools).
- **Resources**: Uses a custom JSON parser for translation data and `QPainter` for high-performance rendering.

*GraphVerse: Bridging the gap between theoretical graph theory and practical application.*
