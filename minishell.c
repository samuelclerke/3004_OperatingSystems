/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20			/* max number of command tokens */
#define NL 100			/* input buffer size */
char            line[NL];	/* command input buffer */


struct job
{
  int id;
  pid_t pid;
  char completed_msg[256];
  int running;
};

struct job jobs[256] = {0};

/*
	shell prompt
 */

void prompt(void)
{
  // ## REMOVE THIS 'fprintf' STATEMENT BEFORE SUBMISSION
  fflush(stdout);
}

int getNextJobSlot()
{
  int idx = 0;
  for (int i = 0; i < 256; i++)
  {
    if (jobs[i].running == 0)
    {
      idx = i;
      break;
    }
  }
  return idx;
}

void sigchld_handler(int sig)
{
  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
    for (int i = 0; i < 256; i++)
    {
      if (jobs[i].pid == pid && jobs[i].running)
      {
        jobs[i].running = 0;
        // Write only the actual message length, not the full buffer
        int msg_len = strlen(jobs[i].completed_msg);
        write(1, jobs[i].completed_msg, msg_len);
        fflush(stdout);
      }
    }
  }
}

void add_job(pid_t pid, char *cmd, int slot)
{
  jobs[slot].id = slot + 1;
  jobs[slot].pid = pid;
  jobs[slot].running = 1;
  snprintf(jobs[slot].completed_msg, sizeof(jobs[slot].completed_msg), "[%d]+ Done %s\n", jobs[slot].id, cmd);
}

void join_tokens(char *dest, char *tokens[], int maxlen)
{
  dest[0] = '\0';
  for (int i = 0; tokens[i] != NULL; i++)
  {
    if(strlen(dest) + strlen(tokens[i]) + 2 >= maxlen)
    {
      break;
    }
    strcat(dest, tokens[i]);
    if (tokens[i+1] != NULL)
    {
      strcat(dest, " ");
    }
  }
}

void wait_for_background_jobs() {
  int has_running_jobs = 1;
  while (has_running_jobs) {
    has_running_jobs = 0;
    for (int i = 0; i < 256; i++) {
      if (jobs[i].running) {
        has_running_jobs = 1;
        break;
      }
    }
    if (has_running_jobs) {
      usleep(100000);
    }
  }
}

/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
int main(int argk, char *argv[], char *envp[])
{
  int             frkRtnVal;	    /* value returned by fork sys call */
  char           *v[NV];	        /* array of pointers to command line tokens */
  char           *sep = " \t\n";  /* command line token separators    */
  int             i;		          /* parse index */
  int             bgProcess;

    /* prompt for and process one command line at a time  */
signal(SIGCHLD, sigchld_handler);
    
  while (1) {			/* do Forever */
    prompt();
    fgets(line, NL, stdin);
    fflush(stdin);

    // This if() required for gradescope
    if (feof(stdin)) {		/* non-zero on EOF  */
      wait_for_background_jobs();
      exit(0);
    }
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000'){
      continue;			/* to prompt */
    }

    line[strcspn(line, "\n")] = '\0';
    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL){
	      break;
      }
    }
    /* assert i is number of tokens + 1 */

    // Real working cd implementation because cd would only change child directory.

    if (strcmp(v[0], "cd") == 0)
    {
      if (v[1] == NULL) // should return currenct directory to the systems home
      {
        char *home = getenv("HOME");
        if (chdir(home) != 0)
        {
          perror("cd");
        }
      }
      else
      {
        if (chdir(v[1]) != 0) // execute change directory and if fails perror
        {
          perror("cd");
        }
      }
      continue; // continue to next event iteration
    }

    // Check if process is intended to be background process via appended &

    bgProcess = 0;
    if (i > 1 && strcmp(v[i-1], "&") == 0)
    {
      bgProcess = 1;
      v[i-1] = NULL;
      i--;
    }

    /* fork a child process to exec the command in v[0] */
    switch (frkRtnVal = fork()) {
      case -1:			/* fork returns error to parent process */
      {
	      break;
      }
      case 0:			/* code executed only by child process */
      {
	      execvp(v[0], v);
        perror(v[0]); // Print error if execvp fails
        exit(1);      // Exit child if execvp fails
      }
      default:			/* code executed only by parent process */
      {
        if (bgProcess)
        {
          int slot = getNextJobSlot();

          char cmdline[256];
          join_tokens(cmdline, v, sizeof(cmdline));
          printf("[%d] %d\n", slot + 1, frkRtnVal);
          add_job(frkRtnVal, cmdline, slot);
        }
        else
        {
          waitpid(frkRtnVal, NULL, 0);
        }
    	  break;
      }
    }				/* switch */
  }				/* while */
}				/* main */