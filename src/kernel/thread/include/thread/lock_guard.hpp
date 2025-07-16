#ifndef MDOS_LOCK_GUARD_H
#define MDOS_LOCK_GUARD_H

namespace MdOS::thread {
template<typename t_lock>
class LockGuard {
public:
	LockGuard() { m_lock.lock(); }
	~LockGuard() { m_lock.release(); }

	LockGuard(const LockGuard &other) = delete;
	LockGuard &operator=(const LockGuard &other) = delete;

private:
	t_lock m_lock;
};
}// namespace MdOS::thread

#endif