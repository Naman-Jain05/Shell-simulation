//BT20CSE087
#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

#define CWD_SIZE 10000	  								// current working directory size
#define MAX_CMDS 10	  									// Max number of strings in our command
#define MAX_CMD_LEN 100  								// Max possible length of any of our command 

char **get_input(char *ip)
{
	char **arg_array = malloc(MAX_CMDS * sizeof(char *));
	char *parsd; 										//(parsd=parsed) To store parsed string after tokenisation is done
	parsd = strtok(ip, " ");							//" " is the string delimiter
	int i = 0;  										//where i is representing the index in argument array
	while (parsd != NULL)
	{
		arg_array[i] = parsd;
		i++;
		parsd = strtok(NULL, " ");
	}
	arg_array[i] = NULL;
	return arg_array;
}

int token_check(char **cmnds, char *token)			// Checks whether the given token exists in command and if it exists it returns 1 otherwise it returns 0
{
	char **arg_array = cmnds;							//cmnds is short form for commands
	while (*arg_array)									// Traversing through all strings in 'arg_array' and checking whether string is equal to token
	{
		if (strcmp(*arg_array, token) == 0)
		{ return 1; }
		*arg_array++;
	}
	return 0;
}

int parseInput(char **arg_array)							 // This function parse input string into multiple command with delimiter ( ##,&&, >, or spaces).
{
	if (strcmp(arg_array[0], "exit") == 0) 					 // If command entered is 'exit' return 0
	{ return 0; }
	if (token_check(arg_array, "&&"))						 //  when shell has to execute parallel commands &&
	{ return 1; }
	if (token_check(arg_array, "##"))				         //  when shell has to execute sequential commands ##
	{ return 2; }
	if (token_check(arg_array, ">"))						//  when shell has to execute command redirection >
	{ return 3; }
	return 4; 									 			// for all other basic commands
}


void change_directory(char **arg_array)						// Changes working directory to the directory mentioned inside argument array
{
	const char *path = arg_array[1];
	if (chdir(path) == -1) 									// If chdir fails then prints a message
	{ printf("Shell: Incorrect command\n"); }
}

void executeCommand(char **arg_array)
{
	if (strlen(arg_array[0]) == 0) 							// When user has entered no command simply return , go to next line 
	{ return; }
	else if (strcmp(arg_array[0], "exit") == 0) 			// if user has entered 'exit' simply exit from the shell 
	{ exit(0); }
	else if (strcmp(arg_array[0], "cd") == 0) 				// for cd command call the change_directory function along with the arguments
	{ change_directory(arg_array); }
	else
	{
		int pid = fork(); 									// creating a child process
		if (pid == 0) 										// for child process
		{												
			signal(SIGINT, SIG_DFL);						// Signal handling	
			signal(SIGTSTP, SIG_DFL);						// Restoring default behaviour for Ctrl-C and Ctrl-Z
			if (execvp(arg_array[0], arg_array) < 0) 		// if incorrect command is entered by user
			{
				printf("Shell: Incorrect command\n");
			}
			exit(0);
		}
		else if (pid < 0)									// if fork is unsuccessful no new process is created 
		{
			printf("Shell: Incorrect command\n");
		}
		else 												// for parent process
		{
			wait(NULL); 									// wait for the child process to complete ,it is nice practice to use wait to avoid conflict
			return;
		}
	}
}

void executeParallelCommands(char **arg_array)				// it  runs multiple commands in parallel
{
	char **temp_array = arg_array;							// Storing copy of 'arg_array' For storing all commands to be executed seperately in temp_array
	int r = 0, c = 0, count = 1;
	char *every_commands[MAX_CMDS][MAX_CMD_LEN];
	
	int pid = 1,for_status;									
	while (*temp_array)
	{
		if (*temp_array && (strcmp(*temp_array, "&&") != 0))	//If it isnt "&&", means it is still part of previous command
		{
			every_commands[r][c++] = *temp_array;
		}
		else												//If it is "&&", hence previous command has ended and hence making changes to store new command
		{
			every_commands[r++][c] = NULL;
			c = 0;
			count++;
		}
		
		if (*temp_array)									//If *temp isnt null, means commands are still remaining to manipulate
		{	
			*temp_array++;
		}
	}
	every_commands[r][c] = NULL;
	every_commands[r + 1][0] = NULL;
	
	for (int r = 0; r < count && pid != 0; ++r)				// Traversing through all commands
	{
		pid = fork(); 										//calling fork
		
		if (pid == 0) 									// Fork succeeds
		{
	
			signal(SIGINT, SIG_DFL);						// Signal handling
			signal(SIGTSTP, SIG_DFL);						// Restoring the default behaviour for Ctrl-C and Ctrl-Z signal
			
			execvp(every_commands[r][0], every_commands[r]);	//Executing command
		}
		else if (pid < 0)  										// Fork fails
		{
			printf("Shell: Incorrect command\n");
			exit(1);
		}
	}
	
	while (count > 0)										// Waiting till all fork executions complete
	{
		waitpid(-1, &for_status, WUNTRACED);
		count--;
	}
}

