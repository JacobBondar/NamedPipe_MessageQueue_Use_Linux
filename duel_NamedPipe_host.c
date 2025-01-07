/*
 * Ex #1a: A duel for the higher value (The host).
 * ==========================================================
 * Written by: Jacob Bondar.
 *
 * This program simulates a competitive game between two processes
 * managed by a host process. The HOST compares between two inputs he
 * recieved from the processes and determine a winner for each round.
 * Scores are updated based on the results, and the game ends when one
 * player wins 120 rounds or both achieve 100 ties.
 *
 * Input: In the vector arguments (argv) we recieve 3 things:
 * 			1. The name of the file, for communication from the
 * 				processes to the host.
 * 			2. The name of the file, for communication from the host
 * 				 to the first process.
 * 			3. The name of the file, for communication from the host
 * 				 to the second process.
 *
 * Output: The winner (if there is one), and the children summary.
 */

 //-------------- include section ---------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

//-------------- const section -----------------------------------------
const int EQUAL = 0;
const int HIGHER = 1;
const int LOWER = -1;
const int WON = 120;
const int FINISED = 100;
const int DIVIDE = 10;
const int NUM_OF_PLAYERS = 2;
const int FIRST_PROCESS = 0;
const int SECOND_PROCESS = 1;

//-------------- prototypes section ------------------------------------

void kill_process(int sig);
void send_results(FILE* fdr, int result);
void run(FILE* fdw_to_dad, const char main_file[],
	const char first_file[], const char second_file[]);
void get_pid(pid_t* first_player, pid_t* second_player, FILE* fd);
void update_values(int* won, int* first_result, int* second_result);
void print_won(pid_t first_player, pid_t second_player, int player_id);
void is_open(FILE* fd);
void open_files(FILE** fdr_first_player, FILE** fdr_second_player,
	const char first_file[], const char second_file[]);
void close_files(FILE* fdw_to_dad, FILE* fdr_first_player,
	FILE* fdr_second_player, const char main_file[],
	const char first_file[], const char second_file[]);
void read_data(FILE* fdw_to_dad, int* first_value, int* second_value);

//-------------- global variables --------------------------------------

bool finish = false;

//-------------- main --------------------------------------------------

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		perror("Not enough arguments!");
		exit(EXIT_FAILURE);
	}

	signal(SIGUSR1, kill_process);

	FILE* fdw_to_dad = fopen(argv[1], "r");
	is_open(fdw_to_dad);

	run(fdw_to_dad, argv[1], argv[2], argv[3]);

	exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function updates the values of the result and win counters for
 *  the players based on the game outcome.
 * The function receives: pointers to the win counter and the results
 *  of the two players.
 * The function returns: void.
 */
void update_values(int* won, int* first_result, int* second_result)
{
	(*won)++;
	*first_result = LOWER;
	*second_result = HIGHER;
}

//----------------------------------------------------------------------

/* The function prints a message when a player process wins and sends
 *  a termination signal to both players processes.
 * The function receives: two player process IDs and the ID of the
 *  winning child.
 * The function returns: void.
 */
void print_won(pid_t first_player, pid_t second_player, int player_id)
{
	printf("player #%d won\n", player_id);
	kill(first_player, SIGUSR1);
	kill(second_player, SIGUSR1);
	finish = true;
}

//----------------------------------------------------------------------

/* The function runs the whole game, recieves input, compares the to
 *  inputs, sends back the result and continues until certain condition.
 * The function receives: File, and 3 names of files.
 * The function returns: void.
 */
void run(FILE* fdw_to_dad, const char main_file[],
	const char first_file[], const char second_file[])
{
	pid_t first_player = 0, second_player = 0;
	FILE* fdr_first_player = NULL, * fdr_second_player = NULL;
	get_pid(&first_player, &second_player, fdw_to_dad);

	open_files(&fdr_first_player, &fdr_second_player,
		first_file, second_file);

	int first_value = 0, second_value = 0, first_won = 0, second_won = 0
		, first_result = 0, second_result = 0;
	while (!finish)
	{
		first_result = 0, second_result = 0;

		read_data(fdw_to_dad, &first_value, &second_value);
		read_data(fdw_to_dad, &first_value, &second_value);

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
			print_won(first_player, second_player, 0);

		}
		else if (second_won == WON)
		{
			print_won(second_player, first_player, 1);
		}

		send_results(fdr_first_player, first_result);
		send_results(fdr_second_player, second_result);
	}
	sleep(1); // Wait for the termination of the players

	close_files(fdw_to_dad, fdr_first_player, fdr_second_player,
		main_file, first_file, second_file);
}

//----------------------------------------------------------------------

/* The function handles SIGUSR1 signals by setting a global flag to
 * indicate termination.
 * The function receives: the signal number.
 * The function returns: void.
 */
void kill_process(int sig)
{
	finish = true;
}

//----------------------------------------------------------------------

/* The function insures that the file was opened correctly.
 * The function receives: The file.
 * The function returns: void.
 */
void is_open(FILE* fd)
{
	if (!fd)
	{
		perror("Can't open file!\n");
		exit(EXIT_FAILURE);
	}
}

//----------------------------------------------------------------------

/* The function receieves and saves the two pids of the players.
 * The function receives: 2 references for the pids, and a file.
 * The function returns: void.
 */
void get_pid(pid_t* first_player, pid_t* second_player, FILE* fd)
{
	int id = 0;
	pid_t process = 0;

	for (int round = 0; round < NUM_OF_PLAYERS; round++)
	{
		fscanf(fd, " %d %d", &id, &process);
		if (id == FIRST_PROCESS)
		{
			*first_player = process;
		}
		else if (id == SECOND_PROCESS)
		{
			*second_player = process;
		}
	}
}

//----------------------------------------------------------------------

/* The function opens the files.
 * The function receives: 2 references to files and 2 file names.
 * The function returns: void.
 */
void open_files(FILE** fdr_first_player, FILE** fdr_second_player,
	const char first_file[], const char second_file[])
{
	*fdr_first_player = fopen(first_file, "w");
	is_open(*fdr_first_player);
	*fdr_second_player = fopen(second_file, "w");
	is_open(*fdr_second_player);
}

//----------------------------------------------------------------------

/* The function closes the files.
 * The function receives: 3 files and 3 file names.
 * The function returns: void.
 */
void close_files(FILE* fdw_to_dad, FILE* fdr_first_player,
	FILE* fdr_second_player, const char main_file[],
	const char first_file[], const char second_file[])
{
	fclose(fdw_to_dad);
	fclose(fdr_first_player);
	fclose(fdr_second_player);

	unlink(main_file);
	unlink(first_file);
	unlink(second_file);
}

//----------------------------------------------------------------------

/* The function sends the results to the players.
 * The function receives: File and the result.
 * The function returns: void.
 */
void send_results(FILE* fdr, int result)
{
	fprintf(fdr, "%d ", result);
	fflush(fdr);
}

//----------------------------------------------------------------------

/* The function reads data from the players.
 * The function receives: File and 2 references for values.
 * The function returns: void.
 */
void read_data(FILE* fdw_to_dad, int* first_value, int* second_value)
{
	int id = 0, temp = 0;
	fscanf(fdw_to_dad, "%d %d", &id, &temp);
	if (id == FIRST_PROCESS)
	{
		*first_value = temp;
	}
	else if (id == SECOND_PROCESS)
	{
		*second_value = temp;
	}
}
