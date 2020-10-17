/********************************
** Tsunagari Tile Engine       **
** new.cpp                     **
** Copyright 2020 Paul Merrill **
********************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#ifdef __APPLE__

#include "os/c.h"
#include "os/os.h"
#include "util/int.h"
#include "util/noexcept.h"

// Note: Do not add noexcept.
void*
operator new(size_t count) {
    return malloc(count);
}

// Note: Do not add noexcept.
void*
operator new[](size_t count) {
    return malloc(count);
}

void
operator delete(void* ptr) noexcept {
    free(ptr);
}

void
operator delete[](void* ptr) noexcept {
    free(ptr);
}

extern "C" void
__cxa_pure_virtual() noexcept {
    exitProcess(1);
}

#endif  // __APPLE__
