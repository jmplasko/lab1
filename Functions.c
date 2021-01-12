#include "Functions.h"

/* Safe Malloc */
void *safe_malloc(size_t size){
  void *new;

  /* malloc returns a NULL pointer upon failure
    This checks for failure. */
  if (NULL == (new=malloc(size))){
    perror("safe_malloc: malloc failure");
    /* Exit program */
    exit(EXIT_FAILURE);
  }
  /* Returns a pointer with no associated data type */
  return new;
}

/* Signal Handler */
void sigintHandler(int sig_num){
  /* async safe print to stdout */
  write(STDOUT_FILENO, "\n", 1);
  fflush(stdout);
}

/* Slices our input string into tokens, stores the tokens in an array
  and returns an int representing the number of tokens found */
int tokenize(char arguments[256][LINEMAX+1], char *input){
  /* input/output flags to indicate the next token is an input/output */
  int next_is_input = 0, next_is_output = 0;
  /* flag to indicate the previous token was a pipe */
  int previous_is_pipe = 0;
  /* Flags to indicate an input and output was set or
  if we're in a pipe, used for error handling */
  int input_f = 0, output_f = 0, pipe_f = 0;
  /* num of tokens, args in command, and args in pipe; respectively */
  int items = 0, arginc = 0, argcpipe = 1;
  /* flag used to output correct command to stderr */
  int cmd_f = 0;
  /* Stores our input string, to be tokenized*/
  char line[LINEMAX+1];
  /* Token */
  char *token;
  /* Stores the command string displayed in error messages*/
  char cmd[LINEMAX+1];

  /* Tokenize our input */
  strcpy(line, input); /* strtok is destructive so we make a copy */
  token = strtok(line, " "); /* Split by spaces */

  /* While we still have tokens */
  while ( token != NULL ){
    /* Add our token to our array */
    strcpy(arguments[items++], token);

    /* Sets the cmd string for error handling purposes for stage 0*/
    if ( arginc == 0 && cmd_f == 0)
      strcpy(cmd, token);

    /* If this token is going to be processed as an input */
    if ( next_is_input == 1 ){
      /* Error handling */
      if ( ( token[0] == '<' )  || ( token[0] == '>' )
        || ( token[0] == '|' ) || ( input_f == 1) ){
        fprintf(stderr, "%s: Bad input redirecion\n", cmd);
        return 0;
      }
      else{
        /* Correctly processed, so we... */
        next_is_input = 0; /* Indicate that the next token isn't an input */
        input_f = 1; /* Set our input flag */
        arginc--; /*Decrement arginc as we do not count this as an argument*/
      }
    }
    /* If this token is going to be processed as an output */
    if ( next_is_output == 1 ){
      /* Error Handling */
      if ( token[0] == '<' ){
        fprintf(stderr, "%s: Bad input redirecion\n", cmd);
        return 0;
      }
      else if ( ( token[0] == '>' ) || ( token[0] == '|' ) || output_f == 1){
        fprintf(stderr, "%s: Bad output redirecion\n", cmd);
        return 0;
      }
      else{
        /* Correctly processed so we... */
        next_is_output = 0; /* Indicate that the next token isn't an output */
        output_f = 1; /* Set our output flag */
        arginc--; /*Decrement arginc as we do not count this as an argument*/
      }
    }

    /* Our previous token was a pipe! */
    if ( previous_is_pipe == 1 ){
      /* Error handling */
      strcpy(cmd, token); /* Reset our cmd string for error handling */
      if ( token[0] == '<' ){
        fprintf(stderr, "%s: Bad input redirecion\n", cmd);
        return 0;
      }
      else if ( token[0] == '>' ){
        fprintf(stderr, "%s: Bad output redirecion\n", cmd);
        return 0;
      }
      else if ( token[0] == '|' ){
        fprintf(stderr, "Invalid null command\n");
        return 0;
      }

      else{
        /* Correctly processed so we... */
        previous_is_pipe = 0; /* Indicate that current token isn't an pipe */
        pipe_f = 1; /* Indicate that we are now in a pipe */
        arginc = 0; /* Reset our arguments in command counter */
      }
    }
    /* If our current token is a < */
    if ( token[0] == '<' ){
      /*Error handling */
      if ( pipe_f == 1 ){
        fprintf(stderr, "%s: Ambiguous input\n", cmd);
        return 0;
      }
      next_is_input = 1; /* Indicate the next token is an input */
      cmd_f = 1; /* Set cmd_f for error handling purposes */
    }
    /* If our current token is a > */
    else if ( token[0] == '>' ){
      next_is_output = 1; /* Indicate the next token is an output */
      cmd_f = 1; /* Set cmd_f for error handling purposes */
    }
    /* If our current token is a | */
    else if ( token[0] == '|' ){
      /* Error handling */
      if ( output_f == 1 ){
        fprintf(stderr, "%s: Ambiguous output\n", cmd);
        return 0;
      }
      argcpipe++; /*Increment arguments in pipe counter */
      previous_is_pipe = 1; /*Indicate that our previous token was a pipe */
    }
    /* Our token is a string */
    else{
      arginc++; /*Increment arguments in command */
      cmd_f = 1; /* Set cmd_f for error handling purposes */
    }

    /* If arguments in command exceed the max, exit */
    if ( arginc > ARGUMENTSMAX ){
      fprintf(stderr, "Too many arguments\n");
      return 0;
    }

    /* If arguments in pipe exceed the max, exit */
    if ( argcpipe > PIPELINEMAX ){
      fprintf(stderr, "Pipeline too deep\n");
      return 0;
    }
    /* Get our next token */
    token = strtok(NULL, " ");
  }

  /*At the end, all our flags should be 0, otherwise we have an error */
  if ( next_is_input == 1 ){
    fprintf(stderr, "%s: Bad input redirecion\n", cmd);
    return 0;
  }

  else if ( next_is_output == 1 ){
    fprintf(stderr, "%s: Bad output redirecion\n", cmd);
    return 0;
  }

  else if ( previous_is_pipe == 1 ){
    fprintf(stderr, "Invalid null command\n");
    return 0;
  }

  /* Return tne number of tokens processed */
  return items;
}

