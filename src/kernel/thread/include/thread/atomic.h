#ifndef MDOS_ATOMIC_H
#define MDOS_ATOMIC_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	volatile bool val;
} mdos_atomic_flag;

#define atomic_load(ptr, ret, memorder) __atomic_load(ptr, ret, memorder)
#define internal_atomic_test_and_set(ptr, memorder) __atomic_test_and_set(ptr, memorder);
#define internal_atomic_clear(ptr, memorder) __atomic_clear(ptr, memorder);

static inline bool atomic_flag_test(mdos_atomic_flag *obj) {
	bool val;
	atomic_load(&obj->val, &val, __ATOMIC_ACQUIRE);
	return val;
}
static inline bool atomic_flag_test_and_set(mdos_atomic_flag *obj) {
	return internal_atomic_test_and_set(&obj->val, __ATOMIC_SEQ_CST);
}

static inline void atomic_flag_clear(mdos_atomic_flag *obj) { internal_atomic_clear(&obj->val, __ATOMIC_RELEASE); }

#ifdef __cplusplus
}
#endif
#endif