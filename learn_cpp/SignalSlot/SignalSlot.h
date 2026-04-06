#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifndef __cplusplus
#include <stdbool.h>
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

#define MAX_SLOTS 8

typedef void (*slot_fn_t)(void *arg);

typedef struct {
    const char *name;
    slot_fn_t slots[MAX_SLOTS];
    uint8_t slot_count;
} signal_t;

#define SIGNAL_INIT(sig_name) {#sig_name, {0}, 0}

int signal_connect(signal_t * const sig, slot_fn_t slot) NOEXCEPT {
    if (!sig || !slot) return EXIT_FAILURE;

    for (uint8_t i = 0; i < sig->slot_count; ++i)
        if (sig->slots[i] == slot) return EXIT_SUCCESS;

    if (sig->slot_count >= MAX_SLOTS) return EXIT_FAILURE;

    sig->slots[sig->slot_count++] = slot;

    return EXIT_SUCCESS;
}

int signal_disconnect(signal_t * const sig, slot_fn_t slot) NOEXCEPT {
    if (!sig || !slot) return EXIT_FAILURE;

    for (uint8_t i = 0; i < sig->slot_count; ++i)
        if (sig->slots[i] == slot) {
            sig->slots[i] = sig->slots[--sig->slot_count];
            sig->slots[sig->slot_count] = NULL;
            return EXIT_SUCCESS;
        }

    return EXIT_FAILURE;
}

void signal_emit(signal_t *sig, void *arg) NOEXCEPT {
    if (!sig) return;

    for (uint8_t i = 0; i < sig->slot_count; ++i)
        if (sig->slots[i]) sig->slots[i](arg);
}
