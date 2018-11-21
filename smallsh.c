#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void get_operation(char *, bool *, bool *, bool *, char **);
void return_input (char *, char *);
void exit_status (int *);
void execute_command (char**, char *, char *, bool , int *);
void is_background_execute(pid_t , int *);
void catchSIGTSTP(int);
void get_operators(char *,bool *, char **, char**);
void execute_command1 (char**, char **, bool , int *);
void exit_status1 (int *,int *);

int allowbackground = true;

void catchSIGTSTP(int signo) {
  if (allowbackground == true) {
    char* message = "Entering foreground-only mode (& is now ignored)\n";
    write(1,message,49);
    fflush(stdout);
    allowbackground = false;
  }
  else {
    char *message = "Exiting foreground-only mode\n";
    write(1,message,29);
    fflush(stdout);
    allowbackground = true;
  }
  printf(":");
  fflush(stdout);
  return;
}

struct sigaction SIGINTact, SIGSTPact;

int index1 = 0;
int index2 = 0;

int main (int argc, char* argv[]) {
  // pid_t = getpid();
  printf("$\n");
  // char *input[512];
  int i = 0;
  // for (i=0;i<512;i++){
  //   input[i] = NULL;
  // }
  // signal(SIGINT, sigintHandler); 

  // Below code handels ctrl-c 
  SIGINTact.sa_handler = SIG_IGN;
  SIGINTact.sa_flags = SA_RESTART;
  sigfillset(&SIGINTact.sa_mask);
  SIGINTact.sa_flags = 0;
  sigaction(SIGINT, &SIGINTact, NULL);
  // Below code handels ctrl-z
  SIGSTPact.sa_handler = catchSIGTSTP;
  SIGSTPact.sa_flags = SA_RESTART;
  sigfillset(&SIGSTPact.sa_mask);
  sigaction(SIGTSTP, &SIGSTPact, NULL);

  int pass = 1;
  // int exit_statuss = 0;
  int childExitMethod = 0;    // Is the exit signal
  while(pass == 1){
    int prev_message = 0;
    exit_status1 (&childExitMethod, &prev_message);   // checks initially for any wrong exits, prints the exit status if any exits other than 0 occur
    char input[2049];   // Is the input array
    memset(input,'\0',sizeof(input));
    bool is_background = false;
    bool is_backwordarrow = false;
    bool is_forwardarrow = false;

    char *data[512];      // Stores everythin before < or >
    memset(data,'\0',sizeof(data));
    char *data2[512];   // Stores all data after < or > including the angular brackets
    memset(data2,'\0',sizeof(data2));
    // for (i=0;i<512;i++) {
    //   data[i] = NULL;
    //   data2[i] = NULL;
    // }
    printf(":");      // Prints the output screen
    fflush(stdout);
    fgets(input, 2048, stdin);      // Takes the input in
    if(!strcmp(input,"\n")) {       // Checks whether the input us an empty string
      // return 0;      
      continue;
    }
    // printf("Input is %s\n",input);
    char temp_copy2[2048];        // Will be copies of the input
    char temp_copy1[2048];        // Will be copies of the input
    strcpy(temp_copy1,input);
    strcpy(temp_copy2,temp_copy1);
    get_operators(input,&is_background, data, data2);
    // get_operation(input,&is_background, &is_forwardarrow, &is_backwordarrow, data);
    // printf(*data); 
    // printf(c);
    fflush(stdout);
    char *tempp;
    // strcpy(tempp,data[0]);
    // printf(data[0]);
    fflush(stdout);
    if (!strcmp(data[0], "exit\n") || strstr(data[0], "exit")) {    // Looks for the exit command to close the small shell
      pass = 0;
      // printf("GOD DAMN");
      break;
    }
    else if (data[0][0] == '#') {   // Is a comment, anything inputted over here will be ignored
      // printf("#  received");
      fgets(input, 2048, stdin);
      continue;    
    }
    else if (!strcmp(data[0], "cd")) {    // Will change the directory
      // printf ("inside");
      // printf(data[1]);
      if (data[1] != NULL) {
        // printf("inside");
        if (chdir(data[1]) == -1) {     // Error statement if the specified directory does not exist
          printf("Directory does not exist");
          fflush(stdout);
        }
      }
      else if (data[1] == NULL) {   // Will open the home directory
        // chdir("/");
        chdir(getenv("HOME"));    
      }
    }
    else if (!strcmp(data[0],"status")) {   // Will display the status
      if (prev_message == 0){
        exit_status(&childExitMethod);
      }
      else {
        // printf("GODDAMN\n");
        continue;
      }
    }
    else {
      // if (is_backwordarrow == true) {
      //   return_input(temp_copy1,open_Rfile);
      // }
      // if (is_forwardarrow == true) {
      //   return_input(temp_copy2,open_Wfile);
      // }
      // if (strstr(open_Wfile, "\n") != NULL) {      // removes the newline from the filename
      //   // printf("error here");
      //   // open_Wfile[][strlen(data[index-1])-1] = 0;
      //   strtok(open_Wfile, "\n");
      // }
      execute_command1(data, data2, is_background, &childExitMethod);   // Will call the function for executing the input given
    }
    index1 = 0;
    index2 = 0;
    // break;
  }
    return 0; 
}

