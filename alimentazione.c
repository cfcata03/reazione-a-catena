#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include "shared.h"
#include "config.h"

static int shm_id, sem_id, msg_id;
static Statistics* stats;

void cleanup(void) {
    if (stats != NULL) {
        detach_shared_memory(stats);
    }
}

/* Create a new atom process */
int create_atom(int atomic_number) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed in alimentazione");
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

    /* Main loop - add new atoms periodically */
    struct timespec sleep_time;
    sleep_time.tv_sec = config.step / 1000000000;
    sleep_time.tv_nsec = config.step % 1000000000;

    while (1) {
        /* Check if simulation is still running */
        sem_wait_op(sem_id, SEM_STATS);
        int running = stats->running;
        sem_signal_op(sem_id, SEM_STATS);

        if (!running) {
            break;
        }

        /* Sleep first */
        nanosleep(&sleep_time, NULL);

        /* Check again after sleep */
        sem_wait_op(sem_id, SEM_STATS);
        running = stats->running;
        sem_signal_op(sem_id, SEM_STATS);

        if (!running) {
            break;
        }

        /* Create new atoms */
        for (int i = 0; i < config.n_nuovi_atomi; i++) {
            /* Random atomic number between 1 and N_ATOM_MAX */
            int atomic_number = (rand() % config.n_atom_max) + 1;

            if (create_atom(atomic_number) != 0) {
                /* Fork failed - signal meltdown */
                sem_wait_op(sem_id, SEM_STATS);
                stats->termination_cause = TERM_MELTDOWN;
                stats->running = 0;
                sem_signal_op(sem_id, SEM_STATS);
                break;
            }
        }
    }

    return 0;
}
