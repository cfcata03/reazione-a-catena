# ⚛️ Reazione a Catena - Chain Reaction Simulator

[![Made with C](https://img.shields.io/badge/Made%20with-C-blue.svg)](https://www.open-std.org/jtc1/sc22/wg14/)
[![Platform](https://img.shields.io/badge/platform-macOS-lightgrey.svg)](https://www.apple.com/macos/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

> A multi-process nuclear chain reaction simulator using IPC mechanisms (shared memory, semaphores, and message queues) for inter-process communication.

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Demo](#-demo)
- [Installation](#-installation)
- [Quick Start](#-quick-start)
- [Configuration](#-configuration)
- [How It Works](#-how-it-works)
- [Termination Conditions](#-termination-conditions)
- [Project Structure](#-project-structure)
- [Implementation Details](#-implementation-details)
- [Documentation](#-documentation)

## 🎯 Overview

This project simulates a nuclear chain reaction where atoms split (via `fork()`) to create new atoms, releasing energy in the process. The simulation involves multiple concurrent processes:

- **Master Process**: Orchestrates the simulation and consumes energy
- **Atom Processes**: Represent atoms that can split when activated
- **Activator Process**: Triggers atom fission reactions
- **Feeding Process**: Introduces new atoms to sustain the reaction

The simulation can terminate in four different ways: **TIMEOUT**, **EXPLODE**, **BLACKOUT**, or **MELTDOWN**.

## ✨ Features

- 🔄 **Multi-process concurrency** using fork()
- 🔗 **IPC mechanisms**: Shared memory, semaphores, and message queues
- ⚙️ **Configurable parameters** via environment variables
- 📊 **Real-time statistics** printed every second
- 🎮 **Multiple termination scenarios** for testing
- 🧹 **Automatic resource cleanup**
- 🍎 **macOS optimized** (adapted from Linux)
- 🚫 **No busy waiting** - efficient synchronization

## 🎬 Demo

```
=== Simulation Statistics (Elapsed: 3 s) ===
Activations: 89 (last sec: 25)
Splits:      124 (last sec: 38)
Energy produced: 8432 (last sec: 2156)
Energy consumed: 30 (last sec: 10)
Current energy:  8402
Waste:       18 (last sec: 7)
Active atoms: 73
==========================================
```

## 🚀 Installation

### Prerequisites

- macOS (tested on Darwin kernel)
- GCC compiler
- Make utility

### Build

```bash
# Clone the repository
cd /path/to/progetto

# Build the project
make
```

This compiles with strict warnings: `-Wvla -Wextra -Werror -D_GNU_SOURCE`

## 🏃 Quick Start

### Run with Default Settings

```bash
./master
```

### Test Scenarios

Three predefined test scripts are included:

#### 1️⃣ Timeout Test
Simulation runs for 10 seconds and terminates gracefully.
```bash
./run_timeout.sh
```

#### 2️⃣ Explode Test
Energy production exceeds the threshold, causing explosion.
```bash
./run_explode.sh
```

#### 3️⃣ Blackout Test
Energy consumption exceeds production, causing blackout.
```bash
./run_blackout.sh
```

## ⚙️ Configuration

All parameters can be configured via environment variables:

| Variable | Description | Default |
|----------|-------------|---------|
| `N_ATOMI_INIT` | Initial number of atoms | 10 |
| `N_ATOM_MAX` | Maximum atomic number | 100 |
| `MIN_N_ATOMICO` | Min atomic number (below becomes waste) | 5 |
| `ENERGY_DEMAND` | Energy consumed per second | 50 |
| `ENERGY_EXPLODE_THRESHOLD` | Energy limit before explosion | 10000 |
| `SIM_DURATION` | Max simulation time (seconds) | 30 |
| `STEP` | Nanoseconds between atom additions | 1000000000 |
| `N_NUOVI_ATOMI` | New atoms added each STEP | 2 |

### Example: Custom Configuration

```bash
export N_ATOMI_INIT=20
export ENERGY_DEMAND=100
export SIM_DURATION=60
./master
```

## 🔬 How It Works

### Energy Calculation

When an atom splits into two atoms with atomic numbers `n1` and `n2`:

```
Energy = n1 × n2 - max(n1, n2)
```

Maximum energy is released when atoms split evenly (n1 ≈ n2).

### Process Flow

```
┌─────────────┐
│   Master    │  Creates & monitors all processes
└──────┬──────┘
       │
       ├──────► Atom₁ ──fork──► Atom₃
       │          │
       ├──────► Atom₂ ──fork──► Atom₄
       │
       ├──────► Activator (sends split signals)
       │
       └──────► Alimentazione (adds new atoms)
```

### Synchronization

- **Shared Memory**: Statistics shared between all processes
- **Semaphores**: Protect critical sections (3 semaphores)
- **Message Queue**: Activator → Atoms communication

## 🛑 Termination Conditions

| Condition | Description | Trigger |
|-----------|-------------|---------|
| ⏰ **TIMEOUT** | Time limit reached | `elapsed >= SIM_DURATION` |
| 💥 **EXPLODE** | Energy too high | `net_energy >= ENERGY_EXPLODE_THRESHOLD` |
| 🔌 **BLACKOUT** | Energy depleted | `current_energy < 0` |
| 🔥 **MELTDOWN** | System failure | `fork()` fails |

## 📁 Project Structure

```
progetto/
├── master.c              # Main orchestrator process
├── atomo.c              # Atom process (splits on activation)
├── attivatore.c         # Activator process (triggers splits)
├── alimentazione.c      # Feeding process (adds atoms)
├── shared.c/h           # IPC utilities
├── config.c/h           # Configuration management
├── Makefile             # Build system
├── run_timeout.sh       # Test script: TIMEOUT
├── run_explode.sh       # Test script: EXPLODE
├── run_blackout.sh      # Test script: BLACKOUT
├── README.md            # This file
├── RELAZIONE.md         # Design document (Italian)
└── QUICK_START.md       # Quick reference guide
```

## 🛠️ Implementation Details

### IPC Mechanisms Used

1. **Shared Memory** (`shmget`, `shmat`)
   - Statistics structure shared across all processes
   - Atomic access protected by semaphores

2. **Semaphores** (`semget`, `semop`)
   - `SEM_STATS`: Protects statistics updates
   - `SEM_ATOMS`: Protects atom count
   - `SEM_BARRIER`: General synchronization

3. **Message Queues** (`msgget`, `msgsnd`, `msgrcv`)
   - Activator sends split messages
   - Atoms receive and process messages

### Key Design Decisions

- ✅ **No busy waiting**: All waits use blocking operations
- ✅ **Modular design**: Each process is a separate executable
- ✅ **Synchronized startup**: All processes wait for `running` flag
- ✅ **Graceful shutdown**: Proper cleanup of all IPC resources
- ✅ **Strict compilation**: Compiled with `-Werror` for code quality

### macOS Adaptations

- Removed duplicate `union semun` definition (provided by macOS)
- Uses `-D_GNU_SOURCE` for POSIX extensions
- Compatible with macOS IPC implementation
- Tested on Darwin kernel 24.5.0

## 📚 Documentation

- **[RELAZIONE.md](RELAZIONE.md)** - Detailed design choices and implementation (Italian)
- **[QUICK_START.md](QUICK_START.md)** - Quick reference for running the simulation
- **Source code comments** - Inline documentation throughout

## 🧹 Cleanup

Remove compiled files and IPC resources:

```bash
make clean
```

This removes executables, object files, and any orphaned IPC resources.

## 📊 Statistics Output

Every second, the simulation displays:

- **Activations**: Times the activator triggered splits
- **Splits**: Number of atom fissions
- **Energy produced**: Total energy from fission
- **Energy consumed**: Energy used by master
- **Current energy**: Available energy (produced - consumed)
- **Waste**: Atoms that became too small to split
- **Active atoms**: Current number of atom processes

## 🎓 Academic Context

This project was developed as part of the Operating Systems course (Sistemi Operativi) 2023/24. It demonstrates:

- Multi-process programming
- Inter-process communication
- Process synchronization
- Resource management
- Concurrent programming patterns

## 📝 License

This project is available under the MIT License.

## 🤝 Contributing

This is an academic project, but suggestions and improvements are welcome!

---

<div align="center">

**Built with ❤️ using C and POSIX IPC**

⭐ Star this repo if you find it helpful!

</div>
