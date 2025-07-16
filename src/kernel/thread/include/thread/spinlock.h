#ifndef MDOS_SPINLOCK_H
#define MDOS_SPINLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <thread/atomic.h>

#define SPINLOCK_MAX_TIMEOUT UINT64_MAX

typedef mdos_atomic_flag mdos_spinlock;

static inline uint64_t spinlock_irqsave() {
	uint64_t flags;
	__asm__ volatile("pushfq\n\t"
					 "pop %0\n\t"
					 "cli"
					 : "=r"(flags)
					 :
					 : "memory");
	return flags;
}

static inline void spinlock_irqrestore(uint64_t flags) {
	if (flags & (1 << 9)) { __asm__ volatile("sti"); }
}

static inline bool spinlock_test_lock(mdos_spinlock *lock) {
	return !atomic_flag_test_and_set((mdos_atomic_flag *) lock);
}

static inline bool spinlock_acquire_lock(mdos_spinlock *lock, uint64_t timeout) {
	while (!spinlock_test_lock(lock) && timeout--) { __builtin_ia32_pause(); }
	if (timeout == 0) { return false; }
	return true;
}

static inline void spinlock_release(mdos_spinlock *lock) { atomic_flag_clear((mdos_atomic_flag *) lock); }

static inline bool spinlock_acquire_irqsave(mdos_spinlock *lock, uint64_t timeout, uint64_t *flags) {
	*flags = spinlock_irqsave();
	return spinlock_acquire_lock(lock, timeout);
}

static inline void spinlock_release_irqrestore(mdos_spinlock *lock, uint64_t flags) {
	spinlock_irqrestore(flags);
	spinlock_release(lock);
}

#ifdef __cplusplus
}
#endif
#endif