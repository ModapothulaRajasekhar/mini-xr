OSPF Link State Database (LSDB) — mini-xr
1. What Is the LSDB?

The LSDB is:

A synchronized graph representation of the network topology.

Every OSPF router in the same area must have:

Identical LSDB

Same LSA versions

Same topology view

If LSDB differs → SPF differs → routing loops occur.

2. LSDB in Real Systems (IOS-XR Model)

In IOS-XR:

ospfd owns LSDB

LSDB is area-scoped

LSAs stored in indexed structures

SPF runs on LSDB change

RIB updated only after SPF convergence

mini-xr will follow the same model.

3. LSDB Data Model
Core Entities
Area
 ├── LSA Type 1 (Router-LSA)
 ├── LSA Type 2 (Network-LSA)
 ├── LSA Type 3 (Summary-LSA)
 └── LSA Type 5 (External-LSA)


For Phase 3:

We implement:

Type 1 (Router-LSA)

Minimal Type 2 (optional)

4. Router-LSA Structure

Router-LSA describes:

Router ID

Links

Metrics

Neighbor router IDs

C++ Representation
struct Link {
    std::string neighbor_id;
    int cost;
};

struct RouterLSA {
    std::string router_id;
    int sequence_number;
    int age;
    std::vector<Link> links;
};

5. LSDB Container Structure

LSDB is area-scoped.

class LSDB {
private:
    std::map<std::string, RouterLSA> router_lsas;

public:
    void install(const RouterLSA& lsa);
    void remove(const std::string& router_id);
    bool has_changed();
};


Key idea:

Router ID is primary key

Sequence number determines freshness

6. LSA Installation Rules

When new LSA arrives:

6.1 Sequence Comparison

If:

new.seq > current.seq → install
new.seq == current.seq → ignore
new.seq < current.seq → discard


This prevents stale data from re-entering LSDB.

7. LSA Aging

Each LSA has:

Age field

MaxAge = 3600 seconds (standard OSPF)

mini-xr Phase 3:

Age increment every 1 second

Remove LSA at MaxAge

Trigger SPF on removal

8. Flooding Model (Simplified)

When LSA installed:

Flood to all FULL neighbors

Do not flood back to origin

Maintain retransmission list

For Phase 3:

We simulate flooding via Redis pub/sub.

9. LSDB Change Detection

We maintain a dirty flag:

bool lsdb_dirty = false;


Whenever:

LSA added

LSA removed

LSA updated

Then:

lsdb_dirty = true


SPF scheduler checks this flag.

10. LSDB → SPF → RIB Flow
LSA change
    ↓
Mark LSDB dirty
    ↓
SPF scheduled (delayed)
    ↓
Compute shortest paths
    ↓
Generate route candidates
    ↓
Publish to RIB (proto.route.add)


This is identical to XR architecture separation.

11. Failure Model
Neighbor Down

Remove that neighbor’s LSA

Mark LSDB dirty

Trigger SPF

LSA Timeout

Age reaches MaxAge

Remove LSA

Trigger SPF

12. Why LSDB Is Separate From RIB

Important concept:

LSDB	RIB
Topology database	Best routes
Graph model	Forwarding decision
Owned by OSPF	Owned by ribd
Protocol-specific	Protocol-agnostic

This separation is core to IOS-XR architecture.

13. Phase 3 Implementation Plan
Step	Component
3.1	OSPF architecture doc
3.2	Neighbor FSM
3.3	LSDB class
3.4	SPF engine
3.5	Route publication
14. mini-xr LSDB Behavior (Phase 3 Scope)

For initial implementation:

Single area (0.0.0.0)

Single router instance

Manual LSA injection

In-memory only

No persistence

Advanced versions later:

Multi-area

Incremental SPF

LSA aging threads

Graceful restart
