#include <HLMD.h>

#define NULL ((void*)0)

HLMD_EXPAND(HLMD_DEFER(HLMD_REMOVE_PAREN) NULL)

int fun() {
    HLMD_BOOL(asdfa);
    HLMD_BOOL(nullptr);
    HLMD_BOOL(NULL);
    HLMD_BOOL(false);
    HLMD_BOOL(true);
    return 0;
}
