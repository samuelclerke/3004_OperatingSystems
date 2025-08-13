#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void sigHandler(int sig) 
{
  switch(sig) {
    case 1:
      write("Ouch!\n");
      break;
    case 2:
      write("Yeah!\n");
      break;
  }

  return;
}

int main(int argc, char** argv) 
{
  int evenNum = 0;
  int num = atoi(argv[1]);

  signal(SIGINT, sigHandler);
  signal(SIGHUP, sigHandler);

  for (int i = 0; i < num; i++) 
  {
    printf("%d\n", evenNum);
    evenNum += 2;
    sleep(5);
  }

  return 0;
}