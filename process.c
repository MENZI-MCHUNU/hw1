#include "process.h"
#include "shell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>

#define WAIT_ANY -1
/**
 * Executes the process p.
 * If the shell is in interactive mode and the process is a foreground process,
 * then p should take control of the terminal.
 */
void launch_process(process *p)
{

 pid_t pid;
 pid_t pgid = p->pgid;

  if (shell_is_interactive)
    {
      /* Put the process into the process group and give the process group
         the terminal, if appropriate.
         This has to be done both by the shell and in the individual
         child processes because of potential race conditions.  */
      pid = getpid ();
      if (pgid == 0) pgid = pid;
      setpgid (pid, pgid);
	tcsetpgrp (shell_terminal, pgid);
      
      /* Set the handling for process control signals back to the default.  */
      signal (SIGINT, SIG_DFL);
      signal (SIGQUIT, SIG_DFL);
      signal (SIGTSTP, SIG_DFL);
      signal (SIGTTIN, SIG_DFL);
      signal (SIGTTOU, SIG_DFL);
      signal (SIGCHLD, SIG_DFL);
      
   }

/* Exec the new process.  Make sure we exit.  */
        if((execvp(*p->argv,p->argv))<0){
       	      perror("execvp");
        	exit(1); 
    }
    
}

/* Put a process in the foreground. This function assumes that the shell
 * is in interactive mode. If the cont argument is true, send the process
 * group a SIGCONT signal to wake it up.
 */
void
put_process_in_foreground (process *p, int cont)
{

    pid_t pid;
 /* Put the process into the foreground.  */
  tcsetpgrp (shell_terminal, p->pgid);

  /* Send the process a continue signal, if necessary.  */
  if (cont)
    {

      tcsetattr (shell_terminal, TCSADRAIN, &p->tmodes);
      if (kill (- p->pgid, SIGCONT) < 0)
        perror ("kill (SIGCONT)");
    }

  /* Wait for it to report.  */
    pid = waitpid (WAIT_ANY, &cont, WUNTRACED);

  /* Put the shell back in the foreground.  */
  tcsetpgrp (shell_terminal, shell_pgid);

  /* Restore the shell’s terminal modes.  */
  tcgetattr (shell_terminal, &p->tmodes);
  tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);
}

/* Put a process in the background. If the cont argument is true, send
 * the process group a SIGCONT signal to wake it up. */
void
put_process_in_background (process *p, int cont)
{

/* Send the process a continue signal, if necessary.  */
  if (cont)
    if (kill (-p->pgid, SIGCONT) < 0)
      perror ("kill (SIGCONT)");

}