/* Checks for specific commands in input */
int command_checker(char *arguments[], int items){
  char directorypath[LINEMAX+3] = {0};
  int i;
  /*If our first argument is cd */
  if (strcmp(arguments[0], "cd") == 0){
    /* Error handling */
    if ( items != 2 ){
      if ( 2 < items )
        fprintf(stderr, "%s: too many arguments.\n", arguments[0]);
      else if ( items == 1 )
        fprintf(stderr, "%s: missing argument.\n", arguments[0]);
      
      for ( i=0; i<items; i++){
        free(arguments[i]);
        arguments[i] = NULL;
      }
      return 1;
    }

    else{
      strcpy(directorypath, "./");
      strcat(directorypath, arguments[1]);
      /* Error handling */
      if (chdir(directorypath) != 0){
        perror(arguments[1]);
      }
      for ( i=0; i<items; i++){
        free(arguments[i]);
        arguments[i] = NULL;
      }
      
      return 1;
    }
  }
  /* Exit command */
  
  else if(strcmp(arguments[0], "exit") == 0){
    free(arguments[0]);
    arguments[0] = NULL;
    return 0;
  }
  
  /* Execute our arguments */
  return execute_commands(arguments, items);
}

/* Executes our arguments */
int execute_commands(char *arguments[], int items){
  pid_t chi[10] = {0};
  pid_t child;
  char* parsed[256] = {NULL};
  char* holdargv[256] = { NULL };
  char out_filename[512] = {0};
  char in_filename[512] = {0};
  int status, i, j;
  int trackerarg = 0;
  int pp[10][2];
  int k = 0, pipestage = 0;
  int pipes = 0, left = 0, right = 0;
  int chit = 0;
  int input_flag = 0, output_flag = 0;
  int input_fd = 0, output_fd = 0;
  int parsed_items = 0;
  int concurrent = 0;
  int saved_stdout = dup(STDOUT_FILENO);
  int saved_stdin = dup(STDIN_FILENO);
  struct sigaction action;
  sigset_t set;

  /* Add SIGINT to our set */
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  /* Initialize our sigaction */
  memset(&action, 0, sizeof(action));
  action.sa_handler = sigintHandler;
  /*action.sa_flags = SA_RESETHAND;*/
  

  if (strcmp(arguments[items - 1], "&") == 0) {
	  items = items - 1;
	  concurrent = 1;
  }

  /* Parse our arguments */
  for(i = 0; i < items; i++){
    /* Sets the in_file if specified */
    if (right == 1){
      strcpy(in_filename, arguments[i]);
      input_flag = 1;
      right = 0;
      continue;
    }
    /* Sets the out_file if specified */
    else if (left == 1){
      strcpy(out_filename, arguments[i]);
      output_flag = 1;
      left = 0;
      continue;
    }
    /* Indicate our next argument is the in_file */
    else if(strcmp(arguments[i], "<") == 0)
      right = 1;
    /* Indicate our next argument is the out_file */
    else if(strcmp(arguments[i], ">") == 0)
      left = 1;
    else{
      if(strcmp(arguments[i], "|") == 0)
        pipes++;
      parsed[parsed_items] = safe_malloc(strlen(arguments[i]) +1);
      strcpy(parsed[parsed_items], arguments[i]);
      parsed_items++;
    }
  }
  
  /* No Pipes */
  if((pipes == 0) && concurrent == 1){
    sigprocmask(SIG_BLOCK, &set, NULL);
    if ( ( child = fork() ) ){
      /* Parent */
      if ( child == -1 ){
        perror("fork");
        exit(EXIT_FAILURE);
      }
      /* Wait for our child */
      /*if ( (-1) == waitpid(child, &status, 0) ) {
        exit(1);
      }*/
      /*Install interrupt*/
      sigprocmask(SIG_UNBLOCK, &set, NULL);
      sigaction(SIGINT, &action, NULL);
      /* Free our parsed items */
      for(i = 0; i <= parsed_items; i++){
        free(parsed[i]);
        parsed[i] = NULL;
      }
      parsed_items = 0;
      
      /* Close input and output file descriptors */
      if(output_flag == 1){
        close(output_fd);
        memset(out_filename, '\0', 512);
      }
      if (input_flag == 1){
        close(input_fd);
        memset(in_filename, '\0', 512);
      }
      /* Restore STDIN and/or STDOUT */
      dup2(saved_stdin, STDIN_FILENO);
      close(saved_stdin);
      dup2(saved_stdout, STDOUT_FILENO);
      close(saved_stdout);
      
      /* Free our arguments */
      for (i = 0; i < items; i++){
          free(arguments[i]);
          arguments[i] = NULL;
      }
      
      /* If we successfully exited the child*/
      if ( WIFEXITED(status) && (WEXITSTATUS(status) == 0)
          && !WIFSIGNALED(status)){
        return 1;
      }
      /* If we unsuccessfully exited the child */
      else{
        printf("Process %d exited with an error value.\n", child);
        fflush(stdout);
        return 1;
      }
     }else{
      /* Child */
      /* Unblock Interrupt Signal */
      sigprocmask(SIG_UNBLOCK, &set, NULL);
      
      /* Input Redirection, if necessary */
      if (input_flag == 1){
        if ((input_fd = open(in_filename, O_RDONLY)) < 0){
          perror(in_filename);
          exit(3);
        }
        dup2(input_fd, STDIN_FILENO);
      }
      /* Output Redirection, if necessary */
      if (output_flag == 1){
        if ((output_fd = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC,
                              S_IRUSR | S_IWUSR))<0){
          perror(out_filename);
          exit(3);
        }
        dup2(output_fd, STDOUT_FILENO);
      }
      
      /* Execute our arguments */
      if (execvp(parsed[0], parsed) == -1){
        perror(parsed[0]);
        exit(4);
      }
    }
    return 1;
  }else if(pipes == 0){
	  sigprocmask(SIG_BLOCK, &set, NULL);
	  if ((child = fork())) {
		  /* Parent */
		  if (child == -1) {
			  perror("fork");
			  exit(EXIT_FAILURE);
		  }
		  /* Wait for our child */
		  if ((-1) == waitpid(child, &status, 0)) {
			  exit(1);
		  }
		  /*Install interrupt*/
		  sigprocmask(SIG_UNBLOCK, &set, NULL);
		  sigaction(SIGINT, &action, NULL);
		  /* Free our parsed items */
		  for (i = 0; i <= parsed_items; i++) {
			  free(parsed[i]);
			  parsed[i] = NULL;
		  }
		  parsed_items = 0;

		  /* Close input and output file descriptors */
		  if (output_flag == 1) {
			  close(output_fd);
			  memset(out_filename, '\0', 512);
		  }
		  if (input_flag == 1) {
			  close(input_fd);
			  memset(in_filename, '\0', 512);
		  }
		  /* Restore STDIN and/or STDOUT */
		  dup2(saved_stdin, STDIN_FILENO);
		  close(saved_stdin);
		  dup2(saved_stdout, STDOUT_FILENO);
		  close(saved_stdout);

		  /* Free our arguments */
		  for (i = 0; i < items; i++) {
			  free(arguments[i]);
			  arguments[i] = NULL;
		  }

		  /* If we successfully exited the child*/
		  if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)
			  && !WIFSIGNALED(status)) {
			  return 1;
		  }
		  /* If we unsuccessfully exited the child */
		  else {
			  /*printf("Process %d exited with an error value.\n", child);
			  fflush(stdout);*/
			  return 1;
		  }
	  }
	  else {
		  /* Child */
		  /* Unblock Interrupt Signal */
		  sigprocmask(SIG_UNBLOCK, &set, NULL);

		  /* Input Redirection, if necessary */
		  if (input_flag == 1) {
			  if ((input_fd = open(in_filename, O_RDONLY)) < 0) {
				  perror(in_filename);
				  exit(3);
			  }
			  dup2(input_fd, STDIN_FILENO);
		  }
		  /* Output Redirection, if necessary */
		  if (output_flag == 1) {
			  if ((output_fd = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC,
				  S_IRUSR | S_IWUSR)) < 0) {
				  perror(out_filename);
				  exit(3);
			  }
			  dup2(output_fd, STDOUT_FILENO);
		  }

		  /* Execute our arguments */
		  if (execvp(parsed[0], parsed) == -1) {
			  perror(parsed[0]);
			  exit(4);
		  }
	  }
	  return 1;
  }

  /* not really necessary but we're too far in to fix this */
  parsed[parsed_items] = safe_malloc(sizeof(char));
  strcpy(parsed[parsed_items], "\0");
  
  /* Create our pipes */
  for(i = 0; i < pipes; i++){
    if(pipe(pp[i])){
      perror("pipe");
      exit(3);
    }
  }
  


  /* Pipe processes */
  for(i = 0; i < (parsed_items+1); i++){
    if((strcmp(parsed[i], "|") == 0) || (strcmp(parsed[i],"\0\0") == 0)){
      sigprocmask(SIG_BLOCK, &set, NULL);
      if((chi[chit] = fork()) == 0){
        /* Child */
        /* Reset Interrupt Signal */
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        /* If we're at stage zero */
        if(pipestage == 0){
          /* Input Redirection, if necessary */
          if (input_flag == 1){
            if ((input_fd = open(in_filename, O_RDONLY)) < 0){
              perror(in_filename);
              exit(3);
            }
            dup2(input_fd, STDIN_FILENO);
          }
          /* Do our Plumbing */
          dup2(pp[chit][1], STDOUT_FILENO);
          
        /* If we are at the final stage */
        }else if (pipestage == pipes){
          /* Output Redirection, if necessary */
          if (output_flag == 1){

			  output_fd = creat(out_filename, S_IRUSR | S_IWUSR);

              if (output_fd < 0){
				perror(in_filename);
				exit(3);
			  }

            dup2(output_fd, STDOUT_FILENO);
          }
          /* Do our Plumbing */
          dup2(pp[chit-1][0], STDIN_FILENO);
        /* If we are at any other stage...*/
        }else{
          /* Do our Plumbing */
          dup2(pp[chit-1][0], STDIN_FILENO);
          dup2(pp[chit][1], STDOUT_FILENO);
        }
        /* Close our pipe fd */
        for(j = 0; j < pipes; j++){
		  close(pp[j][0]);
          close(pp[j][1]);
        }
        
		

        /* Execute our arguments */
        if(trackerarg == 1){
          if(execlp(holdargv[0],holdargv[0], NULL) == -1){
            perror(holdargv[0]);
            exit(3);
          }
        }else{
          if(execvp(holdargv[0],holdargv) == -1){
            perror(holdargv[0]);
            exit(3);
          }
        }
      }
      /* Move on to the next stage */
      pipestage++;
      chit++;
      /* Freeing stage arguments */
      for(k = 0; k < trackerarg; k++){
        free(holdargv[k]);
        holdargv[k] = NULL;
      }
      /* Reset stage argument counter */
      trackerarg = 0;

    }else{
      /* Check for forking error */
      if(chi[chit] == (-1)){
        perror("fork");
        exit(EXIT_FAILURE);
      }
      /* Copy over stage arguments */
      holdargv[trackerarg] = safe_malloc(strlen(parsed[i])+1);
      strcpy(holdargv[trackerarg], parsed[i]);
      trackerarg++;
    }
  }

  /* Parent */
  /* Close our pipe file descriptors */
  for(i = 0; i < pipes; i++){
	close(pp[i][0]);
    close(pp[i][1]);
  }
  
  /* If necessary, close the output file descriptor */
  if (input_flag == 1){
    /*close(input_fd);*/
    memset(in_filename, '\0', 512);
  }
  /* If necessary, close the output file descriptor */
  if(output_flag == 1){

	  /*printf("j%i", output_fd);
	  fflush(stdout);
    close(output_fd);*/
    memset(out_filename, '\0', 512);
  }
  
  /* Wait for each child*/ 
  if (concurrent != 1) {
	  for (i = 0; i < (pipes + 1); i++) {
		  if ((-1) == waitpid(chi[i], &status, 0)) {
			  perror("wait");
		  }
	  }
  }
  /* Reset SIGINT */
  sigprocmask(SIG_UNBLOCK, &set, NULL);
  sigaction(SIGINT, &action, NULL);
  
