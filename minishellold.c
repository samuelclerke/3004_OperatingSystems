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


/*
	shell prompt
 */

void prompt(void)
{
  fflush(stdout);
  return;
}

struct job
{
  pid_t pid;
  unsigned int jobId;
  char cmd[256];
  int running;
};

struct job jobs[100];
int job_count = 0;

void add_job(pid_t pid, char *cmd)
{
  jobs[job_count].jobId = job_count + 1;
  jobs[job_count].pid = pid;
  strncpy(jobs[job_count].cmd, cmd, 255);
  jobs[job_count].cmd[255] = '\0';
  jobs[job_count].running = 1;
  job_count++;
}

void sig_handler(int sig)
{
  int status;
  pid_t pid;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
    for (int i = 0; i < job_count; i++)
    {
      if (jobs[i].pid == pid && jobs[i].running)
      {
        jobs[i].running = 0;
        printf("[%d]+ Done                 %s\n", jobs[i].jobId, jobs[i].cmd);
        fflush(stdout);
      }
    }
  }
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

int jobs_running()
{
  for (int i = 0; i < job_count; i++)
  {
    if (jobs[i].running)
    {
      return 1;
    }
  }
  return 0;
}

/* argk - number of arguments */
/* argv - argument vector from command line */
/* envp - environment pointer */
int main(int argk, char *argv[], char *envp[])
{
  int             frkRtnVal;	    /* value returned by fork sys call */
  int             wpid;		        /* value returned by wait */
  char           *v[NV];	        /* array of pointers to command line tokens */
  char           *sep = " \t\n";  /* command line token separators    */
  int             i;		          /* parse index */
  int             bgProc = 0;

    /* prompt for and process one command line at a time  */

  signal(SIGCHLD, sig_handler);

  while (1) {			/* do Forever */
    prompt();
    fgets(line, NL, stdin);
    fflush(stdin);

    if (feof(stdin)) {		/* non-zero on EOF  */

      while (jobs_running())
      {
        sleep(1);
      }
      exit(0);
    }
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\000'){
      continue;			/* to prompt */
    }

    v[0] = strtok(line, sep);
    for (i = 1; i < NV; i++) {
      v[i] = strtok(NULL, sep);
      if (v[i] == NULL){
	      break;
      }
    }

    bgProc = 0;
    if (i > 1 && strcmp(v[i-1], "&") == 0)
    {
      bgProc = 1;
      v[i-1] = NULL;
    }
    
    if (strcmp(v[0], "cd") == 0) 
    {
      if (v[1] == NULL) 
      {
        char *home = getenv("HOME");
        if (chdir(home) != 0) 
        {
          perror("cd");
        }
      }
      else 
      {
        if (chdir(v[1]) != 0) 
        {
          perror("cd");
        }
      }
      continue;
    }

    /* assert i is number of tokens + 1 */

    /* fork a child process to exec the command in v[0] */
    switch (frkRtnVal = fork()) {
      case -1:			/* fork returns error to parent process */
      {
        perror("fork failed");
	      break;
      }
      case 0:			/* code executed only by child process */
      {
        execvp(v[0], v);
        perror("execvp failed");
        exit(1);
      }
      default:			/* code executed only by parent process */
      {
        if (bgProc)
        {
          char cmdline[256];
          join_tokens(cmdline, v, sizeof(cmdline));
          printf("[%d] %d\n", job_count + 1, frkRtnVal);
          add_job(frkRtnVal, cmdline);
        }
        else 
        {
         wpid = wait(0);
         wpid += 1;
        }
    	  break;
      }
    }				/* switch */
  }				/* while */
}				/* main */