// Places the data into our arrays
void get_operators(char *input,bool *is_background, char** data, char** data2) {
  if (!strcmp(input,"\n")){
    return;
  }
  int len = strlen(input);    // Removes the null character in these lines
  if (input[len-1] == '\n') {
    input[len-1] = '\0';
  }
  index1 = 0, index2 = 0;
  char *token = strtok(input," ");    // Splits the string and looks at the input indiidually
  int found_arrows = 0;
  while (token != NULL) {
    // printf("%s\n",token);
    if (!strcmp(token, "<")) {
      found_arrows = 1;
      // printf("%s\n",token);
      // strcpy(data2[index2++],"ASDSAD");
      data2[index2++] = strdup(token);
      token = strtok(NULL," ");
      data2[index2++] = strdup(token);
      // strcpy(data2[index2++],token);
    }
    else if (!strcmp(token,"&")) {
      // printf("ITS TRUE\n");
      *is_background = true;
    }
    else if (!strcmp(token,">")) {
      found_arrows = 1;
      data2[index2++] = strdup(token);
      // strcpy(data2[index2++],token);
      token = strtok(NULL," ");
      data2[index2++] = strdup(token);
      // strcpy(data2[index2++],token);
      found_arrows = 1;
    }
    else {
      fflush(stdout);
      if (strstr(token,"$$") != NULL) {
        // printf("Inside");
        // printf(token);
        char temp[1000];
        memset(temp,'\0',sizeof(temp));
        int pidd = getpid();
        char ppid[2024];
        sprintf(ppid, "%d", pidd);
        int k = 0;
        for(k=0;k<strlen(token);k++) {
          if (token[k] == '$') {
            if (k+1<strlen(token) && token[k+1] == '$') {
              token[k+1] = '\0';
              token[k] = '\0';
              strcpy(temp,token);
              snprintf(temp,1000,"%s%d",token,pidd);
            }
          }
        }
        // printf("\n");
        // printf(temp);
        // printf("\n");
        data[index1++] = strdup(temp);
        fflush(stdout);
      }
      else if (strcmp(token,"&")){
        if (found_arrows == 0) {
          // printf("%s\n",token);
          data[index1++] = strdup(token);
        }
        else {
          data2[index2++] = strdup(token);
        }
      }
    }
    token = strtok(NULL," ");
  }
  // if (*is_background == false) {

  //   data[index-1][strlen(data[index-1])-1] = 0;
  //   free(token);
  // }
}

void return_input (char *input, char *word) {
  char *token = strtok(input," ");
  // int index = 0;
  // printf(token);
  while (token != NULL) {
    // printf("%s\n",token);
    if (!strcmp(token, "<")) {
      token = strtok(NULL," ");
      // *is_backwordarrow = true;
      // printf(token);
      fflush(stdout);
      strcpy(word,token);
      // return (token);
    }
    else if (!strcmp(token,">")) {
      // *is_forwardarrow = true;
      token = strtok(NULL," ");
      // printf(token);
      fflush(stdout);
      strcpy(word,token);
      // return token;
    }
    token = strtok(NULL," ");
  }
  free(token);
}

