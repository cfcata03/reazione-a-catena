#!/bin/bash

# Configuration to test TIMEOUT termination
export N_ATOMI_INIT=5
export N_ATOM_MAX=50
export MIN_N_ATOMICO=3
export ENERGY_DEMAND=10
export ENERGY_EXPLODE_THRESHOLD=100000
export SIM_DURATION=10
export STEP=2000000000
export N_NUOVI_ATOMI=1

echo "Running simulation with TIMEOUT configuration..."
echo "Expected termination: TIMEOUT after 10 seconds"
./master
