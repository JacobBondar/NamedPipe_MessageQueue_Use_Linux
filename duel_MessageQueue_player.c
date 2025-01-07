/*
 * Ex #2b: A duel for the higher value (The players).
 * ==========================================================
 * Written by: Jacob Bondar.
 *
 * This program implements a process's role in the competitive game.
 * Each process generates random numbers and sends them to the host
 * process via queue. It then receives feedback from the host to update
 * counters for wins, losses, and ties. The process continues until a
 * termination signal is received or a specific condition is met, at
 * which point the child logs its summary and exits.
 *
 * Input: In the vector arguments (argv) we recieve 2 things:
 * 			1. The id of the process.
 * 			2. The seed.
 *
 * Output: The summary of the process points.
 */

 //-------------- include section ---------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

//-------------- const and define section ------------------------------

const int EQUAL = 0;
const int HIGHER = 1;
const int LOWER = -1;
const int FINISED = 100;
const int DIVIDE = 10;
const long ALLOWED = 1;
#define MAX_SIZE 1000

//-------------- struct section ----------------------------------------

struct data
{
    pid_t _pid;
    long _id;
    int _num;
};

struct msg
{
    long _type;
    struct data _context;
};

//-------------- prototypes section ------------------------------------

void start_run(int msgid, long id);
void finish_program(int sig);
void update_counters_after_read(int num_got, int* zero_count,
    int* one_count, int* minus_one_count);
void print_killed(long id, int minus_one_count, int zero_count,
    int one_count, int msgid, struct msg* my_msg);
void send_pid(struct msg* my_msg, int msgid, int id, pid_t process);
void update_members(struct msg* my_msg, int id, pid_t process);

//-------------- global variables --------------------------------------

bool finish = false;

//-------------- main --------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 3) // 
    {
        perror("Not enough arguments!");
        exit(EXIT_FAILURE);
    }
    signal(SIGUSR1, finish_program);

    key_t key;
    int msgid;

    if ((key = ftok(".", '4')) == -1)
    {
        perror("Can't fork\n");
        exit(EXIT_FAILURE);
    }

    if ((msgid = msgget(key, 0)) == -1)
    {
        perror("Cant msgget\n");
        exit(EXIT_FAILURE);
    }

    long id = atoi(argv[1]) + 1; // so we dont get 0 in allowed type
    srand(atoi(argv[2]));

    start_run(msgid, id);

    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function updates counters based on the value received from the
 *  parent process.
 * The function receives: the received value and pointers to three
 *  counters for `-1`, `0`, and `1`.
 * The function returns: void.
 */
void update_counters_after_read(int num_got, int* zero_count,
    int* one_count, int* minus_one_count)
{
    switch (num_got)
    {
    case 0:
    {
        (*zero_count)++;
        break;
    }

    case 1:
    {
        (*one_count)++;
        break;
    }

    case -1:
    {
        (*minus_one_count)++;
        break;
    }
    }
}

//----------------------------------------------------------------------

/* The function prints a message when a player is killed.
 * The function receives: an integer process ID, counters for each
 * possible read value.
 * The function returns: void.
 */
void print_killed(long id, int minus_one_count, int zero_count,
    int one_count, int msgid, struct msg* my_msg)
{
    printf("Player #%ld was killed: %d %d %d\n", id, minus_one_count,
        zero_count, one_count);
    my_msg->_context._num = -1;
    my_msg->_type = 1;
    if (msgsnd(msgid, my_msg, sizeof(struct data), 0) == -1)
    {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function runs the whole game, generates random number, recieves
 * 	input, and continues until certain condition.
 * The function receives: name of the queue, and the id.
 * The function returns: void.
 */
void start_run(int msgid, long id)
{
    int zero_count = 0, minus_one_count = 0, one_count = 0, num_got = 0;
    struct msg my_msg;
    pid_t process = getpid();
    send_pid(&my_msg, msgid, id, process);

    while (true)
    {
        update_members(&my_msg, id, process);
        my_msg._context._num = rand() % 10;

        if (msgsnd(msgid, &my_msg, sizeof(struct data), 0) == -1)
        {
            perror("msgsnd failed");
            exit(EXIT_FAILURE);
        }

        // blocking the  signal
        // calling the SIGUSR1 can interrupt the call, and the msgrcv
        // will return -1. So we make sure the signal is handled only
        // after the msgrcv
        sigset_t block_mask;
        sigemptyset(&block_mask);
        sigaddset(&block_mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &block_mask, NULL);

        if (msgrcv(msgid, &my_msg, sizeof(struct data), (int)process, 0)
            == -1)
        {
            perror("msgrcv failed\n");
            exit(EXIT_FAILURE);
        }
        sigprocmask(SIG_UNBLOCK, &block_mask, NULL); // free signal

        num_got = my_msg._context._num;
        update_counters_after_read(num_got, &zero_count, &one_count,
            &minus_one_count);


        if (finish || zero_count == FINISED)
        {
            print_killed(--id, minus_one_count, zero_count, one_count,
                msgid, &my_msg);
        }
    }
}

//----------------------------------------------------------------------

/* The function handles SIGUSR1 signals by setting a global flag to
 * indicate termination.
 * The function receives: the signal number.
 * The function returns: void.
 */
void finish_program(int sig)
{
    finish = true;
}

//----------------------------------------------------------------------

/* The function sends the pid of the player to the host.
 * The function receives: reference struct, name of queue, id and pid.
 * The function returns: void.
 */
void send_pid(struct msg* my_msg, int msgid, int id, pid_t process)
{
    update_members(my_msg, id, process);

    if (msgsnd(msgid, my_msg, sizeof(struct data), 0) == -1)
    {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msgid, my_msg, sizeof(struct data), (int)process, 0)
        == -1)
    {
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }
}

//----------------------------------------------------------------------

/* The function updates certain memebers of a struct.
 * The function receives: reference struct, id and pid..
 * The function returns: void.
 */
void update_members(struct msg* my_msg, int id, pid_t process)
{
    my_msg->_context._pid = process;
    my_msg->_type = id;
    my_msg->_context._id = id;
}
