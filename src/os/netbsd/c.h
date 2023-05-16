#ifndef SRC_OS_NETBSD_C_H_
#define SRC_OS_NETBSD_C_H_

#include "util/compiler.h"
#include "util/int.h"

#ifdef CXX
extern "C" {
#endif

// amd64/types.h
// sys/ansi.h
// sys/types.h
typedef I64 blkcnt_t;
typedef I32 blksize_t;
typedef int clockid_t;
typedef U64 dev_t;
typedef U32 gid_t;
typedef U64 ino_t;
typedef U32 mode_t;
typedef U32 nlink_t;
typedef I64 off_t;
typedef I32 pid_t;
typedef U32 uid_t;

// sys/dirent.h
struct dirent {
    ino_t d_fileno;
    U16 d_reclen;
    U16 d_namlen;
    U8 d_type;
    char d_name[512];
};
#define DT_DIR 4
#define DT_REG 8

// sys/timespec.h
struct timespec {
    Time tv_sec;
    long tv_nsec;
};

// sys/uio.h
struct iovec {
    void* iov_base;
    Size iov_len;
};
SSize
writev(int, const struct iovec*, int) noexcept;

// sys/errno.h
// errno.h
int*
__errno() noexcept;
#define errno (*__errno())
#define EINTR 4

// fcntl.h
int
open(const char*, int, ...) noexcept;
#define O_RDONLY 0x00000000
#define O_WRONLY 0x00000001
#define O_CREAT  0x00000200
#define O_TRUNC  0x00000400

// sys/mman.h
void*
mmap(void*, Size, int, int, int, off_t) noexcept;
int
munmap(void*, Size) noexcept;
#define MAP_FAILED ((void*)-1)
#define MAP_SHARED 0x0001
#define PROT_READ  0x01

// sys/stat.h
struct stat {
    dev_t st_dev;
    mode_t st_mode;
    ino_t st_ino;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    struct timespec st_birthtim;
    off_t st_size;
    blkcnt_t st_blocks;
    blksize_t st_blksize;
    U32 st_flags;
    U32 st_gen;
    U32 st_spare[2];
};
int
fstat(int, struct stat*) noexcept __asm("__fstat50");
int
mkdir(const char*, mode_t) noexcept;
int
stat(const char*, struct stat*) noexcept __asm("__stat50");
#define S_IFMT     0170000
#define S_IFDIR    0040000
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)

// dirent.h
struct _dirdesc;
typedef struct _dirdesc DIR;
int
closedir(DIR*) noexcept;
DIR*
opendir(const char*) noexcept __asm("__opendir30");
struct dirent*
readdir(DIR*) noexcept __asm("__readdir30");
#define d_ino d_fileno

// math.h
#define atan2f __builtin_atan2f
#define ceilf  __builtin_ceilf
#define cosf   __builtin_cosf
#define floorf __builtin_floorf
#define sinf   __builtin_sinf
#define sqrtf  __builtin_sqrtf

// amd64/types.h
// pthread_types.h
// pthread.h
typedef unsigned char __pthread_spin_t;
struct __pthread_st;
typedef struct __pthread_st* pthread_t;
struct pthread_queue_t {
    struct __pthread_st* ptqh_first;
    struct __pthread_st** ptqh_last;
};
struct pthread_mutex_t {
    unsigned int ptm_magic;
    __pthread_spin_t ptm_errorcheck;
    U8 ptm_pad1[3];
    union {
        unsigned char ptm_ceiling;
        __pthread_spin_t ptm_unused;
    };
    U8 ptm_pad2[3];
    pthread_t ptm_owner;
    pthread_t* ptm_waiters;
    unsigned int ptm_recursed;
    void* ptm_spare2;
};
struct pthread_cond_t {
    unsigned int ptc_magic;
    __pthread_spin_t ptc_lock;
    pthread_queue_t ptc_waiters;
    pthread_mutex_t* ptc_mutex;
    void* ptc_private;
};
int
__libc_mutex_destroy(pthread_mutex_t*) noexcept;
int
__libc_mutex_lock(pthread_mutex_t*) noexcept;
int
__libc_mutex_unlock(pthread_mutex_t*) noexcept;
int
__libc_cond_destroy(pthread_cond_t*) noexcept;
int
__libc_cond_signal(pthread_cond_t*) noexcept;
int
__libc_cond_broadcast(pthread_cond_t*) noexcept;
int
__libc_cond_wait(pthread_cond_t*, pthread_mutex_t*) noexcept;
int
pthread_create(pthread_t*, const void*, void* (*)(void*), void*) noexcept;
int
pthread_join(pthread_t, void**) noexcept;
#define PTHREAD_MUTEX_INITIALIZER \
    { 0x33330003, 0, {0, 0, 0}, {0}, {0, 0, 0}, 0, 0, 0, 0 }
