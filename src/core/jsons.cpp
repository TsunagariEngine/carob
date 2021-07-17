#include "core/jsons.h"

#include "core/measure.h"
#include "core/resources.h"
#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"

JsonDocument
loadJson(StringView path) noexcept {
    String data;
    if (!resourceLoad(path, data)) {
        return JsonDocument();
    }

    TimeMeasure m(String() << "Constructed " << path << " as json");

    return JsonDocument(static_cast<String&&>(data));
}
