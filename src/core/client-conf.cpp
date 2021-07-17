#include "core/client-conf.h"

#include "config.h"
#include "os/os.h"
#include "util/compiler.h"
#include "util/json.h"
#include "util/string.h"
#include "util/string2.h"
#include "util/vector.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

LogVerbosity confVerbosity = LogVerbosity::VERBOSE;
MoveMode confMoveMode;
ivec2 confWindowSize = {640, 480};
bool confFullscreen = false;
int confMusicVolume = 100;
int confSoundVolume = 100;
time_t confCacheTTL = 300;
int confPersistInit = 0;
int confPersistCons = 0;

// Parse and process the client config file, and set configuration defaults for
// missing options.
bool
confParse(StringView filename) noexcept {
    String file;
    bool ok = readFile(filename, file);

    if (!ok) {
        logFatal(filename, String() << "Could not find " << filename);
        return false;
    }

    JsonDocument doc = JsonDocument(static_cast<String&&>(file));
    if (!doc.ok) {
        logErr(filename, String() << "Could not parse " << filename);
        return false;
    }

    JsonValue root = doc.root;

    JsonValue engineValue = root["engine"];
    JsonValue windowValue = root["window"];
    JsonValue audioValue = root["audio"];
    JsonValue cacheValue = root["cache"];

    CHECK(engineValue.isObject() || engineValue.isNull());
    CHECK(windowValue.isObject() || windowValue.isNull());
    CHECK(audioValue.isObject() || audioValue.isNull());
    CHECK(cacheValue.isObject() || cacheValue.isNull());

    if (engineValue.isObject()) {
        JsonValue verbosityValue = engineValue["verbosity"];

        CHECK(verbosityValue.isString() || verbosityValue.isNull());

        if (verbosityValue.isString()) {
            StringView verbosity = verbosityValue.toString();
            if (verbosity == "quiet") {
                confVerbosity = LogVerbosity::QUIET;
            }
            else if (verbosity == "normal") {
                confVerbosity = LogVerbosity::NORMAL;
            }
            else if (verbosity == "verbose") {
                confVerbosity = LogVerbosity::VERBOSE;
            }
            else {
                logErr(filename,
                       "Unknown value for \"engine.verbosity\", using default");
            }
        }
    }

    if (windowValue.isObject()) {
        JsonValue widthValue = windowValue["width"];
        JsonValue heightValue = windowValue["height"];
        JsonValue fullscreenValue = windowValue["fullscreen"];

        CHECK(widthValue.isNumber() || widthValue.isNull());
        CHECK(heightValue.isNumber() || heightValue.isNull());
        CHECK(fullscreenValue.isBool() || fullscreenValue.isNull());

        if (widthValue.isNumber()) {
            confWindowSize.x = widthValue.toInt();
        }
        if (heightValue.isNumber()) {
            confWindowSize.y = heightValue.toInt();
        }
        if (fullscreenValue.isBool()) {
            confFullscreen = fullscreenValue.toBool();
        }
    }

    if (audioValue.isObject()) {
        JsonValue musicvolumeValue = audioValue["musicvolume"];
        JsonValue soundvolumeValue = audioValue["soundvolume"];

        CHECK(musicvolumeValue.isNumber() || musicvolumeValue.isNull());
        CHECK(soundvolumeValue.isNumber() || soundvolumeValue.isNull());

        if (musicvolumeValue.isNumber()) {
            confMusicVolume = musicvolumeValue.toInt();
        }
        if (soundvolumeValue.isNumber()) {
            confSoundVolume = soundvolumeValue.toInt();
        }
    }

    if (cacheValue.isObject()) {
        JsonValue ttlValue = cacheValue["ttl"];

        CHECK(ttlValue.isNumber() || ttlValue.isNull());

        if (ttlValue.isNumber()) {
            confCacheTTL = ttlValue.toInt();
        }
    }

    return true;
}
