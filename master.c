#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include "shared.h"
#include "config.h"

static int shm_id = -1, sem_id = -1, msg_id = -1;
static Statistics* stats = NULL;
static time_t start_time;

/* Cleanup IPC resources */
void cleanup_ipc(void) {
    /* Send termination signal to all processes */
    if (stats != NULL) {
        sem_wait_op(sem_id, SEM_STATS);
        stats->running = 0;
        sem_signal_op(sem_id, SEM_STATS);

        detach_shared_memory(stats);
    }

    /* Wait a bit for children to terminate */
    sleep(1);

    /* Kill any remaining children */
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);

    /* Wait for all children */
    while (wait(NULL) > 0);

    /* Destroy IPC resources */
    if (msg_id != -1) {
        destroy_message_queue(msg_id);
    }
    if (sem_id != -1) {
        destroy_semaphores(sem_id);
    }
    if (shm_id != -1) {
        destroy_shared_memory(shm_id);
    }
}

void signal_handler(int signum) {
    (void)signum; /* Suppress unused parameter warning */
    if (stats != NULL) {
        sem_wait_op(sem_id, SEM_STATS);
        stats->running = 0;
        sem_signal_op(sem_id, SEM_STATS);
    }
}

/* Create a new atom process */
int create_atom(int atomic_number) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed in master");
        return -1;
    } else if (pid == 0) {
        /* Child process - exec atomo */
        char shm_str[32], sem_str[32], msg_str[32], atomic_str[32];
        snprintf(shm_str, sizeof(shm_str), "%d", shm_id);
        snprintf(sem_str, sizeof(sem_str), "%d", sem_id);
        snprintf(msg_str, sizeof(msg_str), "%d", msg_id);
        snprintf(atomic_str, sizeof(atomic_str), "%d", atomic_number);

        execl("./atomo", "atomo", shm_str, sem_str, msg_str, atomic_str, (char*)NULL);
        perror("execl atomo failed");
        exit(EXIT_FAILURE);
    }

    /* Parent */
    return 0;
}

/* Print statistics */
void print_stats(void) {
    sem_wait_op(sem_id, SEM_STATS);

    time_t elapsed = time(NULL) - start_time;

    printf("\n=== Simulation Statistics (Elapsed: %ld s) ===\n", elapsed);
    printf("Activations: %ld (last sec: %ld)\n",
           stats->total_activations, stats->last_sec_activations);
    printf("Splits:      %ld (last sec: %ld)\n",
           stats->total_splits, stats->last_sec_splits);
    printf("Energy produced: %ld (last sec: %ld)\n",
           stats->total_energy_produced, stats->last_sec_energy_produced);
    printf("Energy consumed: %ld (last sec: %ld)\n",
           stats->total_energy_consumed, stats->last_sec_energy_consumed);
    printf("Current energy:  %ld\n", stats->current_energy);
    printf("Waste:       %ld (last sec: %ld)\n",
           stats->total_waste, stats->last_sec_waste);
    printf("Active atoms: %d\n", stats->num_atoms);
    printf("==========================================\n");

    /* Reset last second counters */
    stats->last_sec_activations = 0;
    stats->last_sec_splits = 0;
    stats->last_sec_energy_produced = 0;
    stats->last_sec_energy_consumed = 0;
    stats->last_sec_waste = 0;

    sem_signal_op(sem_id, SEM_STATS);
}

/* Check termination conditions */
int check_termination(void) {
    sem_wait_op(sem_id, SEM_STATS);

    int should_terminate = 0;

    /* Check timeout */
    time_t elapsed = time(NULL) - start_time;
    if (elapsed >= config.sim_duration) {
        stats->termination_cause = TERM_TIMEOUT;
        should_terminate = 1;
    }

    /* Check explode */
    long net_energy = stats->total_energy_produced - stats->total_energy_consumed;
    if (net_energy >= config.energy_explode_threshold) {
        stats->termination_cause = TERM_EXPLODE;
        should_terminate = 1;
    }

    /* Check if already terminated by another process */
    if (stats->termination_cause != TERM_NONE) {
        should_terminate = 1;
    }

    if (should_terminate) {
        stats->running = 0;
    }

    sem_signal_op(sem_id, SEM_STATS);
    return should_terminate;
}

/* Consume energy */
void consume_energy(void) {
    sem_wait_op(sem_id, SEM_STATS);

    stats->total_energy_consumed += config.energy_demand;
    stats->last_sec_energy_consumed += config.energy_demand;
    stats->current_energy -= config.energy_demand;

    /* Check blackout */
    if (stats->current_energy < 0) {
        stats->termination_cause = TERM_BLACKOUT;
        stats->running = 0;
    }

    sem_signal_op(sem_id, SEM_STATS);
}

