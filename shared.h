#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <semaphore.h>

/* Keys for IPC resources */
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define MSG_KEY 0x9ABC

/* Message types */
#define MSG_SPLIT 1
#define MSG_INIT_DONE 2
#define MSG_TERMINATE 3

/* Semaphore indices */
#define SEM_STATS 0
#define SEM_ATOMS 1
#define SEM_BARRIER 2
#define NUM_SEMS 3

/* Statistics structure in shared memory */
typedef struct {
    long total_activations;
    long total_splits;
    long total_energy_produced;
    long total_energy_consumed;
    long total_waste;
    long current_energy;

    long last_sec_activations;
    long last_sec_splits;
    long last_sec_energy_produced;
    long last_sec_energy_consumed;
    long last_sec_waste;

    int running;
    int num_atoms;
    int init_count;
    int init_target;

    enum {
        TERM_NONE,
        TERM_TIMEOUT,
        TERM_EXPLODE,
        TERM_BLACKOUT,
        TERM_MELTDOWN
    } termination_cause;
} Statistics;

/* Message structure */
typedef struct {
    long mtype;
    pid_t target_pid;
    int value;
} Message;

/* Shared memory operations */
int create_shared_memory(void);
Statistics* attach_shared_memory(int shm_id);
void detach_shared_memory(Statistics* stats);
void destroy_shared_memory(int shm_id);

/* Semaphore operations */
int create_semaphores(void);
void init_semaphores(int sem_id);
void sem_wait_op(int sem_id, int sem_num);
void sem_signal_op(int sem_id, int sem_num);
void destroy_semaphores(int sem_id);

/* Message queue operations */
int create_message_queue(void);
int send_message(int msg_id, long mtype, pid_t target_pid, int value);
int receive_message(int msg_id, Message* msg, long mtype);
void destroy_message_queue(int msg_id);

/* Utility functions */
void update_stats_energy(Statistics* stats, int sem_id, long energy);
void update_stats_split(Statistics* stats, int sem_id);
void update_stats_waste(Statistics* stats, int sem_id);
void update_stats_activation(Statistics* stats, int sem_id);

#endif
