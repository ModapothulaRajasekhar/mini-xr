“ospf-architecture.md opened”

Perfect. Now you’re doing this exactly like a real NOS team.

Below is the full, production-grade content for
docs/ospf-architecture.md.

👉 Copy–paste everything in one go, save, and exit.

OSPF Architecture — mini-xr
1. Purpose of ospfd

ospfd is the OSPF control-plane daemon responsible for:

Maintaining OSPF neighbor relationships

Building and maintaining the Link-State Database (LSDB)

Running the SPF (Dijkstra) algorithm

Producing IP routes from LSAs

Publishing route events to the RIB (ribd) via Redis Pub/Sub

ospfd does NOT:

Program kernel routes

Manage interfaces directly

Parse CLI configuration

This strict separation mirrors IOS-XR / Junos architecture.

2. Position in mini-xr Architecture

           +-----------+
           | cfgmgrd   |
           | (config)  |
           +-----------+
                 |
                 v
+-----------+  +----------------+
| ifmgrd    |->|     ospfd      |
| (links)   |  |                |
+-----------+  |  - Neighbor FSM|
               |  - LSDB        |
               |  - SPF         |
               +----------------+
                         |
                         v
              Redis Pub/Sub (proto.route.add / del)
                         |
                         v
                      +------+
                      | ribd |
                      +------+

3. Internal Modules of ospfd

ospfd is internally decomposed into well-defined modules:

ospfd
 ├── Neighbor Manager
 │     - Hello processing
 │     - State transitions
 │
 ├── LSDB Manager
 │     - LSA install/remove
 │     - Aging & refresh
 │
 ├── SPF Engine
 │     - Graph construction
 │     - Shortest path calc
 │
 └── Route Publisher
       - Convert SPF result to routes
       - Emit proto.route.add/del

Each module can be developed and tested independently.

4. OSPF Data Flow (End-to-End)
Step 1: Interface & Config Input

ifmgrd signals interface up/down

cfgmgrd provides OSPF parameters (area, timers)

Step 2: Neighbor Formation

ospfd creates neighbor entries

Hello/Dead timers drive state transitions

Step 3: LSDB Synchronization

LSAs are generated or received

LSDB converges across neighbors

Step 4: SPF Calculation

LSDB is converted into a graph

Dijkstra algorithm computes shortest paths

Step 5: Route Publication

Best paths converted to routes

Routes published to Redis

Step 6: RIB Arbitration

ribd selects best route across protocols

Kernel programming (future phase)

5. Redis Usage Model
State Storage

LSDB snapshots

Neighbor states (optional)

Debug visibility

Event Channels
Channel	Purpose
proto.route.add	Advertise new routes
proto.route.del	Withdraw routes
proto.ospf.event	Debug / trace

Redis acts as the control-plane message bus, not a database of truth.

6. Failure Handling & Restart Behavior

ospfd is designed to:

Restart independently

Rebuild LSDB from config + neighbors

Re-run SPF deterministically

Re-publish routes safely

This supports hitless restarts — a core IOS-XR principle.

7. Why This Architecture Scales

No global locks across daemons

No synchronous IPC dependencies

Stateless protocol engines

Clear ownership of responsibilities

This design supports:

Large topologies

Fast convergence

Independent protocol evolution

8. Non-Goals (Explicitly Out of Scope)

Packet I/O

Real socket programming

Kernel FIB programming

Full RFC-compliant OSPF

mini-xr models engineering architecture, not wire-protocol emulation.

9. Phase Mapping
Phase	Status
Neighbor FSM	Phase 3.2
LSDB	Phase 3.3
SPF	Phase 3.4
RIB integration	Phase 3.5
10. Key Takeaway

ospfd is not a protocol simulator.
It is a control-plane decision engine, designed the same way real routing OS daemons are written.