// char *get_operation(char *input, bool *is_background) {
void get_operation(char *input, bool *is_background, bool *is_forwardarrow, bool *is_backwordarrow, char **data) {
  // printf("INSIDE");
  
  // if (str)
  if (!strcmp(input,"\n")){
    return;
  }
  char *token = strtok(input," ");
  int index = 0;
  while (token != NULL) {
    // printf("%s\n",token);
    if (!strcmp(token, "<")) {
      // token = strtok(NULL," ");
      *is_backwordarrow = true;
      // printf("<fgfgfg");
      // return (token);
    }
    else if (!strcmp(token,"&\n")) {
      *is_background == true;
      // return("&");
    }
    else if (!strcmp(token,">")) {
      // printf("asas>");
      *is_forwardarrow = true;
      // token = strtok(NULL," ");
      // return token;
    }
    else {
    // printf("Oustside\n");
      char *temp = token; // Comment following lines
      char *dataa[40];
      dataa[index] = malloc(sizeof(temp));
      strcat(dataa[index],temp);
      temp = NULL;
      if (strcmp(token,"&\n")){
        data[index++] = strdup(token);
      }
    }
    token = strtok(NULL," ");
  }
  if (*is_background == false) {

    data[index-1][strlen(data[index-1])-1] = 0;
    free(token);
  }
  //  if (strcmp())
}

