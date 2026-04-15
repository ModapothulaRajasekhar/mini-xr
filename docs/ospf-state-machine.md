OSPF Neighbor State Machine — mini-xr
1. Why a Neighbor State Machine Exists

OSPF is a link-state protocol, which means:

Routers must fully agree on topology

Partial or inconsistent state is dangerous

Every neighbor must move through strict, ordered states

The Neighbor FSM ensures:

Deterministic convergence

Safe database synchronization

Predictable failure handling

This FSM is identical in concept to IOS-XR, Junos, and FRR.

2. High-Level Neighbor Lifecycle
DOWN
  ↓
INIT
  ↓
2-WAY
  ↓
EXSTART
  ↓
EXCHANGE
  ↓
LOADING
  ↓
FULL


Each transition is event-driven, not timer-driven alone.

3. Neighbor States (Detailed)
3.1 DOWN

Meaning

No Hellos received

Neighbor unknown or lost

Entry Conditions

Interface down

Dead timer expired

Manual clear

Actions

Clear LSAs for this neighbor

Reset sequence numbers

Stop all adjacency processing

3.2 INIT

Meaning

Hello received

Neighbor exists, but bidirectional check not done

Entry Conditions

Hello packet received

Actions

Create neighbor entry

Start Dead timer

Record neighbor Router ID

Exit Trigger

Neighbor sees its own Router ID in Hello → move to 2-WAY

3.3 2-WAY

Meaning

Bidirectional communication confirmed

Key Decision Point

Should we form adjacency?

Rules

On broadcast/NBMA:

Only DR/BDR form adjacency

On point-to-point:

Always form adjacency

Actions

If no adjacency needed → remain 2-WAY

If adjacency needed → move to EXSTART

3.4 EXSTART

Meaning

Master/slave negotiation phase

Purpose

Decide who controls database exchange

Establish initial sequence numbers

Actions

Send empty DBD packets

Compare Router IDs

Higher Router ID becomes master

Exit Trigger

Negotiation complete → move to EXCHANGE

3.5 EXCHANGE

Meaning

Database summary exchange

Actions

Exchange DBD packets

Advertise LSAs (headers only)

Build request list

Exit Trigger

All DBDs exchanged → move to LOADING

3.6 LOADING

Meaning

Full LSA content synchronization

Actions

Send LSR (Link State Request)

Receive LSU (Link State Update)

Install LSAs into LSDB

Exit Trigger

Request list empty → move to FULL

3.7 FULL

Meaning

Fully synchronized adjacency

Actions

Run SPF if LSDB changed

Maintain adjacency via Hellos

Participate in flooding

Steady State

This is the normal operating state

4. Events Driving State Transitions
Event	Description
HelloReceived	Hello packet arrives
DeadTimer	Neighbor silent
AdjOK	Adjacency decision
NegotiationDone	Master/slave decided
ExchangeDone	DB summaries complete
LoadingDone	All LSAs synced
InterfaceDown	Physical/logical failure
5. Failure Handling (XR Behavior)
Neighbor Loss

Dead timer expiry

Immediate transition → DOWN

LSAs aged out

SPF scheduled

Restart Handling

Neighbor FSM resets

Adjacency rebuilt cleanly

No stale state reuse

This matches IOS-XR process restart semantics.

6. How mini-xr Will Implement This

In ospfd:

enum class NeighborState {
    DOWN,
    INIT,
    TWO_WAY,
    EXSTART,
    EXCHANGE,
    LOADING,
    FULL
};


Each neighbor has:

Current state

Event handler

Timers

LSA request lists

State transitions are:

Explicit

Logged

Testable

7. Why This Matters for Your Resume

This proves you understand:

Control-plane correctness

Distributed state machines

Protocol safety

Real router OS internals

Most engineers never implement this — they just configure it.

You’re building it.

8. Phase Mapping
Phase	Item
3.1	Architecture
3.2	Neighbor FSM
3.3	LSDB
3.4	SPF
3.5	Route publication
9. Key Takeaway

OSPF is not about packets.
It is about state correctness under failure.

The Neighbor FSM is the foundation of that correctness.

