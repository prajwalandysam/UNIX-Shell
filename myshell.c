/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()
#include <dirent.h>   // for performing operations on directory like ls.
#include <errno.h>



int exitflag=0;
int andand=0;
int hashhash=0;
int greater=0;
int aflag=1;

void handleCtrlZ(int signo) {
    // Handle Ctrl + Z signal
    //printf("\nCtrl + Z caught, but ignored.\n");
    printf("\n");
    fflush(stdout);
}

void handleCtrlC(int signo) {
    // Handle Ctrl + C signal
    //printf("\nCtrl + C caught, but ignored.\n");
    printf("\n");
    fflush(stdout);
}

void Space(char *st, char **input)
{
	// for parsing strings with spaces and no other special tokens
	int i=0;
	
	//for (i = 0; i < 1024; i++)
	while(i<1024)
	{
		input[i] = strsep(&st, " "); 
		if (input[i] == NULL)
			break;
		if (strlen(input[i]) == 0)
			i--;
			
		i++;
	}
}
int Sequential(char *st, char **input)
{
	//for parsing strings with "##" seperator, for sequential running
	int i=0;
	//for (i = 0; i < 1024; i++)
	while(i<2014)
	{
		input[i] = strsep(&st, "##"); 
		if (input[i] == NULL)
			break;
		if (strlen(input[i]) == 0)
			i--;
		i++;
	}
	if (input[1] == NULL)
		return 0;
}
int Parallel(char *st, char **input)
{
	// for parsing strings with "&&" seperator, for parallel running
	int i=0;
	//for (i = 0; i < 1024; i++)
	while(i<1024)
	{
		input[i] = strsep(&st, "&&"); 
		if (input[i] == NULL)
			break;
		if (strlen(input[i]) == 0)
			i--;
		i++;
	}
	if (input[1] == NULL)
		return 0;
	return 1;
}

int Redirect(char *st, char **input)
{
	//for parsing strings with ">" seperator, for redirecting output
	int i;
	for (i = 0; i < 2; i++)
	{
		input[i] = strsep(&st, ">"); 
		if (input[i] == NULL)
			break;
		if (strlen(input[i]) == 0)
			i--;
	}
	if (input[1] == NULL)
		return 0;
	return 1;
}

int parseInput(char *str, char **input[])
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	int pflag = 0;
	char *parse[1024];

	//checks for each case
	pflag = Parallel(str, parse); //parallel checking
	if (pflag)
	{
		// for handling if the string is a parallel chain of commands
		int cmd_count = 0;
		while (cmd_count < 1024)
		{
			if (parse[cmd_count] == NULL)
				break;
			Space(parse[cmd_count], input[cmd_count]);
			cmd_count++;
		}
		return 0;
	}

	pflag = Sequential(str, parse); //sequential checking
	if (pflag)
	{
		// for handling if the string is a sequential chain of commands
		int cmd_count = 0;
		while (cmd_count < 1024)
		{
			if (parse[cmd_count] == NULL)
				break;
			Space(parse[cmd_count], input[cmd_count]);
			cmd_count++;
		}
		return 1;
	}

	pflag = Redirect(str, parse); //for checking the redirect case
	if (pflag)
	{
		// for handling the redirect case
		Space(parse[0], input[0]);
		Space(parse[1], input[1]);
		return 2;
	}

	Space(str, input[0]); //for single command cases

	if (strcmp("exit", input[0][0]) == 0) //for exit case
		return 3;
	else if (strcmp("cd", input[0][0]) == 0) //for cd case
		return 4;
	else
	{
		return 5; //for normal commands
	}
}

//EXECUTABLE COMMANDS
void executeCommand(char **A)
{
	// This function will fork a new process to execute a command
	if(strcmp(A[0],"cd")==0)
	{
		chdir(A[1]);
	}
	else
	{
		int id=fork();
		if(id<0)
		{
			int err=errno;
			fprintf(stderr,"Error %s\n : ",strerror(err));
		}
		else if(id==0)
		{
			int status=execvp(A[0],A);
			if(status==-1)
			{
				printf("Shell: Incorrect command\n");
				exit(1);
			}
			exit(0);
		}
		else
		{
			int status=wait(NULL);
		}
	}
}





void executeParallelCommands(char **input)
{
	// This function will run multiple commands in parallel
	int i=0;
	int j=0;
	while(input[i]!=NULL)
	{
		char **temp=(char **)malloc(sizeof(char*));
		while(input[i]!=NULL && strcmp(input[i],"&&")!=0)
		{
			temp[j]=input[i];
			j++;
			i++;
		}
		temp[j]=NULL;
		if(strcmp(temp[0],"cd")==0)
		{
			chdir(temp[1]);
		}
		else
		{
			int id=fork();
			if(id<0)
			{
				int err=errno;
				//fprintf(stderr,"Error : ",strerror(err));
				fprintf(stderr, "Error: %s\n", strerror(err));

				exit(1);
			}
			else if(id==0)
			{
				int status=execvp(temp[0],temp);
				if(status==-1)
				{
					printf("Shell: Incorrect command\n");
					exit(1);
				}
				exit(0);
			}
		}
		i++;
	}
}

