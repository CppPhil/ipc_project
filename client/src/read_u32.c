#include <ctype.h>
#include <stdio.h>

#include "read_u32.h"

uint32_t readU32(bool *ok)
{
    uint32_t buffer = 0;

    int charRead = getc(stdin);

    if (charRead == EOF) {
        *ok = false;
        return 0;
    }

    if (!isspace(charRead)) {
        if (!isdigit(charRead)) {
            *ok = false;
            return 0;
        }

        buffer += ((unsigned char) charRead) - '0';
    }

    while ((charRead = getc(stdin)) != EOF) {
        if (((unsigned char) charRead) == '\n') {
            break;
        }

        if (isspace(charRead)) {
            continue;
        }

        if (!isdigit(charRead)) {
            *ok = false;
            return 0;
        }

        const unsigned char uchar = (unsigned char) charRead;

        buffer *= 10;
        buffer += uchar - '0';
    }

    *ok = true;
    return buffer;
}
