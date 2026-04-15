# Mini-XR Failure and Restart Model

## Purpose

This document defines how Mini-XR behaves under **failure conditions** and
**process restarts**.

The design goal is to ensure that **no single daemon failure causes a system-wide outage**.
This mirrors the fault-containment principles used in carrier-grade network
operating systems such as IOS-XR.

---

## Design Philosophy

Mini-XR follows a **fail-fast, recover-fast** philosophy:

- Failures are expected
- Processes may crash at any time
- Recovery must be automatic and localized
- Global restarts are considered design failures

---

## Failure Domains

Each daemon represents an independent **failure domain**.

| Daemon | Failure Impact |
|------|---------------|
cfgmgrd | Configuration changes unavailable |
ifmgrd | Interface state may become stale |
protod | No new routes learned |
ribd | Route selection frozen |
State DB | Control-plane unavailable |

Failures must not cascade beyond the owning domain.

---

## State Persistence Model

- All authoritative state is stored in the state database
- Daemons do not rely on in-memory state for correctness
- Process memory is considered **ephemeral**

This ensures that:
- Crashes do not erase system state
- Restarted daemons can resume operation

---

## Restart Semantics

### General Rules

On restart, every daemon must:

1. Connect to the state database
2. Read its owned namespace
3. Reconstruct internal state
4. Resume normal operation

A daemon must never assume a clean start.

---

## Failure Scenarios

### 1. Protocol Daemon Crash (`protod`)

**Scenario**
- `protod` crashes while publishing routes

**Expected Behavior**
- Existing route entries remain in the state DB
- `ribd` continues operating using last known routes
- No system restart required

**Recovery**
- Supervisor restarts `protod`
- `protod` republishes routes
- State converges naturally

---

### 2. RIB Daemon Crash (`ribd`)

**Scenario**
- `ribd` crashes during route selection

**Expected Behavior**
- Protocol daemons continue publishing routes
- Best-route state remains unchanged
- No impact on other daemons

**Recovery**
- `ribd` restarts
- Reads existing routes
- Recomputes best paths
- Republishes decisions

---

### 3. Interface Manager Crash (`ifmgrd`)

**Scenario**
- `ifmgrd` crashes while tracking interface state

**Expected Behavior**
- Interface state remains in DB
- Protocol daemons continue operating on last known state
- No control-plane crash

**Recovery**
- `ifmgrd` restarts
- Rehydrates interface state
- Publishes updated status if changed

---

### 4. Configuration Manager Crash (`cfgmgrd`)

**Scenario**
- `cfgmgrd` crashes during a commit

**Expected Behavior**
- Running configuration remains unchanged
- Candidate configuration may be discarded
- System continues with last committed config

**Recovery**
- `cfgmgrd` restarts
- Reads running configuration
- Accepts new candidate configs

---

### 5. State Database Failure

**Scenario**
- Redis becomes unavailable

**Expected Behavior**
- Daemons detect DB disconnect
- Control-plane enters degraded mode
- No daemon attempts unsafe operation

**Recovery**
- DB restored
- Daemons reconnect
- State reconciliation occurs

This is considered the **only global failure** scenario.

---

## Partial Failure Tolerance

Mini-XR prioritizes **partial functionality** over total failure.

Examples:
- Routing continues with stale data
- Configuration changes temporarily blocked
- Recovery without traffic loss (later phases)

---

## Supervision Model

Each daemon is supervised independently.

Recommended approach:
- systemd service per daemon
- Automatic restart on failure
- Rate-limited restarts to avoid loops

Example:
Restart=always
RestartSec=2

---

## Observability During Failure

Failures are diagnosed through:
- Daemon logs
- State DB inspection
- Restart counters

Debugging focuses on:
- State divergence
- Missing updates
- Recovery behavior

---

## Anti-Patterns (Explicitly Avoided)

- Global restarts
- Shared memory recovery
- Implicit ordering assumptions
- Hard dependencies between daemons

These patterns undermine fault isolation.

---

## Validation Criteria

The failure model is considered correct if:

- Any daemon can be killed manually
- Remaining daemons continue running
- Restarted daemon resumes operation
- State converges without manual intervention

---

## Summary

Mini-XR treats failure as a normal operating condition.

Robustness is achieved through:
- Isolation
- State persistence
- Explicit recovery logic

This failure model is foundational to building a resilient routing platform.

