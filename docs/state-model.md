# Mini-XR State Model

## Purpose

This document defines the **state model**, **ownership rules**, and **lifecycle**
of all control-plane state in Mini-XR.

The state database acts as the system’s **control-plane backbone**, similar to
IOS-XR SysDB.

All daemons communicate indirectly by publishing and consuming state through
this database.

---

## Core Principles

1. **Single Source of Truth**
   - All authoritative state lives in the database
   - No daemon keeps exclusive in-memory state

2. **Explicit Ownership**
   - Each daemon owns a strict namespace
   - Only the owning daemon may write to that namespace

3. **Read-Only Consumption**
   - Daemons may read other namespaces
   - Cross-daemon writes are forbidden

4. **Restart Safety**
   - State must survive daemon restarts
   - Daemons must rehydrate state on startup

---

## State Database

### Initial Implementation
- Redis (key-value store)
- Simple schema for clarity
- Easy introspection for debugging

Future implementations may replace Redis without changing daemon behavior.

---

## Namespace Layout

### Configuration State (cfgmgrd)

Owned by: `cfgmgrd`

config:candidate
config:running
config:commit-id


**Description**
- `candidate` holds uncommitted configuration
- `running` holds active configuration
- `commit-id` tracks last successful commit

Example:
```json
{
  "interfaces": {
    "eth0": { "admin_state": "up" }
  }
}


Interface State (ifmgrd)

Owned by: ifmgrd

ifmgrd:interfaces:eth0
ifmgrd:interfaces:eth1


Example:

{
  "admin_state": "up",
  "oper_state": "up",
  "last_change": "2026-02-05T10:20:00Z"
}

Protocol Routes (protod)

Owned by: protod

protod:routes:10.0.0.0/24
protod:routes:20.0.0.0/24


Example:

{
  "next_hop": "192.168.1.1",
  "metric": 10,
  "protocol": "stub"
}


Notes:

Phase 1 uses simulated routes

Later phases integrate FRR

Best Routes (ribd)

Owned by: ribd

ribd:best-routes:10.0.0.0/24


Example:

{
  "selected_by": "ribd",
  "next_hop": "192.168.1.1",
  "reason": "lowest metric"
}

State Flow Lifecycle

cfgmgrd writes desired configuration

ifmgrd publishes interface operational state

protod publishes learned routes

ribd selects best routes and publishes decisions

Each step is independent and asynchronous.

Startup Behavior

On startup, each daemon must:

Connect to the state database

Read existing state in its namespace

Reconstruct internal state

Resume operation without requiring system restart

No daemon assumes it is the first process to start.

Failure Behavior

State persists even if a daemon crashes

Restarted daemons reattach to existing state

Partial system operation is preferred over global failure

Example:

protod crashes → routes remain in DB

ribd continues operating on last known state

Observability & Debugging

State inspection is the primary debugging method.

Useful commands:

redis-cli KEYS "*"
redis-cli GET protod:routes:10.0.0.0/24


This mirrors IOS-XR operational debugging workflows.

Design Constraints

No shared memory

No direct IPC between daemons

No implicit ownership

All behavior must be explainable via state transitions

Summary

The Mini-XR state model enforces discipline, fault isolation, and predictability.

All advanced features are built on top of this foundation.


---

## ✅ Verify

Save and exit, then run:
```bash
ls ~/mini-xr/docs


You should now see:

architecture.md
state-model.md

