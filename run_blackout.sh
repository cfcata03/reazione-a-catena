#!/bin/bash

# Configuration to test BLACKOUT termination
export N_ATOMI_INIT=3
export N_ATOM_MAX=20
export MIN_N_ATOMICO=5
export ENERGY_DEMAND=1000
export ENERGY_EXPLODE_THRESHOLD=100000
export SIM_DURATION=300
export STEP=3000000000
export N_NUOVI_ATOMI=1

echo "Running simulation with BLACKOUT configuration..."
echo "Expected termination: BLACKOUT when energy becomes negative"
./master
