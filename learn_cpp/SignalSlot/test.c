#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "SignalSlot.h"

typedef struct {
	int called_a;
	int called_b;
	int last_value;
} ctx_t;

static void slot_a(void *arg) {
	ctx_t *ctx = (ctx_t *)arg;
	ctx->called_a++;
	ctx->last_value += 1;
    (void)fprintf(stdout, "slot_a called, last_value=%d\n", ctx->last_value);
}

static void slot_b(void *arg) {
	ctx_t *ctx = (ctx_t *)arg;
	ctx->called_b++;
	ctx->last_value += 10;
    (void)fprintf(stdout, "slot_b called, last_value=%d\n", ctx->last_value);
}

static void slot_0(void *arg) {(void)arg;}
static void slot_1(void *arg) {(void)arg;}
static void slot_2(void *arg) {(void)arg;}
static void slot_3(void *arg) {(void)arg;}
static void slot_4(void *arg) {(void)arg;}
static void slot_5(void *arg) {(void)arg;}
static void slot_6(void *arg) {(void)arg;}
static void slot_7(void *arg) {(void)arg;}
static void slot_8(void *arg) {(void)arg;}

static void test_connect_and_deduplicate(void) {
	signal_t sig = SIGNAL_INIT(test_sig);
	assert(signal_connect(&sig, slot_a) == EXIT_SUCCESS);
	assert(signal_connect(&sig, slot_a) == EXIT_SUCCESS);
	assert(sig.slot_count == 1);
}

static void test_connect_limit(void) {
	signal_t sig = SIGNAL_INIT(limit_sig);
	slot_fn_t unique_slots[MAX_SLOTS + 1] = {
		slot_0, slot_1, slot_2, slot_3, slot_4,
		slot_5, slot_6, slot_7, slot_8
	};

	for (int i = 0; i < MAX_SLOTS; ++i) {
		assert(signal_connect(&sig, unique_slots[i]) == EXIT_SUCCESS);
	}

	assert(sig.slot_count == MAX_SLOTS);
	assert(signal_connect(&sig, unique_slots[MAX_SLOTS]) == EXIT_FAILURE);
}

static void test_disconnect(void) {
	signal_t sig = SIGNAL_INIT(disconnect_sig);

	assert(signal_connect(&sig, slot_a) == EXIT_SUCCESS);
	assert(signal_connect(&sig, slot_b) == EXIT_SUCCESS);
	assert(sig.slot_count == 2);

	assert(signal_disconnect(&sig, slot_a) == EXIT_SUCCESS);
	assert(sig.slot_count == 1);
	assert(signal_disconnect(&sig, slot_a) == EXIT_FAILURE);
}

static void test_emit(void) {
	signal_t sig = SIGNAL_INIT(emit_sig);
	ctx_t ctx = {0};

	assert(signal_connect(&sig, slot_a) == EXIT_SUCCESS);
	assert(signal_connect(&sig, slot_b) == EXIT_SUCCESS);

	signal_emit(&sig, &ctx);

	assert(ctx.called_a == 1);
	assert(ctx.called_b == 1);
	assert(ctx.last_value == 11);
}

static void test_null_protection(void) {
	signal_t sig = SIGNAL_INIT(null_sig);

	assert(signal_connect(NULL, slot_a) == EXIT_FAILURE);
	assert(signal_connect(&sig, NULL) == EXIT_FAILURE);
	assert(signal_disconnect(NULL, slot_a) == EXIT_FAILURE);
	assert(signal_disconnect(&sig, NULL) == EXIT_FAILURE);
}

int main(void) {
	test_connect_and_deduplicate();
	test_connect_limit();
	test_disconnect();
	test_emit();
	test_null_protection();

	puts("All signal-slot tests passed.");
	return 0;
}