void execute_command1 (char**data, char **data2, bool is_background, int *childExitMethod) {
    pid_t spawnpid = -5; int ten = 10;
  // int childExitStatus = -5;
  int j = 0;
  spawnpid = fork();
  // printf("SPAWNPID ISSS: %d", spawnpid);
  if (spawnpid == 0) {
    // printf("Initiate Change\n");
  }
        // printf("first%s\n",data[0]);
        // printf("second%s\n",data[1]);
        // printf("third%s\n",data[2]);
  fflush(stdout);
  bool read_input = false, write_input = false;
  switch (spawnpid) { 
    case -1:   // -1 is returned when an error is encountered
      perror("Hull Breach!");
      exit(1); 
      break; 
    case 0:   // If no errors are encountered
      // printf("");
      SIGINTact.sa_handler = SIG_DFL;     // These two lines are used to exit from processes
			sigaction(SIGINT, &SIGINTact, NULL);
      if (strcmp(data[0],"")) {
        // printf("");
      }

      int k = 0;
      char args1[512];
      memset(args1,'\0',sizeof(args1));
      char args2[512];
      memset(args2,'\0',sizeof(args2));
      for (k=0;k<index1;k++) {
        // printf("%s ",data[k]);
        // strcpy(args1[k],data[k]);
      }
      // printf("%d",is_background);
      // printf("\n");
      for (k=0;k<index2;k++) {
        // printf("%s ",data2[k]);
        // if ()
      }

      if (index1 > 1 && index2 == 0) {}

      else if (index1 > 0 && index2 > 0) {
        for (k=0;k<index2;k++) {
          if (!strcmp(data2[k],">")) {
            fflush(stdout);
            // printf(data[k+1]);
            fflush(stdout);
            
            // printf("error here");
            // fflush(stdout);

            int targetFD = open(data2[k+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);  // Opens the file for writing data
            if (targetFD == -1) { 
              perror("COuld not open file"); 
              exit(1); 
            }
            write_input = true;  // Means we have a file to write to
            // if (is_background == false || allowbackground == false) {
              int result = dup2(targetFD, 1);
              if (result == -1) { 
                perror("dup2");
                exit(2); 
              }
            // }
            // else {
              // close(targetFD);
            // }
            // printf("ERROR")
            
            break;
          }
        }
        for (k=0;k<index2;k++) {
          if (!strcmp(data2[k],"<")) {
            int r, newfifo, fd;
            // char completeMessage[512], readBuffer[10]; 
            int sourceFD = open(data2[k+1], O_RDONLY);  // Opens the file for reading
            if (sourceFD == -1) {     // -1 is returned above is the file was not opened properly
              perror("Could not open source"); 
              exit(1); 
            }
            read_input = true;
            // printf("Inside read part\n");
            fflush(stdout);
            // printf("sourceFD == %d\n", sourceFD); // Written to terminal
            // if (is_background == false || allowbackground == false) {
            int result = dup2(sourceFD, 0); 
            if (result == -1) { 
              perror("source dup2()"); 
              exit(2); 
              } 
            // }
            // else {
              // close(sourceFD);
            // }
          }
          break;
        }
      }
      
      char message[2000];
      memset(message,'\0',sizeof(message));
      strcpy(message,data[0]);
      if (strstr(message, "\n") != NULL) {      // removes the newline from the filename
        // printf("error here");
        strtok(message, "\n");
      }
      if (is_background && allowbackground) {
        // printf("ASDASDASD\n");        
        if (read_input && write_input){
          // continue;
        }
        else if ((read_input == false && write_input == false) || (read_input == false && write_input == true)) {
          fflush(stdout);
          // printf(data[k+1]);
          fflush(stdout);
          // printf("ASDASDASD\n");
          fflush(stdout);
            
          // int targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);  // Opens the file for writing data
          // if (targetFD == -1) { 
          //   perror("open() error"); 
          //   exit(1); 
          // }
          // write_input = true;
          // int result = dup2(targetFD, 1);
          // printf("ERROR")
          // if (result == -1) { 
            // perror("dup2");
            // exit(2); 
          // }  
        }
        else if (read_input == true && write_input == false) {
            int r, newfifo, fd;
            // char completeMessage[512], readBuffer[10]; 
            // printf("INSIDE HERE\n");
            int targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);  // Opens the file for writing data
            if (targetFD == -1) { 
              perror("Could not open file\n"); 
              exit(1); 
            }
            write_input = true;  // Means we have a file to write to
              int result = dup2(targetFD, 1);
              if (result == -1) { 
                perror("dup2");
                exit(2); 
              }
              // close(targetFD);
            break;
        }
      }

      if (execvp(data[0], data)) {
          printf("No such file or directory\n");
          fflush(stdout);
          exit(1);
        }

    default: 
    	if (is_background && allowbackground) {
				pid_t actualPid = waitpid(spawnpid, childExitMethod, WNOHANG);
				printf("background pid is %d\n", spawnpid);
        exit_status(childExitMethod);
				fflush(stdout);
			}
      else {
        pid_t actualPid = waitpid(spawnpid, childExitMethod, 0);
      }
      if ((spawnpid = waitpid(-1, childExitMethod, WNOHANG)) > 0) {
			  printf("background %d terminated, ", spawnpid);
			  exit_status(childExitMethod);
			  fflush(stdout);
		}
  }
}

void is_background_execute(pid_t spawnpid, int *childExitMethod) {
  // printf("PARENT(%d): Sleeping for 2 seconds\n", getpid());
  // sleep(2);
  // printf("PARENT(%d): Wait()ing for child(%d) to terminate\n", getpid(), spawnPid); 
  pid_t actualPid = waitpid(spawnpid, &childExitMethod, WNOHANG);
  printf("Background child: %d terminated\n",spawnpid);
  exit_status(childExitMethod);
  // exit(0); break;

}


void exit_status (int *childExitMethod) {
  if (WIFEXITED(*childExitMethod))
    printf("exit value %d\n", WEXITSTATUS(*childExitMethod));  // WEXIT
  else
    printf("Exited by a signal %d\n", WTERMSIG(*childExitMethod));  // Another Macro
}

void exit_status1 (int *childExitMethod, int *prev) {
  if (!WIFEXITED(*childExitMethod)){
    printf("Exited by a signal %d\n", WTERMSIG(*childExitMethod));  // Another Macro
    *prev = 1;
  }
}