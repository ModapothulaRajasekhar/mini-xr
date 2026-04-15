OSPF SPF Engine — mini-xr
1. What Is SPF?

SPF = Shortest Path First

It is:

Dijkstra’s algorithm applied to the LSDB graph.

Input:

LSDB (graph of routers and links)

Output:

Shortest path tree rooted at local router

List of reachable prefixes

Route candidates for RIB

2. SPF Position in Control Plane
LSA change
    ↓
LSDB updated
    ↓
SPF scheduled
    ↓
Shortest Path Tree built
    ↓
Routes generated
    ↓
Publish to RIB (proto.route.add)


SPF is NOT triggered directly by packets.
It is triggered by LSDB state change.

3. Graph Model

We convert LSDB into a graph:

Router A ----10---- Router B
    |                    |
    5                    20
    |                    |
Router C ------------- Router D
              15


Graph representation:

std::map<std::string,
         std::vector<std::pair<std::string,int>>> graph;


Where:

graph["A"] = { {"B",10}, {"C",5} }

4. Dijkstra Algorithm (Concept)

Start from self Router ID

Initialize distance = ∞ for all nodes

Set self distance = 0

Pick lowest-cost unexplored node

Relax neighbors

Repeat until all nodes processed

5. Data Structures
struct SPFNode {
    std::string router_id;
    int distance;
    std::string parent;
};


We maintain:

std::map<std::string, SPFNode> spf_table;
std::set<std::string> visited;

6. SPF Execution Model
Step 1 – Build Graph

From LSDB router LSAs:

for each RouterLSA:
    for each link:
        add edge to graph

Step 2 – Run Dijkstra

Pseudo-code:

initialize distances
while unvisited nodes exist:
    select node with lowest distance
    for each neighbor:
        if new distance < existing:
            update distance
            update parent

7. Generating Routes

After SPF completes:

For each reachable router:

Extract next-hop

Extract metric

Generate route candidate

Example:

publish("proto.route.add",
        prefix="10.1.0.0/24",
        protocol="ospf",
        metric=cost);


ribd will handle arbitration.

8. SPF Scheduling Model (XR-style)

SPF must NOT run immediately on every LSA.

Instead:

LSDB change → mark dirty

Wait X ms (SPF delay)

Run SPF once

Clear dirty flag

We will implement:

constexpr int SPF_DELAY_MS = 200;


Same philosophy as convergence timer in ribd.

9. Incremental vs Full SPF

Real IOS-XR:

Supports incremental SPF

Partial recalculation

mini-xr Phase 3:

Full SPF each time

Simpler

Deterministic

Incremental optimization later.

10. Failure Handling

When router removed:

LSDB entry deleted

Graph rebuilt

SPF recomputed

Routes withdrawn via RIB

This matches real OSPF behavior.

11. Multi-Area (Future)

Phase 3 scope:

Single Area 0

No ABR

No Summary LSAs

Future extension:

Per-area LSDB

Inter-area routes

Type 3 handling

12. Complexity

Dijkstra complexity:

O(N log N)


For lab scale → trivial.

In real routers:

Optimized heaps

Incremental recalculation

SPF throttling

13. Implementation Plan
Step	File
Build LSDB class	ospfd/lsdb.cpp
Build SPF engine	ospfd/spf.cpp
Integrate scheduler	ospfd/main.cpp
Publish routes	via pubsub
