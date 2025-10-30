#!/bin/bash

# Configuration to test EXPLODE termination
export N_ATOMI_INIT=20
export N_ATOM_MAX=100
export MIN_N_ATOMICO=2
export ENERGY_DEMAND=5
export ENERGY_EXPLODE_THRESHOLD=5000
export SIM_DURATION=300
export STEP=500000000
export N_NUOVI_ATOMI=5

echo "Running simulation with EXPLODE configuration..."
echo "Expected termination: EXPLODE when energy exceeds threshold"
./master
