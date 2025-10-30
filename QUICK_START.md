# Quick Start Guide

## Build

```bash
make
```

## Run with Default Configuration

```bash
./master
```

## Test Different Termination Scenarios

### 1. Timeout Test (10 seconds)
```bash
./run_timeout.sh
```

### 2. Explode Test (energy threshold exceeded)
```bash
./run_explode.sh
```

### 3. Blackout Test (insufficient energy)
```bash
./run_blackout.sh
```

## Custom Configuration

Set environment variables before running:

```bash
export N_ATOMI_INIT=15
export N_ATOM_MAX=80
export MIN_N_ATOMICO=4
export ENERGY_DEMAND=20
export ENERGY_EXPLODE_THRESHOLD=8000
export SIM_DURATION=60
export STEP=1000000000
export N_NUOVI_ATOMI=3

./master
```

## Clean Up

Remove compiled files and IPC resources:

```bash
make clean
```

## Understanding the Output

Every second you'll see:
- **Activations**: How many times the activator triggered atom splits
- **Splits**: Number of atom fissions that occurred
- **Energy produced**: Total energy from fission reactions
- **Energy consumed**: Energy consumed by the master process
- **Current energy**: Available energy (produced - consumed)
- **Waste**: Atoms that became too small to split
- **Active atoms**: Current number of active atom processes

## Configuration Parameters Explained

- `N_ATOMI_INIT` (default: 10) - Initial number of atoms
- `N_ATOM_MAX` (default: 100) - Maximum atomic number for new atoms
- `MIN_N_ATOMICO` (default: 5) - Atoms below this become waste
- `ENERGY_DEMAND` (default: 50) - Energy consumed per second
- `ENERGY_EXPLODE_THRESHOLD` (default: 10000) - Energy limit before explosion
- `SIM_DURATION` (default: 30) - Maximum simulation time in seconds
- `STEP` (default: 1000000000) - Nanoseconds between new atom additions
- `N_NUOVI_ATOMI` (default: 2) - Number of atoms added each STEP

## Tips for Different Outcomes

### To get TIMEOUT:
- Low ENERGY_DEMAND
- High ENERGY_EXPLODE_THRESHOLD
- Moderate atom activity

### To get EXPLODE:
- Many initial atoms
- Low ENERGY_DEMAND
- Low ENERGY_EXPLODE_THRESHOLD
- Frequent new atom additions

### To get BLACKOUT:
- Few atoms
- High ENERGY_DEMAND
- High MIN_N_ATOMICO (atoms become waste quickly)
- Infrequent new atom additions

### To get MELTDOWN:
- Set system resource limits very low
- Create too many processes too quickly
