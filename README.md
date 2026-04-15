# Mini-XR (Micro Network OS)

Mini-XR is a modular network operating system inspired by Cisco IOS-XR architecture.

---

## Architecture

The system is designed using independent daemons:

- cfgmgrd → Configuration Manager 
- ifmgrd → Interface Manager 
- ospfd → OSPF routing daemon 
- ribd → Routing Information Base 
- protod → Protocol manager 

---

## Features

- Modular microservices architecture 
- Inter-process communication using Pub/Sub 
- Redis-based messaging 
- systemd service integration 

---

## Project Structure

- `common/` → Shared utilities (logger, pubsub, redis client) 
- `ospfd/` → OSPF protocol implementation 
- `ribd/` → Routing table management 
- `systemd/` → Service files 

---

## Future Work

- ConfD integration 
- NETCONF/YANG support 
- CLI interface 
- gRPC-based communication 

---

## Build Instructions

### Prerequisites

- g++ 
- make 
- Redis (optional for pub/sub) 

---

### Build

```bash
g++ -o cfgmgrd/cfgmgrd cfgmgrd/main.cpp common/*.cpp
g++ -o ifmgrd/ifmgrd ifmgrd/main.cpp common/*.cpp
g++ -o ribd/ribd ribd/main.cpp common/*.cpp
g++ -o ospfd/ospfd ospfd/main.cpp ospfd/*.cpp common/*.cpp
g++ -o protod/protod protod/main.cpp

### Run

```bash
./cfgmgrd/cfgmgrd &
./ifmgrd/ifmgrd &
./ribd/ribd &
./ospfd/ospfd &

Each component runs as an independent process and communicates using pub/sub mechanisms.

