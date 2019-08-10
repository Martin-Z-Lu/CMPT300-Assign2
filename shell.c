// Shell starter file
// You may make any changes to any part of this file.
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <signal.h>


#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10

char *com_array[HISTORY_DEPTH];
int num_array[HISTORY_DEPTH];
int front = -1;
int rear = -1;

void insert(char *tokens[], int deepth){

	//full
	if((front == 0 && rear == HISTORY_DEPTH-1) || (front == rear+1)){
		return;
	}

	//empty
	if (front == -1){
		front = 0;
		rear = 0;
		//initialization
		for(int i=0; i<10; i++){
			com_array[i] = malloc(sizeof(COMMAND_LENGTH));
		}
	}
	else{
		if(rear == HISTORY_DEPTH-1){
			rear = 0;
		}
		else{
			rear = rear+1;
		}
	}

	num_array[rear] = deepth;

	for(int i=0; tokens[i] != NULL; i++){
		strcat(com_array[rear], tokens[i]);
		strcat(com_array[rear], " ");
	}
}

void print_array(){
	int temp_front = front;
	int temp_rear = rear;
	char con_int[5] = {'x', 'x', 'x', 'x', 'x'};
	//int temp_deepth = 0;

	if((front == -1) || (rear == -1)){
		write(STDOUT_FILENO, "No command", strlen("No command"));
		return;
	}

	if(temp_front <= temp_rear){
		while(temp_front <= temp_rear){
			sprintf(con_int, "%d", num_array[temp_front]);
			//strcpy(con_int, sprintf(con_int, "%d", num_array[temp_front]));
			write(STDOUT_FILENO, con_int, strlen(con_int));
			write(STDOUT_FILENO, "	", strlen("	"));
			write(STDOUT_FILENO, com_array[temp_front], strlen(com_array[temp_front]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			temp_front++;
		}
	}
	else{
		while(temp_front <= HISTORY_DEPTH-1){
			sprintf(con_int, "%d", num_array[temp_front]);
			write(STDOUT_FILENO, &num_array[temp_front], sizeof(num_array[temp_front]));
			write(STDOUT_FILENO, "	", strlen("	"));
			write(STDOUT_FILENO, com_array[temp_front], strlen(com_array[temp_front]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			temp_front++;
		}

		temp_front = 0;

		while(temp_front <= temp_rear){
			sprintf(con_int, "%d", num_array[temp_front]);
			write(STDOUT_FILENO, &num_array[temp_front], sizeof(num_array[temp_front]));
			write(STDOUT_FILENO, "	", strlen("	"));
			write(STDOUT_FILENO, com_array[temp_front], strlen(com_array[temp_front]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			temp_front++;
		}
	}
}


char *retrieve(int deepth){

	char *temp_buff = NULL;
	int array_size = sizeof(num_array);
	int track_num = 0;

	for(int i=0; i<array_size; i++){
		if(num_array[i] == deepth){
			track_num = i;
		}
	}

	temp_buff = com_array[track_num];
	return temp_buff;
}

void handle_SIGINT(){
	if(strcmp(com_array[0], " ") == 0){
		write(STDOUT_FILENO, "no history", strlen("no history"));
	}

	for(int i=0; com_array[i] != NULL; i++){
		write(STDOUT_FILENO, com_array[i], strlen(com_array[i]));
	}
}

/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

  	// if (length < 0){
   //   	perror("Unable to read command. Terminating.\n");
   //   	exit(-1);  /* terminate with error */
  	// }

  	//for signal

  	if ( (length < 0) && (errno != EINTR) ){
      	perror("Unable to read command. Terminating.\n");
      	exit(-1);  /* terminate with error */
  	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}


/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{




	int real_deepth = 1;

	char input_buffer[COMMAND_LENGTH];

	char *my_input_buffer = malloc(sizeof(COMMAND_LENGTH));

	char *tokens[NUM_TOKENS];




	//write(STDOUT_FILENO, "cao", strlen("cao"));

	while (true) {

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		write(STDOUT_FILENO, "> ", strlen("> "));
		_Bool in_background = false;

		struct sigaction handler;
		handler.sa_handler = handle_SIGINT;
		handler.sa_flags = 0;
		sigemptyset(&handler.sa_mask);
		sigaction(SIGINT, &handler, NULL);

		read_command(input_buffer, tokens, &in_background);



		//get from array
		int track = 9999;

		//check start with !
		if(strcmp(tokens[0], "!!") == 0){
			//write(STDOUT_FILENO, "haha", strlen("haha"));
			track = num_array[rear];
			my_input_buffer = retrieve(track);
			//strcpy(my_input_buffer, &(retrieve(track)));
			tokenize_command(my_input_buffer, tokens);
			}
		else if(tokens[0][0] == '!'){
			//check if it number
			track = atoi(&tokens[0][1]);
		//!n and n is valid
			if((track <= num_array[rear]) && (track >= num_array[front])){
				
			//strcpy(my_input_buffer, &(retrieve(track)));
				my_input_buffer = retrieve(track);
				tokenize_command(my_input_buffer, tokens);
			}
		//!n is number but not valid
			else{
				write(STDOUT_FILENO, "Need a valid and in-range number", strlen("Need a valid number"));
			}
		}
		else{
			insert(tokens, real_deepth);
			real_deepth++;
		}


		//insert


		//new_line
		char new_path[1024];
		if(tokens[0] == NULL){
			if(getcwd(new_path, sizeof(new_path)) != NULL){
				write(STDOUT_FILENO, new_path, strlen(new_path));
				continue;
			}
		}




		//history
		if(strcmp(tokens[0],"history") == 0){
			print_array();
			continue;
		}






		// DEBUG: Dump out arguments:
		for (int i = 0; tokens[i] != NULL; i++) {
			write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		if (in_background) {
			write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		}



		//my_cd
		if(strcmp(tokens[0],"cd") == 0){
			//error
			if(chdir(tokens[1]) == 0){
				chdir(tokens[1]);
				continue;
			}
			else{
				write(STDOUT_FILENO, "cd error", strlen("cd error"));
				continue;
			}
		}


		//my_pwd
		char my_path[1024];
		if(strcmp(tokens[0],"pwd") == 0){
			if(getcwd(my_path, sizeof(my_path)) != NULL){
				write(STDOUT_FILENO, my_path, strlen(my_path));
				continue;
			}
		}


		//my_exit
		if(strcmp(tokens[0],"exit") == 0){
			exit(0);
		}

		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */

		pid_t pid = fork();

		if(pid < 0){
			printf("fork failed\n");
		}
		
		if(pid == 0){
			for(int i=0; tokens[i] != NULL; i++){
				if(execvp(tokens[i], tokens) < 0){
					write(STDOUT_FILENO, "ERR: execution fail", strlen("ERR: execution fail"));
				}
				else{
					execvp(tokens[i], tokens);
				}
			}
			exit(0);
		}
		else{
			if(in_background == false){

				pid_t wait_child = waitpid(pid, NULL, 0);

				if(wait_child == pid){}
			}
			else{
				continue;
			}
		}



	}
	return 0;
}
