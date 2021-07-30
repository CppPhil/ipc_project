#include <stdio.h>
#include <stdlib.h>

#include "launch_process.h"

int main(void)
{
  printf("Launcher.\n");

  if (!launchProcess("server", MODE_NAMED_PIPE)) {
    fprintf(stderr, "Could not launch server!\n");
  }

  if (!launchProcess("client", MODE_NAMED_PIPE)) {
    fprintf(stderr, "Could not launch client.\n");
  }

  return EXIT_SUCCESS;
}
