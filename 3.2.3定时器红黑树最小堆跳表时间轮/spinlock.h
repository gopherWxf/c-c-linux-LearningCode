#ifndef SPINLOCK_H
#define SPINLOCK_H

struct spinlock {
	int lock;
};

void spinlock_init(struct spinlock *lock) {
	lock->lock = 0;
}

void spinlock_lock(struct spinlock *lock) {
	while (__sync_lock_test_and_set(&lock->lock, 1)) {}
}

int spinlock_trylock(struct spinlock *lock) {
	return __sync_lock_test_and_set(&lock->lock, 1) == 0;
}

void spinlock_unlock(struct spinlock *lock) {
	__sync_lock_release(&lock->lock);
}

void spinlock_destroy(struct spinlock *lock) {
	(void) lock;
}

#endif