int main(void) {
    /* Load configuration */
    load_config();

    printf("Chain Reaction Simulation\n");
    printf("Configuration:\n");
    printf("  N_ATOMI_INIT: %d\n", config.n_atomi_init);
    printf("  N_ATOM_MAX: %d\n", config.n_atom_max);
    printf("  MIN_N_ATOMICO: %d\n", config.min_n_atomico);
    printf("  ENERGY_DEMAND: %d\n", config.energy_demand);
    printf("  ENERGY_EXPLODE_THRESHOLD: %d\n", config.energy_explode_threshold);
    printf("  SIM_DURATION: %ld seconds\n", config.sim_duration);
    printf("  STEP: %ld nanoseconds\n", config.step);
    printf("  N_NUOVI_ATOMI: %d\n", config.n_nuovi_atomi);
    printf("\n");

    /* Create IPC resources */
    shm_id = create_shared_memory();
    if (shm_id == -1) {
        fprintf(stderr, "Failed to create shared memory\n");
        exit(EXIT_FAILURE);
    }

    sem_id = create_semaphores();
    if (sem_id == -1) {
        destroy_shared_memory(shm_id);
        fprintf(stderr, "Failed to create semaphores\n");
        exit(EXIT_FAILURE);
    }

    msg_id = create_message_queue();
    if (msg_id == -1) {
        destroy_semaphores(sem_id);
        destroy_shared_memory(shm_id);
        fprintf(stderr, "Failed to create message queue\n");
        exit(EXIT_FAILURE);
    }

    /* Attach to shared memory */
    stats = attach_shared_memory(shm_id);
    if (stats == NULL) {
        destroy_message_queue(msg_id);
        destroy_semaphores(sem_id);
        destroy_shared_memory(shm_id);
        exit(EXIT_FAILURE);
    }

    /* Register cleanup */
    atexit(cleanup_ipc);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Initialize shared memory */
    memset(stats, 0, sizeof(Statistics));
    stats->running = 0;
    stats->termination_cause = TERM_NONE;
    stats->num_atoms = 0;
    stats->init_count = 0;
    stats->init_target = config.n_atomi_init + 2; /* atoms + attivatore + alimentazione */

    /* Initialize semaphores */
    init_semaphores(sem_id);

    /* Seed random number generator */
    srand(time(NULL));

    /* Create initial atoms */
    printf("Creating %d initial atoms...\n", config.n_atomi_init);
    for (int i = 0; i < config.n_atomi_init; i++) {
        int atomic_number = (rand() % config.n_atom_max) + 1;

        if (create_atom(atomic_number) == -1) {
            fprintf(stderr, "Failed to create atom %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    /* Create attivatore process */
    printf("Creating attivatore process...\n");
    pid_t attivatore_pid = fork();
    if (attivatore_pid == -1) {
        perror("fork attivatore failed");
        exit(EXIT_FAILURE);
    } else if (attivatore_pid == 0) {
        char shm_str[32], sem_str[32], msg_str[32];
        snprintf(shm_str, sizeof(shm_str), "%d", shm_id);
        snprintf(sem_str, sizeof(sem_str), "%d", sem_id);
        snprintf(msg_str, sizeof(msg_str), "%d", msg_id);

        execl("./attivatore", "attivatore", shm_str, sem_str, msg_str, (char*)NULL);
        perror("execl attivatore failed");
        exit(EXIT_FAILURE);
    }

    /* Create alimentazione process */
    printf("Creating alimentazione process...\n");
    pid_t alimentazione_pid = fork();
    if (alimentazione_pid == -1) {
        perror("fork alimentazione failed");
        exit(EXIT_FAILURE);
    } else if (alimentazione_pid == 0) {
        char shm_str[32], sem_str[32], msg_str[32];
        snprintf(shm_str, sizeof(shm_str), "%d", shm_id);
        snprintf(sem_str, sizeof(sem_str), "%d", sem_id);
        snprintf(msg_str, sizeof(msg_str), "%d", msg_id);

        execl("./alimentazione", "alimentazione", shm_str, sem_str, msg_str, (char*)NULL);
        perror("execl alimentazione failed");
        exit(EXIT_FAILURE);
    }

    /* Wait for all processes to initialize */
    printf("Waiting for all processes to initialize...\n");
    while (1) {
        sem_wait_op(sem_id, SEM_STATS);
        int init_count = stats->init_count;
        sem_signal_op(sem_id, SEM_STATS);

        if (init_count >= stats->init_target) {
            break;
        }

        usleep(100000); /* 100ms */
    }

    printf("All processes initialized. Starting simulation...\n\n");

    /* Start simulation */
    stats->running = 1;
    start_time = time(NULL);

    /* Main loop */
    while (1) {
        /* Sleep for 1 second */
        sleep(1);

        /* Print statistics */
        print_stats();

        /* Consume energy */
        consume_energy();

        /* Check termination conditions */
        if (check_termination()) {
            break;
        }
    }

    /* Print termination cause */
    printf("\n=== Simulation Terminated ===\n");
    switch (stats->termination_cause) {
        case TERM_TIMEOUT:
            printf("Cause: TIMEOUT - Simulation duration reached\n");
            break;
        case TERM_EXPLODE:
            printf("Cause: EXPLODE - Energy threshold exceeded\n");
            break;
        case TERM_BLACKOUT:
            printf("Cause: BLACKOUT - Insufficient energy\n");
            break;
        case TERM_MELTDOWN:
            printf("Cause: MELTDOWN - Fork failure\n");
            break;
        default:
            printf("Cause: UNKNOWN\n");
            break;
    }
    printf("=============================\n\n");

    /* Print final statistics */
    print_stats();

    return 0;
}
