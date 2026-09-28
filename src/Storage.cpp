#include "Storage.h"
#include <Preferences.h>

namespace {
Preferences prefs;
const char *NS = "pixelgochi";
const char *KEY = "state";
} // namespace

namespace Storage {

bool load(void *data, size_t len) {
    // Abre em leitura-escrita: em modo só-leitura o ESP32 loga erro
    // quando o namespace ainda não existe (primeiro boot).
    prefs.begin(NS, false);
    bool ok = prefs.getBytesLength(KEY) == len && prefs.getBytes(KEY, data, len) == len;
    prefs.end();
    return ok;
}

void save(const void *data, size_t len) {
    prefs.begin(NS, false);
    prefs.putBytes(KEY, data, len);
    prefs.end();
}

} // namespace Storage
