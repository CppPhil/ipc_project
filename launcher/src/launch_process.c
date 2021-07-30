#include <sys/types.h>
#include <unistd.h>

#include "launch_process.h"

bool launchProcess(const char *fileToRun, Mode mode)
{
    char *argv[]
        = {(char *) fileToRun, (char *) modeToString(mode), (char *) NULL};

    const pid_t pid = fork();

    /* Handle failure to fork */
    if (pid == -1) {
        return false;
    }

    if (pid == 0) {
        /* Child code */
        const int statusCode = execv(fileToRun, argv);

        if (statusCode == -1) {
            return false;
        }

        return false;
    }
    else {
        /* Parent code */
        return true;
    }
}
