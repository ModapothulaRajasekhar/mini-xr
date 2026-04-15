# Mini-XR (Micro Network OS)

Mini-XR is a modular network operating system inspired by Cisco IOS-XR architecture.

## Architecture

The system is designed using independent daemons:

- cfgmgrd → Configuration Manager
- ifmgrd → Interface Manager
- ospfd → OSPF routing daemon
- ribd → Routing Information Base
- protod → Protocol manager

## Features

- Modular microservices architecture
- Inter-process communication using Pub/Sub
- Redis-based messaging
- systemd service integration

## Project Structure

- common/ → Shared utilities (logger, pubsub, redis client)
- ospfd/ → OSPF protocol implementation
- ribd/ → Routing table management
- systemd/ → Service files

## Future Work

- ConfD integration
- NETCONF/YANG support
- CLI interface
- gRPC-based communication
