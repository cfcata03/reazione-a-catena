#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "shared.h"
#include "config.h"

static int shm_id, sem_id, msg_id;
static Statistics* stats;

void cleanup(void) {
    if (stats != NULL) {
        detach_shared_memory(stats);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <shm_id> <sem_id> <msg_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    shm_id = atoi(argv[1]);
    sem_id = atoi(argv[2]);
    msg_id = atoi(argv[3]);

    /* Attach to shared memory */
    stats = attach_shared_memory(shm_id);
    if (stats == NULL) {
        exit(EXIT_FAILURE);
    }

    /* Register cleanup */
    atexit(cleanup);

    /* Load configuration */
    load_config();

    /* Signal initialization complete */
    sem_wait_op(sem_id, SEM_STATS);
    stats->init_count++;
    sem_signal_op(sem_id, SEM_STATS);

    /* Seed random number generator */
    srand(time(NULL) ^ getpid());

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

    /* Main loop - activate atoms periodically */
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = 100000000; /* 100ms */

    while (1) {
        /* Check if simulation is still running */
        sem_wait_op(sem_id, SEM_STATS);
        int running = stats->running;
        int num_atoms = stats->num_atoms;
        sem_signal_op(sem_id, SEM_STATS);

        if (!running) {
            break;
        }

        /* Activate atoms if there are any */
        if (num_atoms > 0) {
            /* Decide how many atoms to activate (1-3) */
            int num_activations = (rand() % 3) + 1;

            for (int i = 0; i < num_activations; i++) {
                /* Send split message to any atom (target_pid = 0) */
                if (send_message(msg_id, MSG_SPLIT, 0, 0) == 0) {
                    update_stats_activation(stats, sem_id);
                }
            }
        }

        /* Sleep to avoid busy waiting */
        nanosleep(&sleep_time, NULL);
    }

    return 0;
}
