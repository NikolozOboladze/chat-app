#ifndef THREADS_H
#define THREADS_H

#ifdef _WIN32
#include <windows.h>
typedef HANDLE thread_t;
typedef DWORD thread_ret_t;
#define THREAD_CALL __stdcall
#define thread_create(thr, func, arg) ((*(thr) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(func), (arg), 0, NULL)) == NULL ? -1 : 0)
#define thread_join(thr) WaitForSingleObject((thr), INFINITE)
#define thread_detach(thr) CloseHandle((thr))

typedef SRWLOCK rwlock_t;
#define rwlock_init(lock) InitializeSRWLock(lock)
#define rwlock_readerlock(lock) AcquireSRWLockShared(lock)
#define rwlock_writerlock(lock) AcquireSRWLockExclusive(lock)
#define rwlock_readerunlock(lock) ReleaseSRWLockShared(lock)
#define rwlock_writerunlock(lock) ReleaseSRWLockExclusive(lock)

#else
#include <pthread.h>
typedef pthread_t thread_t;
typedef void *thread_ret_t;
#define THREAD_CALL
#define thread_create(thr, func, arg) pthread_create((thr), NULL, (func), (arg))
#define thread_join(thr) pthread_join((thr), NULL)
#define thread_detach(thr) pthread_detach((thr))

typedef pthread_rwlock_t rwlock_t;
#define rwlock_init(lock) pthread_rwlock_init(lock, NULL)
#define rwlock_readerlock(lock) pthread_rwlock_rdlock(lock)
#define rwlock_writerlock(lock) pthread_rwlock_wrlock(lock)
#define rwlock_readerunlock(lock) pthread_rwlock_unlock(lock)
#define rwlock_writerunlock(lock) pthread_rwlock_unlock(lock)

#endif

#endif