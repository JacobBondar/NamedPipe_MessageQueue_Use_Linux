/*
 * Ex #3a: Prime Number Filter via Message Queue
 * ==========================================================
 * Written by: Jacob Bondar.
 *
 * This program creates a message queue to receive sentences from
 * clients, processes the sentences to extract prime numbers, and
 * sends the processed result back to the clients. The program runs
 * continuously until terminated using a SIGINT signal, at which point
 * the message queue is safely removed.
 *
 * Input: Sentences received via message queue containing
 * 		  space-separated integers.
 * Output: Sentences containing only prime numbers sent back to the
 * 		   clients.
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

//-------------- const and define section ------------------------------

#define MAX_SIZE 1000
const long int ALLOWED = 1;

//-------------- global variables --------------------------------------

int msgid;

//-------------- struct section ----------------------------------------

struct data
{
    pid_t _process_number;
    char _msg[MAX_SIZE];
};

struct msg
{
    long _type;
    struct data _context;
};

//-------------- prototypes section ------------------------------------

void start_run(int msgid);
void finish_program(int sig);
bool is_prime(int number);
void update_prime(const char sentence[], char return_msg[]);

//-------------- main --------------------------------------------------

int main()
{
    signal(SIGINT, finish_program);

    key_t key;

    if ((key = ftok(".", 'p')) == -1)
    {
        perror("Can't fork\n");
        exit(EXIT_FAILURE);
    }

    if ((msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0600)) == -1)
    {
        perror("Cant msgget\n");
        exit(EXIT_FAILURE);
    }

    start_run(msgid);

    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function recieves and sends the result of the prime answer.
 * The function receives: name of the queue.
 * The function returns: void.
 */
void start_run(int msgid)
{
    struct msg my_msg;

    while (1)
    {
        if (msgrcv(msgid,
            &my_msg,
            MAX_SIZE,
            ALLOWED,
            0) == -1)
        {
            perror("msgrcv failed\n");
            exit(EXIT_FAILURE);
        }

        char return_msg[MAX_SIZE];
        memset(return_msg, 0, MAX_SIZE);

        update_prime(my_msg._context._msg, return_msg);

        my_msg._type = my_msg._context._process_number;
        strcpy(my_msg._context._msg, return_msg);

        if (msgsnd(msgid,
            &my_msg,
            MAX_SIZE,
            0) == -1)
        {
            perror("msgsnd failed");
            exit(EXIT_FAILURE);
        }
    }
}

//----------------------------------------------------------------------

/* Extracts prime numbers from a space-separated sentence.
 * The function receives: a sentence containing integers (as a string)
 * and a buffer to store the extracted primes.
 * The function returns: void.
 */
void update_prime(const char sentence[], char return_msg[])
{
    int index = 0, prime_index = 0;
    char prime_arr[MAX_SIZE];

    while (1)
    {
        if (sentence[index] == ' ' || sentence[index] == '\0')
        {
            prime_arr[prime_index] = ' ';
            prime_arr[prime_index + 1] = '\0';

            if (is_prime(atoi(prime_arr)))
            {
                strncat(return_msg, prime_arr, strlen(prime_arr));
            }
            prime_index = 0;

            if (sentence[index] == '\0')
            {
                break;
            }
        }

        else
        {
            prime_arr[prime_index] = sentence[index];
            prime_index++;
        }

        index++;
    }
}

//----------------------------------------------------------------------

/* Determines whether a number is prime.
 * The function receives: an integer to check.
 * The function returns: true if the number is prime, false otherwise.
 */
bool is_prime(int number)
{
    for (int i = 2; i * i <= number; i++)
    {
        if (number % i == 0)
        {
            return false;
        }
    }
    return true;
}

//----------------------------------------------------------------------

/* Handles program termination by removing the message queue.
 * The function receives: the signal number (SIGINT).
 * The function returns: void.
 */
void finish_program(int sig)
{
    if (msgctl(msgid, IPC_RMID, NULL) == -1)
    {
        perror("msgctl failed");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
