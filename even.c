#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

/*
  Signal Handler
   - Handles appropriate signals such as SIGHUP (1), and SIGINT (2), 
     with correct responses by not killing the program and printing a response.
   - Uses signal safe printing to terminal by utilising the write() function.
*/
void sigHandler(int sig) 
{
  const char *response = "";

  switch(sig) {
    case 1: // SIGHUP
      response = "Ouch!\n";
      write(1, response, 6); // file_descriptor, response_msg, n_bytes
      break;
    case 2: // SIGINT
      response = "Yeah!\n";
      write(1, response, 6); // file_descriptor, response_msg, n_bytes
      break;
  }

  return;
}

int main(int argc, char** argv) 
{
  int evenNum = 0;
  char evenNumStr[12];
  int num = atoi(argv[1]);

  signal(SIGINT, sigHandler);
  signal(SIGHUP, sigHandler);

  // Prints n even numbers.
  for (int i = 0; i < num; i++) 
  {
    // Cast int evenNum to char[] evenNumStr for valid printing through write.
    int len = snprintf(evenNumStr, sizeof(evenNumStr), "%d\n", evenNum);
    write(1, evenNumStr, len); // file_descriptor, response_msg, n_bytes
    evenNum += 2;
    sleep(5);
  }

  return 0;
}