void executeSequentialCommands(char **arg_array)			// This function will run multiple commands sequentially
{
	char **temp_array = arg_array;								// Checking all words in command entered in terminal
	while (*temp_array)
	{
		char **singl_command = malloc(MAX_CMDS * sizeof(char *)); 		// To store one by one command to be executed
		singl_command[0] = NULL;
		int j = 0; 											// Index for one_command variable
		
		while (*temp_array && (strcmp(*temp_array, "##") != 0))				// Traversing through arguments and keep storing values in 'one_command' until we encounter "##"
		{
			singl_command[j] = *temp_array++;
			j++;
		}
		executeCommand(singl_command);						// Executing the command got from above traversal
		if (*temp_array)											// Checking if temp is not null and proceeding accordingly
		{
			*temp_array++;
		}
	}
}

void executeCommandRedirection(char **temp_array)				// This function will run a single command with output redirected to an output file specificed by user
{
	char **arg_array = temp_array;
	arg_array[1] = NULL;										// For ensuring no problem while executing execvp() function	
	if (strlen(temp_array[2]) == 0) 							// if file name is empty
	{
		printf("Shell: Incorrect command\n");
	}
	else
	{
		int pid = fork(); 								// creating a new child process using fork 
		if (pid < 0)									// when fork is unsuccessful
		{
			printf("Shell: Incorrect command\n");
			return;
		}
		else if (pid == 0) 								// for child process
		{
			signal(SIGINT, SIG_DFL);					// Restoring default behaviour for Ctrl-C and Ctrl-Z
			signal(SIGTSTP, SIG_DFL);
			close(STDOUT_FILENO);
			open(temp_array[2], O_CREAT | O_WRONLY | O_APPEND);
			execvp(arg_array[0], arg_array);			// executing command
			return;
		}
		else											// for parent process
		{
			wait(NULL);
			return;
		}
	}
}


int main()
{
	//Initial declarations
	signal(SIGINT, SIG_IGN);  							// Ignore signal interrupt (Ctrl+C)
	signal(SIGTSTP, SIG_IGN); 							// Ignore signal of suspending execution (Ctrl+Z)
	char cwd[CWD_SIZE];		  							// Variable to store current working directory
	char *cmnds = NULL;	  							// lineptr for getline function, and if it is set to null, buffer will be allocated to store line
	int command_val = 0;	  							// To keep track of command to be executed
	size_t cmd_size = 0;

	while (1)
	{
		command_val = -1;
		char **arg_array = NULL; 						// To store strings seperately from command given in terminal

		getcwd(cwd, CWD_SIZE);							//to get current working directory 
		printf("%s$", cwd);

		getline(&cmnds, &cmd_size, stdin);			// accept input with 'getline()'
		
		cmnds = strsep(&cmnds, "\n");				// Extract token surrounded by delimiter '\n'
		
		if (strlen(cmnds) == 0)						//when no command is provided
		{
			continue;
		}

		arg_array = get_input(cmnds);					// Extracting tokens surrounded by delimiter " "
		

		if (strcmp(arg_array[0], "##") == 0)			// When in sequential execution first command is empty
		{
			continue;
		}

		command_val = parseInput(arg_array);			// parse the input to check whether command is a single word or has &&, ##, >

		if (command_val == 0)
		{
			printf("Exiting shell...\n");
			break;
		}
		if (command_val == 1)
		{
			executeParallelCommands(arg_array); 			// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		}
		else if (command_val == 2)
		{
			executeSequentialCommands(arg_array); 			// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		}
		else if (command_val == 3)
		{
			executeCommandRedirection(arg_array); 			// This function is invoked when user wants redirect output of a single command to and output file 
		}
		else
		{
			executeCommand(arg_array); 						// This function is invoked when user wants to run a single commands
		}
	}
	
	//printf("thank you");
	
	return 0;
}
