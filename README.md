# NamedPipe_MessageQueue_Use_Linux

File: duel_NamedPipe_host.c 

Ex #1a: A duel for the higher value (The host).
==========================================================
Written by: Jacob Bondar.

This program simulates a competitive game between two processes 
 managed by a host process. The HOST compares between two inputs he 
 recieved from the processes and determine a winner for each round. 
 Scores are updated based on the results, and the game ends when one 
 player wins 120 rounds or both achieve 100 ties.

Compile: gcc -Wall duel_NamedPipe_host.c –o duel_NamedPipe_host
Run: ./duel_NamedPipe_host <File Name> <File Name> <File Name>

Input: In the vector arguments (argv) we recieve 3 things:
		1. The name of the file, for communication from the 
			processes to the host.
		2. The name of the file, for communication from the host
			 to the first process.
		3. The name of the file, for communication from the host
			 to the second process.
 
Output: The winner (if there is one), and the children summary.

------------------------------------------------------------------

File: duel_NamedPipe_player.c 

Ex #1b: A duel for the higher value (The players).
==========================================================
Written by: Jacob Bondar.

This program implements a process's role in the competitive game. 
 Each process generates random numbers and sends them to the host
 process via fifo. It then receives feedback from the host to update
 counters for wins, losses, and ties. The process continues until a
 termination signal is received or a specific condition is met, at 
 which point the child logs its summary and exits.

Compile: gcc -Wall duel_NamedPipe_player.c –o duel_NamedPipe_player
Run: ./duel_NamedPipe_player <File Name> <id> <seed>

Input: In the vector arguments (argv) we recieve 3 things:
		1. The name of the file, for communication from the 
			processes to the host.
		2. The id of the process.
		3. The seed.
 
Output: The summary of the process points.

----------------------------------------------------------------

File: duel_MessageQueue_host.c 

Ex #2a: A duel for the higher value (The host).
==========================================================
Written by: Jacob Bondar.

This program simulates a competitive game between two processes 
 managed by a host process. The host compares between two inputs he 
 recieved from the processes and determine a winner for each round. 
 Scores are updated based on the results, and the game ends when one 
 player wins 120 rounds or both achieve 100 ties.

Compile: gcc -Wall duel_MessageQueue_host.c –o duel_MessageQueue_host
Run: ./duel_MessageQueue_host

Input: void.
 
Output: The winner (if there is one), and the children summary.

------------------------------------------------------------------

File: duel_MessageQueue_player.c

Ex #2b: A duel for the higher value (The players).
==========================================================
Written by: Jacob Bondar.

This program implements a process's role in the competitive game. 
 Each process generates random numbers and sends them to the host
 process via queue. It then receives feedback from the host to update
 counters for wins, losses, and ties. The process continues until a
 termination signal is received or a specific condition is met, at 
 which point the child logs its summary and exits.

Compile: gcc -Wall duel_MessageQueue_player.c –o duel_MessageQueue_player
Run: ./duel_MessageQueue_player <id> <seed value>

Input: In the vector arguments (argv) we recieve 2 things:
		1. The id of the process.
		2. The seed.
 
Output: The summary of the process points.

---------------------------------------------------------------------

File: Prime.c 

Ex #3a: Prime Number Filter via Message Queue
==========================================================
Written by: Jacob Bondar.

This program creates a message queue to receive sentences from 
 clients, processes the sentences to extract prime numbers, and 
 sends the processed result back to the clients. The program runs 
 continuously until terminated using a SIGINT signal, at which point 
 the message queue is safely removed.

Compile: gcc -Wall Prime.c –o Prime
Run: ./Prime

Input: Sentences received via message queue containing space-separated 
       integers.
 
Output: Sentences containing only prime numbers sent back to the clients.

-----------------------------------------------------------------------

File: Arithmetic.c 

Ex #3b: Arithmetic Expression Processor via Message Queue
==========================================================
Written by: Jacob Bondar.

This program receives arithmetic expressions from clients through 
 a message queue, evaluates the result of the expressions, and sends 
 the results back to the clients. It operates in an infinite loop 
 until terminated with a SIGINT signal, at which point the message 
 queue is safely removed.

Compile: gcc -Wall Arithmetic.c –o Arithmetic
Run: ./Arithmetic

Input: Arithmetic expressions in the format 
	  "<integer><operator><integer>" received via the message queue.
 
Output: Evaluation results sent back to the respective client.

-------------------------------------------------------------------

File: prime_Arithmetic_client.c 

Ex #3c: Client for Prime and Arithmetic Servers
==========================================================
Written by: Jacob Bondar.

This program acts as a client for two servers: one that processes 
 prime numbers and another that evaluates arithmetic expressions. 
 Based on user input, the program sends requests to the appropriate 
 server via message queues and displays the results received.
 When typing 'p', you need to follow with 'enter', and then for every
 new number, you need to press 'enter' again, until zero.

Compile: gcc -Wall prime_Arithmetic_client.c –o prime_Arithmetic_client
Run: ./prime_Arithmetic_client

Input: User commands ('p' for primes, 'a' for arithmetic) followed 
       by respective data.
 
Output: Processed results (prime numbers or evaluated expressions) 
       from the servers.