/* For each child... */
for (i = 0; i < (pipes + 1); i++) {
	/* Check their exit status */
	if (!WIFEXITED(status) || WEXITSTATUS(status)) {
		/* If we successfully exited the child*/
		/*printf("Process %d exited with an error value.\n", chi[i]);
		fflush(stdout);*/
		/* Restore STDIN if necessary */
		if (i == 0) {
			if (input_flag == 1) {
				dup2(saved_stdin, STDIN_FILENO);
			}
		}
		/* Restore STDOUT if necessary */
		if (i == pipes) {
			if (output_flag == 1) {
				dup2(saved_stdout, STDOUT_FILENO);
			}
		}
	}
	else {
		/* If we unsuccessfully exited the child*/
		/*printf("Process %d succeeded.\n", chi[i]);
		fflush(stdout);*/
		/* Restore STDIN if necessary */
		if (i == 0) {
			if (input_flag == 1) {
				dup2(saved_stdin, STDIN_FILENO);
			}
		}
		/* Restore STDOUT if necessary */
		if (i == pipes) {
			if (output_flag == 1) {
				dup2(saved_stdout, STDOUT_FILENO);
			}
		}
	}
}

close(saved_stdin);
close(saved_stdout);

/* Free our arguments */
for (i = 0; i <= items; i++) {
	free(arguments[i]);
	arguments[i] = NULL;
}
for (i = 0; i <= parsed_items; i++) {
	free(parsed[i]);
	parsed[i] = NULL;
}

