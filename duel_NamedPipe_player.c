/*
 * Ex #1b: A duel for the higher value (The players).
 * ==========================================================
 * Written by: Jacob Bondar.
 *
 * This program implements a process's role in the competitive game.
 * Each process generates random numbers and sends them to the host
 * process via fifo. It then receives feedback from the host to update
 * counters for wins, losses, and ties. The process continues until a
 * termination signal is received or a specific condition is met, at
 * which point the child logs its summary and exits.
 *
 * Input: In the vector arguments (argv) we recieve 3 things:
 * 			1. The name of the file, for communication from the
 * 				processes to the host.
 * 			2. The id of the process.
 * 			3. The seed.
 *
 * Output: The summary of the process points.
 */

 //-------------- include section ---------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

//-------------- const and define section ------------------------------
const int EQUAL = 0;
const int HIGHER = 1;
const int LOWER = -1;
const int WON = 120;
const int FINISED = 100;
const int DIVIDE = 10;
const int FIRST_PROCESS = 0;
const int SECOND_PROCESS = 1;
#define FIRST_FILE "fifo1"
#define SECOND_FILE "fifo2"

//-------------- prototypes section ------------------------------------

void print_killed(int id, int minus_one_count, int zero_count,
	int one_count);
void is_open(FILE* fd);
void start_run(FILE* fdw_to_dad, FILE* fdr, int id, char main_file[]);
void kill_process(int sig);
void update_counters_after_read(int num_got, int* zero_count,
	int* one_count, int* minus_one_count);
void open_files(FILE** fdw_to_dad, FILE** fdr, char file_name[], int id);
void close_files(FILE* fdw_to_dad, FILE* fdr, char main_file[], int id);

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

	FILE* fdw_to_dad, * fdr;
	open_files(&fdw_to_dad, &fdr, argv[1], atoi(argv[2]));

	srand(atoi(argv[3]));

	start_run(fdw_to_dad, fdr, atoi(argv[2]), argv[1]);

	// Unreachable code
	exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function runs the whole game, generates random number, recieves
 * 	input, and continues until certain condition.
 * The function receives: 2 files, id and a name of a file.
 * The function returns: void.
 */
void start_run(FILE* fdw_to_dad, FILE* fdr, int id, char main_file[])
{
	int zero_count = 0, minus_one_count = 0, one_count = 0, num_got = 0,
		status = 0;

	while (true)
	{
		int random_number = rand() % 10;
		fprintf(fdw_to_dad, "%d %d ", id, random_number);
		fflush(fdw_to_dad);

		status = fscanf(fdr, "%d", &num_got);
		if (status == EOF)
		{
			finish = true;
		}
		update_counters_after_read(num_got, &zero_count, &one_count,
			&minus_one_count);

		if (finish || zero_count == FINISED)
		{
			close_files(fdw_to_dad, fdr, main_file, id);
			print_killed(id, minus_one_count, zero_count, one_count);
		}
	}
}

//----------------------------------------------------------------------

/* The function handles SIGUSR1 signals by setting a global flag to
 * 	indicate termination.
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

/* The function updates counters based on the value received from the
 *  host process.
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
void print_killed(int id, int minus_one_count, int zero_count,
	int one_count)
{
	printf("Player #%d was killed: %d %d %d\n", id, minus_one_count,
		zero_count, one_count);
	exit(EXIT_SUCCESS);
}

//----------------------------------------------------------------------

/* The function opens the files.
 * The function receives: 2 references to files, file name and id.
 * The function returns: void.
 */
void open_files(FILE** fdw_to_dad, FILE** fdr, char file_name[], int id)
{
	*fdw_to_dad = fopen(file_name, "w");
	is_open(*fdw_to_dad);

	fprintf(*fdw_to_dad, "%d %d ", id, (int)getpid());
	fflush(*fdw_to_dad);

	if (id == 0)
	{
		*fdr = fopen(FIRST_FILE, "r"); // define
	}

	else if (id == 1)
	{
		*fdr = fopen(SECOND_FILE, "r");
	}
	is_open(*fdr);
}

//----------------------------------------------------------------------

/* The function closes the files.
 * The function receives: 2 files, file name and id.
 * The function returns: void.
 */
void close_files(FILE* fdw_to_dad, FILE* fdr, char main_file[], int id)
{
	fclose(fdw_to_dad);
	fclose(fdr);
	unlink(main_file);

	if (id == FIRST_PROCESS)
	{
		unlink(FIRST_FILE);
	}

	else if (id == SECOND_PROCESS)
	{
		unlink(SECOND_FILE);
	}
}