void executeSequentialCommands(char **input[])
{
	// This function will run multiple commands in sequence
	int i;
	int size = 1024;
	int ex;
	for (i = 0; i <size; i++)
	{
		if (input[i]== NULL || input[i][0] == NULL)
			break;
		executeCommand(input[i]);
		if  (aflag == -1)
			break;
	}
}
void executeCommandRedirection(char **input[]) {
    // This function will run a single command with output redirected to an output file specified by the user

    if (input[1] == NULL || input[1][0] == NULL || strlen(input[1][0]) == 0) {
        printf("Shell: Incorrect command\n");
        return;
    }

    // Create a pipe to capture the command's output
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return;
    }

    pid_t child_id = fork();

    if (child_id == 0) {
        // Child process

        // Close the read end of the pipe since we're only writing to it
        close(pipe_fd[0]);

        // Redirect stdout to the write end of the pipe
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }

        // Close the write end of the pipe as it's no longer needed
        close(pipe_fd[1]);

        // Execute the command
        if (execvp(input[0][0], input[0]) < 0) {
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    } else if (child_id > 0) {
        // Parent process

        // Close the write end of the pipe since we're only reading from it
        close(pipe_fd[1]);

        // Create or open the specified output file
        int output_fd = open(input[1][0], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (output_fd == -1) {
            perror("open");
            close(pipe_fd[0]);
            return;
        }

        // Redirect the input of the output file to the read end of the pipe
        ssize_t bytes_read;
        char buffer[4096];

        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
            write(output_fd, buffer, bytes_read);
        }

        // Close the file descriptor for the output file and the read end of the pipe
        close(output_fd);
        close(pipe_fd[0]);

        // Wait for the child process to finish
        int status;
        waitpid(child_id, &status, 0);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                printf("Child process exited with status %d\n", exit_status);
            }
        } else if (WIFSIGNALED(status)) {
            int signal_number = WTERMSIG(status);
            printf("Child process terminated by signal %d\n", signal_number);
        }
    } else {
        perror("fork");
    }
}
int main()
{
	// Initial declarations
	signal(SIGTSTP, handleCtrlZ); // Set up handler for Ctrl + Z
    signal(SIGINT, handleCtrlC);  // Set up handler for Ctrl + C
	
	while(1)	// This loop will keep your shell running until user exits.
	{
		signal(SIGTSTP, handleCtrlZ); // Set up handler for Ctrl + Z
    signal(SIGINT, handleCtrlC);  
		// Print the prompt in format - currentWorkingDirectory$
		char curr_dir[1024];
		char **input[1025];
		char* buff=NULL;
		size_t buffer=0;
		int pflag,error;
		int t=0;
		while(t<1024)
		{
			input[t]=(char**)malloc(1024*sizeof(char**));
			t++;
		}
		
		input[1024]=NULL;
			
		
		
		if(getcwd(curr_dir, sizeof(curr_dir) )!=NULL) // getcwd = get current working directory is a function in <unistd.h> library used to find out the current working directory
		
		{
			printf("%s$",curr_dir);
		}
		else {
        		perror("getcwd");// std lib funciton to print the error if the current working directory is not found
        		
        	}
		
		// accept input with 'getline()'
		getline(&buff,&buffer,stdin);
		buff = strsep(&buff, "\n");
		if (strlen(buff) == 0)
		{
			continue;
		}
		
		
		

		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		pflag=parseInput(buff,input); 
		
		if (pflag == 0)
		{
			
			andand=1;
			if  (aflag == -1)
				break;
				
		}
		else if (pflag == 1)
		{
			
			hashhash=1;
			if  (aflag == -1)
				break;
		}
		else if (pflag == 2)
		{
			greater=1;
			if  (aflag == -1)
				break;
		}
		else if (pflag == 3) // When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		else if (pflag == 4) //When user uses cd command.
		{
			error = chdir(input[0][1]);
			if (error == -1)
			printf("Shell: Incorrect command\n"); //checked
		}
		else
		{
			executeCommand(input[0]); // This function is invoked when user wants to run a single commands
		}
			
		
		
		
		
		int single=0;
		//while(tokens[i]!=NULL)
		
			if(andand==1)//if(/*condition*/)
			{
			single=1;
			
			executeParallelCommands(input[0]);
			
			
			}
			
			else if(hashhash==1)//else if(/*condition*/)
			{
			single=1;
			executeSequentialCommands(input);
			
			
			}
			else if(greater==1)//else if(/*condition*/)
			{
			single=1;
			executeCommandRedirection(input);
			
			}
			
			
			
		
		
				
			
			
			
			
			
		
		//if(/*condition*/)
			//executeParallelCommands();		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		//else if(/*condition*/)
			//executeSequentialCommands();	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		//else if(/*condition*/)
			//executeCommandRedirection();	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		//else
		//	executeCommand();		// This function is invoked when user wants to run a single commands
		//	
				
	}
	
	return 0;
}
