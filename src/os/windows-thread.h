#ifndef SRC_OS_WINDOWS_THREAD_H_
#define SRC_OS_WINDOWS_THREAD_H_

#include "os/c.h"
#include "util/compiler.h"
#include "util/function.h"
#include "util/int.h"

extern "C" {
typedef struct _SYSTEM_INFO {
    union {
        DWORD dwOemId;
        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        } s;
    } u;
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

WINBASEAPI BOOL WINAPI
CloseHandle(HANDLE hObject) noexcept;
WINBASEAPI VOID WINAPI
GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) noexcept;
WINBASEAPI DWORD WINAPI
WaitForSingleObjectEx(HANDLE hHandle,
                      DWORD dwMilliseconds,
                      BOOL bAlertable) noexcept;
_ACRTIMP
size_t __cdecl _beginthreadex(void* security,
                              unsigned stack_size,
                              unsigned(WINAPI* start_address)(void*),
                              void* arglist,
                              unsigned initflag,
                              unsigned* thrdaddr) noexcept;

#define INFINITE    0xFFFFFFFF  // Infinite timeout.
#define WAIT_FAILED 0xFFFFFFFF
}

static unsigned WINAPI
beginthreadex_thunk(void* data) noexcept {
    Function<void()>* f = static_cast<Function<void()>*>(data);
    (*f)();
    delete f;
    return 0;
}

class Thread {
 public:
    inline explicit Thread(Function<void()> f) noexcept {
        Function<void()>* data =
                new Function<void()>(static_cast<Function<void()>&&>(f));
        id = reinterpret_cast<HANDLE>(_beginthreadex(0,
                                                     0,
                                                     beginthreadex_thunk,
                                                     static_cast<void*>(data),
                                                     0,
                                                     0));
        assert_(id);
    }
    Thread(Thread&& other) noexcept : id(other.id) { other.id = 0; }
    inline ~Thread() noexcept { assert_(id == 0); }

    Thread(const Thread&) = delete;
    Thread&
    operator=(const Thread&) = delete;

    inline void
    join() noexcept {
        assert_(id != 0);

        DWORD err = WaitForSingleObjectEx(id, INFINITE, false);
        (void)err;
        assert_(err != WAIT_FAILED);  // GetLastError();

        BOOL err2 = CloseHandle(id);
        (void)err;
        assert_(err2);  // GetLastError();

        id = 0;
    }

    HANDLE id = 0;
};

static inline unsigned
threadHardwareConcurrency() noexcept {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

static void
threadDisableTimerCoalescing() noexcept { }

#endif  // SRC_OS_WINDOWS_THREAD_H_
