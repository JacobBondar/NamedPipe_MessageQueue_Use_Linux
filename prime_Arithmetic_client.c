/*
 * Ex #3c: Client for Prime and Arithmetic Servers
 * ===============================================
 * Written by: Jacob Bondar.
 *
 * This program acts as a client for two servers: one that processes
 * prime numbers and another that evaluates arithmetic expressions.
 * Based on user input, the program sends requests to the appropriate
 * server via message queues and displays the results received.
 *
 * Input: User commands ('p' for primes, 'a' for arithmetic) followed
 *        by respective data.
 * Output: Processed results (prime numbers or evaluated expressions)
 *         from the servers.
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

#define MAX_SIZE 1000
const long int ALLOWED = 1;

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

void create_key(key_t key, int* msgid, char c);
int connect_to_queue(char c);
void do_prime(struct msg* my_msg, int msgid);
void do_math(struct msg* my_msg, int msgid);

//-------------- main --------------------------------------------------

int main()
{
    struct msg my_msg;
    my_msg._context._process_number = getpid();
    my_msg._type = ALLOWED;

    int msgid_prime = connect_to_queue('p');
    int msgid_action = connect_to_queue('a');

    while (1)
    {
        char c;
        scanf("%c", &c);

        if (c == 'p')
        {
            do_prime(&my_msg, msgid_prime);
        }

        else if (c == 'a')
        {
            do_math(&my_msg, msgid_action);
        }
    }

    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* Connects to the specified server's message queue.
 * The function receives: a character representing the server
 *  ('p' or 'a').
 * The function returns: the message queue ID.
 */
int connect_to_queue(char c)
{
    key_t key_prime = 0, key_action = 0;
    int msgid;

    if (c == 'p')
    {
        create_key(key_prime, &msgid, c);
    }

    else if (c == 'a')
    {
        create_key(key_action, &msgid, c);
    }
    return msgid;
}

//----------------------------------------------------------------------

/* Creates a key and connects to the specified server's message queue.
 * The function receives: a reference to the key, a reference to the
 * 	message queue ID,
 * and a character representing the server.
 * The function returns: void.
 */
void create_key(key_t key, int* msgid, char c)
{
    if ((key = ftok(".", c)) == -1)
    {
        perror("Can't fork\n");
        exit(EXIT_FAILURE);
    }

    if ((*msgid = msgget(key, 0)) == -1)
    {
        perror("Cant msgget\n");
        exit(EXIT_FAILURE);
    }
}

//----------------------------------------------------------------------

/* Sends a list of numbers to the prime number server for processing.
 * The function receives: a pointer to the message structure and the
 * message queue ID.
 * The function returns: void.
 */
void do_prime(struct msg* my_msg, int msgid)
{
    int num;
    char text[MAX_SIZE];
    char final[MAX_SIZE];
    memset(final, 0, MAX_SIZE);

    while (scanf("%d", &num))
    {
        if (num == 0) break;
        snprintf(text, MAX_SIZE, "%d ", num);

        strncat(final, text, MAX_SIZE);
    }
    final[strlen(final) - 1] = '\0'; // erase last space
    strcpy(my_msg->_context._msg, final);

    if (msgsnd(msgid,
        my_msg,
        MAX_SIZE,
        0) == -1)
    {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msgid,
        my_msg,
        MAX_SIZE,
        getpid(),
        0) == -1)
    {
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", my_msg->_context._msg);
    my_msg->_type = ALLOWED;
}


//----------------------------------------------------------------------

/* Sends an arithmetic expression to the arithmetic server for
 * 	processing.
 * The function receives: a pointer to the message structure and the
 * message queue ID.
 * The function returns: void.
 */
void do_math(struct msg* my_msg, int msgid)
{
    getchar(); // remove enter
    fgets(my_msg->_context._msg, MAX_SIZE, stdin);

    if (msgsnd(msgid,
        my_msg,
        MAX_SIZE,
        0) == -1)
    {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msgid,
        my_msg,
        MAX_SIZE,
        getpid(),
        0) == -1)
    {
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }

    printf("%d\n", atoi(my_msg->_context._msg));
    my_msg->_type = ALLOWED;
}
