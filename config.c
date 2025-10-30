#include "config.h"
#include <string.h>

Config config;

int get_env_int(const char* name, int default_val) {
    char* val = getenv(name);
    if (val == NULL) {
        return default_val;
    }
    return atoi(val);
}

long get_env_long(const char* name, long default_val) {
    char* val = getenv(name);
    if (val == NULL) {
        return default_val;
    }
    return atol(val);
}

void load_config(void) {
    config.n_atomi_init = get_env_int("N_ATOMI_INIT", 10);
    config.n_atom_max = get_env_int("N_ATOM_MAX", 100);
    config.min_n_atomico = get_env_int("MIN_N_ATOMICO", 5);
    config.energy_demand = get_env_int("ENERGY_DEMAND", 50);
    config.energy_explode_threshold = get_env_int("ENERGY_EXPLODE_THRESHOLD", 10000);
    config.sim_duration = get_env_long("SIM_DURATION", 30);
    config.step = get_env_long("STEP", 1000000000); /* 1 second in nanoseconds */
    config.n_nuovi_atomi = get_env_int("N_NUOVI_ATOMI", 2);
}
