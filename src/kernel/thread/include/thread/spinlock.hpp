#ifndef MDOS_SPINLOCK_HPP
#define MDOS_SPINLOCK_HPP

#include <thread/spinlock.h>

namespace MdOS::thread {
class Spinlock {
public:
	Spinlock() { spinlock_release(&m_lock); }
	~Spinlock() { release(); }

	bool test() {
		if (spinlock_test_lock(&m_lock)) {
			flags = spinlock_irqsave();
			return true;
		}
		return false;
	}
	bool lock(uint64_t timeout = SPINLOCK_MAX_TIMEOUT) { return spinlock_acquire_irqsave(&m_lock, timeout, &flags); }
	void release() { spinlock_release_irqrestore(&m_lock, flags); }

private:
	uint64_t flags;
	mdos_spinlock m_lock;
};
}// namespace MdOS::thread


#endif