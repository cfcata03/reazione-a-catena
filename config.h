#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int n_atomi_init;           /* Initial number of atoms */
    int n_atom_max;             /* Maximum atomic number */
    int min_n_atomico;          /* Minimum atomic number before becoming waste */
    int energy_demand;          /* Energy consumed by master per second */
    int energy_explode_threshold; /* Energy threshold for explosion */
    long sim_duration;          /* Simulation duration in seconds */
    long step;                  /* Nanoseconds between new atom additions */
    int n_nuovi_atomi;          /* Number of new atoms added each STEP */
} Config;

extern Config config;

/* Load configuration from environment variables or use defaults */
void load_config(void);

/* Get integer from environment or return default */
int get_env_int(const char* name, int default_val);

/* Get long from environment or return default */
long get_env_long(const char* name, long default_val);

#endif
