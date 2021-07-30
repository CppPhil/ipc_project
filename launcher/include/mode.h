#ifndef INCG_MODE_H
#define INCG_MODE_H

typedef enum
{
    MODE_SHARED_MEMORY,
    MODE_NAMED_PIPE,
    MODE_SOCKET
} Mode;

const char* modeToString(Mode mode);

#endif /* INCG_MODE_H */
