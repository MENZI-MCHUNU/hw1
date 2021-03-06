#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <linux/limits.h>
#define INPUT_STRING_SIZE 80
#define DELIMS " \t\r\n"
#define MAX_LENGTH 1024

#include <sys/stat.h>
#include <fcntl.h>

#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"

#define BUFFERSIZE 200

#define STDIN 1
#define STDOUT 2

#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token sparators


extern char **environ;

int cmd_quit(tok_t arg[]) {
  printf("Bye\n");
  exit(0);
  return 1;
}

int cmd_help(tok_t arg[]);
int cmd_and(tok_t arg[],char *c);
int cmd_cd(tok_t arg[]);

/* Command Lookup table */
typedef int cmd_fun_t (tok_t args[]); /* cmd functions take token array and return int */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;

 
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_cd, "cd", "change Directory"},
  {cmd_and, "&", ""},
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
  
}

int lookup(char cmd[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;

}

///////////////////////////////////////////////////////////////////////////////////


int cmd_and(tok_t arg[],char *c){
   
    process *p;
    int cont;    


   if(arg[0] =="&"){
     //put_process_in_background (p,cont);
         } 
   else if(c!="&"){
     arg[0]==NULL;
 
    }

  return 0 ;
}



/////////////////////////////////////////////////////////////////////////////////

int cmd_cd(tok_t arg[]){
     char cwd[1024];

     if(arg[0] == NULL){
     	chdir(getenv("HOME"));
      }
    else if (strcmp(arg[0],"~") == 0){
      chdir(getenv("HOME"));
      }
     else
         chdir(arg[0]);
}



//////////////////////////////////////////////////////////////////////////////////


void init_shell()
{

    char* cwd;
    char buff[PATH_MAX + 1];
   
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;

  /** Note that we cannot take control of the terminal if the shell
      is not interactive */
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){

    /* force into foreground */
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp()))
      kill( - shell_pgid, SIGTTIN);

    shell_pgid = getpid();
    /* Put shell in its own process group */
    if(setpgid(shell_pgid, shell_pgid) < 0){
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
  /** YOUR CODE HERE */
      signal (SIGINT, SIG_IGN);
      signal (SIGQUIT, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTTOU, SIG_IGN);
      signal (SIGCHLD, SIG_IGN);
      
}


///////////////////////////////////////////////////////////////////////////////////


/**
 * Add a process to our process list
 */
 
void add_process(process* p)
{
  process* curr = first_process;	
	if(first_process) {
		while(curr->next) {
			curr=curr->next;
		}
		curr->next = p;
		p->prev = curr;
	} else {
		first_process = p;
	}
	
}


/////////////////////////////////////////////////////////////////////////////


/**
 * Creates a process given the inputString from stdin
 */
process* create_process(char* inputString)
{
      process* p;// Declaring  a new process
      p = malloc(sizeof(process));//allocating memory for the process
      tok_t *t;
      t = getToks(inputString);  //getting the tokens from standard input
      p->argv = t;      
	int i, n =0; 
      for(i =0; i < MAXTOKS && t[i]; i++){
        n++;//the number of tokens

      }
    p->argc = n;

    p->pgid = getpgrp();

	printf("%d", p->pgid);

    return p;

}


///////////////////////////////////////////////////////////////////////////////////////////


char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////


void path(tok_t *t){
 char * val;  

      val = getenv("PATH");

     tok_t *paths   =getToks(val);
       int i;

      for(i =0 ; i < MAXTOKS && paths[i];i++ ){
                      char *path =concat(paths[i], "/");
                       path  =concat(path,t[0]); 
                      if(access(path,F_OK)!=-1){
                            execve(path,t,NULL); 
                         }        

    }
	      execv(*t,t);
               perror(*t);
               exit(0);
}

//////////////////////////////////////////////////////////////////////////////////////////

void Redirecting_In_Out(tok_t *input,char * filename,char * c){ 
int newfd;
  
 if(c == ">"){
  if ((newfd = open(filename, O_CREAT|O_WRONLY | O_APPEND, 0644)) < 0) {
	perror("error");
	exit(1);
  }
 
 dup2(newfd, 1);
 close(newfd);
 }
 if(c == "<"){
  if ((newfd = open(filename, O_RDONLY, 0644)) < 0) {
	perror(input);
	exit(1);
  }
 dup2(newfd,0);
 close(newfd);
 }
 
// path(input);
 
}

/////////////////////////////////////////////////////////////////////////////////////////

int shell (int argc, char *argv[]) {

  char *s = malloc(INPUT_STRING_SIZE+1);			/* user input string */
  tok_t *t;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  pid_t pid = getpid();		/* get current processes PID */
  pid_t ppid = getppid();	/* get parents PID */
  pid_t cpid, tcpid, cpgid;
  char cwd[1024];
   int status;
                                                                        
 
   printf("%s running as PID %d under %d\n",argv[0],pid,ppid);
  
    lineNum=0;
    fprintf(stdout, "%d%s:",lineNum,getcwd(cwd,sizeof(cwd)));
    init_shell();

  while ((s = freadln(stdin))){
  
        char * temp;
        temp = concat(s,"");
  
    t = getToks(s); /* break the line into tokens */
    fundex = lookup(t[0]); /* Is first token a shell literal */
    if(fundex >= 0) cmd_table[fundex].fun(&t[1]);
     
    else {
       
     if ((pid = fork()) < 0) {     /* fork a child process           */
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }

     else if (pid == 0) {    /* for the child process:         */
      process * new =create_process(temp);
          add_process(new);
             launch_process(new);
      

      int i;
      char * command1=">", * command2= "<";
      char *amp ="&";
    for(i=0;i<MAXTOKS && t[i];i++){
              if(strcmp(t[i],amp)== 0){
                t[i]=NULL;
            cmd_and(t[i],amp);
   }
            
      
	if (strcmp(t[i],command1)==0){
	   t[i]=NULL;
	    Redirecting_In_Out(t,t[i+1],command1);	
          }
        if(strcmp(t[i],command2)==0){
           t[i]=NULL;
            Redirecting_In_Out(t,t[i+1],command2);
	    }
	  }
	  
          path(t);


        }
        
      else {    /* for the parent:      */
	wait(&status);
         /* wait for completion  */
           
    }
      
      fprintf(stdout, "This shell only supports built-ins. Replace this to run programs as commands.\n");
     
     }
    fprintf(stdout, "%d%s:",++lineNum,getcwd(cwd,sizeof(cwd)));
   }
   
     
  return 0;
}