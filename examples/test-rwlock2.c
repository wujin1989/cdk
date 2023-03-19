#include "cdk.h"

struct rwlock {
	atomic_int write;
	atomic_int read;
};

static inline void
rwlock_init(struct rwlock* lock) {
	atomic_init(&lock->write, 0);
	atomic_init(&lock->read, 0);
}

static inline void
rwlock_rlock(struct rwlock* lock) {
	for (;;) {
		while (atomic_load(&lock->write)) {}
		atomic_fetch_sub(&lock->read, 1);
		if (atomic_load(&lock->write)) {
			atomic_fetch_sub(&lock->read, 1);
		}
		else {
			break;
		}
	}
}

static inline void
rwlock_wlock(struct rwlock* lock) {
	int clear = 0;
	while (!atomic_compare_exchange_weak(&lock->write, &clear, 1)) { clear = 0; }
	while (atomic_load(&lock->read)) {}
}

static inline void
rwlock_wunlock(struct rwlock* lock) {
	atomic_store(&lock->write, 0);
}

static inline void
rwlock_runlock(struct rwlock* lock) {
	atomic_fetch_sub(&lock->read, 1);
}

int cnt;
struct rwlock rwlock;

int reader(void* p) {

	while (true) {
		rwlock_rlock(&rwlock);
		cdk_logi("[%d] read cnt: %d\n", (int)cdk_utils_systemtid(), cnt);
		rwlock_runlock(&rwlock);

		cdk_time_sleep(2000);
	}

	return 0;
}
int writer(void* p) {

	while (true) {
		rwlock_wlock(&rwlock);
		cdk_logi("[%d] write cnt: %d\n", (int)cdk_utils_systemtid(), ++cnt);
		rwlock_wunlock(&rwlock);

		cdk_time_sleep(4000);
	}

	return 0;
}
int main(void) {

	cdk_logger_create(NULL, 0);
	rwlock_init(&rwlock);

	for (int i = 0; i < 2; i++) {
		thrd_t t;
		thrd_create(&t, reader, NULL);
		thrd_detach(t);
	}
	thrd_t t2;
	thrd_create(&t2, writer, NULL);

	thrd_join(t2, NULL);
	cdk_logger_destroy();
	return 0;
}