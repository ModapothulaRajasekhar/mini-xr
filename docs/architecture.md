# Mini-XR Architecture

## Overview

Mini-XR is a Linux-based modular routing platform inspired by the
architectural principles of Cisco IOS-XR.

The project demonstrates how a traditional monolithic network operating
system can be decomposed into **isolated services** with **explicit state
ownership**, **centralized state management**, and **fault containment**.

This is **not** an attempt to reimplement IOS-XR.  
Instead, Mini-XR focuses on reproducing the **architectural thinking**
behind carrier-grade network operating systems.

---

## Design Goals

The primary goals of Mini-XR are:

- Strong process isolation
- Deterministic state ownership
- Restart-safe operation
- Clear separation of control-plane responsibilities
- Architecture-first development

Performance and feature completeness are intentionally out of scope for
early phases.

---

## Core Design Principles

### 1. Process Isolation

Each functional subsystem runs as an independent daemon:

- Separate address space
- Independent lifecycle
- Individually restartable

A failure in one daemon must not crash or corrupt others.

---

### 2. Single Source of Truth

All operational and configuration state is stored in a **central state
database**.

- No shared memory
- No global variables
- No direct daemon-to-daemon calls

State persistence enables crash recovery and restart safety.

---

### 3. Explicit State Ownership

Each daemon owns a **strict namespace** in the state database.

Rules:
- A daemon may only write to its own namespace
- Other daemons may only read that state
- Ownership violations are design bugs

This prevents hidden coupling and undefined behavior.

---

### 4. Event-Driven State Flow

State changes propagate through database updates, not function calls.

Typical flow:

Producer daemon → State DB → Consumer daemon


This enables loose coupling and fault containment.

---

### 5. Restart Safety

Any daemon may be killed and restarted at any time.

On startup, a daemon:
- Reconnects to the state database
- Reads existing state
- Resumes operation without requiring system restart

---

## High-Level Architecture

+------------------+
| CLI | (stateless)
+------------------+
|
+------------------+
| cfgmgrd | (config ownership)
+------------------+
|
+----------------------------------+
| STATE DATABASE |
| (Redis) |
+----------------------------------+
| | |
ifmgrd ribd protod





---

## Component Responsibilities

### cfgmgrd — Configuration Manager

**Responsibilities**
- Maintains candidate configuration
- Performs validation
- Commits configuration atomically
- Writes desired state to the database

**Does NOT**
- Touch interfaces directly
- Manipulate routes
- Interact with the kernel

---

### ifmgrd — Interface Manager

**Responsibilities**
- Owns interface operational state
- Tracks admin and oper status
- Publishes interface state changes

**Notes**
- Phase 1 uses simulated interfaces
- Kernel interaction is added later

---

### protod — Protocol Daemon (Stub)

**Responsibilities**
- Represents routing protocol producers
- Publishes learned routes into the state DB

**Phase 1 Behavior**
- Generates static or simulated routes
- No real protocol logic

Later phases integrate FRR.

---

### ribd — Routing Information Base Daemon

**Responsibilities**
- Consumes routes from protocol daemons
- Selects best paths (stub logic initially)
- Publishes selected routes

**Notes**
- No kernel programming in Phase 1
- Acts as control-plane decision point

---

## Communication Model

- Daemons never communicate directly
- All communication is indirect via the state DB
- Database updates represent events

Example:
protod publishes route
→ ribd detects change
→ ribd selects best route
→ ribd publishes decision



---

## Fault Containment Model

- Daemon crashes are isolated
- State persists independently of process lifetime
- Supervisors restart failed daemons automatically

The system continues operating in degraded mode rather than failing
globally.

---

## Observability

System state can be inspected directly via the state database.

Example:
redis-cli KEYS "*"
redis-cli GET ifmgrd:interfaces:eth0


This mirrors IOS-XR SysDB inspection workflows.

---

## Out of Scope (Phase 1)

The following are intentionally excluded from Phase 1:

- Real routing protocols
- Kernel FIB programming
- High availability
- Performance optimization
- Security hardening

These are introduced incrementally after architectural stability is proven.

---

## Summary

Mini-XR demonstrates that carrier-grade reliability is achieved through
architecture, not code volume.

Isolation, state discipline, and restart safety are the foundations on
which advanced routing functionality is built.

