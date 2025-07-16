#ifndef MDOS_SPINLOCK_HPP
#define MDOS_SPINLOCK_HPP

#include <thread/spinlock.h>

namespace MdOS::thread {
class Spinlock {
public:
	Spinlock() { spinlock_release(&m_lock); }
	~Spinlock() {}

	bool test() { return spinlock_test_lock(&m_lock); }
	bool lock(uint64_t timeout = SPINLOCK_MAX_TIMEOUT) { return spinlock_acquire_lock(&m_lock, timeout); }
	void release() { spinlock_release(&m_lock); }

private:
	mdos_spinlock m_lock;
};
}// namespace MdOS::thread


#endif