/* Continue prompting for input */
return 1;
}

/* Prompts the user for arguments */
int prompt_user() {
	/* Array to hold our input line
	3 larger to account for newline character,
	null character from fgets, and error handling purposes */
	char input[BUFFERSIZE] = { 0 };
	/* Will eventually hold our tokens */
	char arguments[256][LINEMAX + 1];
	static char argumentsHistory[256][LINEMAX + 1];
	/* Our number of tokens and the index of the newline character */
	int newline_index;
	int items = 0;
	static int itemsHistory = 0;
	char* passed_in[256] = { NULL };
	/* Used for iterating */
	int i;

	/* Get our input from the user */
	printf("osh>");
	fflush(stdout);
	fgets(input, BUFFERSIZE, stdin);
	/* CTRL - D? */
	if (feof(stdin)) {
		exit(0);
	}
	/* If our string is over LINEMAX, EXIT*/
	if (input[LINEMAX + 1] != '\0') {
		fprintf(stderr, "%s, File name too long\n", input);
		memset(input, '\0', LINEMAX + 3);
		return 1;
	}

	/* Removes newline character from our input */
	newline_index = (int)strlen(input) - 1;
	input[newline_index] = '\0';

	/*Tokenize our input and acquire the number of items inputted*/
	items = tokenize(arguments, input);

	/* No Input Case */
	if (items == 0) {
		return 1; /*Empty Input */
	}

	if (strcmp(arguments[0], "!!") == 0) {
		if (strcmp(argumentsHistory[0], "") == 0){
			printf("No commands in history\n");
			return 1;
		}

		printf("Echo: ");
		fflush(stdout);

	  /* Copy over our input */
	  for (i = 0; i < itemsHistory; i++) {
		  printf("%s ",argumentsHistory[i]);
		  fflush(stdout);
		  passed_in[i] = safe_malloc(strlen(argumentsHistory[i]) + 1);
		  strcpy(passed_in[i], argumentsHistory[i]);
	  }

	  printf("\n");
	  fflush(stdout);

	  return command_checker(passed_in, itemsHistory);

  }
  else {
	  /* Copy over our input */
	  for (i = 0; i < items; i++) {
		  passed_in[i] = safe_malloc(strlen(arguments[i]) + 1);
		  strcpy(passed_in[i], arguments[i]);
		  memset(argumentsHistory[i], '\0', LINEMAX + 1);
		  strcpy(argumentsHistory[i], arguments[i]);
		  itemsHistory = items;
		  memset(arguments[i], '\0', LINEMAX + 1);
	  }

	  return command_checker(passed_in, items);

  }
  
  /* Check our arguments */
  return command_checker(passed_in, items);
}

