#include "mode.h"

const char* modeToString(Mode mode)
{
    switch (mode) {
    case MODE_SHARED_MEMORY:
        return "shared_memory";
    case MODE_NAMED_PIPE:
        return "named_pipe";
    case MODE_SOCKET:
        return "socket";
    default:
        break;
    }

    return "ERROR";
}
