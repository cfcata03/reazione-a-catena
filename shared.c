#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int create_shared_memory(void) {
    int shm_id = shmget(SHM_KEY, sizeof(Statistics), IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id == -1) {
        perror("shmget");
        return -1;
    }
    return shm_id;
}

Statistics* attach_shared_memory(int shm_id) {
    Statistics* stats = (Statistics*)shmat(shm_id, NULL, 0);
    if (stats == (Statistics*)-1) {
        perror("shmat");
        return NULL;
    }
    return stats;
}

void detach_shared_memory(Statistics* stats) {
    if (shmdt(stats) == -1) {
        perror("shmdt");
    }
}

void destroy_shared_memory(int shm_id) {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
    }
}

int create_semaphores(void) {
    int sem_id = semget(SEM_KEY, NUM_SEMS, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id == -1) {
        perror("semget");
        return -1;
    }
    return sem_id;
}

void init_semaphores(int sem_id) {
    union semun arg;
    arg.val = 1;

    for (int i = 0; i < NUM_SEMS; i++) {
        if (semctl(sem_id, i, SETVAL, arg) == -1) {
            perror("semctl SETVAL");
            exit(EXIT_FAILURE);
        }
    }
}

void sem_wait_op(int sem_id, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = -1;
    sb.sem_flg = 0;

    while (semop(sem_id, &sb, 1) == -1) {
        if (errno != EINTR) {
            perror("semop wait");
            exit(EXIT_FAILURE);
        }
    }
}

void sem_signal_op(int sem_id, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = 1;
    sb.sem_flg = 0;

    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop signal");
        exit(EXIT_FAILURE);
    }
}

void destroy_semaphores(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
    }
}

int create_message_queue(void) {
    int msg_id = msgget(MSG_KEY, IPC_CREAT | IPC_EXCL | 0666);
    if (msg_id == -1) {
        perror("msgget");
        return -1;
    }
    return msg_id;
}

int send_message(int msg_id, long mtype, pid_t target_pid, int value) {
    Message msg;
    msg.mtype = mtype;
    msg.target_pid = target_pid;
    msg.value = value;

    if (msgsnd(msg_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
        if (errno != EIDRM && errno != EINVAL) {
            perror("msgsnd");
        }
        return -1;
    }
    return 0;
}

int receive_message(int msg_id, Message* msg, long mtype) {
    if (msgrcv(msg_id, msg, sizeof(Message) - sizeof(long), mtype, 0) == -1) {
        if (errno != EIDRM && errno != EINVAL && errno != EINTR) {
            perror("msgrcv");
        }
        return -1;
    }
    return 0;
}

void destroy_message_queue(int msg_id) {
    if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
        if (errno != EIDRM && errno != EINVAL) {
            perror("msgctl IPC_RMID");
        }
    }
}

void update_stats_energy(Statistics* stats, int sem_id, long energy) {
    sem_wait_op(sem_id, SEM_STATS);
    stats->total_energy_produced += energy;
    stats->last_sec_energy_produced += energy;
    stats->current_energy += energy;
    sem_signal_op(sem_id, SEM_STATS);
}

void update_stats_split(Statistics* stats, int sem_id) {
    sem_wait_op(sem_id, SEM_STATS);
    stats->total_splits++;
    stats->last_sec_splits++;
    sem_signal_op(sem_id, SEM_STATS);
}

void update_stats_waste(Statistics* stats, int sem_id) {
    sem_wait_op(sem_id, SEM_STATS);
    stats->total_waste++;
    stats->last_sec_waste++;
    stats->num_atoms--;
    sem_signal_op(sem_id, SEM_STATS);
}

void update_stats_activation(Statistics* stats, int sem_id) {
    sem_wait_op(sem_id, SEM_STATS);
    stats->total_activations++;
    stats->last_sec_activations++;
    sem_signal_op(sem_id, SEM_STATS);
}
