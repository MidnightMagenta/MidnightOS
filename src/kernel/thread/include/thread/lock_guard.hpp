#ifndef MDOS_LOCK_GUARD_H
#define MDOS_LOCK_GUARD_H

namespace MdOS::thread {
template<typename t_lock>
concept Lockable = requires(t_lock t) {
	t.lock();
	t.release();
};

template<Lockable t_lock>
class LockGuard {
public:
	explicit LockGuard(t_lock *lock) {
		m_lock = lock;
		m_lock->lock();
	}
	~LockGuard() { m_lock->release(); }

	LockGuard(const LockGuard &other) = delete;
	LockGuard &operator=(const LockGuard &other) = delete;

private:
	t_lock *m_lock = nullptr;
};

template<typename T, Lockable t_lock>
class LockPtr {
public:
	explicit LockPtr(T *ptr, t_lock *lock) {
		m_ptr = ptr;
		m_lock = lock;
		m_lock->lock();
	}
	~LockPtr() { m_lock->release(); }

	LockPtr(const LockPtr &other) = delete;
	LockPtr &operator=(const LockPtr &other) = delete;

	T *operator->() { return m_ptr; }

private:
	T *m_ptr = nullptr;
	t_lock *m_lock = nullptr;
};
}// namespace MdOS::thread

#endif