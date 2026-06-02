# Mini-XR CLI Architecture and Configuration Workflow

## Overview

Mini-XR is a Cisco IOS-XR inspired Network Operating System built using independent daemons and a centralized state database.

Current implementation includes:

* Multi-mode CLI
* Candidate configuration database
* Validation framework
* Commit model
* Rollback functionality
* Redis-based centralized state storage

The design follows the philosophy used in modern network operating systems such as:

* Cisco IOS-XR
* Juniper JunOS
* Nokia SR OS

---

# High-Level Architecture

Current architecture:

CLI → Redis

```text
+----------------+
|    User CLI    |
+----------------+
         |
         v
+----------------+
|    cli.py      |
+----------------+
         |
         v
+----------------+
|    Redis DB    |
+----------------+
```

Redis acts as the centralized configuration and operational database.

---

# CLI Modes

Mini-XR currently supports four operational modes.

## User EXEC Mode

Prompt:

```text
mini-xr>
```

Purpose:

* Initial login mode
* Limited commands

Example:

```bash
mini-xr> enable
```

---

## Privileged EXEC Mode

Prompt:

```text
mini-xr#
```

Purpose:

* Operational commands
* Configuration entry point

Example:

```bash
mini-xr# show hostname
mini-xr# show candidate
mini-xr# show diff
mini-xr# configure terminal
```

---

## Global Configuration Mode

Prompt:

```text
mini-xr(config)#
```

Purpose:

* Modify candidate configuration
* Validate configuration
* Commit changes
* Rollback changes

Example:

```bash
mini-xr(config)# hostname Router1
mini-xr(config)# validate
mini-xr(config)# commit
```

---

## Interface Configuration Mode

Prompt:

```text
mini-xr(config-if-eth0)#
```

Purpose:

* Configure interface parameters

Example:

```bash
mini-xr(config-if-eth0)# ip address 10.0.0.1/24
mini-xr(config-if-eth0)# no shutdown
```

---

# Candidate Configuration Model

Mini-XR follows a candidate configuration workflow.

Configuration changes are not applied immediately.

Instead:

```text
User Change
      ↓
Candidate Database
      ↓
Validate
      ↓
Commit
      ↓
Running Configuration
```

This approach prevents accidental configuration errors.

---

# Running Configuration

Running configuration represents the active state of the router.

Stored in Redis:

```text
config:running:hostname
```

Example:

```bash
mini-xr# show hostname

AMMALIRAJ
```

CLI retrieves data from:

```text
config:running:hostname
```

---

# Candidate Configuration

When configuration commands are entered, changes are stored in candidate configuration.

Example:

```bash
mini-xr(config)# hostname RouterRaj
```

Stored in Redis:

```text
config:candidate:hostname = RouterRaj
```

The running configuration remains unchanged.

---

# Show Candidate

Command:

```bash
show candidate
```

Purpose:

Display all pending changes before commit.

Example:

```text
config:candidate:hostname = RouterRaj
```

---

# Show Diff

Command:

```bash
show diff
```

Purpose:

Compare candidate configuration with running configuration.

Example:

```text
config:candidate:hostname

candidate : RouterRaj
running   : AMMALIRAJ
```

This helps operators understand pending changes before commit.

---

# Validation Workflow

Command:

```bash
validate
```

Purpose:

Check candidate configuration before commit.

Current implementation validates:

* Hostname presence
* Basic configuration consistency

Workflow:

```text
Candidate Configuration
           ↓
      Validation
           ↓
      Pass/Fail
```

Example:

```bash
mini-xr(config)# validate

Validating configuration...
[OK] Validation successful
```

---

# Commit Workflow

Command:

```bash
commit
```

Purpose:

Move candidate configuration into running configuration.

Workflow:

```text
Candidate Configuration
           ↓
         Commit
           ↓
 Running Configuration
           ↓
 Remove Candidate
```

Example:

```bash
mini-xr(config)# commit

Committing configuration...
[OK] Commit successful
```

Result:

```text
config:running:hostname = RouterRaj
```

---

# Rollback Workflow

Command:

```bash
rollback
```

Purpose:

Discard all uncommitted candidate changes.

Before rollback:

```text
config:candidate:hostname = Raja
config:running:hostname   = AMMALIRAJ
```

After rollback:

```text
config:candidate:* removed
config:running:hostname = AMMALIRAJ
```

Example:

```bash
mini-xr(config)# rollback

[OK] Rollback complete
```

Verification:

```bash
mini-xr# show candidate

No candidate config
```

---

# Redis Data Model

Current Redis keys:

```text
config:running:hostname

config:candidate:hostname

config:commit-id

interface:eth0

interface:eth0:ip

interface:eth0:state

rib:best:2.2.2.2/32

rib:best:3.3.3.3/32

proto:route:10.1.0.0/24
```

Redis currently acts as a lightweight SysDB.

---

# Verification Commands

Display Redis contents:

```bash
redis-cli
keys *
```

Verify running hostname:

```bash
get config:running:hostname
```

Verify candidate hostname:

```bash
get config:candidate:hostname
```

---

# Comparison with Cisco IOS-XR

Current Mini-XR implementation resembles:

```text
CLI
 ↓
Candidate Configuration
 ↓
Validate
 ↓
Commit
 ↓
Running Configuration
```

which is conceptually similar to:

```text
IOS-XR CLI
 ↓
Candidate Database
 ↓
Commit Engine
 ↓
SysDB
```

The major difference is that Mini-XR currently writes directly to Redis.

---

# Future Architecture (Next Phase)

Current:

```text
CLI
 ↓
Redis
```

Target architecture:

```text
CLI
 ↓
cfgmgrd
 ↓
Redis (SysDB)
 ↓
ifmgrd
 ↓
ribd
 ↓
ospfd
 ↓
protod
```

In this design:

* CLI does not modify Redis directly.
* cfgmgrd owns configuration management.
* Daemons subscribe to configuration updates.
* Redis acts as centralized state storage.

This architecture more closely resembles Cisco IOS-XR distributed control-plane design.

---

# Project Status

Completed:

* CLI Framework
* User/Enable/Config/Interface Modes
* Candidate Configuration Database
* Validation Engine
* Commit Model
* Rollback Mechanism
* Diff Engine
* Redis State Database

Next Target:

* cfgmgrd-based configuration ownership
* Redis Pub/Sub integration
* Distributed daemon notifications
* SysDB-style architecture
