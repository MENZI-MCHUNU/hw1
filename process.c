#include "process.h"
#include "shell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>

/**
 * Executes the process p.
 * If the shell is in interactive mode and the process is a foreground process,
 * then p should take control of the terminal.
 */
void launch_process(process *p)
{

  /** YOUR CODE HERE */
    pid_t pid ;
    pid_t pgid;
   int infile;
    int outfile;
   int errfile;    
      int foreground = 0;
    if (shell_is_interactive){
           pid =getpid();
           if(pgid == 0)pgid = pid;
              setpgid(pid,pgid);
             if(foreground)
              tcsetpgrp(shell_terminal,pgid);
              
              
              /* Set the handling for job control signals back to the default.  */
       
      signal (SIGINT, SIG_DFL);
      signal (SIGQUIT, SIG_DFL);
      signal (SIGTSTP, SIG_DFL);
      signal (SIGTTIN, SIG_DFL);
      signal (SIGTTOU, SIG_DFL);
      signal (SIGCHLD, SIG_DFL);
            
         }   
     /* Set the standard input/output channels of the new process.  */
      
     /* if(infile !=STDIN_FILENO){
       dup2(infile, STDIN_FILENO);
       close(infile);
      }
       if(outfile !=STDOUT_FILENO){
        dup2(outfile,STDOUT_FILENO);
        close(outfile);
       }
       
       if(errfile !=STDERR_FILENO){
        dup2(errfile,STDERR_FILENO);
        close(errfile); 
       }
       */
        if((execvp(*p->argv,p->argv))<0){
       	      perror("execvp");
        	exit(1); 
    }
}

/* Put a process in the foreground. This function assumes  that the shell
 * is in interactive mode. If the cont argument is true, send the process
 * group a SIGCONT signal to wake it up.
 */
void
put_process_in_foreground (process *p, int cont)
{
  /** YOUR CODE HERE */
  
     /* Put the process into the foreground.  */
  tcsetpgrp (shell_terminal, /*j*/p->pgid);

  /* Send the job a continue signal, if necessary.  */
  if (cont)
    {
      tcsetattr (shell_terminal, TCSADRAIN, /*&j*/&p->tmodes);
      if (kill (- /*j*/p->pgid, SIGCONT) < 0)
        perror ("kill (SIGCONT)");
    }

  /* Wait for it to report.  */
  //wait_for_process (p);

  /* Put the shell back in the foreground.  */
  tcsetpgrp (shell_terminal, shell_pgid);

  /* Restore the shell's terminal modes.  */
  tcgetattr (shell_terminal, /*&j*/&p->tmodes);
  tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);
}

/* Put a process in the background. If the cont argument is true, send
 * the process group a SIGCONT signal to wake it up. */
void
put_process_in_background (process *p, int cont)
{
  /** YOUR CODE HERE */
/* Send the process a continue signal, if necessary.  */

  signal (SIGCHLD, SIG_IGN);
    if (cont)
    if (kill (-p->pgid, SIGCONT) < 0)
      perror ("kill (SIGCONT)");
   
  
}
