#ifndef SRC_OS_WINDOWS_CONDITION_VARIBLE_H_
#define SRC_OS_WINDOWS_CONDITION_VARIBLE_H_

#include "os/c.h"
#include "os/mutex.h"
#include "util/assert.h"
#include "util/compiler.h"

extern "C" {
typedef struct {
    PVOID Ptr;
} CONDITION_VARIABLE, *PCONDITION_VARIABLE;

#define CONDITION_VARIABLE_INIT \
    { 0 }

WINBASEAPI VOID WINAPI
WakeConditionVariable(PCONDITION_VARIABLE ConditionVariable) noexcept;

WINBASEAPI VOID WINAPI
WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable) noexcept;

WINBASEAPI BOOL WINAPI
SleepConditionVariableSRW(PCONDITION_VARIABLE ConditionVariable,
                          PSRWLOCK SRWLock,
                          DWORD dwMilliseconds,
                          ULONG Flags) noexcept;

#define INFINITE 0xFFFFFFFF  // Infinite timeout.
}

class ConditionVariable {
 public:
    inline ConditionVariable() noexcept : cv(CONDITION_VARIABLE_INIT) { }

    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable&
    operator=(const ConditionVariable&) = delete;

    inline void
    notifyOne() noexcept {
        WakeConditionVariable(&cv);
    }
    inline void
    notifyAll() noexcept {
        WakeAllConditionVariable(&cv);
    }

    inline void
    wait(LockGuard& lock) noexcept {
        BOOL ok = SleepConditionVariableSRW(&cv, &lock.m.m, INFINITE, 0);
        (void)ok;
        assert_(ok);  // GetLastError();
    }

    CONDITION_VARIABLE cv;
};

#endif  // SRC_OS_WINDOWS_CONDITION_VARIBLE_H_
