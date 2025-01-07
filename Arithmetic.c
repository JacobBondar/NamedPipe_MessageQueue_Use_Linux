/*
 * Ex #3b: Arithmetic Expression Processor via Message Queue
 * ==========================================================
 * Written by: Jacob Bondar. 
 *
 * This program receives arithmetic expressions from clients through
 * a message queue, evaluates the result of the expressions, and sends
 * the results back to the clients. It operates in an infinite loop
 * until terminated with a SIGINT signal, at which point the message
 * queue is safely removed.
 *
 * Input: Arithmetic expressions in the format
 * 		  "<integer><operator><integer>" received via the message queue.
 * Output: Evaluation results sent back to the respective client.
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

//-------------- global variables --------------------------------------

int msgid;

//-------------- prototypes section ------------------------------------

void do_work(struct msg* my_msg, int msgid);
void calc_value(const char sentence[], char result[]);
int do_action(int first_value, int second_value, char action);
void finish_program(int sig);

//-------------- main --------------------------------------------------

int main()
{
    signal(SIGINT, finish_program);

    key_t key;
    struct msg my_msg;

    if ((key = ftok(".", 'a')) == -1)
    {
        perror("Can't fork\n");
        exit(EXIT_FAILURE);
    }

    if ((msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0600)) == -1)
    {
        perror("Cant msgget\n");
        exit(EXIT_FAILURE);
    }


    while (1)
    {
        do_work(&my_msg, msgid);
    }

    exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* Processes a single message: receives,evaluates, and sends the result.
 * The function receives: a pointer to the message structure and the
 * message queue ID.
 * The function returns: void.
 */
void do_work(struct msg* my_msg, int msgid)
{
    if (msgrcv(msgid,
        my_msg,
        MAX_SIZE,
        ALLOWED,
        0) == -1)
    {
        perror("msgrcv failed\n");
        exit(EXIT_FAILURE);
    }

    char result[MAX_SIZE];
    calc_value(my_msg->_context._msg, result);

    my_msg->_type = my_msg->_context._process_number;
    strcpy(my_msg->_context._msg, result);

    if (msgsnd(msgid,
        my_msg,
        MAX_SIZE,
        0) == -1)
    {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
}

//----------------------------------------------------------------------

/* Evaluates an arithmetic expression in the format "<int><op><int>".
 * The function receives: an arithmetic expression string and a buffer
 * for the result.
 * The function returns: void.
 */
void calc_value(const char sentence[], char result[])
{
    int index = 0;
    int first_value = 0, second_value = 0;
    char action;
    char temp_hold_value[MAX_SIZE];

    first_value = atoi(sentence);

    while (sentence[index] != '+' &&
        sentence[index] != '-' &&
        sentence[index] != '*' &&
        sentence[index] != '/')
    {
        index++;
    }

    action = sentence[index];

    index += 2;
    int index_temp = 0;
    while (sentence[index] != '\0')
    {
        temp_hold_value[index_temp] = sentence[index];
        index_temp++;
        index++;
    }
    temp_hold_value[index_temp] = '\0';
    second_value = atoi(temp_hold_value);

    int num = do_action(first_value, second_value, action);

    snprintf(result, MAX_SIZE, "%d", num);
}

//----------------------------------------------------------------------

/* Performs the specified arithmetic operation on two integers.
 * The function receives: two integers and an operator character.
 * The function returns: the result of the arithmetic operation.
 */
int do_action(int first_value, int second_value, char action)
{
    switch (action)
    {
    case '+':
    {
        return first_value + second_value;
        break;
    }

    case '-':
    {
        return first_value - second_value;
        break;
    }

    case '*':
    {
        return first_value * second_value;
        break;
    }

    case '/':
    {
        return first_value / second_value;
        break;
    }
    }
    return 0;
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

