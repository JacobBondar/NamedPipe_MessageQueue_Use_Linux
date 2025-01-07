/*
 * Ex #2a: A duel for the higher value (The host).
 * ==========================================================
 * Written by: Jacob Bondar.
 *
 * This program simulates a competitive game between two processes
 * managed by a host process. The host compares between two inputs he
 * recieved from the processes and determine a winner for each round.
 * Scores are updated based on the results, and the game ends when one
 * player wins 120 rounds or both achieve 100 ties.
 *
 * Input: void.
 *
 * Output: The winner (if there is one), and the children summary.
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

//-------------- const section -----------------------------------------
const int EQUAL = 0;
const int HIGHER = 1;
const int LOWER = -1;
const int WON = 120;
const int FINISED = 100;
const int DIVIDE = 10;
const int NUM_OF_PLAYERS = 2;
const int ALLOWED = 0;
const long FIRST_PROCESS = 1;
const long SECOND_PROCESS = 2;
#define MAX_SIZE 1000

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

void run(int msgid);
void finish_program(int sig);
void send_info(struct msg* my_msg, long id, int result, int msgid,
	pid_t process);
int recieve_value(struct msg* my_msg, int msgid, long id);
void update_values(int* won, int* first_result, int* second_result);
void print_won(int id, pid_t first_process, pid_t second_process);
pid_t recieve_pid(struct msg* my_msg, int msgid, int id);

bool finish = false;

//-------------- main --------------------------------------------------

int main()
{
	key_t key;
	int msgid;

	if ((key = ftok(".", '4')) == -1)
	{
		perror("Can't fork\n");
		exit(EXIT_FAILURE);
	}

	if ((msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0600)) == -1)
	{
		perror("Cant msgget\n");
		exit(EXIT_FAILURE);
	}

	run(msgid);

	exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function updates the values of the result and win counters for
 *  the children based on the game outcome.
 * The function receives: pointers to the win counter and the results
 *  of the two children.
 * The function returns: void.
 */
void update_values(int* won, int* first_result, int* second_result)
{
	(*won)++;
	*first_result = LOWER;
	*second_result = HIGHER;
}

//----------------------------------------------------------------------

/* The function prints a message when a child process wins and sends
 *  a termination signal to both child processes.
 * The function receives: two child process IDs and the ID of the
 *  winning child.
 * The function returns: void.
 */
void print_won(int id, pid_t first_process, pid_t second_process)
{
	printf("player #%d won\n", id);
	kill(first_process, SIGUSR1);
	kill(second_process, SIGUSR1);
	finish = true;
}

//----------------------------------------------------------------------

/* The function handles the actions of the parent process, including
 *  reading data from child processes and determining the winner.
 * The function receives: two child process IDs and three pipes for
 *  communication.
 * The function returns: void.
 */
void run(int msgid)
{
	struct msg my_msg;
	int first_result = 0, second_result = 0, first_won = 0,
		second_won = 0, first_value = 0, second_value = 0;

	pid_t first_process = recieve_pid(&my_msg, msgid, FIRST_PROCESS);
	pid_t second_process = recieve_pid(&my_msg, msgid, SECOND_PROCESS);

	while (!finish)
	{
		first_result = 0, second_result = 0;

		first_value = recieve_value(&my_msg, msgid, FIRST_PROCESS);
		second_value = recieve_value(&my_msg, msgid, SECOND_PROCESS);

		if (second_value > first_value)
		{
			update_values(&second_won, &first_result, &second_result);
		}

		else if (second_value < first_value)
		{
			update_values(&first_won, &second_result, &first_result);
		}

		if (first_won == WON)
		{
			print_won(0, first_process, second_process);
		}

		else if (second_won == WON)
		{
			print_won(1, first_process, second_process);
		}

		send_info(&my_msg, FIRST_PROCESS, first_result, msgid,
			first_process);
		send_info(&my_msg, SECOND_PROCESS, second_result, msgid,
			second_process);
	}
	sleep(1);
	if (msgctl(msgid, IPC_RMID, NULL) == -1)
	{
		perror("msgctl failed");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
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

/* The function reads the value from the input.
 * The function receives: reference to struct, name of queue and id.
 * The function returns: the number it recieved.
 */
int recieve_value(struct msg* my_msg, int msgid, long id)
{
	if (msgrcv(msgid, my_msg, sizeof(struct data), id, 0) == -1)
	{
		perror("msgrcv failed\n");
		exit(EXIT_FAILURE);
	}

	if (my_msg->_context._num == -1)
	{
		if (msgctl(msgid, IPC_RMID, NULL) == -1)
		{
			perror("msgctl failed");
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}

	return my_msg->_context._num;
}

//----------------------------------------------------------------------

/* The function sends the data to the players.
 * The function receives: reference to struct, name of queue, id and
 *  pid of the player.
 * The function returns: void.
 */
void send_info(struct msg* my_msg, long id, int result, int msgid,
	pid_t process)
{
	my_msg->_context._num = result;
	my_msg->_type = process;
	my_msg->_context._id = id;

	if (msgsnd(msgid,
		my_msg,
		sizeof(struct data),
		0) == -1)
	{
		perror("msgsnd failed");
		exit(EXIT_FAILURE);
	}
}

//----------------------------------------------------------------------

/* The function reads the pid of the player from the input.
 * The function receives: reference to struct, name of queue and id.
 * The function returns: the process number it recieved.
 */
pid_t recieve_pid(struct msg* my_msg, int msgid, int id)
{
	if (msgrcv(msgid, my_msg, sizeof(struct data), id, 0) == -1)
	{
		perror("msgrcv failed\n");
		exit(EXIT_FAILURE);
	}

	my_msg->_type = my_msg->_context._pid;
	if (msgsnd(msgid, my_msg, sizeof(struct data), 0) == -1)
	{
		perror("msgsnd failed");
		exit(EXIT_FAILURE);
	}

	return my_msg->_context._pid;
}