#define PTHREAD_COND_INITIALIZER \
    { 0x55550005, 0, {0, 0}, 0, 0 }
#define pthread_mutex_destroy  __libc_mutex_destroy
#define pthread_mutex_lock     __libc_mutex_lock
#define pthread_mutex_unlock   __libc_mutex_unlock
#define pthread_cond_destroy   __libc_cond_destroy
#define pthread_cond_signal    __libc_cond_signal
#define pthread_cond_broadcast __libc_cond_broadcast
#define pthread_cond_wait      __libc_cond_wait

// stdio.h
struct __sbuf {
    unsigned char* _base;
    int _size;
};
struct FILE {
    unsigned char* _p;
    int _r;
    int _w;
    unsigned short _flags;
    short _file;
    struct __sbuf _bf;
    int _lbfsize;
    void* _cookie;
    int (*_close)(void*);
    SSize (*_read)(void*, void*, Size);
    off_t (*_seek)(void*, off_t, int);
    SSize (*_write)(void*, const void*, Size);
    struct __sbuf _ext;
    unsigned char* _up;
    int _ur;
    unsigned char _ubuf[3];
    unsigned char _nbuf[1];
    int (*_flush)(void*);
    char _lb_unused[sizeof(struct __sbuf) - sizeof(int (*)(void*))];
    int _blk_size;
    off_t _offset;
};
int
fclose(FILE*) noexcept;
FILE*
fopen(const char*, const char*) noexcept;
int
fprintf(FILE*, const char*, ...) noexcept;
Size
fread(void*, Size, Size, FILE*) noexcept;
int
printf(const char*, ...) noexcept;
int
sprintf(char*, const char*, ...) noexcept;
extern FILE __sF[3];
#define stdin  (&__sF[0])
#define stdout (&__sF[1])
#define stderr (&__sF[2])

// stdlib.h
int
abs(int) noexcept;
void
exit(int) noexcept;
int
rand() noexcept;
void
srand(unsigned) noexcept;
double
strtod(const char*, char**) noexcept;
long
strtol(const char*, char**, int) noexcept;
unsigned long
strtoul(const char*, char**, int) noexcept;

// string.h
void*
memmem(const void*, Size, const void*, Size) noexcept;
void*
memmove(void*, const void*, Size) noexcept;
Size
strlen(const char*) noexcept;
#define memchr __builtin_memchr
#define memcmp __builtin_memcmp
#define memcpy __builtin_memcpy
#define memset __builtin_memset

// sys/time.h
// time.h
int
clock_gettime(clockid_t, struct timespec*) noexcept __asm("__clock_gettime50");
int
nanosleep(const struct timespec*, struct timespec*) noexcept
    __asm("__nanosleep50");
#define CLOCK_MONOTONIC 3

// sys/unistd.h
// unistd.h
int
close(int) noexcept;
void
_exit(int) noexcept __attribute__((noreturn));
int
ftruncate(int, off_t) noexcept;
int
isatty(int) noexcept;
SSize
pread(int, void*, Size, off_t) noexcept;
SSize
pwrite(int, const void*, Size, off_t) noexcept;
SSize
read(int, void*, Size) noexcept;
long
sysconf(int) noexcept;
SSize
write(int, const void*, Size) noexcept;
#define _SC_NPROCESSORS_ONLN 1002

#ifdef CXX
}
#endif

#endif  // SRC_OS_NETBSD_C_H_
