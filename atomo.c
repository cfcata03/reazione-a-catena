#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "shared.h"
#include "config.h"

static int shm_id, sem_id, msg_id;
static Statistics* stats;
static int atomic_number;

/* Calculate energy from fission */
long calculate_energy(int n1, int n2) {
    int max_n = (n1 > n2) ? n1 : n2;
    return (long)n1 * n2 - max_n;
}

/* Split the atom */
void split_atom(void) {
    if (atomic_number <= config.min_n_atomico) {
        /* Atom becomes waste */
        update_stats_waste(stats, sem_id);
        exit(EXIT_SUCCESS);
    }

    /* Calculate split - try to split evenly for maximum energy */
    int n1, n2;
    if (atomic_number % 2 == 0) {
        n1 = atomic_number / 2;
        n2 = atomic_number / 2;
    } else {
        n1 = atomic_number / 2;
        n2 = atomic_number / 2 + 1;
    }

    /* Calculate energy before fork */
    long energy = calculate_energy(n1, n2);

    /* Fork new atom */
    pid_t pid = fork();

    if (pid == -1) {
        /* Fork failed - meltdown */
        perror("fork failed in atomo");
        sem_wait_op(sem_id, SEM_STATS);
        stats->termination_cause = TERM_MELTDOWN;
        stats->running = 0;
        sem_signal_op(sem_id, SEM_STATS);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        /* Child process - new atom */
        atomic_number = n2;

        /* Increment atom count */
        sem_wait_op(sem_id, SEM_ATOMS);
        stats->num_atoms++;
        sem_signal_op(sem_id, SEM_ATOMS);
    } else {
        /* Parent process */
        atomic_number = n1;
    }

    /* Update statistics */
    update_stats_split(stats, sem_id);
    update_stats_energy(stats, sem_id, energy);
}

void cleanup(void) {
    if (stats != NULL) {
        /* Decrement atom count */
        sem_wait_op(sem_id, SEM_ATOMS);
        stats->num_atoms--;
        sem_signal_op(sem_id, SEM_ATOMS);

        detach_shared_memory(stats);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <shm_id> <sem_id> <msg_id> <atomic_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    shm_id = atoi(argv[1]);
    sem_id = atoi(argv[2]);
    msg_id = atoi(argv[3]);
    atomic_number = atoi(argv[4]);

    /* Attach to shared memory */
    stats = attach_shared_memory(shm_id);
    if (stats == NULL) {
        exit(EXIT_FAILURE);
    }

    /* Register cleanup */
    atexit(cleanup);

    /* Load configuration */
    load_config();

    /* Increment atom count */
    sem_wait_op(sem_id, SEM_ATOMS);
    stats->num_atoms++;
    sem_signal_op(sem_id, SEM_ATOMS);

    /* Signal initialization complete */
    sem_wait_op(sem_id, SEM_STATS);
    stats->init_count++;
    sem_signal_op(sem_id, SEM_STATS);

    pid_t my_pid = getpid();

    /* Wait for simulation to start */
    while (1) {
        sem_wait_op(sem_id, SEM_STATS);
        int running = stats->running;
        sem_signal_op(sem_id, SEM_STATS);

        if (running) {
            break;
        }

        usleep(10000); /* 10ms */
    }

    /* Main loop - wait for split messages */
    while (1) {
        Message msg;

        /* Check if simulation is still running */
        sem_wait_op(sem_id, SEM_STATS);
        int running = stats->running;
        sem_signal_op(sem_id, SEM_STATS);

        if (!running) {
            break;
        }

        /* Try to receive split message */
        if (receive_message(msg_id, &msg, MSG_SPLIT) == 0) {
            /* Check if this message is for us or for any atom */
            if (msg.target_pid == 0 || msg.target_pid == my_pid) {
                split_atom();
            }
        } else {
            /* Check again if we should terminate */
            sem_wait_op(sem_id, SEM_STATS);
            running = stats->running;
            sem_signal_op(sem_id, SEM_STATS);

            if (!running) {
                break;
            }
        }
    }

    return 0;
}
