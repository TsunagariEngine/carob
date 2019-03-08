/*************************************
** Tsunagari Tile Engine            **
** ui-log.cpp                       **
** Copyright 2016-2019 Paul Merrill **
*************************************/

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

// FIXME: Pre-declare operator new.
#include <new>

#include "pack/ui.h"

#include "os/mutex.h"

#include "pack/pool.h"

#include "util/string.h"
#include "util/unique.h"

static Unique<Pool> pool(Pool::makePool("ui", 1));
static Mutex mutex;
static bool scheduled = false;
static String buf;
bool verbose = false;

static void flush() {
    LockGuard lock(mutex);
    printf("%s", buf.null().get());
    buf.clear();
    scheduled = false;
}

static void scheduleMessage(StringView message) {
    if (!verbose) {
        return;
    }

    LockGuard lock(mutex);
    buf << message;
    if (!scheduled) {
        pool->schedule(flush);
        scheduled = true;
    }
}

void uiShowSkippedMissingFile(StringView path) {
    scheduleMessage(String() << "Skipped "
                             << path
                             << ": file not found\n");
}

void uiShowAddedFile(StringView path, size_t size) {
    scheduleMessage(String() << "Added "
                             << path
                             << ": "
                             << size
                             << " bytes\n");
}

void uiShowWritingArchive(StringView archivePath) {
    scheduleMessage(String() << "Writing to "
                             << archivePath
                             << "\n");
}

void uiShowListingEntry(StringView blobPath, uint64_t blobSize) {
    scheduleMessage(String() << blobPath
                             << ": "
                             << blobSize
                             << " bytes\n");
}

void uiShowExtractingFile(StringView blobPath, uint64_t blobSize) {
    scheduleMessage(String() << "Extracting "
                             << blobPath
                             << ": "
                             << blobSize
                             << " bytes\n");
}