int interpret_script(char script_line[BUFFERSIZE]){
  /* Will eventually hold our tokens */
  char arguments[256][LINEMAX+1];
  static char argumentsHistory[256][LINEMAX + 1];
  /* Our number of tokens and the index of the newline character */
  int newline_index;
  int items = 0;
  static int itemsHistory = 0;
  char* passed_in[256] = { NULL };
  char *newlinecheck = NULL;
  /* Used for iterating */
  int i;

  /* Removes newline character from our input */
  newlinecheck = strchr(script_line, '\n');
  if (newlinecheck != NULL){
    newline_index = (int) strlen(script_line) - 1;
    script_line[newline_index] = '\0';
  }
  /* If our string is over LINEMAX, EXIT*/
  if ( script_line[LINEMAX+1] != '\0' ){
    fprintf(stderr, "%s: File name too long\n", script_line);
    return 1;
  }
  /*Tokenize our input and acquire the number of items inputted*/
  items = tokenize(arguments, script_line);

  /* No Input Case */
  if (items == 0){
    return 1; /*Empty Input */
  }


  if (strcmp(arguments[0], "!!") == 0) {
	  if (strcmp(argumentsHistory[0], "") == 0) {
		  printf("No commands in history\n");
		  return 1;
	  }

	  printf("Echo: ");
	  fflush(stdout);

	  /* Copy over our input */
	  for (i = 0; i < itemsHistory; i++) {
		  printf("%s ", argumentsHistory[i]);
		  fflush(stdout);
		  passed_in[i] = safe_malloc(strlen(argumentsHistory[i]) + 1);
		  strcpy(passed_in[i], argumentsHistory[i]);
	  }

	  printf("\n");
	  fflush(stdout);

	  return command_checker(passed_in, itemsHistory);
  }
  else {
	  /* Copy over our input */
	  for (i = 0; i < items; i++) {
		  passed_in[i] = safe_malloc(strlen(arguments[i]) + 1);
		  strcpy(passed_in[i], arguments[i]);
		  memset(argumentsHistory[i], '\0', LINEMAX + 1);
		  strcpy(argumentsHistory[i], arguments[i]);
		  itemsHistory = items;
		  memset(arguments[i], '\0', LINEMAX + 1);
	  }
  }
  /* Check our arguments */
  return command_checker(passed_in, items);
}

/* Shell Initializer */
void run_mush(){
  int continue_prompting;
  struct sigaction action;
  
  /* Set our interrupt signal */
  memset(&action, 0, sizeof(action));
  action.sa_handler = sigintHandler;
  sigaction(SIGINT, &action, NULL);
  /* Our shell loop */
  do{
    if ((isatty(STDOUT_FILENO) == 1) && (isatty(STDIN_FILENO) == 1)){
      continue_prompting = prompt_user();
    }
    else{
      continue_prompting = 0;
    }
  }while(continue_prompting);
}
